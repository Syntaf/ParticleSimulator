#include "cl_particle_updater.hpp"
#include "cl_tools.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <iterator>
#include <sstream>
#include <algorithm>
#include <set>
#include <fstream>
#include <streambuf>
#ifndef _WIN32
    #include <GL/glx.h>
#endif

// ensure that gl-cl-sharing extension is available
#ifndef cl_khr_gl_sharing
    #error OPENCL Error: cl_khr_gl_sharing extension is not available! Please update your OpenCL version.
#endif

static std::set<std::string> split_str(std::string props)
{
    std::istringstream iss(props);

    std::istream_iterator<std::string> begin(iss), end;

    std::set<std::string> result(begin, end);

    return result;
}

static cl_device_id find_cl_device()
{
    cl_die("Not implemented yet");

    return 0;
}

static cl_device_id find_opengl_device()
{
	cl_int err;

	// query number of platforms
	cl_uint num_platforms;
	err = clGetPlatformIDs(0, NULL, &num_platforms);
	cl_ensure(err);

	// Die if no platforms were found
	if(num_platforms < 1) cl_die("No OpenCL platform found!");

	// get platforms
	std::vector<cl_platform_id> platforms(num_platforms);
	err = clGetPlatformIDs(num_platforms, &platforms[0], NULL);
	cl_ensure(err);

	// search for the right platform
	for(std::vector<cl_platform_id>::iterator it = platforms.begin(); it != platforms.end(); it++)
	{
		cl_platform_id platform = *it;

		// get platform extension string
		size_t extension_string_size;
		err = clGetPlatformInfo(platform, CL_PLATFORM_EXTENSIONS, 0, NULL, &extension_string_size);
		std::vector<char> extension_string_vec(extension_string_size);
		err = clGetPlatformInfo(platform, CL_PLATFORM_EXTENSIONS, extension_string_size,
								&extension_string_vec[0], NULL);
		std::string extension_string(extension_string_vec.begin(), extension_string_vec.end());

		// split the extension string;
		std::set<std::string> extensions = split_str(extension_string);

		// check if cl-gl sharing is available
		if(extensions.find("cl_khr_gl_sharing") == extensions.end()) continue;

		// get clGetGLContestInfoKHR extension function pointer
		clGetGLContextInfoKHR_fn clGetGLContextInfo = (clGetGLContextInfoKHR_fn)
				clGetExtensionFunctionAddress("clGetGLContextInfoKHR");
		if(!clGetGLContextInfo) cl_die("Unable to get clGetGLContextInfo function pointer!");

		// set up properties that we use to find a device
        #ifdef _WIN32 // WINDOWS
    		cl_context_properties props[] = {
    			CL_GL_CONTEXT_KHR, (cl_context_properties) wglGetCurrentContext(),
    			CL_WGL_HDC_KHR, (cl_context_properties) wglGetCurrentDC(),
    			CL_CONTEXT_PLATFORM, (cl_context_properties) platform,
    			0
    		};
        #else // LINUX
    		cl_context_properties props[] = {
    			CL_GL_CONTEXT_KHR, (cl_context_properties) glXGetCurrentContext(),
    			CL_GLX_DISPLAY_KHR, (cl_context_properties) glXGetCurrentDisplay(),
    			CL_CONTEXT_PLATFORM, (cl_context_properties) platform,
    			0
    		};
        #endif

        std::cout << "glXGetCurrentContext: " << glXGetCurrentContext() << std::endl;
        std::cout << "glXGetCurrentDisplay: " << glXGetCurrentDisplay() << std::endl;

		// query for an openGL-connected device
		cl_device_id device;
		size_t was_successful;
		err = clGetGLContextInfo(props,
								CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR,
								sizeof(cl_device_id),
								&device,
								&was_successful);
		cl_ensure(err);
		if(was_successful == 0)	continue;

		// print the device name, for verbosity
		size_t devicename_length;
		err = clGetDeviceInfo(device, CL_DEVICE_NAME, 0, NULL, &devicename_length);
		cl_ensure(err);
		std::vector<char> devicename_vec(devicename_length);
		err = clGetDeviceInfo(device, CL_DEVICE_NAME, devicename_length, &devicename_vec[0], NULL);
		std::string devicename(devicename_vec.begin(), devicename_vec.end());
		std::cout << "Found associated CL device: " << devicename << std::endl;

		return device;
	}

	cl_die("No associated OpenCL device found!");

}

