//
//  lpc.h
//  lpc
//
//  Created by ZhaoYue on 15/11/12.
//  Copyright © 2015年 ZhaoYue. All rights reserved.
//

#ifndef lpc_h
#define lpc_h

void ffilter(float *b,int len_b, float *a, int len_a, float* input,int len, float* output);
void lpc(float* input,int len, int N, int stride, float* aut, float* coef, float* pred, float* var);

#endif /* lpc_h */
