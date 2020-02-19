// Copyright 2019 <Ciupitu Dennis-Mircea 333CA>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <string.h>

#include <complex.h>
 
double PI;
typedef double complex cplx;

double PI = atan2(1, 1) * 4;
int numThreads, N;
cplx* buf;
cplx* aux;
FILE* in;
FILE* out;
pthread_t tid[8];

struct mystruct {
	cplx* buf;
	cplx* aux;
	int n;
	int step;
	int thread_id;	
};

void getArgs(int argc, char **argv)
{
	if(argc < 4) {
		printf("Not enough paramters: ./homeworkFT inputValues.txt outputValues.txt numThreads\nprintLevel: 0=no, 1=some, 2=verbouse\n");
		exit(1);
	}
	in = fopen (argv[1], "r");
	out = fopen (argv[2], "w");
	numThreads = atoi(argv[3]);
}

void init() {
	fscanf (in, "%d", &N); 
	
	buf = malloc(N * sizeof(cplx));
	aux = malloc(N * sizeof(cplx));

	for (int i = 0; i < N; i++) {
		double x;
		fscanf (in, "%lf", &x);
		buf[i] = x;
		aux[i] = x;
	}
}

void fft(cplx buf[], cplx aux[], int n, int step)
{
	if (step < n) {
		fft(aux, buf, n, step * 2);
		fft(aux + step, buf + step, n, step * 2);
 
		for (int i = 0; i < n; i += 2 * step) {
			cplx t = cexp(-I * PI * i / n) * aux[i + step];
			buf[i / 2]     = aux[i] + t;
			buf[(i + n)/2] = aux[i] - t;
		}
	}
}

void* threadFunction(void* var) {

	struct mystruct mythread = *(struct mystruct *)var;

	fft(mythread.buf, mythread.aux, mythread.n, mythread.step);

	return NULL;
}

void* threadFunction2(void* var) {

	struct mystruct mythread = *(struct mystruct *)var;

	struct mystruct thread_aux, thread_aux2;
	thread_aux.step = 4;
	thread_aux.thread_id = mythread.thread_id + 1;
	thread_aux.n = N;
	thread_aux.buf = mythread.aux;
	thread_aux.aux = mythread.buf;

	thread_aux2.step = 4;
	thread_aux2.thread_id = mythread.thread_id + 2;
	thread_aux2.n = N;
	thread_aux2.buf = mythread.aux + mythread.step;
	thread_aux2.aux = mythread.buf + mythread.step;

	pthread_t tidd[2];

	pthread_create(&(tidd[0]), NULL, threadFunction, &thread_aux);

	pthread_create(&(tidd[1]), NULL, threadFunction, &thread_aux2);

	for(int i = 0; i < 2; i++) {
		pthread_join(tidd[i], NULL);
	}

	for (int i = 0; i < mythread.n; i += 2 * mythread.step) {
		cplx t = cexp(-I * PI * i / mythread.n) * mythread.aux[i + mythread.step];
		mythread.buf[i / 2]     = mythread.aux[i] + t;
		mythread.buf[(i + mythread.n)/2] = mythread.aux[i] - t;
	}

	return NULL;
}

void show(cplx buf[]) {
	char* str;

	str = malloc(100000 * sizeof(char));
	sprintf(str, "%d\n", N);


	for (int j = 0; j < strlen(str); j++) {
		fputc(str[j], out);
	}

	free(str);

	for (int i = 0; i < N; i++) {

		str = malloc(100000 * sizeof(char));

		sprintf(str, "%lf %lf\n", creal(buf[i]), cimag(buf[i]));

		for (int j = 0; j < strlen(str); j++) {
			fputc(str[j], out);
		}

		free(str);
	}
}

int main(int argc, char **argv)
{
	getArgs(argc, argv);
	init();

	pthread_t tid[numThreads];

	struct mystruct mythr, mythr2;
	mythr.n = N;
	mythr.step = 1;
	mythr.thread_id = 0;
	mythr.buf = buf;
	mythr.aux = aux;

	if (numThreads == 1) { // case of 1 thread
		pthread_create(&(tid[0]), NULL, threadFunction, &mythr);
		pthread_join(tid[0], NULL);
	} else if (numThreads == 2) { // case of 2 thread

		mythr.buf = aux;
		mythr.aux = buf;
		mythr.step = 2;
		pthread_create(&(tid[0]), NULL, threadFunction, &mythr);

		mythr2.n = N;
		mythr2.step = 2;
		mythr2.thread_id = 1;
		mythr2.buf = aux + 1;
		mythr2.aux = buf + 1;
		
		pthread_create(&(tid[1]), NULL, threadFunction, &mythr2);

		for(int i = 0; i < numThreads; i++) {
			pthread_join(tid[i], NULL);
		}

		for (int i = 0; i < N; i += 2) {
			cplx t = cexp(-I * PI * i / N) * aux[i + 1];
			buf[i / 2]     = aux[i] + t;
			buf[(i + N)/2] = aux[i] - t;
		}
	} else { // case of 4 thread
		mythr.buf = aux;
		mythr.aux = buf;
		mythr.step = 2;
		pthread_create(&(tid[0]), NULL, threadFunction2, &mythr);

		mythr2.n = N;
		mythr2.step = 2;
		mythr2.thread_id = 1;
		mythr2.buf = aux + 1;
		mythr2.aux = buf + 1;
		
		pthread_create(&(tid[1]), NULL, threadFunction2, &mythr2);

		for(int i = 0; i < 2; i++) {
			pthread_join(tid[i], NULL);
		}

		for (int i = 0; i < N; i += 2) {
			cplx t = cexp(-I * PI * i / N) * aux[i + 1];
			buf[i / 2]     = aux[i] + t;
			buf[(i + N)/2] = aux[i] - t;
		}

	}

	show(buf); // show output

	fclose(in);
	fclose(out);

	return 0;
}