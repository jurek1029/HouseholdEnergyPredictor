idf_component_register(SRCS "solcast_api.cc" "http_request.cc" "time_sync.c" "Api_test_2.cc"
                    INCLUDE_DIRS "include"
                    EMBED_TXTFILES server_root_cert.pem)

target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
