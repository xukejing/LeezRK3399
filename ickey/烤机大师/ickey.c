#include<pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>  
#include <sys/stat.h>  
#include <fcntl.h> 
#include <unistd.h>
#include <omp.h>
#include <math.h>
#include <CL/cl.h>
#include <string.h>
#include <sys/time.h>

#define MICRO_IN_SEC 1000000.00

#define TEMP_PATH "/sys/class/thermal/thermal_zone0/temp"  
#define CPU4_PATH "/sys/devices/system/cpu/cpufreq/policy4/cpuinfo_cur_freq"  
#define CPU0_PATH "/sys/devices/system/cpu/cpufreq/policy0/cpuinfo_cur_freq"  

void *thread_term(void *arg);
void *thread_cpu(void *arg);
void *thread_ocl(void *arg);

double microtime();
int get_ocl_string(const char *file_name, char *ocl_string);

double timecpu;
double timeocl;
int iscpu = 0, isocl = 0;
int main()
{
	pthread_t tid_term;
	char* p_term = NULL;
	pthread_create(&tid_term, NULL, thread_term, NULL);

	pthread_t tid_cpu;
	char* p_cpu = NULL;
	pthread_create(&tid_cpu, NULL, thread_cpu, NULL);

	pthread_t tid_ocl;
	char* p_ocl = NULL;
	pthread_create(&tid_ocl, NULL, thread_ocl, NULL);

	sleep(1);

	pthread_join(tid_term, (void **)&p_term);
	pthread_join(tid_cpu, (void **)&p_cpu);
	pthread_join(tid_ocl, (void **)&p_ocl);;
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
		if (isocl == 1)
		{
			printf("ocl used = %lf s\n", timeocl);
			isocl = 0;
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

void *thread_ocl(void *arg)
{
	int array_a[20] = { 0, 1, 8, 7, 6, 2, 3, 5, 4, 9,17,19,15,10,18,16,14,13,12,11 };
	int array_b[20] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0,0 };
	size_t datasize = 20 * sizeof(int);
	size_t ocl_string_size;
	char *ocl_string;
	double start_time, dt, dt_err;
	start_time = microtime();
	dt_err = microtime() - start_time;

	ocl_string = (char *)malloc(400);
	//ocl_string = (char *)malloc(20);

	cl_platform_id platform_id;
	cl_device_id device_id;
	cl_context context;
	cl_command_queue command_queue;
	cl_mem buffer_a, buffer_b;
	cl_program program;
	cl_kernel kernel;
	cl_event kernelEvent;

	clGetPlatformIDs(1, &platform_id, NULL);
	clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);

	context = clCreateContext(NULL, 1, &device_id, NULL, NULL, NULL);
	command_queue = clCreateCommandQueue(context, device_id, 0, NULL);

	buffer_a = clCreateBuffer(context, CL_MEM_READ_ONLY, datasize, NULL, NULL);
	buffer_b = clCreateBuffer(context, CL_MEM_READ_ONLY, datasize, NULL, NULL);


	ocl_string_size = get_ocl_string("test.cl", ocl_string);
	clEnqueueWriteBuffer(command_queue, buffer_a, CL_FALSE, 0, \
		datasize, array_a, 0, NULL, NULL);
	clEnqueueWriteBuffer(command_queue, buffer_b, CL_FALSE, 0, \
		datasize, array_b, 0, NULL, NULL);
	program = clCreateProgramWithSource(context, 1, (const char **)&ocl_string, \
		&ocl_string_size, NULL);

	clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
	kernel = clCreateKernel(program, "test", NULL);

	clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer_a);
	clSetKernelArg(kernel, 1, sizeof(cl_mem), &buffer_b);


	size_t global_work_size[1] = { 20 };
	while (1)
	{
		start_time = microtime();
		clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, \
			global_work_size, NULL, 0, NULL, &kernelEvent);
		clWaitForEvents(1, &kernelEvent);
		clEnqueueReadBuffer(command_queue, buffer_b, CL_TRUE, 0, \
			datasize, array_b, 0, NULL, NULL);
		dt = microtime() - start_time - dt_err;
		timeocl = dt;
		isocl = 1;
	}

	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseCommandQueue(command_queue);
	clReleaseMemObject(buffer_a);
	clReleaseMemObject(buffer_b);
	clReleaseContext(context);
	free(ocl_string);
}

double microtime() {
	int tv_sec, tv_usec;
	double time;
	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv, &tz);
	return tv.tv_sec + tv.tv_usec / MICRO_IN_SEC;
}

int get_ocl_string(const char *file_name, char *ocl_string)
{
	FILE *fp;
	int file_length;
	int status = 0;

	fp = fopen(file_name, "r");
	if (fp == NULL)
		return -1;

	fseek(fp, 0, SEEK_END);
	file_length = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	status = fread(ocl_string, 1, file_length, fp);
	if (status == -1)
		return -1;

	return file_length;
}