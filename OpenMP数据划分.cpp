#include<iostream>
#include<iomanip>
#include<omp.h>
#include<pthread.h>
#include <sys/time.h>
using namespace std;
void generateSample(float** A, int N) {
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < i; j++) {
			A[i][j] = 0;//�����Ǹ�ֵΪ0;
		}
		A[i][i] = 1.0;//�Խ��߸�ֵΪ1;
		for (int j = i; j < N; j++) {
			A[i][j] = rand();//�����Ǹ�ֵΪ����ֵ;
		}
	}
	for (int k = 0; k < N; k++) {
		for (int i = k + 1; i < N; i++) {
			for (int j = 0; j < N; j++) {
				A[i][j] += A[k][j];//ÿһ�ж����ϱ��Լ��±�С����;
			}
		}
	}
}
void show(float** A, int N) {//��ӡ���;
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			cout << fixed << setprecision(0) << A[i][j] << " ";
		}
		cout << endl;
	}
}

//�����㷨:
void serialSolution(float** A, int N) {
	for (int k = 0; k < N; k++) {
		for (int j = k + 1; j < N; j++) {
			A[k][j] /= A[k][k];
		}
		A[k][k] = 1.0;
		for (int i = k + 1; i < N; i++) {
			for (int j = k + 1; j < N; j++) {
				A[i][j] -= A[i][k] * A[k][j];
			}
			A[i][k] = 0;
		}
	}
}

//��̬����:
void Static(float** A, int N) {
    int NUM_THREADS = 7;
    #pragma omp parallel num_threads(NUM_THREADS),shared(A)
	for (int k = 0; k < N; k++) {
		#pragma omp single
		for (int j = k + 1; j < N; j++) {
			A[k][j] /= A[k][k];
		}
		A[k][k] = 1.0;
        #pragma omp for
		for (int i = k + 1; i < N; i++) {
			for (int j = k + 1; j < N; j++) {
				A[i][j] -= A[i][k] * A[k][j];
			}
			A[i][k] = 0;
		}
	}
}
//��̬����:
void Dynamic(float** A, int N) {
    int NUM_THREADS = 7;
    #pragma omp parallel num_threads(NUM_THREADS),shared(A)
	for (int k = 0; k < N; k++) {
		#pragma omp single
		for (int j = k + 1; j < N; j++) {
			A[k][j] /= A[k][k];
		}
		A[k][k] = 1.0;
        #pragma omp for schedule(dynamic)
		for (int i = k + 1; i < N; i++) {
			for (int j = k + 1; j < N; j++) {
				A[i][j] -= A[i][k] * A[k][j];
			}
			A[i][k] = 0;
		}
	}
}

int main() {
	float** A;
	float** B;
	float** C;
	int N = 1280;
	A = new float* [N];
	for (int i = 0; i < N; i++) {
		A[i] = new float[N];//����ռ�;
	}
	B = new float* [N];
	for (int i = 0; i < N; i++) {
		B[i] = new float[N];//����ռ�;
	}
	C = new float* [N];
	for (int i = 0; i < N; i++) {
		C[i] = new float[N];//����ռ�;
	}
	int step = 64;
	int counter1;
	int counter2;
	int counter3;
	struct timeval start1;
	struct timeval end1;
	struct timeval start2;
	struct timeval end2;
	struct timeval start3;
	struct timeval end3;
	cout.flags(ios::left);

	for (int i = step; i <= N; i += step) {
		//�����㷨
		generateSample(A, i);
		counter1 = 0;
		gettimeofday(&start1, NULL);
		gettimeofday(&end1, NULL);
		while ((end1.tv_sec - start1.tv_sec) < 1) {
			counter1++;
			serialSolution(A, i);
			//parallelSolution(B, i);
			gettimeofday(&end1, NULL);
		}

		//��̬�����㷨:
		generateSample(B, i);
		counter2 = 0;
		gettimeofday(&start2, NULL);
		gettimeofday(&end2, NULL);
		while ((end2.tv_sec - start2.tv_sec) < 1) {
			counter2++;
			//serialSolution(A, i);
			Static(B, i);
			gettimeofday(&end2, NULL);
		}

		//��̬�����㷨:
		generateSample(C, i);
		counter3 = 0;
		gettimeofday(&start3, NULL);
		gettimeofday(&end3, NULL);
		while ((end3.tv_sec - start3.tv_sec) < 1) {
			counter3++;
			Dynamic(C, i);
			gettimeofday(&end3, NULL);
		}

		//��ʱͳ��:
		float time1 = (end1.tv_sec - start1.tv_sec) + float((end1.tv_usec - start1.tv_usec)) / 1000000;//��λs;
		float time2 = (end2.tv_sec - start2.tv_sec) + float((end2.tv_usec - start2.tv_usec)) / 1000000;//��λs;
		float time3 = (end3.tv_sec - start3.tv_sec) + float((end3.tv_usec - start3.tv_usec)) / 1000000;//��λs;

		cout << fixed << setprecision(6);
        cout << "�����ģ" <<  i << ": " << endl;
		cout << " " << setw(18) << "���߳�ƽ����ʱ��" << setw(20) << time1 / counter1 << endl;
        cout << " " << setw(18) << "��̬ƽ����ʱ��" << setw(20) << time2 / counter2 << endl;
		cout << " " <<setw(18) << "��̬ƽ����ʱ��" << setw(20) << time3 / counter3 << endl;
        cout << endl;
	}
	return 0;
}