static void CL_CALLBACK cl_error_callback (const char *errinfo,
                                           const void *private_info,
                                           size_t cb,
                                           void *user_data)
{
    cl_die("Internal OpenCL error: " << errinfo);
}



<<<<<<< HEAD
cl_particle_updater::cl_particle_updater(HGLRC glContext, HDC glDC, size_t max_num_particles_, GLuint posBuffer_, GLuint colBuffer_)
    : max_num_particles(max_num_particles_)
{
    std::cout << "Initializing CL Particle Updater ..." << std::endl;

    cl_int err;

    // get the opencl device associated with the glContext
    device = find_opengl_device(glContext, glDC);

    // create context
    cl_context_properties props[] = {
            CL_GL_CONTEXT_KHR, (cl_context_properties) glContext,
            CL_WGL_HDC_KHR, (cl_context_properties) glDC,
            0
    };
    context = clCreateContext(props, 1, &device, cl_error_callback, NULL, &err);
    cl_ensure(err);

    // create command queue
    command_queue = clCreateCommandQueue(context, device, (cl_command_queue_properties)0, &err);
    cl_ensure(err);

    // determine wether or not to use opengl buffers
    if(posBuffer_ == 0 || colBuffer_ == 0) use_gl_buffers = false;

    // initialize buffers
    size_t pos_buffer_size   = max_num_particles * sizeof(pos_buffer_type);
    size_t speed_buffer_size = max_num_particles * sizeof(speed_buffer_type);
    size_t col_buffer_size   = max_num_particles * sizeof(col_buffer_type);
    size_t specs_buffer_size = max_num_particles * sizeof(specs_buffer_type);
    size_t mouse_buffer_size = 5 * sizeof(mouse_buffer_type);
    speed_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE, speed_buffer_size, NULL, &err); cl_ensure(err);
    specs_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE, specs_buffer_size, NULL, &err); cl_ensure(err);
    mouse_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY,  mouse_buffer_size, NULL, &err); cl_ensure(err);
    if(!use_gl_buffers) {
        pos_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE, pos_buffer_size, NULL, &err); cl_ensure(err);
        col_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, col_buffer_size, NULL, &err); cl_ensure(err);
    } else {
        // check if sizes match
        GLint pos_gl_size, col_gl_size;
        glBindBuffer(GL_ARRAY_BUFFER, posBuffer_);
        glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &pos_gl_size);
        glBindBuffer(GL_ARRAY_BUFFER, colBuffer_);
        glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &col_gl_size);
        std::cout << "Pos CL Buffer created from GL Buffer (size: " << pos_gl_size << "/" << pos_buffer_size << ")" << std::endl;
        std::cout << "Col CL Buffer created from GL Buffer (size: " << col_gl_size << "/" << col_buffer_size << ")" << std::endl;
        if(pos_gl_size < pos_buffer_size || col_gl_size < col_buffer_size) cl_die("OpenGL buffers are too small!");

        pos_buffer = clCreateFromGLBuffer(context, CL_MEM_READ_WRITE, posBuffer_, &err); cl_ensure(err);
        col_buffer = clCreateFromGLBuffer(context, CL_MEM_READ_WRITE, colBuffer_, &err); cl_ensure(err);
    }

    // read kernel source from file
    std::ifstream file("kernels/update_particles.cl");
    if(file.eof() || file.fail()) cl_die("Unable to open update_particles.cl!");

    // read filesize
    file.seekg(0, std::ios_base::end);
    size_t fileSize = file.tellg();
    if(fileSize < 1) cl_die("Invalid kernel file!");

    // read content
    std::vector<char> kernel_src(fileSize);
    file.seekg(0, std::ios_base::beg);
    file.read(&kernel_src[0], fileSize);

    //std::cout << std::string(kernel_src.begin(), kernel_src.end()) << std::endl;

    // create program from source
    const char* kernel_src_ptr = &kernel_src[0];
    size_t kernel_size = kernel_src.size();
    cl_program prog = clCreateProgramWithSource(context, 1, &kernel_src_ptr, &kernel_size, &err);
    cl_ensure(err);

    // build program
    err = clBuildProgram(prog, 1, &device, "", NULL, NULL);
    if(err == CL_BUILD_PROGRAM_FAILURE)
    {
        // print build log if it failed
        size_t log_size;
        err = clGetProgramBuildInfo(prog, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
        cl_ensure(err);

        std::vector<char> buildlog(log_size);
        err = clGetProgramBuildInfo(prog, device, CL_PROGRAM_BUILD_LOG, log_size, &buildlog[0], NULL);
        cl_ensure(err);

        std::cerr << "################################" << std::endl
                  << "# CL BUILD ERROR. Build log:" << std::endl << std::endl
                  << std::string(buildlog.begin(), buildlog.end()) << std::endl
                  << "#" << std::endl
                  << "################################" << std::endl;

        cl_ensure(CL_BUILD_PROGRAM_FAILURE);
    }
    cl_ensure(err);

    // create kernel
    calculation_kernel = clCreateKernel(prog, "update_particles", &err);
    cl_ensure(err);

    // release the program
    err = clReleaseProgram(prog);
    cl_ensure(err);

    // attach the buffers to the kernel
    err = clSetKernelArg(calculation_kernel, 0, sizeof(cl_mem), &pos_buffer);   cl_ensure(err);
    err = clSetKernelArg(calculation_kernel, 1, sizeof(cl_mem), &speed_buffer); cl_ensure(err);
    err = clSetKernelArg(calculation_kernel, 2, sizeof(cl_mem), &col_buffer);   cl_ensure(err);
    err = clSetKernelArg(calculation_kernel, 3, sizeof(cl_mem), &specs_buffer); cl_ensure(err);
    err = clSetKernelArg(calculation_kernel, 4, sizeof(cl_mem), &mouse_buffer); cl_ensure(err);

=======
cl_particle_updater::cl_particle_updater(size_t max_num_particles_, GLuint posBuffer_, GLuint colBuffer_)
	: max_num_particles(max_num_particles_)
{
	std::cout << "Initializing CL Particle Updater ..." << std::endl;

	cl_int err;

	// determine wether or not to use opengl buffers
	if(posBuffer_ == 0 || colBuffer_ == 0)
        use_gl_buffers = false;
    else
        use_gl_buffers = true;

	// get the opencl device associated with the glContext
    if(use_gl_buffers)
    	device = find_opengl_device();
    else
        device = find_cl_device();

	// create context
    if(use_gl_buffers)
    {
        #ifdef _WIN32 // WINDOWS
    		cl_context_properties props[] = {
    			CL_GL_CONTEXT_KHR, (cl_context_properties) wglGetCurrentContext(),
    			CL_WGL_HDC_KHR, (cl_context_properties) wglGetCurrentDC(),
    			0
    		};
        #else // LINUX
    		cl_context_properties props[] = {
    			CL_GL_CONTEXT_KHR, (cl_context_properties) glXGetCurrentContext(),
    			CL_GLX_DISPLAY_KHR, (cl_context_properties) glXGetCurrentDisplay(),
    			0
    		};
        #endif

    	context = clCreateContext(props, 1, &device, cl_error_callback, NULL, &err);
    	cl_ensure(err);
    }
    else
    {
    	context = clCreateContext(NULL, 1, &device, cl_error_callback, NULL, &err);
    	cl_ensure(err);
    }


	// create command queue
	command_queue = clCreateCommandQueue(context, device, (cl_command_queue_properties)0, &err);
	cl_ensure(err);

	// initialize buffers
	size_t pos_buffer_size   = max_num_particles * sizeof(pos_buffer_type);
	size_t speed_buffer_size = max_num_particles * sizeof(speed_buffer_type);
	size_t col_buffer_size   = max_num_particles * sizeof(col_buffer_type);
	size_t specs_buffer_size = max_num_particles * sizeof(specs_buffer_type);
	size_t mouse_buffer_size = 5 * sizeof(mouse_buffer_type);
	speed_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE, speed_buffer_size, NULL, &err); cl_ensure(err);
	specs_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE, specs_buffer_size, NULL, &err); cl_ensure(err);
	mouse_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY,  mouse_buffer_size, NULL, &err); cl_ensure(err);
	if(!use_gl_buffers) {
		pos_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE, pos_buffer_size, NULL, &err); cl_ensure(err);
		col_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, col_buffer_size, NULL, &err); cl_ensure(err);
	} else {
		// check if sizes match
		GLint pos_gl_size, col_gl_size;
		glBindBuffer(GL_ARRAY_BUFFER, posBuffer_);
		glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &pos_gl_size);
		glBindBuffer(GL_ARRAY_BUFFER, colBuffer_);
		glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &col_gl_size);
		std::cout << "Pos CL Buffer created from GL Buffer (size: " << pos_gl_size << "/" << pos_buffer_size << ")" << std::endl;
		std::cout << "Col CL Buffer created from GL Buffer (size: " << col_gl_size << "/" << col_buffer_size << ")" << std::endl;
		if((size_t)pos_gl_size < pos_buffer_size || (size_t)col_gl_size < col_buffer_size) cl_die("OpenGL buffers are too small!");

		pos_buffer = clCreateFromGLBuffer(context, CL_MEM_READ_WRITE, posBuffer_, &err); cl_ensure(err);
		col_buffer = clCreateFromGLBuffer(context, CL_MEM_READ_WRITE, colBuffer_, &err); cl_ensure(err);
	}

	// read kernel source from file
	std::ifstream file("kernels/update_particles.cl");
	if(file.eof() || file.fail()) cl_die("Unable to open update_particles.cl!");

	// read filesize
	file.seekg(0, std::ios_base::end);
	size_t fileSize = file.tellg();
	if(fileSize < 1) cl_die("Invalid kernel file!");

	// read content
	std::vector<char> kernel_src(fileSize);
	file.seekg(0, std::ios_base::beg);
	file.read(&kernel_src[0], fileSize);

	//std::cout << std::string(kernel_src.begin(), kernel_src.end()) << std::endl;

	// create program from source
	const char* kernel_src_ptr = &kernel_src[0];
	size_t kernel_size = kernel_src.size();
	cl_program prog = clCreateProgramWithSource(context, 1, &kernel_src_ptr, &kernel_size, &err);
	cl_ensure(err);

	// build program
	err = clBuildProgram(prog, 1, &device, "", NULL, NULL);
	if(err == CL_BUILD_PROGRAM_FAILURE)
	{
		// print build log if it failed
		size_t log_size;
		err = clGetProgramBuildInfo(prog, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
		cl_ensure(err);

		std::vector<char> buildlog(log_size);
		err = clGetProgramBuildInfo(prog, device, CL_PROGRAM_BUILD_LOG, log_size, &buildlog[0], NULL);
		cl_ensure(err);

		std::cerr << "################################" << std::endl
				  << "# CL BUILD ERROR. Build log:" << std::endl << std::endl
				  << std::string(buildlog.begin(), buildlog.end()) << std::endl
				  << "#" << std::endl
				  << "################################" << std::endl;

		cl_ensure(CL_BUILD_PROGRAM_FAILURE);
	}
	cl_ensure(err);

	// create kernel
	calculation_kernel = clCreateKernel(prog, "update_particles", &err);
	cl_ensure(err);

	// release the program
	err = clReleaseProgram(prog);
	cl_ensure(err);

	// attach the buffers to the kernel
	err = clSetKernelArg(calculation_kernel, 0, sizeof(cl_mem), &pos_buffer);   cl_ensure(err);
	err = clSetKernelArg(calculation_kernel, 1, sizeof(cl_mem), &speed_buffer); cl_ensure(err);
	err = clSetKernelArg(calculation_kernel, 2, sizeof(cl_mem), &col_buffer);   cl_ensure(err);
	err = clSetKernelArg(calculation_kernel, 3, sizeof(cl_mem), &specs_buffer); cl_ensure(err);
	err = clSetKernelArg(calculation_kernel, 4, sizeof(cl_mem), &mouse_buffer); cl_ensure(err);

>>>>>>> 44d8c75a0712e5a24c4e6d21d3bf8d23ddaac5cb
}



