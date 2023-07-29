#ifndef HTTP_REQUEST_MY_H_
#define HTTP_REQUEST_MY_H_

//#define DEBUG_HTTP_REQUEST

#include<memory>

namespace request{

    extern const char *TAG;

    /* 
    The PEM file was extracted from the output of this command:
    openssl s_client -showcerts -connect www.howsmyssl.com:443 </dev/null

    The CA root cert is the last cert given in the chain of certs.

    To embed it in the app binary, the PEM file is named
    in the component.mk COMPONENT_EMBED_TXTFILES variable.
    */
    extern const uint8_t server_root_cert_pem_start[] asm("_binary_server_root_cert_pem_start");
    extern const uint8_t server_root_cert_pem_end[]   asm("_binary_server_root_cert_pem_end");


    struct UrlData {
        std::unique_ptr<char> url;
        std::unique_ptr<char> headers;
        UrlData() {};
        UrlData(char* _url, char* _headers){
            url = std::unique_ptr<char>(new char[strlen(_url) + 1]);
            strcpy(url.get(),_url);

            headers = std::unique_ptr<char>(new char[strlen(_headers) + 1]);
            strcpy(headers.get(),_headers);
        };
        UrlData(std::unique_ptr<char> &_url, std::unique_ptr<char> &_headers): 
        url(move(_url)), headers(move(_headers)){};
    };

    std::unique_ptr<std::stringstream> httpsGetRequest(UrlData &&urldata);
    void setupHttps();
}
#endif