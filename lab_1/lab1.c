#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

/*
58. Найти в заданном диапазоне от N до M максимальное число, имеющее наибольшее количество различных простых делителей
Как собирать: gcc -o lab1 lab1.c -fopenmp
*/


void sequential(int N, int M, int *primes) {
	int maxDelitCount = 1;
	int maxNumber = N;

	for (int currentNumber = N; currentNumber <= M; ++currentNumber) {
		int _maxDelitCount = 1;
		for (int i = 0; primes[i] * 2 < currentNumber; ++i) {

			if (currentNumber % primes[i] == 0) {
				_maxDelitCount ++;
			}
		}

		if (_maxDelitCount >= maxDelitCount && currentNumber >= maxNumber) {
			maxDelitCount = _maxDelitCount;
			maxNumber = currentNumber;
		}
	}

	printf("%s %d\n%s %d\n", "Число:", maxNumber, "кол-во делителей:", maxDelitCount);
}

void parrallel(int N, int M, int *primes) {
	int maxDelitCount = 1;
	int maxNumber = N;

	#pragma omp parallel for
	for (int currentNumber = N; currentNumber <= M; ++currentNumber) {
		int _maxDelitCount = 1;

		for (int i = 0; primes[i] * 2 < currentNumber; ++i) {
			if (currentNumber % primes[i] == 0) {
				_maxDelitCount ++;
			}
		}

		if (_maxDelitCount >= maxDelitCount && currentNumber >= maxNumber) {
			maxDelitCount = _maxDelitCount;
			maxNumber = currentNumber;
		}
	}

	printf("%s %d\n%s %d\n", "Число:", maxNumber, "кол-во делителей:", maxDelitCount);
}


int main(int argc, char const *argv[])
{   
	if (argc != 3) {
		printf("%s\n", "Недостаточно параметров" );
	} else {

		double start, stop;
		int N, M;
		N = atoi(argv[1]);
		M = atoi(argv[2]);

		FILE *file;
		file = fopen("primes1.txt", "r");
		int primes[M/2];
		for (int i = 0; i< M/2; i++) {
			fscanf(file, "%d", &primes[i]);
		}
		fclose(file);

		printf("\n%s\n", "Параллельно: ");
		start = omp_get_wtime();
		parrallel(N, M, primes);
		stop = omp_get_wtime();
		printf("%f секунд\n", stop - start);

		printf("\n%s\n", "Последовательно: ");
		start = omp_get_wtime();
		sequential(N, M, primes);
		stop = omp_get_wtime();
		printf("%f секунд\n", stop - start);
	}

	return 0;
}
