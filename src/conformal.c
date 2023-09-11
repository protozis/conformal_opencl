#define KERNEL_FUNC "propagate"
#define CL_TARGET_OPENCL_VERSION 300
#define PI 3.1415926

#ifdef MAC
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#include "conformal.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

//Default

void conformal_ocl(int selected_platform, int selected_device,uint32_t numTri, float *n, float *v, uint32_t numPos, float *p, float *z, char *programFileName)
{

	cl_device_id device;
	cl_context context;
	cl_program program;
	cl_kernel kernel;
	cl_command_queue queue;
	cl_int err;
	cl_mem vec_buffer;
	cl_mem pos_buffer;
	cl_mem norm_buffer;
	cl_mem z_buffer;

	device = create_device(selected_platform, selected_device);

	const size_t global_size = numPos;
	const size_t local_size = 1;

	
	fprintf(stdout, "\n[Selected device]: ");
	print_device_name(stdout, device);
	fprintf(stdout, "\n");
	
	context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
	check_err(err, "Couldn't create a context");
	program = build_program(context, device, programFileName);

	queue = clCreateCommandQueueWithProperties(context, device, 0, &err);
	check_err(err, "Couldn't create a command queue");

	kernel = clCreateKernel(program, KERNEL_FUNC, &err);
	check_err(err, "Couldn't create a kernel");

	print_kernel_info(stdout, kernel, device);

	pos_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(float) * 2 * numPos, &p[0], &err);
	z_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(float) * numPos, &z[0], &err);
	vec_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(float) * 9 * numTri, &v[0], &err);
	norm_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(float) * 3 * numTri, &n[0], &err);
	check_err(err, "Couldn't create a buffer");

	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &pos_buffer);
	err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &vec_buffer);
	err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &norm_buffer);
	err |= clSetKernelArg(kernel, 3, sizeof(cl_int), &numTri);
	err |= clSetKernelArg(kernel, 4, sizeof(cl_mem), &z_buffer);
	

	check_err(err, "Couldn't create a kernel argument");
	err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);
	check_err(err, "Couldn't enqueue the kernel");
	err = clEnqueueReadBuffer(queue, z_buffer, CL_TRUE, 0, numPos * sizeof(float), &z[0], 0, NULL, NULL);
	check_err(err, "Couldn't read buffers");

	clReleaseMemObject(vec_buffer);
	clReleaseMemObject(norm_buffer);
	clReleaseMemObject(pos_buffer);
	clReleaseMemObject(z_buffer);

	clReleaseKernel(kernel);
	clReleaseCommandQueue(queue);
	clReleaseProgram(program);
	clReleaseContext(context);
}

void print_device_name(FILE *f, cl_device_id device)
{
	char *value;
	size_t valueSize;
	// print device name
	clGetDeviceInfo(device, CL_DEVICE_NAME, 0, NULL, &valueSize);
	value = (char *)malloc(valueSize);
	clGetDeviceInfo(device, CL_DEVICE_NAME, valueSize, value, NULL);
	fprintf(f, "%s", value);
	free(value);
}
void print_kernel_info(FILE *f, cl_kernel kernel, cl_device_id device)
{
	size_t kernel_value, err;
	fprintf(f, "[Kernel info]:\n");
	err = clGetKernelWorkGroupInfo(kernel, device, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), NULL, &kernel_value);
	check_err(err, "Couldn't check kernel value");
	fprintf(f, "\tCL_KERNEL_WORK_GROUP_SIZE: %d\n", kernel_value);
}

