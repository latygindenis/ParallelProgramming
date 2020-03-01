#include <iostream>
#include <fstream>
#include <time.h>
#include <cuda.h>
#include <cuda_runtime.h>
#include "device_launch_parameters.h"
#include <omp.h>
#include <stdlib.h>
#include <stdio.h>

using namespace std;

/*
40. Найти минимальное число, которое может быть разложено в сумму простых чисел не менее, чем N различными способами (например, для числа 10 существует ровно 5 таких способов: 10 = 7+3 = 5+5 = 5+3+2 = 3+3+2+2 = 2+2+2+2+2)

Как собрать: nvcc -o lab3 -g lab3.cu -Xcompiler "-fopenmp"
*/

__device__ void pullParallel1(int sum, int num, int index, int br, bool stop, bool *mass, int *cnt)
{
	if (!stop)
	{
		sum += num;
		if (sum == index)
		{
			cnt[blockIdx.x * blockDim.x + threadIdx.x] += 1;
			br = true;
			return;
		}
	}
	else
	{
		stop = 0;
		cnt[blockIdx.x * blockDim.x + threadIdx.x] = 0;
		if (sum == index)
			cnt[blockIdx.x * blockDim.x + threadIdx.x] += 1;
	}
	for (int i = num; i + sum <= index; i == 2 ? i++ : i += 2)
		if (mass[i] && i != index)
		{
			pullParallel1(sum, i, index, br, stop, mass, cnt);
			if (br)
				break;
		}
}

void pull(int sum, int num, int index, int br, bool stop, bool *mass, int &cnt)
{
	if (!stop)
	{
		sum += num;
		if (sum == index)
		{
			cnt += 1;
			br = true;
			return;
		}
	}
	else
	{
		stop = 0;
		if (sum == index){
			cnt += 1;
		}
	}
	for (int i = num; i + sum <= index; i == 2 ? i++ : i += 2)
		if (mass[i] && i != index)
		{
			pull(sum, i, index, br, stop, mass, cnt);
			if (br) {
				break;
			}
		}
}

__global__ void start(bool *mass, int i, int *cnt)
{
	pullParallel1(2 * (blockIdx.x * blockDim.x + threadIdx.x) + 2, 3, i, 0, 1, mass, cnt);
}

int sum(int *m, int n)
{
	int s = 0;
	for (int i = 0; i < n; i++)
		s += m[i];
	return s;
}

int searchParallel(bool *mass, int N, int size)
{
	int i = 4, decomp = 0;
	int c = 0;
	bool *mass1;
	cudaMalloc((void **)&mass1, size * sizeof(bool));
	cudaMemcpy(mass1, mass, size * sizeof(bool), cudaMemcpyHostToDevice);
	int *cnt;
	int *count = new int[i / 2];

	while (decomp < N)
	{
		decomp = 0;
		cudaMalloc((void **)&cnt, i / 2 * sizeof(int));
		int bl = 1;
		int N1 = i / 2;
		while (N1 > i / 2)
		{
			if (N1 % 2)
				break;
			bl *= 2;
			N1 /= 2;
		}
		start<<<bl, N1>>>(mass1, i, cnt);
		delete[] count;
		count = new int[i / 2];
		cudaMemcpy(count, cnt, i / 2 * sizeof(int), cudaMemcpyDeviceToHost);
		decomp += sum(count, i / 2);
		cudaFree(cnt);
		for (int j = 3; j <= i - 2; j += 2)
			if (mass[j])
				pull(0, j, i, 0, 0, mass, c);
		decomp += c;
		i++;
		c = 0;
	}
	return (i - 1);
}

int search(bool *mass, int N)
{
	int i = 4, decomp = 0, c = 0;
	while (decomp < N)
	{
		decomp = 0;
		for (int k = 2; k <= i; k += 2)
			pull(k, 3, i, 0, 1, mass, c);
		for (int j = 3; j <= i - 2; j += 2)
			if (mass[j])
				pull(0, j, i, 0, 0, mass, c);
		decomp += c;
		c = 0;
		i++;
	}
	return (i - 1);
}

int main()
{
	setlocale(LC_CTYPE, "russian");
	int N, result, result1;
	double time, time1;
	bool *mass1, *mass;
	cout << "Введите число N: ";
	cin >> N;

	int size = log(double(N)) * 10;
	mass1 = new bool[size];
	mass = new bool[size];
	for (int i = 0; i < size; i++)
		mass[i] = mass1[i] = 1;

	for (int i = 2; i < size; i++)
		if (mass1[i])
			for (int j = i * i; j < size; j += i)
				mass[j] = mass1[j] = false;

	cout << "Решение в последовательном варианте." << endl;
	time = omp_get_wtime();
	result = search(mass, N);
	time = omp_get_wtime() - time;
	cout << "Число: " << result << "." << endl;
	cout << "Затраченное время: " << time << "." << endl;
	cout << "Решение в параллельном варианте." << endl;
	time1 = omp_get_wtime();
	result1 = searchParallel(mass1, N, size);
	time1 = omp_get_wtime() - time1;
	cout << "Затраченное время: " << time1 << "." << endl;
	cout << "Число: " << result << "." << endl;
	delete[] mass1;
	return 0;
}
