/*
42. Найти максимальное число, меньшее заданного,
которое может быть представлено как сумма степеней 2, 3 и 4 простых чисел
(минимальное такое число есть 28 = 2^2+2^3+2^4)

Как собрать: mpicc -o lab2 lab2.c
Как запускать: mpirun -np 'Количество потоков' ./lab2 'Заданное число'
*/

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("%s\n", "Неправильное количество аргументов" );
	} else {
		int rank = 0, size;

		MPI_Init(&argc, &argv);
		MPI_Comm_size(MPI_COMM_WORLD, &size);
		MPI_Comm_rank(MPI_COMM_WORLD, &rank);

		char str[10];
		snprintf(str, sizeof(str), "%d", rank);

		double start, stop;
		int N;
		int *primes;
		int maxSimpleIndex;
		int maxNumber = 0;
		int im = 0, jm = 0, km = 0;

		if (rank == 0) {
			N = atoi(argv[1]);
			FILE *file = fopen("primes1.txt", "r");
			primes = (int*)malloc(N / 4 * sizeof(int));

			for (int i = 0; i < N / 4; i++) {
				fscanf(file, "%d", &primes[i]);
			}
			fclose(file);

			start = MPI_Wtime();

			for (maxSimpleIndex = 0; primes[maxSimpleIndex] * primes[maxSimpleIndex] <= N; maxSimpleIndex++) {}

			for (int i = 0; i < maxSimpleIndex; i++) {
				for (int j = 0; j < maxSimpleIndex; j++ ) {
					for (int k = 0; k < maxSimpleIndex ; k++) {
						int buf = primes[i] * primes[i]
						          + primes[j] * primes[j] * primes[j]
						          + primes[k] * primes[k] * primes[k] * primes[k];

						if (buf > maxNumber && buf < N) {
							maxNumber = buf;
							im = i;
							jm = j;
							km = k;
						}
					}

				}
			}
			stop = MPI_Wtime();

			printf("1 процесс %s%d = %d%s%d%s%d%s\n", "максимальное число: ", maxNumber, primes[im], "^2 + ", primes[jm], "^3 + ", primes[km], "^4" );
			printf("%f секунд\n", stop - start);
		}

		maxNumber = 0;
		MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&maxSimpleIndex, 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&maxNumber, 1, MPI_INT, 0, MPI_COMM_WORLD);

		if (rank != 0) {
			primes = (int*)malloc(N / 4 * sizeof(int));
		}

		if (rank == 0)
			for (int i = 1; i < size; i++) {
				MPI_Send(primes, N / 4, MPI_INT, i, 0, MPI_COMM_WORLD);
			}
		else
		{
			MPI_Status status;
			MPI_Recv(primes, N / 4, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		}

		start = MPI_Wtime();

		for (int i = 0; i < maxSimpleIndex; i += 1) {
			for (int j = 0; j < maxSimpleIndex; j += 1) {
				for (int k = rank; k < maxSimpleIndex ; k += size) {
					int buf = primes[i] * primes[i]
					          + primes[j] * primes[j] * primes[j]
					          + primes[k] * primes[k] * primes[k] * primes[k];

					if (buf > maxNumber && buf < N) {
						maxNumber = buf;
					}
				}
			}
		}

		int res;
		MPI_Reduce(&maxNumber, &res, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);

		if (rank == 0) {
			stop = MPI_Wtime();
			printf("%d процесс(ов), максимальное число: %d\n", size, res);
			printf("%f секунд\n", stop - start);
		}
	}

	MPI_Finalize();
	return 0;
}
