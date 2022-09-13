#ifndef EXPONENTIAL_SMOOTHING_H_
#define EXPONENTIAL_SMOOTHING_H_

#include "esp_system.h"

class ExpSmoothing {
    private:
        const float ALPHA = 0.031324384588082115;
        const float BETA = 0;
        const float GAMMA = 1.0429298296798e-06;
        const int L = 56; // 24*60/180 =8*7 
        const int PRED_N = 16;

        const float ALPHAC = 1 - ALPHA;
        const float BETAC = 1 - BETA;
        const float GAMMAC = 1 - GAMMA;

        float s[2]; // = {1.0994286907946866f, 0.0f};
        float b[2]; // = {0, 0};
        float* c;
        int predPos;

    public:
        ExpSmoothing();
        float next(float x);
        esp_err_t save_s();
        esp_err_t save_c();
        esp_err_t save_pred_pos();
        esp_err_t load_s();
        esp_err_t load_c();
        esp_err_t load_pred_pos();
        // esp_err_t save_b();
        // esp_err_t load_b();
        ~ExpSmoothing();
};

#endif  // EXPONENTIAL_SMOOTHING_H_