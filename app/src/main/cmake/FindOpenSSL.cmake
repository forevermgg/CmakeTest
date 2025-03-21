set(OPENSSL_ROOT_DIR ${PROJECT_SOURCE_DIR}/../../../../prefix/openssl/${ANDROID_ABI})
message(OPENSSL_ROOT_DIR: ${OPENSSL_ROOT_DIR})
if (NOT TARGET OpenSSL::Crypto)
    message("NOT TARGET OpenSSL::Crypto")
    add_library(OpenSSL::Crypto STATIC IMPORTED)
    set_target_properties(OpenSSL::Crypto PROPERTIES
            IMPORTED_LOCATION "${OPENSSL_ROOT_DIR}/lib/libcrypto.a"
            INTERFACE_INCLUDE_DIRECTORIES "${OPENSSL_ROOT_DIR}/include"
            INTERFACE_LINK_LIBRARIES ""
            )
endif ()

if (NOT TARGET OpenSSL::SSL)
    message("NOT TARGET OpenSSL::SSL")
    add_library(OpenSSL::SSL STATIC IMPORTED)
    set_target_properties(OpenSSL::SSL PROPERTIES
            IMPORTED_LOCATION "${OPENSSL_ROOT_DIR}/lib/libssl.a"
            INTERFACE_INCLUDE_DIRECTORIES "${OPENSSL_ROOT_DIR}/include"
            INTERFACE_LINK_LIBRARIES ""
            )
endif ()