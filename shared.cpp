#include <signal.h>
#include "shared.hpp"


bool SIG_COND;


void error(const char* msg) {
  fprintf(stderr, "\nERROR: %s\n", msg);
  perror("\t");
  exit(-1);
}


void handler(int s) {
  if(SIG_COND)
    fprintf(stderr,"\n\"%s\" signal caught! Will try to exit nicely (no more threads are created, we will wait for the current threads to finish)\n", strsignal(s));
  SIG_COND = false;
}


//we are threading so we want make a nice signal handler for ctrl+c
void catch_SIG(){
  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sa.sa_handler = handler;

  //sigaction(SIGKILL, &sa, 0);
  sigaction(SIGTERM, &sa, 0);
  sigaction(SIGINT, &sa, 0);
  sigaction(SIGQUIT, &sa, 0);
  sigaction(SIGABRT, &sa, 0);
  sigaction(SIGPIPE, &sa, 0);
}


double check_interv(double value, bool verbose) {
  double errTol = 1e-5;

  if (value != value) {
    error("\nWARN: value is NaN!\n");
  } else if(value < errTol) {
    value = 0;
    if(verbose && value < 0) printf("\nWARN: value %f < 0!\n", value);
  } else if(value > 1 - errTol) {
    value = 1;
    if(verbose && value > 1) printf("\nWARN: value %f > 1!\n", value);
  }

  return value;
}


int array_max_pos(double* array, int size) {
  int res = 0;
  double max = -INFINITY;

  for (int cnt = 0; cnt < size; cnt++) {
    if (array[cnt] > max) {
      res = cnt;
      max = array[cnt];
    }
  }
  return res;
}



void call_geno(double* geno, int n_geno) {

  int max_pos = array_max_pos(geno, n_geno);
    
  for (int g = 0; g < n_geno; g++)
    geno[g] = -INFINITY;

  geno[max_pos] = log(1);
}



//function does: log(exp(a)+exp(b)) while protecting for underflow
double logsum(double* a, uint64_t n){
  // Find maximum value
  double sum = 0;
  double M = a[0];

  for(uint64_t i = 1; i < n; i++)
    M = max(a[i], M);

  for(uint64_t i = 0; i < n; i++)
    sum += exp(a[i] - M);

  return(log(sum) + M);
}


// Special logsum case for size == 2
double logsum2(double a, double b){
  double buf[2];
  buf[0] = a;
  buf[1] = b;
  return logsum(buf, 2);
}


// Special logsum case for size == 3
double logsum3(double a, double b, double c){
  double buf[3];
  buf[0] = a;
  buf[1] = b;
  buf[2] = c;
  return logsum(buf, 3);
}

/***************************
split()
 function to, given a 
 string (str), splits into 
 array (out) on char (sep)
***************************/
uint64_t split(char* str, const char* sep, int** out){
  uint64_t i = strlen(str);
  int* buf = new int[i];

  i = 0;
  char* pch = strtok(str, sep);
  while(pch != NULL){
    buf[i++] = atoi(pch);
    pch = strtok(NULL, sep);
  }

  *out = new int[i]; // FGV: why the need for *out?!?!!?
  memcpy(*out, buf, i*sizeof(int));

  delete [] buf;
  return(i);
}


uint64_t split(char* str, const char* sep, float** out){
  uint64_t i = strlen(str);
  float* buf = new float[i];

  i = 0;
  char* pch = strtok(str, sep);
  while(pch != NULL){
    buf[i++] = atof(pch);
    pch = strtok(NULL, sep);
  }

  *out = new float[i];
  memcpy(*out, buf, i*sizeof(float));

  delete [] buf;
  return(i);
}


uint64_t split(char* str, const char* sep, double** out){
  uint64_t i = strlen(str);
  double* buf = new double[i];

  i = 0;
  char* pch = strtok(str, sep);
  while(pch != NULL){
    buf[i++] = atof(pch);
    pch = strtok(NULL, sep);
  }

  *out = new double[i];
  memcpy(*out, buf, i*sizeof(double));

  delete [] buf;
  return(i);
}