void list_devices()
{

	char *value;
	size_t valueSize;
	cl_int platform_count;
	cl_platform_id *platforms;
	cl_int device_count;
	cl_device_id *devices;
	cl_device_id res_dev;
	cl_int max_compute_units;
	int err;
	size_t *val;

	/* Identify all platform */
	err = clGetPlatformIDs(0, NULL, &platform_count);
	platforms = (cl_platform_id *)malloc(sizeof(cl_platform_id) * platform_count);
	clGetPlatformIDs(platform_count, platforms, NULL);
	check_err(err, "Couldn't identigy a platform");

	printf("[Opencl devices info]:\n");
	for (int i = 0; i < platform_count; i++)
	{
		clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, NULL, &device_count);
		devices = (cl_device_id *)malloc(sizeof(cl_device_id) * device_count);
		clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, device_count, devices, NULL);
		for (int j = 0; j < device_count; j++)
		{
			printf("\t");
			print_device_name(stdout, devices[j]);
			printf("\n");
			// print device CL_DEVICE_MAX_WORK_ITEM_SIZES
			clGetDeviceInfo(devices[j], CL_DEVICE_MAX_WORK_ITEM_SIZES, 0, NULL, &valueSize);
			val = (size_t *)malloc(valueSize);
			clGetDeviceInfo(devices[j], CL_DEVICE_MAX_WORK_ITEM_SIZES, valueSize, val, NULL);
			printf("\t\tCL_DEVICE_MAX_WORK_ITEM_SIZES:  %d/%d/%d\n", val[0], val[1], val[2]);
			free(val);
			// print device CL_DEVICE_MAX_WORK_GROUP_SIZE
			clGetDeviceInfo(devices[j], CL_DEVICE_MAX_WORK_GROUP_SIZE, 0, NULL, &valueSize);
			val = (size_t *)malloc(valueSize);
			clGetDeviceInfo(devices[j], CL_DEVICE_MAX_WORK_GROUP_SIZE, valueSize, &val[0], NULL);
			printf("\t\tCL_DEVICE_MAX_WORK_GROUP_SIZE:  %d\n", val[0]);
			free(val);
			// print device CL_DEVICE_MAX_COMPUTE_UNITS
			clGetDeviceInfo(devices[j], CL_DEVICE_MAX_COMPUTE_UNITS, 0, NULL, &valueSize);
			val = (size_t *)malloc(valueSize);
			clGetDeviceInfo(devices[j], CL_DEVICE_MAX_COMPUTE_UNITS, valueSize, &val[0], NULL);
			printf("\t\tCL_DEVICE_MAX_COMPUTE_UNITS:  %d\n", val[0]);
		}
	}
}

int read_points(float **p, char *input)
{
	FILE *f = fopen(input,"r");
	if (f == NULL)
	{
		perror("Error opening file");
		return 0;
	}
	int num_p;
	float x,y;
	fscanf(f,"%d",&num_p);
	*p = (float *)malloc(sizeof(float)*2*num_p);
	for(int i=0;i<num_p;i++){
		fscanf(f,"%f %f\n",&(*p)[i*2],&(*p)[i*2+1]);	
	}
	return num_p;
}

uint32_t read_stl_vertex(float **v, float **n, char *input_stl)
{
	FILE *stlFile = fopen(input_stl, "rb");
	if (stlFile == NULL)
	{
		perror("Error opening file");
		return 0;
	}
	char header[80];
	if (fread(header, sizeof(char), 80, stlFile) != 80)
	{
		perror("Error reading header");
		fclose(stlFile);
		return 0;
	}

	uint32_t numTriangles;
	if (fread(&numTriangles, sizeof(uint32_t), 1, stlFile) != 1)
	{
		perror("Error reading number of triangles");
		fclose(stlFile);
		return 0;
	}
	*v = (float *)malloc(sizeof(float) *9*numTriangles);
	*n = (float *)malloc(sizeof(float) *3*numTriangles);

	uint16_t attrByteCount;

	for (uint32_t i = 0; i < numTriangles; i++)
	{

		if (fread(&(*n)[i*3], sizeof(float), 3, stlFile) != 3)
		{
			perror("Error reading normal");
			fclose(stlFile);
			return 0;
		}
		if (fread(&(*v)[i*9], sizeof(float), 3, stlFile) != 3 ||
			fread(&(*v)[i*9+3], sizeof(float), 3, stlFile) != 3 ||
			fread(&(*v)[i*9+6], sizeof(float), 3, stlFile) != 3 ||
			fread(&attrByteCount, sizeof(uint16_t),1, stlFile) != 1)
		{
			perror("Error reading vertices");
			fclose(stlFile);
			return 0;
		}

		// Process the triangle data (e.g., store it in your data structure)
	}
	fclose(stlFile);
	return numTriangles;
}


