#pragma OPENCL EXTENSION cl_khr_int64_base_atomics: enable

float xy_cross(float *v0, float *v1, float *p)
{
	return (v1[0] - v0[0])*(p[1] - v0[1]) - (v1[1] - v0[1])*(p[0] - v0[0]);
}

int is_inside(float *pos, float *tri)
{
	//v1
	float sign[3];
	sign[0] = xy_cross(&tri[0],&tri[3],&pos[0]);
	sign[1] = xy_cross(&tri[3],&tri[6],&pos[0]);
	sign[2] = xy_cross(&tri[6],&tri[0],&pos[0]);

	if(sign[0] > 0 && sign[1] > 0 && sign[2] > 0){
		return 1; //plus z
	}else if (sign[0] < 0 && sign[1] < 0 && sign[2] < 0){
		return 2; //minus z
	}
	else{
		return 0; //not inside
	}
}

__kernel void propagate(__global float *pos, __global float *vec,__global float *norm, int tri_num, __global float *z){
	//printf("addr: %d\n", (int)get_global_id(0));
	//printf("num_pos: %d\n", (int)get_global_size(0));
	int addr = get_global_id(0);
	int pos_num = get_global_size(0);
	__private float p[2] = {pos[2*addr], pos[2*addr+1]};
	__private float v[9];

	for(int i=0;i<tri_num;i++)
	{
		for(int j=0;j<9;j++)
		{
			v[j] = vec[9*i+j];
		}
		if(is_inside(&p[0],&v[0]) == 1){
			z[addr] =  v[2] - (norm[i*3]*(p[0]-v[0]) + norm[i*3+1]*(p[1]-v[1]))/norm[i*3+2];	
			return;
		}
	}
}
