include_directories("${GLM_ROOT}")
include_directories("${GLEW_ROOT}")
include_directories("${SFML_INCLUDE_DIR}")
add_library(common shader.cpp texture.cpp controls.cpp)
