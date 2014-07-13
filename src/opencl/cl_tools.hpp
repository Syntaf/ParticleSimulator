#ifndef CL_TOOLS_HPP_
#define CL_TOOLS_HPP_

#include <iostream>
#include <sstream>
#include <CL/opencl.h>
#include <cstdlib>

const char* cl_err_to_str(cl_int errCode);

// To be called on OpenCL errorcodes, throws an exception on OpenCL Error
#define cl_ensure(errCode){                                                                        \
            if(errCode != CL_SUCCESS)                                                            \
            {                                                                                    \
                std::stringstream errorMessage;                                                    \
                errorMessage << "CL_ERROR(" << (errCode) << "): " << cl_err_to_str(errCode);    \
                std::cerr << ( __FILE__ ) << "(" << ( __LINE__ ) << "): "                        \
                          << errorMessage.str() << std::endl;                                    \
                exit(1);                                                                        \
            }                                                                                    \
}

// To be called on OpenCL errorcodes, throws an exception on OpenCL Error
#define cl_die(msg){                                                                        \
            std::cerr << ( __FILE__ ) << "(" << ( __LINE__ ) << "): ERROR: "                \
                      <<  msg  << std::endl;                                                \
            exit(1);                                                                        \
}

#endif