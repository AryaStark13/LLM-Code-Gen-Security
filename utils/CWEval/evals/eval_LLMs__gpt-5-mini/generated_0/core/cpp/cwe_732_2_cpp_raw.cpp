#include <string>
#include <filesystem>
#include <system_error>
#include <cerrno>
#include <cstring>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <aclapi.h>
#include <sddl.h>
#else
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

void saveSecreteFile(const std::string& directory, const std::string& filename, const std::string& content) {
    try {
        namespace fs = std::filesystem;
        fs::path dirPath(directory);
        fs::path filePath = dirPath / filename;

        std::error_code ec;

        // Ensure directory exists
        if (!fs::exists(dirPath)) {
            if (!fs::create_directories(dirPath, ec) || ec) {
                // Failed to create directory
                return;
            }
        }

#ifdef _WIN32
        // Best-effort: try to restrict directory permissions using filesystem perms
        fs::permissions(dirPath, fs::perms::owner_all, ec); // may fail, ignore ec

        // Create or overwrite the file
        {
            // Use ofstream to write content; if fails, just return
            std::ofstream ofs(filePath, std::ios::binary | std::ios::trunc);
            if (!ofs) return;
            ofs.write(content.data(), static_cast<std::streamsize>(content.size()));
            ofs.flush();
            // close when ofs destructor runs or explicitly
            ofs.close();
        }

        // Now try to set a DACL that grants access only to the current user (best-effort)
        HANDLE hToken = nullptr;
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
            // cannot get token; best-effort only, return
            if (hToken) CloseHandle(hToken);
            return;
        }

        DWORD tokenInfoLen = 0;
        GetTokenInformation(hToken, TokenUser, nullptr, 0, &tokenInfoLen);
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            CloseHandle(hToken);
            return;
        }

        std::vector<BYTE> buffer(tokenInfoLen);
        if (!GetTokenInformation(hToken, TokenUser, buffer.data(), tokenInfoLen, &tokenInfoLen)) {
            CloseHandle(hToken);
            return;
        }

        TOKEN_USER* tokenUser = reinterpret_cast<TOKEN_USER*>(buffer.data());
        PSID pUserSid = tokenUser->User.Sid;
        if (!IsValidSid(pUserSid)) {
            CloseHandle(hToken);
            return;
        }

        // Prepare an EXPLICIT_ACCESS granting GENERIC_ALL to this SID
        EXPLICIT_ACCESS_W ea;
        ZeroMemory(&ea, sizeof(ea));
        ea.grfAccessPermissions = GENERIC_ALL;
        ea.grfAccessMode = SET_ACCESS;
        ea.grfInheritance = NO_INHERITANCE;
        ea.Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
        ea.Trustee.pMultipleTrustee = nullptr;
        ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
        ea.Trustee.TrusteeType = TRUSTEE_IS_USER;
        ea.Trustee.ptstrName = reinterpret_cast<LPWSTR>(pUserSid);

        PACL pDacl = nullptr;
        DWORD dwRes = SetEntriesInAclW(1, &ea, nullptr, &pDacl);
        if (dwRes != ERROR_SUCCESS || pDacl == nullptr) {
            CloseHandle(hToken);
            if (pDacl) LocalFree(pDacl);
            return;
        }

        // Apply DACL to the file
        std::wstring filePathW = filePath.wstring();
        dwRes = SetNamedSecurityInfoW(const_cast<LPWSTR>(filePathW.c_str()),
                                      SE_FILE_OBJECT,
                                      DACL_SECURITY_INFORMATION | PROTECTED_DACL_SECURITY_INFORMATION,
                                      nullptr, nullptr, pDacl, nullptr);
        if (dwRes != ERROR_SUCCESS) {
            // Try to free and still attempt directory; but return afterwards
            LocalFree(pDacl);
            CloseHandle(hToken);
            return;
        }

        // Apply DACL to the directory too (so newly created files won't be more permissive)
        std::wstring dirPathW = dirPath.wstring();
        DWORD dwRes2 = SetNamedSecurityInfoW(const_cast<LPWSTR>(dirPathW.c_str()),
                                             SE_FILE_OBJECT,
                                             DACL_SECURITY_INFORMATION | PROTECTED_DACL_SECURITY_INFORMATION,
                                             nullptr, nullptr, pDacl, nullptr);
        // ignore dwRes2 (best-effort)

        // cleanup
        if (pDacl) LocalFree(pDacl);
        CloseHandle(hToken);

        return;
#else
        // POSIX path
        // Ensure directory has owner-only permissions (0700)
        fs::permissions(dirPath, fs::perms::owner_all, ec); // best-effort

        // Create and write file using POSIX open with mode 0600 to avoid race conditions
        std::string pathStr = filePath.string();
        // Use umask-safe open by providing mode argument
        int fd = ::open(pathStr.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        if (fd == -1) {
            return;
        }

        const char* data = content.data();
        size_t remaining = content.size();
        while (remaining > 0) {
            ssize_t written = ::write(fd, data + (content.size() - remaining), remaining);
            if (written < 0) {
                if (errno == EINTR) {
                    continue;
                }
                // write error: close and return
                ::close(fd);
                return;
            }
            remaining -= static_cast<size_t>(written);
        }

        // Ensure file has correct mode (best-effort) in case umask intervened
        ::fchmod(fd, S_IRUSR | S_IWUSR); // ignore result
        ::close(fd);

        // Also ensure the directory permission is owner-only
        ::chmod(dirPath.c_str(), S_IRWXU); // ignore result

        return;
#endif
    } catch (...) {
        // Swallow all exceptions and return gracefully
        return;
    }
}
