#ifndef WEBSOCKET_HELPER_H_
#define WEBSOCKET_HELPER_H_

#include "esp_websocket_client.h"

#define WEBSOCKET_URI "ws://192.168.1.22"

namespace websocket{
    void setupWebSocket(void (*f)(esp_websocket_event_data_t *data));
    void closeWebSocket();
    void sendData(char* data, int len);
    void sendDataADC2Clear(char* data, int len);
    void openWebSocket();

    extern bool isInitilalized;
}

#endif // WEBSOCKET_HELPER_H_