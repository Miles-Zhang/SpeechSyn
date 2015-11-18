#include <stdio.h>
#include <math.h>
extern void ffilter(double *b,int len_b, double *a, int len_a, double* input,int len, double* output);

int findpitch(double *s, int len) {
    //[B, A] = butter(5, 700/4000);
	double A[6]={1.0000000e+0, -3.2268610e+00,4.3810649e+00,-3.0782333e+00,1.1112175e+00,-1.6406087e-01};
	double B[6]={7.2272542e-04,3.6136271e-03,7.2272542e-03,7.2272542e-03,3.6136271e-03,7.2272542e-04};
    //s = filter(B,A,s);
    ffilter(B,6,A,6,s,len,s);
    //R = zeros(143,1);
    double R[143];
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
    double R1=0,R2=0,R3=0,n1=0,n2=0,n3=0;
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
    double Rop=R1;
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
