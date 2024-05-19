#include<iostream>
#include<iomanip>
#include<omp.h>
#include<pthread.h>
#include<xmmintrin.h> //SSE
#include<emmintrin.h> //SSE2
#include<pmmintrin.h> //SSE3
#include<tmmintrin.h> //SSSE3
#include<smmintrin.h> //SSE4.1
#include<nmmintrin.h> //SSSE4.2
#include<immintrin.h> //AVX、AVX2
#include <sys/time.h>
using namespace std;
void generateSample(float** A, int N) {
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < i; j++) {
			A[i][j] = 0;//下三角赋值为0;
		}
		A[i][i] = 1.0;//对角线赋值为1;
		for (int j = i; j < N; j++) {
			A[i][j] = rand();//上三角赋值为任意值;
		}
	}
	for (int k = 0; k < N; k++) {
		for (int i = k + 1; i < N; i++) {
			for (int j = 0; j < N; j++) {
				A[i][j] += A[k][j];//每一行都加上比自己下标小的行;
			}
		}
	}
}
void show(float** A, int N) {//打印结果;
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			cout << fixed << setprecision(0) << A[i][j] << " ";
		}
		cout << endl;
	}
}

//串行算法:
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

//并行算法:
void parallelSolution(float** A, int N) {
    int NUM_THREADS = 7;
    #pragma omp parallel num_threads(NUM_THREADS),shared(A)
    for (int k = 0; k < N; k++) {
		__m128 vt = _mm_set1_ps(A[k][k]);
		for (int j = k + 1; j + 4 <= N; j += 4) {
			__m128 va = _mm_loadu_ps(&A[k][j]);
			va = _mm_div_ps(va, vt);
			_mm_storeu_ps(&A[k][j], va);
			if (j + 8 > N) {//处理末尾
				while (j < N) {
					A[k][j] /= A[k][k];
					j++;
				}
				break;
			}
		}
		A[k][k] = 1.0;
        #pragma omp for
		for (int i = k + 1; i < N; i++) {
			__m128 vaik = _mm_loadu_ps(&A[i][k]);
			for (int j = k + 1; j + 4 <= N; j += 4) {
				__m128 vakj = _mm_loadu_ps(&A[k][j]);
				__m128 vaij = _mm_loadu_ps(&A[i][j]);
				__m128 vx = _mm_mul_ps(vakj, vaik);
				vaij = _mm_sub_ps(vaij, vx);
				_mm_storeu_ps(&A[i][j], vaij);
				if (j + 8 > N) {//处理末尾
					while (j < N) {
						A[i][j] -= A[i][k] * A[k][j];
						j++;
					}
					break;
				}
			}
			A[i][k] = 0;
		}
    }
}

int main() {
	float** A;
	float** B;
	int N = 1280;
	A = new float* [N];
	for (int i = 0; i < N; i++) {
		A[i] = new float[N];//申请空间;
	}
	B = new float* [N];
	for (int i = 0; i < N; i++) {
		B[i] = new float[N];//申请空间;
	}
	int step = 64;
	int counter1;
	int counter2;
	struct timeval start1;
	struct timeval end1;
	struct timeval start2;
	struct timeval end2;
	cout.flags(ios::left);

	for (int i = step; i <= N; i += step) {
		//串行算法
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

		//并行算法:
		generateSample(B, i);
		counter2 = 0;
		gettimeofday(&start2, NULL);
		gettimeofday(&end2, NULL);
		while ((end2.tv_sec - start2.tv_sec) < 1) {
			counter2++;
			//serialSolution(A, i);
			parallelSolution(B, i);
			gettimeofday(&end2, NULL);
		}

		//用时统计:
		float time1 = (end1.tv_sec - start1.tv_sec) + float((end1.tv_usec - start1.tv_usec)) / 1000000;//单位s;
		float time2 = (end2.tv_sec - start2.tv_sec) + float((end2.tv_usec - start2.tv_usec)) / 1000000;//单位s;


		cout << fixed << setprecision(6);
        cout << setw(14) << "数组规模" <<  i << ": " << "单线程平均用时：" << setw(20) << time1 / counter1 << endl;
        cout << setw(14) << "  " << "多线程+SIMD平均用时：" << setw(20) << time2 / counter2 << endl;
        cout << endl;
	}
	return 0;
}