cl_particle_updater::~cl_particle_updater()
{
    std::cout << "Shutting down CL Particle Updater ..." << std::endl;

    clReleaseKernel(calculation_kernel);
    clReleaseMemObject(mouse_buffer);
    clReleaseMemObject(specs_buffer);
    clReleaseMemObject(col_buffer);
    clReleaseMemObject(speed_buffer);
    clReleaseMemObject(pos_buffer);
    clReleaseCommandQueue(command_queue);
    clReleaseContext(context);
}


void
cl_particle_updater::set_particle_values(std::vector<Particle> & particles)
{
<<<<<<< HEAD

    if(particles.size() > max_num_particles) cl_die("Our CL buffers can't hold that many particles! (" << particles.size() << "/" << max_num_particles << ")");

    // collect all data in vectors
    std::vector<pos_buffer_type>   vec_pos;
    std::vector<speed_buffer_type> vec_speed;
    std::vector<col_buffer_type>   vec_col;
    std::vector<specs_buffer_type> vec_specs;

    vec_pos.reserve(particles.size());
    vec_speed.reserve(particles.size());
    vec_col.reserve(particles.size());
    vec_specs.reserve(particles.size());

    for(std::vector<Particle>::iterator particle = particles.begin(); particle != particles.end(); particle++)
    {
        pos_buffer_type   pos   = {particle->pos.x, particle->pos.y, particle->pos.z, particle->size};
        vec_pos.push_back(pos);

        speed_buffer_type speed = {particle->speed.x, particle->speed.y, particle->speed.z};
        vec_speed.push_back(speed);

        col_buffer_type   col   = {particle->r, particle->g, particle->b, particle->a};
        vec_col.push_back(col);

        specs_buffer_type specs = {particle->mass, particle->life, particle->cameradistance};
        vec_specs.push_back(specs);
    }

    // copy vectors to gpu
    clEnqueueWriteBuffer(command_queue, pos_buffer,   FALSE, 0, particles.size() * sizeof(pos_buffer_type),   &vec_pos[0],   0, NULL, NULL);
    clEnqueueWriteBuffer(command_queue, speed_buffer, FALSE, 0, particles.size() * sizeof(speed_buffer_type), &vec_speed[0], 0, NULL, NULL);
    clEnqueueWriteBuffer(command_queue, col_buffer,   FALSE, 0, particles.size() * sizeof(col_buffer_type),   &vec_col[0],   0, NULL, NULL);
    clEnqueueWriteBuffer(command_queue, specs_buffer, FALSE, 0, particles.size() * sizeof(specs_buffer_type), &vec_specs[0], 0, NULL, NULL);
    clFinish(command_queue);
=======

	if(particles.size() > max_num_particles) cl_die("Our CL buffers can't hold that many particles! (" << particles.size() << "/" << max_num_particles << ")");

	// collect all data in vectors
	std::vector<pos_buffer_type>   vec_pos;
	std::vector<speed_buffer_type> vec_speed;
	std::vector<col_buffer_type>   vec_col;
	std::vector<specs_buffer_type> vec_specs;

	vec_pos.reserve(particles.size());
	vec_speed.reserve(particles.size());
	vec_col.reserve(particles.size());
	vec_specs.reserve(particles.size());

	for(std::vector<Particle>::iterator particle = particles.begin(); particle != particles.end(); particle++)
	{
		pos_buffer_type   pos   = {particle->pos.x, particle->pos.y, particle->pos.z, particle->size};
		vec_pos.push_back(pos);

		speed_buffer_type speed = {particle->speed.x, particle->speed.y, particle->speed.z};
		vec_speed.push_back(speed);

		col_buffer_type   col   = {particle->r, particle->g, particle->b, particle->a};
		vec_col.push_back(col);

		specs_buffer_type specs = {particle->mass, particle->life, particle->cameradistance};
		vec_specs.push_back(specs);
	}

	// copy vectors to gpu
	clEnqueueWriteBuffer(command_queue, pos_buffer,   CL_FALSE, 0, particles.size() * sizeof(pos_buffer_type),   &vec_pos[0],   0, NULL, NULL);
	clEnqueueWriteBuffer(command_queue, speed_buffer, CL_FALSE, 0, particles.size() * sizeof(speed_buffer_type), &vec_speed[0], 0, NULL, NULL);
	clEnqueueWriteBuffer(command_queue, col_buffer,   CL_FALSE, 0, particles.size() * sizeof(col_buffer_type),   &vec_col[0],   0, NULL, NULL);
	clEnqueueWriteBuffer(command_queue, specs_buffer, CL_FALSE, 0, particles.size() * sizeof(specs_buffer_type), &vec_specs[0], 0, NULL, NULL);
	clFinish(command_queue);
>>>>>>> 44d8c75a0712e5a24c4e6d21d3bf8d23ddaac5cb

}

