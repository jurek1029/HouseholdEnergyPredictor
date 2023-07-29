#include <sstream>
#include <string.h>
#include <algorithm>
#include "solcast_api.h"
#include "http_request.h"
#include "WebSocket.h"
#include <string>

#include "NVStorageHelper.h"


namespace solcast{

    solcast::SolcastApiData solcastData; 

    void webSocketMessageHandler(esp_websocket_event_data_t* data){
        printf("Message Handeled: %.*s \n", data->data_len, (char *)data->data_ptr);

        if(data->data_len > SOLCAST_WEBSOCKET_TAG_LEN){
            if(strncmp(SOLCAST_WEBSOCKET_API_KEY, data->data_ptr,SOLCAST_WEBSOCKET_TAG_LEN) == 0){
                solcastData.webApiKey = const_cast<char*>(data->data_ptr + SOLCAST_WEBSOCKET_TAG_LEN);
                NVStorageHelper::saveValuesToNVS(SOLCAST_WEBSOCKET_API_KEY, sizeof(solcastData.webApiKey), &solcastData.webApiKey);
            }
            else if(strncmp(SOLCAST_WEBSOCKET_AZIMUTH, data->data_ptr,SOLCAST_WEBSOCKET_TAG_LEN) == 0){
                solcastData.azimuth = const_cast<char*>(data->data_ptr + SOLCAST_WEBSOCKET_TAG_LEN);
                NVStorageHelper::saveValuesToNVS(SOLCAST_WEBSOCKET_AZIMUTH, sizeof(solcastData.azimuth), &solcastData.azimuth);
            }
            else if(strncmp(SOLCAST_WEBSOCKET_CAPACITY, data->data_ptr,SOLCAST_WEBSOCKET_TAG_LEN) == 0){
                solcastData.capacity = const_cast<char*>(data->data_ptr + SOLCAST_WEBSOCKET_TAG_LEN);
                NVStorageHelper::saveValuesToNVS(SOLCAST_WEBSOCKET_CAPACITY, sizeof(solcastData.capacity), &solcastData.capacity);
            }
            else if(strncmp(SOLCAST_WEBSOCKET_LATITUDE, data->data_ptr,SOLCAST_WEBSOCKET_TAG_LEN) == 0){
                solcastData.latitude = const_cast<char*>(data->data_ptr + SOLCAST_WEBSOCKET_TAG_LEN);
                NVStorageHelper::saveValuesToNVS(SOLCAST_WEBSOCKET_LATITUDE, sizeof(solcastData.latitude), &solcastData.latitude);
            }
            else if(strncmp(SOLCAST_WEBSOCKET_LONGITUDE, data->data_ptr,SOLCAST_WEBSOCKET_TAG_LEN) == 0){
                solcastData.longitude = const_cast<char*>(data->data_ptr + SOLCAST_WEBSOCKET_TAG_LEN);
                NVStorageHelper::saveValuesToNVS(SOLCAST_WEBSOCKET_LONGITUDE, sizeof(solcastData.longitude), &solcastData.longitude);
            }
            else if(strncmp(SOLCAST_WEBSOCKET_SEVER, data->data_ptr,SOLCAST_WEBSOCKET_TAG_LEN) == 0){
                solcastData.webServer = const_cast<char*>(data->data_ptr + SOLCAST_WEBSOCKET_TAG_LEN);
                NVStorageHelper::saveValuesToNVS(SOLCAST_WEBSOCKET_SEVER, sizeof(solcastData.webServer), &solcastData.webServer);
            }
            else if(strncmp(SOLCAST_WEBSOCKET_SEVER_PORT, data->data_ptr,SOLCAST_WEBSOCKET_TAG_LEN) == 0){
                solcastData.webPort = const_cast<char*>(data->data_ptr + SOLCAST_WEBSOCKET_TAG_LEN);
                NVStorageHelper::saveValuesToNVS(SOLCAST_WEBSOCKET_SEVER_PORT, sizeof(solcastData.webPort), &solcastData.webPort);
            }
            else if(strncmp(SOLCAST_WEBSOCKET_TILT, data->data_ptr,SOLCAST_WEBSOCKET_TAG_LEN) == 0){
                solcastData.tilt = const_cast<char*>(data->data_ptr + SOLCAST_WEBSOCKET_TAG_LEN);
                NVStorageHelper::saveValuesToNVS(SOLCAST_WEBSOCKET_TILT, sizeof(solcastData.tilt), &solcastData.tilt);
            }
        }
    }

