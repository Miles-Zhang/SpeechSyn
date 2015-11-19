#include <stdio.h>
#include <math.h>
#ifndef PI
#define PI 3.141592653
#endif

/*
 void hamming(int len, float* win){
    int i = 0;
    for (i = 0; i < len; ++i){
        win[len] = 0.54 - 0.46 * cos(2*PI*i/len);        
    }
}
*/

void ffilter(float *b,int len_b, float *a, int len_a, float* input,int len, float* output){
/* filters the data in "input" with the filter described by vectors a and b to create the filtered data "output".
 a[0]*y[n] = b[0]*x[n] + b[1]*x[n-1] + ... + b[len_b-1]*x[n-len_b+1]
                       - a[1]*y[n-1] - ... - a[len_a-1]*y[n-len_a+1]
 Input:
    float* b: filter coefficients
    int len_b: length of coefficient b
    float* a: filter coefficients
    int len_a: length of coefficient a
    float* input: input sample points
    int len: length of input sample points
 Output:
     float* output: output sample points
*/
    int i, j, k;
    //normalizes the filter coefficients by a[0]
    for (i = 1; i < len_a; ++i){
        a[i] = a[i]/a[0];
    }
    for (i = 0; i < len_b; ++i){
        b[i] = b[i]/a[0];
    }
    // filter goes here ..
    for (i = 0; i < len; ++i){
        int J = (len_b>i) ? (i+1) : len_b;
        for (j = 0; j < J;++j){
            output[i] += b[j]*input[i-j];
        }
        int K = (len_a>i) ? (i+1) : len_a;
        for (k = 1; k < K; ++k){
            output[i] -= a[k]*output[i-k];
        }
    }
}

void lpc(float* input,int len, int N, int stride, float* aut, float* coef, float* pred, float *var){
/* return the N lpc coefficients of the input signal
Input:
 	float* input: input samples points
 	int len: length of sample points
 	int N: length of coefficients
 	int stride: stride (two-channel)
Middle:
 	float* aut: autocorrelation
Output:
	float* coef: lpc coefficients
    float* pred: predicted sample points by lpc
	float error: error
*/
    float sum;
    int i,j;
    int k,n;
    for (i = 0; i <= N; ++i) { //autocorrelation
    	sum = 0;
    	for (j = 0; j < len-i; ++j){
    	    sum += input[j*stride] * input[(j+i)*stride];
    	}
    	aut[i] = sum;
    }
    // calculate lpc coefficients by Levinson-Durbin recursion
    coef[0] = 1;
    float Ek = aut[0];
    for (k = 0; k < N; ++k){
        float lambda = 0;
        for (j = 0; j <= k; ++j){
            lambda -= coef[j] * aut[k+1-j];
        }
        lambda /= Ek;
        // update coef[0,..,k]
        for (n = 0; n <= (k+1)/2; ++n){
            float temp = coef[k+1-n] + lambda * coef[n];
            coef[n] = coef[n] + lambda * coef[k+1-n];
            coef[k+1-n] = temp;
        }
        Ek *= 1-lambda*lambda;
    }
    // calculate predicted sample points
    for (i = 0; i < len; i++){
        int l = (N>i) ? i : N;
        for (j = 1; j<=l; j++){
            pred[i] -= coef[j] * input[i-1-j];
        }
    }
    // calculate predition error variances
    *var = 0;
    for (i = 0; i < len; i++){
        *var += pow(pred[i]-input[i],2);
    }
    *var /= len;
}