void
cl_particle_updater::read_pos_and_col(GLfloat* pos_data, GLubyte* col_data, size_t num_particles)
{
    if(sizeof(GLfloat) * 4 != sizeof(pos_buffer_type) || sizeof(GLubyte) * 4 != sizeof(col_buffer_type))
        cl_die("Buffer types don't match!");

<<<<<<< HEAD
    clEnqueueReadBuffer(command_queue, pos_buffer, FALSE, 0, num_particles * sizeof(pos_buffer_type), pos_data, 0, NULL, NULL);
    clEnqueueReadBuffer(command_queue, col_buffer, FALSE, 0, num_particles * sizeof(col_buffer_type), col_data, 0, NULL, NULL);
    clFinish(command_queue);
=======
	clEnqueueReadBuffer(command_queue, pos_buffer, CL_FALSE, 0, num_particles * sizeof(pos_buffer_type), pos_data, 0, NULL, NULL);
	clEnqueueReadBuffer(command_queue, col_buffer, CL_FALSE, 0, num_particles * sizeof(col_buffer_type), col_data, 0, NULL, NULL);
	clFinish(command_queue);
>>>>>>> 44d8c75a0712e5a24c4e6d21d3bf8d23ddaac5cb

}

void
cl_particle_updater::update(glm::vec4 mousepos, bool mouse_pressed, float delta, size_t num_particles)
{

<<<<<<< HEAD
    // write data to mouse buffer
    mouse_buffer_type mousedata[5];
    mousedata[0] = mousepos.x;
    mousedata[1] = mousepos.y;
    mousedata[2] = mousepos.z;
    mousedata[3] = mouse_pressed? 1.0f : 0.0f;
    mousedata[4] = delta;

    // send mouse data to gpu
    clEnqueueWriteBuffer(command_queue, mouse_buffer, FALSE, 0, 5 * sizeof(mouse_buffer_type), mousedata, 0, NULL, NULL);

    // start calculation. needs synchronization at some point, will be at read_pos_and_col or release_gl_buffer
    size_t zero = 0;
    clEnqueueNDRangeKernel(command_queue, calculation_kernel, 1, &zero, &num_particles, NULL, 0, NULL, NULL);
=======
	// write data to mouse buffer
	mouse_buffer_type mousedata[5];
	mousedata[0] = mousepos.x;
	mousedata[1] = mousepos.y;
	mousedata[2] = mousepos.z;
	mousedata[3] = mouse_pressed? 1.0f : 0.0f;
	mousedata[4] = delta;

	// send mouse data to gpu
	clEnqueueWriteBuffer(command_queue, mouse_buffer, CL_FALSE, 0, 5 * sizeof(mouse_buffer_type), mousedata, 0, NULL, NULL);

	// start calculation. needs synchronization at some point, will be at read_pos_and_col or release_gl_buffer
	size_t zero = 0;
	clEnqueueNDRangeKernel(command_queue, calculation_kernel, 1, &zero, &num_particles, NULL, 0, NULL, NULL);
>>>>>>> 44d8c75a0712e5a24c4e6d21d3bf8d23ddaac5cb

}