cl_device_id create_device(int sel_plat, int sel_dev)
{

	char *value;
	size_t valueSize;
	cl_int platform_count;
	cl_platform_id *platforms;
	cl_int device_count;
	cl_device_id *devices;
	cl_device_id res_dev;
	cl_int max_compute_units;
	int err;
	size_t *val;

	/* Identify all platform */
	err = clGetPlatformIDs(0, NULL, &platform_count);
	platforms = (cl_platform_id *)malloc(sizeof(cl_platform_id) * platform_count);
	clGetPlatformIDs(platform_count, platforms, NULL);
	check_err(err, "Couldn't identigy a platform");

	if (sel_plat + 1 > platform_count)
	{
		fprintf(stderr, "selected platform not exist: p%d\n", sel_plat);
		exit(1);
	}

	for (int i = 0; i < platform_count; i++)
	{
		clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, NULL, &device_count);
		devices = (cl_device_id *)malloc(sizeof(cl_device_id) * device_count);
		clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, device_count, devices, NULL);
		for (int j = 0; j < device_count; j++)
		{
			if (i == sel_plat)
			{
				if (sel_dev + 1 > device_count)
				{
					fprintf(stderr, "selected device not exist: d%d\n", sel_dev + 1);
					exit(1);
				}
				if (j == sel_dev)
				{
					res_dev = devices[j];
				}
			}
		}
	}
	if (res_dev == NULL)
	{
		fprintf(stderr, "Selected devices (P%d D%d) not found.", sel_plat, sel_dev);
	}
	return res_dev;
}

