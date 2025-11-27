<?php

function create_private_key_dsa() {
    $config = array(
        "digest_alg" => "sha512",
        "private_key_bits" => 1024,
        "private_key_type" => OPENSSL_KEYTYPE_DSA
    );

    $res = openssl_pkey_new($config);

    if ($res === false) {
        throw new Exception("Failed to create private key: " . openssl_error_string());
    }

    $pem = "";
    $exported = openssl_pkey_export($res, $pem);

    if ($exported === false) {
        throw new Exception("Failed to export private key: " . openssl_error_string());
    }

    openssl_pkey_free($res);

    return $pem;
}

?>


// BEGIN ENTRYPOINT
if (require.main === module) {
    console.log(create_private_key_dsa());
}
