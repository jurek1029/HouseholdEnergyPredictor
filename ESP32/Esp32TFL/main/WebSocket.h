#ifndef WEBSOCKET_HELPER_H_
#define WEBSOCKET_HELPER_H_

#include "esp_websocket_client.h"

namespace websocket{
    void setupWebSocket(const char* uri, void (*f)(esp_websocket_event_data_t *data));
    void closeWebSocket();
    void sendData(char* data, int len);
    void sendDataADC2Clear(char* data, int len);
    void openWebSocket();
}

#endif // WEBSOCKET_HELPER_H_