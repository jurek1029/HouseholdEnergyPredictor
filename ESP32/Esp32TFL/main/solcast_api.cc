#include <sstream>
#include <string.h>
#include <algorithm>
#include "solcast_api.h"
#include "http_request.h"
#include "WebSocket.h"

namespace solcast{

    void webSocketMessageHandler(esp_websocket_event_data_t* data){
        printf("Message Handeled: %.*s \n", data->data_len, (char *)data->data_ptr);
    }

    void setupSolCastApi(){
        request::setupHttps();
        websocket::setupWebSocket(&webSocketMessageHandler);
        websocket::openWebSocket();
    }

    void sendDataToServer(Data& APIdata){
        vTaskDelay(300 / portTICK_PERIOD_MS);
        int samples = APIdata.power.size();//atoi(apiConfigData.hours);
        
        char data[samples*9 + 40];
        char s_power[samples*9 + 9];

        int j = 0;
        for (int i = 0; i < samples; i++){
            if(i != samples - 1){
                j += sprintf(s_power+j, "%3.5f,",APIdata.power[i]);
            }
            else{
                j += sprintf(s_power+j, "%3.5f",APIdata.power[i]);
            }
        }

        int len = sprintf(data, "{\"type\":\"power\",\"value\":[%s]}", s_power);
        websocket::sendData(data,len);

        // char s_time[samples*32 + 9];
        // j = 0;
        // for (int i = 0; i < samples; i++){
        //     if(i != samples - 1){
        //         j += sprintf(s_time+j, "%s,",APIdata.time[i].c_str());
        //     }
        //     else{
        //         j += sprintf(s_time+j, "%s",APIdata.time[i].c_str());
        //     }
        // }

        // len = sprintf(data, "{\"type\":\"power_time\",\"value\":[%s]}", s_time);
        // websocket::sendData(data,len);
        // #ifdef DEBUG_PRINT_LOGS
        // printf("formated values to JSON: %s\n", s_pred);
        // #endif

       
    }

    std::unique_ptr<Data> requestForecastData(SolcastApiData &&data){
        std::unique_ptr<char> url = std::unique_ptr<char>(new char[200]);
        sprintf(url.get(), WEB_URL, data.latitude, data.longitude, data.capacity, data.tilt, data.azimuth, data.hours);
        std::unique_ptr<char> bufHeader = std::unique_ptr<char>(new char[strlen(url.get()) + strlen(HEADERS) + 16]);
        sprintf(bufHeader.get(), "GET %s HTTP/1.1\r\n%s", url.get(), HEADERS);
        std::unique_ptr<std::stringstream> ss = request::httpsGetRequest(request::UrlData(url,bufHeader));
        std::unique_ptr<Data> pData = parseCSVData(move(ss));
        sendDataToServer(*pData);
        return move(pData);
    }

    std::unique_ptr<Data> parseCSVData(std::unique_ptr<std::stringstream> ss){
        std::unique_ptr<Data> pData(new Data());
        bool dataStarted = false;
        std::string line = "";
        int col = 0;
        while(std::getline(*ss.get(), line)){
            #ifdef DEBUG_SOLCAST_API
            printf("line: %s\n", line.c_str());
            #endif
            if(dataStarted){ 
                std::stringstream lss(line);
                std::string tag;
                while(std::getline(lss,tag,',')){
                    if(col % 3 == 0){
                        #ifdef DEBUG_SOLCAST_API
                        printf("%f \n", std::stof(tag.c_str()));
                        #endif
                        pData->power.push_back(std::stof(tag.c_str()));
                    }
                    else if(col % 3 == 1){
                        pData->time.push_back(tag);
                    }
                    else{
                        if (!pData->period){
                            #ifdef DEBUG_SOLCAST_API
                            printf("%s\n",tag.c_str());
                            #endif
                            auto it = std::remove_if(tag.begin(),tag.end(),[](char c){return c < '0' || c > '9';});
                            tag.erase(it,tag.end());
                            pData->period = std::stof(tag);
                        }
                    }
                    col++;
                    #ifdef DEBUG_SOLCAST_API
                    printf("tag: %s\n",tag.c_str());
                    #endif
                }
            }
            else if(line.find("pv_estimate") != std::string::npos){
                dataStarted = true;
            }
        }
        return move(pData);
    }

    void printData(Data& pData){
        printf("power: \n");
        int i = 0;
        for(auto& v : pData.power){
            if(++i % 10 == 0) printf("\n");
            printf("%f,",v);
        }
        printf("\n time: \n");
        i=0;
        for(auto& v : pData.time){
            if(++i % 10 == 0) printf("\n");
            printf("%s,",v.c_str());
        }

        printf("\npower size: %d \n",pData.time.size());
        printf("time size: %d \n", pData.power.size());
        printf("period: %f\n", pData.period);
    }
}