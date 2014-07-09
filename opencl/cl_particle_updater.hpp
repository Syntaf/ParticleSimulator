#ifndef CL_PARTICLE_UPDATER_HPP_
#define CL_PARTICLE_UPDATER_HPP_


#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <CL/opencl.h>

#include "../Particle.hpp"


class cl_particle_updater
{

public:
	/////////////////////////////////////
	// public functions
	
	// constructor
	cl_particle_updater(HGLRC glContext, HDC glDC, size_t particlecount, GLuint posBuffer = (GLuint)0, GLuint colBuffer = (GLuint)0);
	
	// destructor
	~cl_particle_updater();
	
	// the main calculation query
	void update(glm::vec4 mousepos, bool mouse_pressed, float delta, size_t num_particles);

	// the initialization
	void set_particle_values(std::vector<Particle> & particles);

	// the read, if necessary.
	void read_pos_and_col(GLfloat* pos_data, GLubyte* col_data, size_t num_particles);

	// map und unmap opengl buffers. needs to be called before update(), set_particle_values() and read_pos_and_col().
	void lock_gl_buffers();
	void unlock_gl_buffers();

private:
	/////////////////////////////////////
	// private functions


private:
	/////////////////////////////////////
	// private attributes

	cl_context context;
	cl_device_id device;
	cl_command_queue command_queue;
	bool use_gl_buffers;
	
	size_t max_num_particles;
	typedef cl_float4 pos_buffer_type;
	typedef cl_float3 speed_buffer_type;
	typedef cl_uchar4 col_buffer_type;
	typedef cl_float3 specs_buffer_type;
	typedef cl_float  mouse_buffer_type;



	cl_mem pos_buffer;		// Holds positions and size. Can be GL buffer.
	cl_mem speed_buffer;	// Holds the velocity vectors
	cl_mem col_buffer;		// Holds color. Can be GL buffer.
	cl_mem specs_buffer;	// Holds mass, life, cameradistance

	cl_mem mouse_buffer;	// Holds mousepos and wether or not the mous is pressed and the delta time

	cl_kernel calculation_kernel;	// the actual calculation

};


#endif