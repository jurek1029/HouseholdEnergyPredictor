#include <sstream>
#include <string.h>
#include <algorithm>
#include "solcast_api.h"
#include "http_request.h"

namespace solcast{
    std::unique_ptr<Data> requestForecastData(SolcastApiData &&data){
        std::unique_ptr<char> url = std::unique_ptr<char>(new char[200]);
        sprintf(url.get(), WEB_URL, data.latitude, data.longitude, data.capacity, data.tilt, data.azimuth, data.hours);
        std::unique_ptr<char> bufHeader = std::unique_ptr<char>(new char[strlen(url.get()) + strlen(HEADERS) + 16]);
        sprintf(bufHeader.get(), "GET %s HTTP/1.1\r\n%s", url.get(), HEADERS);
        std::unique_ptr<std::stringstream> ss = request::httpsGetRequest(request::UrlData(url,bufHeader));
        std::unique_ptr<Data> pData = parseCSVData(move(ss));
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
}