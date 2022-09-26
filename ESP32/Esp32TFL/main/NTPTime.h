#ifndef NTP_TIME_H_
#define NTP_TIME_H_

namespace NTPTime{
    const char TIME_ZONE[] = "UTC-2";

    void obtain_time();
    void setupNTPTime();
}

#endif  // NTP_TIME_H_
