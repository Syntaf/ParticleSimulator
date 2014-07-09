__kernel void update_particles(global float4* pos,
							   global float3* speed,
							   global uchar4* col,
							   global float3* specs,
							   constant float*  mousedata)
{
	// split input data
	float mouse_x 		= mousedata[0];
	float mouse_y 		= mousedata[1];
	float mouse_z 		= mousedata[2];
	bool mouse_pressed  = (mousedata[3] > 0);
	float delta 		= mousedata[4];
	
	size_t tid = get_global_id(0);
	
	// example:
	uchar4 col_dif = {2,3,1,0};
	col[tid] = col[tid] + col_dif; // constantly increases color, will flip to 0 on overflow again

}