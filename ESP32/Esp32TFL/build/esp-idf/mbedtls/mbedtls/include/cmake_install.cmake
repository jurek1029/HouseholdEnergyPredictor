# Install script for directory: E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/hello_world")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "E:/Programing/Tools/Espressif/tools/xtensa-esp32-elf/esp-2021r2-patch3-8.4.0/xtensa-esp32-elf/bin/xtensa-esp32-elf-objdump.exe")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/mbedtls" TYPE FILE PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ FILES
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/aes.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/aesni.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/arc4.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/aria.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/asn1.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/asn1write.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/base64.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/bignum.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/blowfish.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/bn_mul.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/camellia.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ccm.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/certs.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/chacha20.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/chachapoly.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/check_config.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/cipher.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/cipher_internal.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/cmac.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/compat-1.3.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/config.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/config_psa.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/constant_time.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ctr_drbg.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/debug.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/des.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/dhm.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ecdh.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ecdsa.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ecjpake.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ecp.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ecp_internal.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/entropy.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/entropy_poll.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/error.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/gcm.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/havege.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/hkdf.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/hmac_drbg.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/md.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/md2.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/md4.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/md5.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/md_internal.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/memory_buffer_alloc.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/net.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/net_sockets.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/nist_kw.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/oid.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/padlock.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/pem.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/pk.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/pk_internal.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/pkcs11.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/pkcs12.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/pkcs5.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/platform.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/platform_time.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/platform_util.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/poly1305.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/psa_util.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ripemd160.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/rsa.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/rsa_internal.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/sha1.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/sha256.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/sha512.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ssl.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ssl_cache.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ssl_ciphersuites.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ssl_cookie.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ssl_internal.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ssl_ticket.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/threading.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/timing.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/version.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/x509.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/x509_crl.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/x509_crt.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/x509_csr.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/xtea.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/psa" TYPE FILE PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ FILES
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/psa/crypto.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/psa/crypto_builtin_composites.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/psa/crypto_builtin_primitives.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/psa/crypto_compat.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/psa/crypto_config.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/psa/crypto_driver_common.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/psa/crypto_driver_contexts_composites.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/psa/crypto_driver_contexts_primitives.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/psa/crypto_extra.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/psa/crypto_platform.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/psa/crypto_se_driver.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/psa/crypto_sizes.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/psa/crypto_struct.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/psa/crypto_types.h"
    "E:/Programing/_Libs/esp2/esp-idf/components/mbedtls/mbedtls/include/psa/crypto_values.h"
    )
endif()

