#include <stdio.h>

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

void hamming(int len, double* win){
    int i = 0;
    for (i = 0; i < len; ++i){
        win[len] = 0.54 - 0.46 * cos(2*PI*i/len);        
    }
}

void Process_Data(double *input, int FL, double *output)
  int WL=3*FL; //window length
  int P = 10; //number of prediction coefficients
  double exc[FL], //excitation signal (prediction error)
    zi_pre[P], //prediction filter status
    s_rec[FL], //reconstructed sound
    zi_rec[P],
    //synthesis filter
    exc_syn[FL], //synthesized excitation (pulse string）
    s_syn[FL], //synthesized sound
    zi_syn[P];
  int last1=2*FL, last2=2*FL, PTlast1=0, PTlast2=0;
    
    // slow down by half speed
    double exc_syn_v[2*FL], //synthesized excitation (pulse string）
    s_syn_v[2*FL], //synthesized sound
    zi_v[P];
/*
    //hamming window
    double hw[WL];
    hamming(hw,&WL);
*/    

        % 计算预测系数（不需要掌握）
        s_w = s(n*FL-WL+1:n*FL).*hw;    %汉明窗加权后的语音
        [A E] = lpc(s_w, P);            %用线性预测法计算P个预测系数
                                        % A是预测系数，E会被用来计算合成激励的能量
        
        s_f = s((n-1)*FL+1:n*FL);       % 本帧语音，下面就要对它做处理

        % (4) 在此位置写程序，用filter函数s_f计算激励，注意保持滤波器状态
        [y1,zi_pre]=filter(A,1,s_f,zi_pre); %s is input and e is output, save filter state
            
        % exc((n-1)*FL+1:n*FL) = ... 将你计算得到的激励写在这里
        exc((n-1)*FL+1:n*FL)=y1.';
        
        % (5) 在此位置写程序，用filter函数和exc重建语音，注意保持滤波器状态
        [y2,zi_rec]=filter(1,A,y1,zi_rec); %reconstruct voice
        
        % s_rec((n-1)*FL+1:n*FL) = ... 将你计算得到的重建语音写在这里       
        s_rec((n-1)*FL+1:n*FL)=y2.';

        % 注意下面只有在得到exc后才会计算正确
        s_Pitch = exc(n*FL-222:n*FL);
        PT = findpitch(s_Pitch);    % 计算基音周期PT（不要求掌握）
        G = sqrt(E*PT);           % 计算合成激励的能量G（不要求掌握）

        
        % (10) 在此位置写程序，生成合成激励，并用激励和filter函数产生合成语音
        
        % exc_syn((n-1)*FL+1:n*FL) = ... 将你计算得到的合成激励写在这里
        for p=(n-1)*FL+1:n*FL
            if p-last1==PTlast1||p-last1==PT
                exc_syn(p)=G;
                last1=p;
                PTlast1=PT;
            end
        end
        
        % s_syn((n-1)*FL+1:n*FL) = ...   将你计算得到的合成语音写在这里
        [y3,zi_syn]=filter(1,A,exc_syn((n-1)*FL+1:n*FL),zi_syn);
        s_syn((n-1)*FL+1:n*FL)=y3.';
        
        % (11) 不改变基音周期和预测系数，将合成激励的长度增加一倍，再作为filter
        % 的输入得到新的合成语音，听一听是不是速度变慢了，但音调没有变。
        
        % exc_syn_v((n-1)*FL_v+1:n*FL_v) = ... 将你计算得到的加长合成激励写在这里
        exc_syn_v(2*(n-1)*FL+1:2*n*FL)=[exc_syn((n-1)*FL+1:n*FL),...
            exc_syn((n-1)*FL+1:n*FL)];     

        % s_syn_v((n-1)*FL_v+1:n*FL_v) = ...   将你计算得到的加长合成语音写在这里
        [s_syn_v(2*(n-1)*FL+1:2*n*FL),zi_v]=filter(1,A,...
            exc_syn_v(2*(n-1)*FL+1:2*n*FL),zi_v);
        
        % (13) 将基音周期减小一半，将共振峰频率增加150Hz，重新合成语音，听听是啥感受〜
        
        % exc_syn_t((n-1)*FL+1:n*FL) = ... 将你计算得到的变调合成激励写在这里
        PT=round(PT/2);
        for p=(n-1)*FL+1:n*FL
            if p-last2==PTlast2||p-last2==PT
                exc_syn_t(p)=G;
                last2=p;
                PTlast2=PT;
            end
        end
        %exc_syn_t((n-1)*FL+1:n*FL)=exc_syn_v(2*(n-1)*FL+1:2:2*n*FL);
        %both transforms double the original excitation
        
        % s_syn_t((n-1)*FL+1:n*FL) = ...   将你计算得到的变调合成语音写在这里
        [bb,aa]=eqtflength(1,A);
        [zz,pp,kk]=tf2zp(bb,aa); %compute origial zeros and poles 
        for nn=1:length(pp) %compute new poles
            if imag(pp(nn))>0
                pp(nn)=pp(nn)*exp(j*2*pi*150*Ts);
            else if imag(pp(nn))<0
                pp(nn)=pp(nn)*exp(-j*2*pi*150*Ts);
                end
            end
        end
        [bbb,aaa]=zp2tf(zz,pp,kk);
        [s_syn_t((n-1)*FL+1:n*FL), zi_t]=filter(bbb,aaa,...
            exc_syn_t((n-1)*FL+1:n*FL), zi_t);
        if n==27
            subplot(1,2,2); %plot the new poles along the previous graph
            zplane(zz,pp);
        end    
    end
    
   


% 计算一段语音的基音周期，不要求掌握
function PT = findpitch(s)
[B, A] = butter(5, 700/4000);
s = filter(B,A,s);
R = zeros(143,1);
for k=1:143
    R(k) = s(144:223)'*s(144-k:223-k);
end
[R1,T1] = max(R(80:143));
T1 = T1 + 79;
R1 = R1/(norm(s(144-T1:223-T1))+1);
[R2,T2] = max(R(40:79));
T2 = T2 + 39;
R2 = R2/(norm(s(144-T2:223-T2))+1);
[R3,T3] = max(R(20:39));
T3 = T3 + 19;
R3 = R3/(norm(s(144-T3:223-T3))+1);
Top = T1;
Rop = R1;
if R2 >= 0.85*Rop
    Rop = R2;
    Top = T2;
end
if R3 > 0.85*Rop
    Rop = R3;
    Top = T3;
end
PT = Top;
return