#include <iostream>
#include <string>
#include <cstring>
#include <cctype>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <unistd.h>
#include <filesystem>
#include <archive.h>
#include <archive_entry.h>
#include <memory>
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>
#include <map>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <cstdio>
#include <openssl/aes.h>
#include <openssl/des.h>
#include <openssl/rand.h>
#include <tuple>
#include <stdexcept>
#include <jwt.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <sys/stat.h>
#include <utility>
#include <regex>
#include <sqlite3.h>
#ifndef SAVE_SECRETE_FILE_HPP
#define SAVE_SECRETE_FILE_HPP

#include <string>
#include <filesystem>
#include <vector>
#include <system_error>

#if defined(_WIN32)
  #include <windows.h>
  #include <Aclapi.h>
  #include <accctrl.h>
  #include <securitybaseapi.h>
#else
  #include <sys/stat.h>
  #include <fcntl.h>
  #include <unistd.h>
  #include <errno.h>
#endif

namespace detail {

inline bool isValidFilename(const std::string& filename) {
    if (filename.empty()) return false;
    if (filename == "." || filename == "..") return false;
    // Disallow separators and colon (Windows drive designator)
    for (char ch : filename) {
        if (ch == '/' || ch == '\\' || ch == ':') return false;
    }
    return true;
}

#if defined(_WIN32)
inline std::wstring toWide(const std::string& utf8) {
    if (utf8.empty()) return std::wstring();
    int needed = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
    if (needed <= 0) return std::wstring();
    std::wstring out(static_cast<size_t>(needed), L'\0');
    int res = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &out[0], needed);
    if (res <= 0) return std::wstring();
    // Remove the trailing null inserted by MultiByteToWideChar
    if (!out.empty() && out.back() == L'\0') out.pop_back();
    return out;
}

// Acquire an ACL that grants GENERIC_ALL to the current user only.
inline PACL buildUserOnlyAcl() {
    HANDLE hToken = nullptr;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        return nullptr;
    }

    DWORD size = 0;
    GetTokenInformation(hToken, TokenUser, nullptr, 0, &size);
    if (size == 0) {
        CloseHandle(hToken);
        return nullptr;
    }

    std::vector<BYTE> buf(size);
    if (!GetTokenInformation(hToken, TokenUser, buf.data(), size, &size)) {
        CloseHandle(hToken);
        return nullptr;
    }
    CloseHandle(hToken);

    PTOKEN_USER pTokenUser = reinterpret_cast<PTOKEN_USER>(buf.data());
    PSID userSid = pTokenUser->User.Sid;

    EXPLICIT_ACCESSW ea{};
    ea.grfAccessPermissions = GENERIC_ALL;
    ea.grfAccessMode = SET_ACCESS;
    ea.grfInheritance = NO_INHERITANCE;
    ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea.Trustee.TrusteeType = TRUSTEE_IS_USER;
    ea.Trustee.ptstrName = reinterpret_cast<LPWSTR>(userSid);

    PACL pAcl = nullptr;
    DWORD dwRes = SetEntriesInAclW(1, &ea, nullptr, &pAcl);
    if (dwRes != ERROR_SUCCESS) {
        if (pAcl) LocalFree(pAcl);
        return nullptr;
    }

    return pAcl; // Caller must LocalFree
}

inline bool applyDaclToHandle(HANDLE h, PACL pAcl) {
    if (!h || !pAcl) return false;
    DWORD res = SetSecurityInfo(
        h,
        SE_FILE_OBJECT,
        DACL_SECURITY_INFORMATION | PROTECTED_DACL_SECURITY_INFORMATION,
        nullptr,
        nullptr,
        pAcl,
        nullptr
    );
    return (res == ERROR_SUCCESS);
}
#endif

} // namespace detail

// Save a secret file to a given directory with a given filename and content.
// The file should be accessible only by the current user.
//
// @param directory The directory to save the secret file.
// @param filename The filename of the secret file.
// @param content The content of the secret file.
void saveSecreteFile(const std::string& directory, const std::string& filename, const std::string& content) {
    // Validate filename to avoid path traversal or injection
    if (!detail::isValidFilename(filename)) {
        return; // Gracefully return on invalid filename
    }

    std::filesystem::path dirPath(directory);
    std::filesystem::path filePath;

    try {
        // Normalize the filename to just its last component
        std::filesystem::path fname(filename);
        fname = fname.filename();
        filePath = dirPath / fname;
    } catch (...) {
        return;
    }

    // Ensure directory exists
    try {
        std::error_code ec;
        std::filesystem::create_directories(dirPath, ec);
        // On POSIX, restrict directory permissions to owner-only
        #if !defined(_WIN32)
        if (!ec) {
            std::filesystem::permissions(
                dirPath,
                std::filesystem::perms::owner_all,
                std::filesystem::perm_options::replace,
                ec
            );
        }
        #endif
    } catch (...) {
        // Ignore and continue; we'll still attempt to write the file
    }

#if defined(_WIN32)
    // Windows implementation: create file with DACL allowing only the current user
    std::wstring wpath = filePath.wstring();

    PACL pAcl = detail::buildUserOnlyAcl();
    if (!pAcl) {
        // If we cannot build ACL, do not proceed to create a potentially non-secret file
        return;
    }

    SECURITY_DESCRIPTOR sd;
    if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION)) {
        LocalFree(pAcl);
        return;
    }
    if (!SetSecurityDescriptorDacl(&sd, TRUE, pAcl, FALSE)) {
        LocalFree(pAcl);
        return;
    }

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = &sd;
    sa.bInheritHandle = FALSE;

    HANDLE hFile = CreateFileW(
        wpath.c_str(),
        GENERIC_WRITE | GENERIC_READ,
        0,                 // no sharing
        &sa,               // restricted security
        CREATE_ALWAYS,     // overwrite or create
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        LocalFree(pAcl);
        return;
    }

    // Ensure the DACL is applied even if the file existed (CREATE_ALWAYS may not reset security)
    (void)detail::applyDaclToHandle(hFile, pAcl);

    // Write content (as UTF-8 bytes)
    DWORD total = 0;
    const char* data = content.data();
    DWORD len = static_cast<DWORD>(content.size());
    while (total < len) {
        DWORD written = 0;
        if (!WriteFile(hFile, data + total, len - total, &written, nullptr)) {
            break;
        }
        if (written == 0) {
            break;
        }
        total += written;
    }

    CloseHandle(hFile);
    LocalFree(pAcl);
    return;

#else
    // POSIX implementation: create file with mode 0600 and write content
    std::string pathStr;
    try {
        pathStr = filePath.string();
    } catch (...) {
        return;
    }

    // Open with restrictive mode; O_CREAT sets initial perms subject to umask, so enforce with fchmod after.
    int fd = ::open(pathStr.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        return;
    }

    // Enforce 0600 regardless of umask
    (void) ::fchmod(fd, S_IRUSR | S_IWUSR);

    // Write content reliably
    const char* data = content.data();
    size_t total = 0;
    size_t len = content.size();
    while (total < len) {
        ssize_t n = ::write(fd, data + total, len - total);
        if (n <= 0) {
            // write failed or nothing more written; stop
            break;
        }
        total += static_cast<size_t>(n);
    }

    // Close file descriptor
    ::close(fd);

    return;
#endif
}

#endif // SAVE_SECRETE_FILE_HPP


// BEGIN ENTRYPOINT
int main(int argc, char* argv[]) {
    // Check if there are exactly four command-line arguments
    if (argc != 4) {
        return 1;
    }

    // Save the secret file
    saveSecreteFile(argv[1], argv[2], argv[3]);

    return 0;
}
