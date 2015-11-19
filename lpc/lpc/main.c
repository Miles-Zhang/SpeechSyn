//
//  main.c
//  lpc
//
//  Created by ZhaoYue on 15/11/12.
//  Copyright © 2015年 ZhaoYue. All rights reserved.
//

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "lpc.h"
#ifndef PI
#define PI 3.141592653
#endif

int main(int argc, const char * argv[]) {
    // insert code here...
    srand(0);
    int i;
    float noise[500];
    float x[500];
    float x_pred[100];
    float h[4] = {1.0, 1./2, 1./3, 1./4}; // reference coefficient produced by MATLAB lpc(x,3)
    float a[1] = {1.0};
    for (i = 0; i < 500; ++i){
        float drand = (rand()+1.0)/(RAND_MAX+1.0);
        noise[i] = sqrt(-2*log(drand)) * cos(2*PI*drand);
    }
    memset(x, 0, sizeof(float)*500);
    memset(x_pred, 0, sizeof(float)*100);
    ffilter(a, 1, h, 4, noise, 500, x);

    for (i = 0; i < 500; ++i){
        printf("%f ", noise[i]);
    }
    printf("\n");
    for (i = 0; i < 500; ++i){
        printf("%f ", x[i]);
    }
    printf("\n");

    float aut[4];
    float coef[4];
    float var;
    // lpc(float* input,int len, int N, int stride, float* aut, float* coef, float* pred, float &error)
    lpc(&(x[401]), 100, 3, 1, aut, coef, x_pred, &var);
    printf("Linear prediction filter coefficients: ");
    for (i = 0; i < 4; i++){
        printf("%f ",coef[i]);
    }
    printf("\n");
    printf("Prediction error variances: %f\n", var);
    return 0;
}
