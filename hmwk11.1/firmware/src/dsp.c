#include "dsp.h"

float maf(float data[], int size) {
    int i = 0;
    float sum = 0;
    for (i = 0; i < size; i++) {
        sum = sum + data[i];
    }
    return sum/(float)size;
}

float iir(float data[], float prev_avg){
    float a_coef = 0.9;
    float b_coef = 0.1;
    return a_coef*prev_avg + b_coef*data[0];
//    return (a_coef*old_avg + b_coef*data[0]);
    
}

float fir(float data[], int size){
    float sum = 0;
//    float weight[10] = {-0.0053 , -0.0028 ,   0.0435  ,  0.1695   , 0.2951   , 0.2951 ,   0.1695   , 0.0435  , -0.0028  , -0.0053};
    float weight[10] = { 0.0022  ,  0.0174   , 0.0737 ,   0.1662  ,  0.2405   , 0.2405    ,0.1662  ,  0.0737  ,  0.0174  ,  0.0022};
//    float weight[6] = {0.0197  ,  0.1324   , 0.3479   , 0.3479   , 0.1324   , 0.0197};
//    float weight[21] = {-0.0000  , -0.0021   ,-0.0063  , -0.0116   ,-0.0124  ,  0.0000  ,  0.0318  ,  0.0814  ,  0.1375  ,  0.1821  ,  0.1992  ,  0.1821, 0.1375  ,  0.0814 ,   0.0318 ,   0.0000  , -0.0124 ,  -0.0116 ,  -0.0063 ,  -0.0021  , -0.0000};
    int i = 0;
    for (i = 0; i < size; i++) {
        sum = sum + data[i]*weight[i];
    }
    return sum;
}