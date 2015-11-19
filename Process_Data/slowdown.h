int findpitch(double *s, int len);
void hamming(int len, double* win);
void Process_Data(double *input, double *output, double *framebuf, double *excbuf, int FL, int step, int last, int PTlast);