//
//  lpc.h
//  lpc
//
//  Created by ZhaoYue on 15/11/12.
//  Copyright © 2015年 ZhaoYue. All rights reserved.
//

#ifndef lpc_h
#define lpc_h

void ffilter(double *b,int len_b, double *a, int len_a, double* input,int len, double* output);
void lpc(double* input,int len, int N, int stride, double* aut, double* coef, double* pred, double* var);

#endif /* lpc_h */
