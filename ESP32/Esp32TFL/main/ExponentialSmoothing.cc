#include "ExponentialSmoothing.h"
#include <stdio.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"


#define STORAGE_NAMESPACE "storage"


ExpSmoothing::ExpSmoothing(){

    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );

    
    err = load_s();
    if (err != ESP_OK) printf("Error (%s) reading 's' from NVS!\n", esp_err_to_name(err));
    err = load_pred_pos();
    if (err != ESP_OK) printf("Error (%s) reading 'pred_pos' from NVS!\n", esp_err_to_name(err));
    err = load_c();
    if (err != ESP_OK) printf("Error (%s) reading 'c' from NVS!\n", esp_err_to_name(err));

    b[0] = 0;
}

ExpSmoothing::~ExpSmoothing(){
    printf("saving ExpSmoothing Values \n");
    save_pred_pos();
    save_s();
    save_c();
}

esp_err_t ExpSmoothing::load_pred_pos(){   
    predPos = 0;

    nvs_handle_t  handle;
    esp_err_t err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) return err;
       
    err = nvs_get_i32(handle, "pred_pos", &predPos);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;

    nvs_close(handle);
    return ESP_OK;
    
}

esp_err_t ExpSmoothing::save_pred_pos(){
    nvs_handle_t  handle;
    esp_err_t err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) return err;
       
    err = nvs_set_i32(handle, "pred_pos", predPos);
    if (err != ESP_OK) return err;

    err = nvs_commit(handle);
    if (err != ESP_OK) return err;

    nvs_close(handle);
    return ESP_OK;
}

esp_err_t ExpSmoothing::load_s(){
    s[0] = 1.0994286907946866f;

    nvs_handle_t  handle;
    esp_err_t err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) return err;
       
    size_t required_size = 0;  
    err = nvs_get_blob(handle, "s_value", NULL, &required_size);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;

    if (required_size > 0) {
        err = nvs_get_blob(handle, "s_value", s[0], &required_size);
        if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND){
            return err;  
        } 
    }
    nvs_close(handle);
    return ESP_OK;
    
}

esp_err_t ExpSmoothing::save_s(){
    nvs_handle_t  handle;
    esp_err_t err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) return err;
       
    size_t required_size = sizeof(float); 
    err = nvs_set_blob(handle, "s_value", &s[0], required_size);
    if (err != ESP_OK) return err;

    // Commit
    err = nvs_commit(handle);
    if (err != ESP_OK) return err;

    // Close
    nvs_close(handle);
    return ESP_OK;
}

esp_err_t ExpSmoothing::load_c(){
    // i realy love you c++ if this is only way to initialize value's to array 
    c = new float[L];
    c[0] = -0.9335412979125977;
    c[1] = -0.9428023099899292;
    c[2] = 0.7556760311126709;
    c[3] = -0.8164358139038086;
    c[4] = -0.8204375505447388;
    c[5] = -0.08866075426340103;
    c[6] = 1.75569486618042;
    c[7] = 0.005341673269867897;
    c[8] = -0.9301676154136658;
    c[9] = -0.944689929485321;
    c[10] = 0.5793561339378357;
    c[11] = -0.7564494013786316;
    c[12] = -0.7227949500083923;
    c[13] = 0.057678285986185074;
    c[14] = 1.9204846620559692;
    c[15] = -0.10770472139120102;
    c[16] = -0.9215030074119568;
    c[17] = -0.9418848752975464;
    c[18] = 0.5223917365074158;
    c[19] = -0.661611020565033;
    c[20] = -0.6668349504470825;
    c[21] = 0.06539791822433472;
    c[22] = 1.546073317527771;
    c[23] = 0.09131716191768646;
    c[24] = -0.9430896043777466;
    c[25] = -0.9339984655380249;
    c[26] = 0.5075753331184387;
    c[27] = 2.4773547649383545;
    c[28] = 1.176513910293579;
    c[29] = 0.8252004981040955;
    c[30] = 0.8504698276519775;
    c[31] = -0.2055792212486267;
    c[32] = -0.9456291794776917;
    c[33] = -0.9362808465957642;
    c[34] = 0.345670610666275;
    c[35] = 1.2368050813674927;
    c[36] = 1.0247019529342651;
    c[37] = 0.5537694096565247;
    c[38] = 1.011054515838623;
    c[39] = -0.24606357514858246;
    c[40] = -0.9267645478248596;
    c[41] = -0.9417620301246643;
    c[42] = 0.6208714842796326;
    c[43] = -0.7007784843444824;
    c[44] = -0.766292154788971;
    c[45] = -0.18781453371047974;
    c[46] = 1.6430203914642334;
    c[47] = -0.16853319108486176;
    c[48] = -0.9409453272819519;
    c[49] = -0.9429402351379395;
    c[50] = 0.4631054401397705;
    c[51] = -0.7717548608779907;
    c[52] = -0.8088725805282593;
    c[53] = 0.02668880484998226;
    c[54] = 1.6311900615692139;
    c[55] = -0.06401696801185608;

    nvs_handle_t  handle;
    esp_err_t err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) return err;

    size_t required_size = 0; 
    err = nvs_get_blob(handle, "c_value", NULL, &required_size);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;

    if (required_size > 0) {
        err = nvs_get_blob(handle, "c_value", c, &required_size);
        if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND){
            return err;  
        } 
    }
    nvs_close(handle);
    return ESP_OK;
}

esp_err_t ExpSmoothing::save_c(){
    nvs_handle_t  handle;
    esp_err_t err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) return err;
       
    size_t required_size = sizeof(float)*L; 
    err = nvs_set_blob(handle, "c_value", c, required_size);
    if (err != ESP_OK) return err;

    // Commit
    err = nvs_commit(handle);
    if (err != ESP_OK) return err;

    // Close
    nvs_close(handle);
    return ESP_OK;
}

float ExpSmoothing::next(float x){
    s[1] = ALPHA * (x - c[predPos % L]) + ALPHAC * (s[0] + b[0]);
    b[1] = BETA * (s[1] - s[0]) + (BETAC * b[0]);

    float predicted = s[1] + PRED_N*b[1] + c[(predPos + PRED_N) % L];

    c[predPos % L] = GAMMA * (x - GAMMA * (s[0] + b[0] )) + (GAMMAC * c[predPos% L]);
    
    predPos++;
    s[0] = s[1];
    b[0] = b[1];

    return predicted;
}

void ExpSmoothing::print(){
    printf("\n");
    printf("s value: %f, pred pos: %d \n", s[0], predPos);
    printf("c[0]: %f, c[1]: %f, c[2]: %f, c[3]: %f", c[0],c[1],c[2],c[3]);
}

void ExpSmoothing::saveAll(){
    printf("saving");
    save_pred_pos();
    save_s();
    save_c();
}

void ExpSmoothing::removeFromNVS(){
    nvs_flash_erase();
}