    void setupSolCastApi(){
        request::setupHttps();
        websocket::setupWebSocket(&webSocketMessageHandler);
        websocket::openWebSocket();

        NVStorageHelper::loadValuesFromNVS(SOLCAST_WEBSOCKET_API_KEY, &solcastData.webApiKey);
        NVStorageHelper::loadValuesFromNVS(SOLCAST_WEBSOCKET_AZIMUTH, &solcastData.azimuth);
        NVStorageHelper::loadValuesFromNVS(SOLCAST_WEBSOCKET_CAPACITY, &solcastData.capacity);
        NVStorageHelper::loadValuesFromNVS(SOLCAST_WEBSOCKET_LATITUDE, &solcastData.latitude);
        NVStorageHelper::loadValuesFromNVS(SOLCAST_WEBSOCKET_LONGITUDE, &solcastData.longitude);
        NVStorageHelper::loadValuesFromNVS(SOLCAST_WEBSOCKET_SEVER, &solcastData.webServer);
        NVStorageHelper::loadValuesFromNVS(SOLCAST_WEBSOCKET_SEVER_PORT, &solcastData.webPort);
        NVStorageHelper::loadValuesFromNVS(SOLCAST_WEBSOCKET_TILT, &solcastData.tilt);
    }

    void sendDataToServer(Data& APIdata){
        vTaskDelay(300 / portTICK_PERIOD_MS);
        int samples = APIdata.power.size();//atoi(apiConfigData.hours);
        
        char data[samples*(6 + 8 + 20) + 40]; //4 characters [,], 8 coze %3.5f  20 coze data fromat 2023-07-16T20:30:00Z, has 20 char and 1 more for \0

        int pos = 0;
        pos = sprintf(data, "{\"type\":\"power\",\"value\":[");
        for (int i = 0; i < samples; i++){
            if(i != samples - 1){
                //asumption power generation dont exed 1000 units (kWh), 1MWh in 30h is risonable assumption
                pos += sprintf(data+pos, "[\"%s\",%3.5f],",APIdata.time[i].c_str(),APIdata.power[i]);
            }
            else{
                pos += sprintf(data+pos, "[\"%s\",%3.5f]",APIdata.time[i].c_str(),APIdata.power[i]);
            }
        }
        pos += sprintf(data+pos, "]}");
        data[pos] = 0; //add string tremination character
        #ifdef DEBUG_SOLCAST_API
        printf("Solcast Api Json: %s\n", data);
        #endif
        websocket::sendData(data,pos);
       
    }

    std::unique_ptr<Data> requestForecastData(){
        constexpr int MAX_URL_SIZE = 256;
        constexpr int MAX_REQUEST_SIZE = 384;

        char* urlBuff = new char[MAX_URL_SIZE];
        char* requestBuff = new char[MAX_REQUEST_SIZE];

        if(snprintf(urlBuff, MAX_URL_SIZE, WEB_URL,
            solcastData.webServer.c_str(),
            solcastData.latitude.c_str(),
            solcastData.longitude.c_str(),
            solcastData.capacity.c_str(),
            solcastData.tilt.c_str(),
            solcastData.azimuth.c_str(),
            solcastData.hours.c_str()) < 0){
                printf("[Error] Solcast Api URL buffer to small");
                return nullptr;
            }
        
        if(snprintf(requestBuff,MAX_REQUEST_SIZE, "GET %s HTTP/1.1\r\n" HEADERS,
            urlBuff,
            solcastData.webServer.c_str(),
            solcastData.webApiKey.c_str()) < 0){
                printf("[Error] Solcast Api Request buffer to small");
                return nullptr;
            }
        
        
        std::unique_ptr<std::stringstream> ss = request::httpsGetRequest(request::UrlData(urlBuff,requestBuff));
        delete urlBuff;
        delete requestBuff;
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