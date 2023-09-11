#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "conformal.h"

#define PROGRAM_FILE "conformal_ocl.cl"

#define PLATFORM 1
#define DEVICE 0

int main(int argc, char *argv[])
{

	char input_stl[] = "data/input.stl";
	char input_pos[] = "data/points.txt";
	char output_pos[] = "data/output.txt";

	clock_t start, end;
	double used;
	list_devices();
	float *n;
	float *v;
	uint32_t num_tri = read_stl_vertex(&v,&n,input_stl);

	float *p;
	int num_p = read_points(&p,input_pos);

	float z[num_p];

	printf("[INPUT_DATA]:\n");
	printf("\tNumber Triangles: %d\n",num_tri);
	printf("\tNumber points: %d\n",num_p);

	start = clock();
	conformal_ocl(1,0,num_tri, n, v, num_p, p, &z[0], PROGRAM_FILE);
	end = clock();
	used = ((double)(end - start)/CLOCKS_PER_SEC);

	printf("[RESULT]:\n");
	printf("\tCal. time: %f secs\n", used);
	printf("\tOutput file: %s\n",output_pos);

	FILE *f = fopen(output_pos,"w");
	for(int i=0;i<num_p;i++)
	{
		fprintf(f,"%f %f %f\n",p[i*2],p[i*2+1],z[i]);
	}
	fclose(f);
	return 0;
}
