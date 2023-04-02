#ifndef SOLCAST_API
#define SOLCAST_API

#include<vector>
#include<string>
#include<memory>

//#define DEBUG_SOLCAST_API

#define WEB_SERVER "api.solcast.com.au"
#define WEB_PORT "443"
#define WEB_URL "https://api.solcast.com.au/world_pv_power/forecasts?latitude=%s&longitude=%s&output_parameters=pv_power_rooftop&capacity=%s&tilt=%s&azimuth=%s&hours=%s&format=csv"
//#define WEB_URL "https://api.solcast.com.au/world_radiation/forecasts?latitude=-33.856784&longitude=151.215297&output_parameters=air_temp,dni,ghi"

static const char HEADERS[] = "Host: " WEB_SERVER "\r\n"
                             "User-Agent: esp-idf/1.0 esp32\r\n"
                             "Authorization: Bearer q_m_SaOSVkHui2bOMrGgzY6T68TbEpN7\r\n"
                             "\r\n";

namespace solcast{
    struct SolcastApiData{
    const char* latitude;
    const char* longitude;
    const char* capacity;
    const char* tilt;
    const char* azimuth;
    const char* hours;
    SolcastApiData(const char* lat, const char* lon, const char* cap, const char* _tilt, const char* _azimuth, const char* _hours):
            latitude(lat), longitude(lon), capacity(cap), tilt(_tilt), azimuth(_azimuth), hours(_hours){};
    };

    struct Data {
        std::vector<float> power;
        std::vector<std::string> time;
        float period = 0;
        Data():power(),time(){}
    };

    void setupSolCastApi();
    std::unique_ptr<Data> requestForecastData(SolcastApiData &&data);
    std::unique_ptr<Data> parseCSVData(std::unique_ptr<std::stringstream> ss);

    void printData(Data& pData);
}

#endif