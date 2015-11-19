#include <Talkthrough.h>
struct bufferedframe {
  float data[FRAMESIZE];
  struct bufferedframe *nextframe;
};
int findpitch(float *s, int len);
void hamming(int len, float* win);
void slowdown(float *input, float *output, bufferedframe *framebuf, bufferedframe *excbuf, int step, int last, int PTlast);
extern void ffilter(float *b,int len_b, float *a, int len_a, float* input,int len, float* output);
extern void lpc(float* input,int len, int N, int stride, float* aut, float* coef, float* pred, float* var);
