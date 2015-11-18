#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <math.h>
#ifndef PI
#define PI 3.141592653
#endif
extern void ffilter(double *b,int len_b, double *a, int len_a, double* input,int len, double* output);
/* filters the data in "input" with the filter described by vectors a and b to create the filtered data "output".
 a[0]*y[n] = b[0]*x[n] + b[1]*x[n-1] + ... + b[len_b-1]*x[n-len_b+1]
                       - a[1]*y[n-1] - ... - a[len_a-1]*y[n-len_a+1]
 Input:
    double* b: filter coefficients
    int len_b: length of coefficient b
    double* a: filter coefficients
    int len_a: length of coefficient a
    double* input: input sample points
    int len: length of input sample points
 Output:
     double* output: output sample points
*/
extern void lpc(double* input,int len, int N, int stride, double* aut, double* coef, double* pred, double* var);
/* return the N lpc coefficients of the input signal
Input:
 	double* input: input samples points
 	int len: length of sample points
 	int N: length of coefficients
 	int stride: stride (two-channel)
Middle:
 	double* aut: autocorrelation
Output:
	double* coef: lpc coefficients
    double* pred: predicted sample points by lpc
	double error: error
*/

extern int findpitch(double *s, int len);
/*find the pitch period of given excitation s*/

void hamming(int len, double* win){
    int i = 0;
    for (i = 0; i < len; ++i){
        win[len] = 0.54 - 0.46 * cos(2*PI*i/len);
    }
}

void Process_Data(double *input, double *output, double *framebuf, double *excbuf, int FL, int step, int last, int PTlast) {
/* computes the slowed-down voice
Input:
  double *input: current frame of one channel
  double *framebuf: a circular buffer containing 3 frames of input
  double *excbuf: a circular buffer containing 3 frames of excitation
  int FL: frame length
  int step: two channels, step=2
Output:
  double *output: the slowed-down voice of double length
*/
    double *fbufpt=framebuf, *ebufpt=excbuf;
    int WL=3*FL, FL2=2*FL; //window length and twice frame length
    memcpy(fbufpt+FL2,input,8*FL); //copy input to buffer
    int P=10; //number of prediction coefficients
    double //exc[FL], //excitation signal (prediction error)
      //zi_pre[P], //prediction filter status
		*exc_syn, //[FL2], //synthesized excitation (pulse string）
		//*s_syn, //[FL], //synthesized sound
      //zi_syn[P], //synthesis filter
		*s_w, //[WL];
		*s_syn_v, //[FL2], //synthesized excitation (pulse string）
		*hw; //[WL]
	exc_syn=(double *)malloc(sizeof(double)*FL2);
	s_w=(double *)malloc(sizeof(double)*WL);
	hw=(double *)malloc(sizeof(double)*WL);    //hamming window 
	/*
    for (int i = 0; i < P; i++) {
      zi_pre[i]=0;
    }
	*/
    hamming(WL, hw);
    for (int i = 0; i < WL-1; i++) {
      if (i<FL2-1) s_w[i]=hw[i]*framebuf[i];
      else s_w[i]=hw[i]*input[i-FL2];
    }
    double *coef, *aut, *pred, *var;
	coef=(double *)malloc(sizeof(double)*P);
	aut=(double *)malloc(sizeof(double)*P);
	pred=(double *)malloc(sizeof(double)*WL);
    lpc(s_w,FL,P,step, aut, coef, pred, var);
    //compute P coefficients with LPC algorithm
    //coef is the predicted coefficent array
    //var is error variance to calculate excitation power
    double b=1;
    //[y1,zi_pre]=filter(A,1,s_f,zi_pre); %s is input and e is output, save filter state
    //exc((n-1)*FL+1:n*FL)=y1.';
    ffilter(coef, P, &b, 1, input, FL, excbuf+FL2); //output is the excitation
    //note that the coef is "b" here and b=1 is "a"
    double *s_Pitch=fbufpt+WL-222;
    int PT; double G;
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