uint64_t split(char* str, const char* sep, char*** out){
  uint64_t i = strlen(str);
  char** buf = new char*[i];

  i = 0;
  buf[i] = strtok(str, sep);

  while(buf[i] != NULL)
    buf[++i] = strtok(NULL, sep);

  *out = new char*[i];
  for(uint64_t cnt = 0; cnt < i; cnt++)
    *out[i] = buf[i];

  delete [] buf;
  return(i);
}



char* merge(uint64_t* array, uint64_t size, const char* sep){
  char* buf = new char[size*10000];
  
  sprintf(buf, "%lu", array[0]);
  uint64_t len = strlen(buf);

  for(uint64_t cnt = 1; cnt < size; cnt++){
    sprintf(buf+len, "%s%lu", sep, array[cnt]);
    len = strlen(buf);
  }
  
  char* str = new char[len+1];
  strcpy(str, buf);
  delete [] buf;

  return str;
}



char* merge(double* array, uint64_t size, const char* sep){
  char* buf = new char[size*10000];
  
  sprintf(buf, "%.10f", array[0]);
  uint64_t len = strlen(buf);

  for(uint64_t cnt = 1; cnt < size; cnt++){
    sprintf(buf+len, "%s%.10f", sep, array[cnt]);
    len = strlen(buf);
  }

  char* str = new char[len+1];
  strcpy(str, buf);
  delete [] buf;

  return str;
}



uint64_t* init_uint64(uint64_t A, uint64_t init){
  uint64_t* ptr = new uint64_t[A];
  for(uint64_t a = 0; a < A; a++)
    ptr[a] = init;

  return ptr;
}



uint64_t** init_uint64(uint64_t A, uint64_t B, uint64_t init){
  uint64_t** ptr = new uint64_t*[A];
  for(uint64_t a = 0; a < A; a++)
    ptr[a] = init_uint64(B, init);

  return ptr;
}



double* init_double(uint64_t A, double init){
  double* ptr = new double[A];
  for(uint64_t a = 0; a < A; a++)
    ptr[a] = init;

  return ptr;
}



double** init_double(uint64_t A, uint64_t B, double init){
  double** ptr = new double*[A];
  for(uint64_t a = 0; a < A; a++)
    ptr[a] = init_double(B, init);

  return ptr;
}



double*** init_double(uint64_t A, uint64_t B, uint64_t C, double init){
  double*** ptr = new double**[A];
  for(uint64_t a = 0; a < A; a++)
    ptr[a] = init_double(B, C, init);

  return ptr;
}



char* init_char(uint64_t A, const char* init){
  char* ptr = new char[A];
  memset(ptr, '\0', A*sizeof(char));

  strcpy(ptr, init);

  return ptr;
}



char** init_char(uint64_t A, uint64_t B, const char* init){
  char** ptr = new char*[A];
  for(uint64_t a = 0; a < A; a++){
    char init_tmp[1000];
    strcpy(init_tmp, init);

    char* pch = strchr(init_tmp, '#');
    if(pch)
      sprintf(pch, "%lu", a);

    ptr[a] = init_char(B, init_tmp);
  }

  return ptr;
}



void free_ptr(void* ptr){
  delete [] (char*)ptr;
}



void free_ptr(void** ptr, uint64_t A){
  for(uint64_t a = 0; a < A; a++)
    free_ptr(ptr[a]);

  free_ptr(ptr);
}



void free_ptr(void*** ptr, uint64_t A, uint64_t B){
  for(uint64_t a = 0; a < A; a++)
    free_ptr(ptr[a], B);

  free_ptr(ptr);
}



void cpy(void* dest, void* orig, uint64_t A, uint64_t size){
  memcpy(dest, orig, A * size);
}



void cpy(void* dest, void* orig, uint64_t A, uint64_t B, uint64_t size){
  for(uint64_t a = 0; a < A; a++)
    cpy( ((char**)dest)[a], ((char**)orig)[a], B, size);
}



void cpy(void* dest, void* orig, uint64_t A, uint64_t B, uint64_t C, uint64_t size){
  for(uint64_t a = 0; a < A; a++)
    cpy( ((char***)dest)[a], ((char***)orig)[a], B, C, size);
}