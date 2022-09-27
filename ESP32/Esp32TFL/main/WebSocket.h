#ifndef WEBSOCKET_HELPER_H_
#define WEBSOCKET_HELPER_H_

namespace websocket{
    void setupWebSocket(const char* uri);
    void closeWebSocket();
    void sendData(char* data, int len);
}

#endif // WEBSOCKET_HELPER_H_