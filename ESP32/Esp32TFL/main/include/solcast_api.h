#ifndef SOLCAST_API
#define SOLCAST_API

#include<vector>
#include<string>
#include<memory>
#include <string.h>

//#define DEBUG_SOLCAST_API

#define SOLCAST_WEBSOCKET_API_KEY       "API_KEY:"
#define SOLCAST_WEBSOCKET_AZIMUTH       "azimuth:"
#define SOLCAST_WEBSOCKET_CAPACITY      "capacit:"
#define SOLCAST_WEBSOCKET_LATITUDE      "latitud:"
#define SOLCAST_WEBSOCKET_LONGITUDE     "longitu:"
#define SOLCAST_WEBSOCKET_SEVER         "server-:"
#define SOLCAST_WEBSOCKET_SEVER_PORT    "port---:"
#define SOLCAST_WEBSOCKET_TILT          "tilt---:"
#define SOLCAST_WEBSOCKET_TAG_LEN 9

#define WEB_SERVER "api.solcast.com.au"
#define WEB_PORT "443"
#define WEB_API_KEY "q_m_SaOSVkHui2bOMrGgzY6T68TbEpN7"
#define WEB_URL "https://%s/world_pv_power/forecasts?latitude=%s&longitude=%s&output_parameters=pv_power_rooftop&capacity=%s&tilt=%s&azimuth=%s&hours=%s&format=csv"
//#define WEB_URL "https://api.solcast.com.au/world_radiation/forecasts?latitude=-33.856784&longitude=151.215297&output_parameters=air_temp,dni,ghi"

// static const char HEADERS[] = "Host: " WEB_SERVER "\r\n"
//                              "User-Agent: esp-idf/1.0 esp32\r\n"
//                              "Authorization: Bearer q_m_SaOSVkHui2bOMrGgzY6T68TbEpN7\r\n"
//                              "\r\n";
#define HEADERS "Host: %s\r\nUser-Agent: esp-idf/1.0 esp32\r\nAuthorization: Bearer %s\r\n\r\n"

namespace solcast{
    struct SolcastApiData{
        std::string webServer;
        std::string webPort;
        std::string webApiKey;
        std::string latitude;
        std::string longitude;
        std::string capacity;
        std::string tilt;
        std::string azimuth;
        std::string hours;
        SolcastApiData(const char* wServ, const char* wPort, const char* apiKey, const char* lat, const char* lon,const char* cap, const char* _tilt, const char* _azimuth,const char* _hours):
            webServer(wServ), webPort(wPort),webApiKey(apiKey),
            latitude(lat), longitude(lon), capacity(cap), tilt(_tilt), azimuth(_azimuth), hours(_hours){};

        SolcastApiData(const char* wServ, const char* apiKey, const char* lat, const char* lon, const char* cap, const char* _tilt, const char* _azimuth, const char* _hours):
            webServer(wServ),webApiKey(apiKey),latitude(lat), longitude(lon), capacity(cap), tilt(_tilt), azimuth(_azimuth), hours(_hours){
                webPort = WEB_PORT;
            };

        SolcastApiData(const char* lat, const char* lon, const char* cap, const char* _tilt, const char* _azimuth, const char* _hours):
            latitude(lat), longitude(lon), capacity(cap), tilt(_tilt), azimuth(_azimuth), hours(_hours){
                webServer = WEB_SERVER;
                webPort = WEB_PORT;
                webApiKey = WEB_API_KEY;
            };
        SolcastApiData()
        {
            webServer = WEB_SERVER;
            webPort = WEB_PORT;
            webApiKey = WEB_API_KEY;
            latitude = "-33.856784";
            longitude = "151.215297";
            capacity = "5";
            tilt = "42";
            azimuth = "180";
            hours = "12";
        };
    };

    struct Data {
        std::vector<float> power;
        std::vector<std::string> time;
        float period = 0;
        Data():power(),time(){}
    };

    void setupSolCastApi();
    std::unique_ptr<Data> requestForecastData();
    std::unique_ptr<Data> parseCSVData(std::unique_ptr<std::stringstream> ss);

    void printData(Data& pData);

    extern solcast::SolcastApiData solcastData;
}

#endif