/* Create program from a file and compile it */
cl_program build_program(cl_context ctx, cl_device_id dev, const char *filename)
{

	cl_program program;
	FILE *program_handle;
	char *program_buffer, *program_log;
	size_t program_size, log_size;
	int err;

	/* Read program file and place content into buffer */
	program_handle = fopen(filename, "r");
	if (program_handle == NULL)
	{
		perror("Couldn't find the program file");
		exit(1);
	}
	fseek(program_handle, 0, SEEK_END);
	program_size = ftell(program_handle);
	rewind(program_handle);
	program_buffer = (char *)malloc(program_size + 1);
	program_buffer[program_size] = '\0';
	fread(program_buffer, sizeof(char), program_size, program_handle);
	fclose(program_handle);

	/* Create program from file 

	   Creates a program from the source code in the add_numbers.cl file. 
	   Specifically, the code reads the file's content into a char array 
	   called program_buffer, and then calls clCreateProgramWithSource.
	   */
	program = clCreateProgramWithSource(ctx, 1,
										(const char **)&program_buffer, &program_size, &err);
	if (err < 0)
	{
		perror("Couldn't create the program");
		exit(1);
	}
	free(program_buffer);

	/* Build program 

	   The fourth parameter accepts options that configure the compilation. 
	   These are similar to the flags used by gcc. For example, you can 
	   define a macro with the option -DMACRO=VALUE and turn off optimization 
	   with -cl-opt-disable.
	   */
	err = clBuildProgram(program, 0, NULL, "-I. -I /usr/include -I /usr/lib/gcc/x86_64-pc-linux-gnu/10.2.0/include ", NULL, NULL);
	if (err < 0)
	{

		/* Find size of log and print to std output */
		clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
		program_log = (char *)malloc(log_size + 1);
		program_log[log_size] = '\0';
		clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG,
							  log_size + 1, program_log, NULL);
		printf("%s\n", program_log);
		free(program_log);
		exit(1);
	}

	return program;
}
const char *getErrorString(cl_int error)
{
	switch (error)
	{
	// run-time and JIT compiler errors
	case 0:
		return "CL_SUCCESS";
	case -1:
		return "CL_DEVICE_NOT_FOUND";
	case -2:
		return "CL_DEVICE_NOT_AVAILABLE";
	case -3:
		return "CL_COMPILER_NOT_AVAILABLE";
	case -4:
		return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
	case -5:
		return "CL_OUT_OF_RESOURCES";
	case -6:
		return "CL_OUT_OF_HOST_MEMORY";
	case -7:
		return "CL_PROFILING_INFO_NOT_AVAILABLE";
	case -8:
		return "CL_MEM_COPY_OVERLAP";
	case -9:
		return "CL_IMAGE_FORMAT_MISMATCH";
	case -10:
		return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
	case -11:
		return "CL_BUILD_PROGRAM_FAILURE";
	case -12:
		return "CL_MAP_FAILURE";
	case -13:
		return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
	case -14:
		return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
	case -15:
		return "CL_COMPILE_PROGRAM_FAILURE";
	case -16:
		return "CL_LINKER_NOT_AVAILABLE";
	case -17:
		return "CL_LINK_PROGRAM_FAILURE";
	case -18:
		return "CL_DEVICE_PARTITION_FAILED";
	case -19:
		return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";

		// compile-time errors
	case -30:
		return "CL_INVALID_VALUE";
	case -31:
		return "CL_INVALID_DEVICE_TYPE";
	case -32:
		return "CL_INVALID_PLATFORM";
	case -33:
		return "CL_INVALID_DEVICE";
	case -34:
		return "CL_INVALID_CONTEXT";
	case -35:
		return "CL_INVALID_QUEUE_PROPERTIES";
	case -36:
		return "CL_INVALID_COMMAND_QUEUE";
	case -37:
		return "CL_INVALID_HOST_PTR";
	case -38:
		return "CL_INVALID_MEM_OBJECT";
	case -39:
		return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
	case -40:
		return "CL_INVALID_IMAGE_SIZE";
	case -41:
		return "CL_INVALID_SAMPLER";
	case -42:
		return "CL_INVALID_BINARY";
	case -43:
		return "CL_INVALID_BUILD_OPTIONS";
	case -44:
		return "CL_INVALID_PROGRAM";
	case -45:
		return "CL_INVALID_PROGRAM_EXECUTABLE";
	case -46:
		return "CL_INVALID_KERNEL_NAME";
	case -47:
		return "CL_INVALID_KERNEL_DEFINITION";
	case -48:
		return "CL_INVALID_KERNEL";
	case -49:
		return "CL_INVALID_ARG_INDEX";
	case -50:
		return "CL_INVALID_ARG_VALUE";
	case -51:
		return "CL_INVALID_ARG_SIZE";
	case -52:
		return "CL_INVALID_KERNEL_ARGS";
	case -53:
		return "CL_INVALID_WORK_DIMENSION";
	case -54:
		return "CL_INVALID_WORK_GROUP_SIZE";
	case -55:
		return "CL_INVALID_WORK_ITEM_SIZE";
	case -56:
		return "CL_INVALID_GLOBAL_OFFSET";
	case -57:
		return "CL_INVALID_EVENT_WAIT_LIST";
	case -58:
		return "CL_INVALID_EVENT";
	case -59:
		return "CL_INVALID_OPERATION";
	case -60:
		return "CL_INVALID_GL_OBJECT";
	case -61:
		return "CL_INVALID_BUFFER_SIZE";
	case -62:
		return "CL_INVALID_MIP_LEVEL";
	case -63:
		return "CL_INVALID_GLOBAL_WORK_SIZE";
	case -64:
		return "CL_INVALID_PROPERTY";
	case -65:
		return "CL_INVALID_IMAGE_DESCRIPTOR";
	case -66:
		return "CL_INVALID_COMPILER_OPTIONS";
	case -67:
		return "CL_INVALID_LINKER_OPTIONS";
	case -68:
		return "CL_INVALID_DEVICE_PARTITION_COUNT";

		// extension errors
	case -1000:
		return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
	case -1001:
		return "CL_PLATFORM_NOT_FOUND_KHR";
	case -1002:
		return "CL_INVALID_D3D10_DEVICE_KHR";
	case -1003:
		return "CL_INVALID_D3D10_RESOURCE_KHR";
	case -1004:
		return "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR";
	case -1005:
		return "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR";
	default:
		return "Unknown OpenCL error";
	}
}
void check_err(int err, char *str)
{
	if (err < 0)
	{
		fprintf(stderr, "%s: %s\n", str, getErrorString(err));
		exit(1);
	}
}
