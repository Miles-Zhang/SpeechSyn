#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <math.h>
#include <slowdown.h>
#ifndef PI
#define PI 3.141592653
#endif


//extern int findpitch(float *s, int len);
/*find the pitch period of given excitation s*/

void hamming(int len, float* win){
    int i = 0;
    for (i = 0; i < len; ++i){
        win[len] = 0.54 - 0.46 * cos(2*PI*i/len);
    }
}

void slowdown(float *input, float *output, bufferedframe *framebuf, bufferedframe *excbuf, int FL, int step, int last, int PTlast) {
/* computes the slowed-down voice
Input:
  float *input: current frame of one channel
  float *framebuf: a circular buffer containing 3 frames of input
  float *excbuf: a circular buffer containing 3 frames of excitation
  int FL: frame length
  int step: two channels, step=2
Output:
  float *output: the slowed-down voice of float length
*/

    int WL=3*FL, FL2=2*FL; //window length and twice frame length
    memcpy(frambuf->data,input,8*FL); //copy input to buffer
    int P=10; //number of prediction coefficients
    float *s_w, //windowed speech [WL]
    *exc_syn, //[FL2]
		*s_syn_v, //[FL2], //synthesized excitation (pulse string）
		*hw; //[WL]
	exc_syn=(float *)malloc(sizeof(float)*FL2);
	s_w=(float *)malloc(sizeof(float)*WL);
	hw=(float *)malloc(sizeof(float)*WL);    //hamming window
  float *coef, *aut, *pred, *var;
  coef=(float *)malloc(sizeof(float)*P);
  aut=(float *)malloc(sizeof(float)*P);
  pred=(float *)malloc(sizeof(float)*WL);
  lpc(s_w,FL,P,step, aut, coef, pred, var);
  //compute P coefficients with LPC algorithm
  //coef is the predicted coefficent array
  //var is error variance to calculate excitation power

    hamming(WL, hw);
    for (int i = 0; i < WL-1; i++) {
      if (i<FL2-1) s_w[i]=hw[i]*framebuf[i];
      else s_w[i]=hw[i]*input[i-FL2];
    }

    float b=1;
    //[y1,zi_pre]=filter(A,1,s_f,zi_pre); %s is input and e is output, save filter state
    //exc((n-1)*FL+1:n*FL)=y1.';
    ffilter(coef, P, &b, 1, input, FL, excbuf+FL2); //output is the excitation
    //note that the coef is "b" here and b=1 is "a"
    float *s_Pitch=frambuf+WL-222;
    int PT; float G;
    PT = findpitch(s_Pitch, FL);    //find pitch period
    G = sqrt(*var*PT);           //find power
    //the synthesized excitation as below guarantees no intermediate PT value at the conjunction of two frames
    int p=0;
    if (PT<=PTlast) p=PTlast-last-1;
    else p=PT-last-1;
    while (p<FL) {
      exc_syn[p]=G;
      p=p+PT;
    };
    last=FL-(p-PT)-1; //number of sample after last pulse in this frame
    PTlast=PT; //PT of this frame
    /*
    [y3,zi_syn]=filter(1,A,exc_syn((n-1)*FL+1:n*FL),zi_syn);
    s_syn((n-1)*FL+1:n*FL)=y3.';
    exc_syn_v(2*(n-1)*FL+1:2*n*FL)=[exc_syn((n-1)*FL+1:n*FL),...
        exc_syn((n-1)*FL+1:n*FL)];
    [s_syn_v(2*(n-1)*FL+1:2*n*FL),zi_v]=filter(1,A,...
        exc_syn_v(2*(n-1)*FL+1:2*n*FL),zi_v);
    */
    memcpy(exc_syn+FL,exc_syn,8*FL);
    ffilter(&b,1, coef, 1, exc_syn, FL2, output);
    free()

}

int findpitch(float *s, int len) {
    //[B, A] = butter(5, 700/4000);
	float A[6]={1.0000000e+0, -3.2268610e+00,4.3810649e+00,-3.0782333e+00,1.1112175e+00,-1.6406087e-01};
	float B[6]={7.2272542e-04,3.6136271e-03,7.2272542e-03,7.2272542e-03,3.6136271e-03,7.2272542e-04};
    //s = filter(B,A,s);
    ffilter(B,6,A,6,s,len,s);
    //R = zeros(143,1);
    float R[143];
    /*
    for k=1:143
        R(k) = s(144:223)'*s(144-k:223-k);
    end
    */
    for (int i = 0; i < 143; i++) {
      R[i]=0;
      for (int j = 143; j < 222; j++) {
          R[i]+=s[j]*s[j-i];
      }
    }
    /*
    [R1,T1] = max(R(80:143));
    T1 = T1 + 79;
    R1 = R1/(norm(s(144-T1:223-T1))+1);
    [R2,T2] = max(R(40:79));
    T2 = T2 + 39;
    R2 = R2/(norm(s(144-T2:223-T2))+1);
    [R3,T3] = max(R(20:39));
    T3 = T3 + 19;
    R3 = R3/(norm(s(144-T3:223-T3))+1);
    */
    float R1=0,R2=0,R3=0,n1=0,n2=0,n3=0;
    int T1=0,T2=0,T3=0;
    for (int i = 79; i < 142; i++) {
      if (R[i]>R1) {
        R1=R[i];
        T1=i;
      }
    }
    for (int i = 143-T1; i < 222-T1; i++) {
      n1+=s[i]*s[i];
    }
    R1=R1/(sqrt(n1)+1);
    for (int i = 39; i < 78; i++) {
      if (R[i]>R2) {
        R2=R[i];
        T2=i;
      }
    }
    for (int i = 143-T2; i < 222-T2; i++) {
      n2+=s[i]*s[i];
    }
    R2=R2/(sqrt(n2)+1);
    for (int i = 19; i < 38; i++) {
      if (R[i]>R3) {
        R3=R[i];
        T3=i;
      }
    }
    for (int i = 143-T3; i < 222-T3; i++) {
      n3+=s[i]*s[i];
    }
    R3=R3/(sqrt(n1)+1);
    int Top=T1;
    float Rop=R1;
    if (R2>=0.85*Rop) {
      Rop=R2;
      Top=T2;
    }
    if (R3>0.85*Rop) {
      Rop=R3;
      Top=T3;
    }
    return Top;
}