void
cl_particle_updater::lock_gl_buffers()
{
<<<<<<< HEAD
    cl_int err;
=======

    // don't do anything if we don't have gl buffers
    if(!use_gl_buffers) return;

	cl_int err;
>>>>>>> 44d8c75a0712e5a24c4e6d21d3bf8d23ddaac5cb

    // unbind buffers from GL
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // aquire buffers for CL calculation
    cl_mem bufs[] = {pos_buffer, col_buffer};
    err = clEnqueueAcquireGLObjects(command_queue, 2, bufs, 0, NULL, NULL);
    cl_ensure(err);

}

void
cl_particle_updater::unlock_gl_buffers()
{
<<<<<<< HEAD
    cl_int err;
=======

    // don't do anything if we don't have gl buffers
    if(!use_gl_buffers) return;

	cl_int err;
>>>>>>> 44d8c75a0712e5a24c4e6d21d3bf8d23ddaac5cb

    // release buffers from CL, can be used for OpenGL purposes again.
    cl_mem bufs[] = {pos_buffer, col_buffer};
    err = clEnqueueReleaseGLObjects(command_queue, 2, bufs, 0, NULL, NULL);
    cl_ensure(err);

    // wait for release to finish
    err = clFinish(command_queue);
    cl_ensure(err);

}
