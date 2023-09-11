#ifndef LBMCL
#define LBMCL

#define CL_TARGET_OPENCL_VERSION 300

#ifdef MAC
#include<OpenCL/cl.h>
#else
#include<CL/cl.h>
#endif

#include<stdio.h>

int read_points(float **p, char *input);
uint32_t read_stl_vertex(float **v, float **n, char *input_stl);
void conformal_ocl(int selected_platform, int selected_device,uint32_t numTri, float *n, float *v, uint32_t numPos, float *p, float *z, char *programFileName);
void list_devices();
void print_device_name(FILE *f,cl_device_id device);
void print_kernel_info(FILE *f,cl_kernel kernel, cl_device_id device);
cl_device_id create_device(int sel_plat, int sel_dev);

cl_program build_program(cl_context ctx, cl_device_id dev, const char* filename);

const char *getErrorString(cl_int error);

void check_err(int err, char *str);
#endif
