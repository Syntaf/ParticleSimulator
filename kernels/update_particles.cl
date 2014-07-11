__kernel void update_particles(global float4* pos,
							   global float3* speed,
							   global uchar4* col,
							   global float3* specs,
							   constant float*  mousedata)
{
	size_t tid = get_global_id(0);

	float DRAG = 20.0f;
	
	// split input data
	float mouse_x 		= mousedata[0];
	float mouse_y 		= mousedata[1];
	float mouse_z 		= mousedata[2];
	float mouse_pressed = mousedata[3];
	float delta 		= mousedata[4];
	
	// specs are float3: {mass, life, cameradistance}
	float3 myspecs 		= specs[tid];
	float3 myspeed 		= speed[tid];
	float mass = myspecs.x;
	
	// subtract delta from life
	float3 deltavec = {0.0f, delta, 0.0f};
	myspecs -= deltavec;
	
	// get old position
	float4 oldpos4 = pos[tid];
	float3 oldpos = {oldpos4.x, oldpos4.y, oldpos4.z};
	
	// calculate force from mouse
	float3 mousevec = {-mouse_x * 500.0f, -mouse_y * 500.0f, -70.0f};
	mousevec -= oldpos;
	float3 mousedistvec = {mouse_x, mouse_y, -70.0f};
	float mousedist = distance(mousedistvec, oldpos) + 10.0f;
	float3 force = mousevec * mouse_pressed * 50000.0f / (mousedist * mousedist);
	
	// calculate force from drag
	force -= myspeed * DRAG;
	
	float3 newpos = oldpos + myspeed * delta + 0.5f * force / mass * delta * delta;
	myspeed = (newpos - oldpos) / delta;

	float4 newpos4 = {newpos.x, newpos.y, newpos.z, oldpos4.w};
	
	float2 speed2 = {myspeed.x, myspeed.y};
	float normspeed = length(speed2);
	
	uchar4 newcol = {120, (uchar)(clamp(200.0f-normspeed*20.0f, 5.0f, 255.0f)), 10, 255};

	col[tid] = newcol;
	pos[tid] = newpos4;
	speed[tid] = myspeed;
	
}
