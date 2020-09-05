#include<pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>  
#include <sys/stat.h>  
#include <fcntl.h> 
#include <unistd.h>
#include <omp.h>
#include <math.h>
#include <string.h>
#include <sys/time.h>

#define MICRO_IN_SEC 1000000.00

#define TEMP_PATH "/sys/class/thermal/thermal_zone0/temp"  
#define CPU4_PATH "/sys/devices/system/cpu/cpufreq/policy4/cpuinfo_cur_freq"  
#define CPU0_PATH "/sys/devices/system/cpu/cpufreq/policy0/cpuinfo_cur_freq"  

void *thread_term(void *arg);
void *thread_cpu(void *arg);


double microtime();
int get_ocl_string(const char *file_name, char *ocl_string);

double timecpu;

int iscpu = 0, isocl = 0;
int main()
{
	pthread_t tid_term;
	char* p_term = NULL;
	pthread_create(&tid_term, NULL, thread_term, NULL);

	pthread_t tid_cpu;
	char* p_cpu = NULL;
	pthread_create(&tid_cpu, NULL, thread_cpu, NULL);


	sleep(1);

	pthread_join(tid_term, (void **)&p_term);
	pthread_join(tid_cpu, (void **)&p_cpu);

	return 0;
}

void *thread_term(void *arg)
{
	int fd;
	while (1)
	{
		fd = open(TEMP_PATH, O_RDONLY);
		char buf[20];
		read(fd, buf, 20);
		double temp;
		temp = atoi(buf) / 1000.0;
		printf("temperature: %.1lf\n",temp);
		close(fd);

		system("cat /sys/devices/system/cpu/online");

		fd = open(CPU0_PATH, O_RDONLY);
		char buf1[20];
		read(fd, buf1, 20);
		temp = atoi(buf1) / 1000000.0;
		printf("A53 Freq: %.2lf\n", temp);
		close(fd);

		fd = open(CPU4_PATH, O_RDONLY);
		char buf2[20];
		read(fd, buf2, 20);
		temp = atoi(buf2) / 1000000.0;
		printf("A72 Freq: %.2lf\n", temp);
		close(fd);
		printf("\n");

		if (iscpu == 1)
		{
			printf("cpu used = %lf s\n", timecpu);
			iscpu = 0;
		}
				sleep(5);
	}

}
void *thread_cpu(void *arg)
{
	while (1)
	{
		double s = 1;
		double pi = 0;
		double i = 1.0;
		double n = 1.0;
		double dt;
		double start_time;
		int cnt = 0;
		start_time = microtime();
#pragma omp parallel for num_threads(6)
		for (cnt = 0; cnt<100000000; cnt++)
		{
			pi += i;
			n = n + 2;
			s = -s;
			i = s / n;
		}
		pi = 4 * pi;
		dt = microtime() - start_time;
		timecpu = dt;
		iscpu = 1;
	}
}


double microtime() {
	int tv_sec, tv_usec;
	double time;
	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv, &tz);
	return tv.tv_sec + tv.tv_usec / MICRO_IN_SEC;
}
