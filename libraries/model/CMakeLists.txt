cmake_minimum_required(VERSION 3.17)
project(lib_model)

add_library(model STATIC
        private/Model.cpp
        private/ModelInstanceInfo.cpp
        private/Mesh.cpp
        private/AssimpModelLoader.cpp)
target_include_directories(model
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/public
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/private)
add_dependencies(model assimp glm fmt)
target_link_libraries(model PUBLIC assimp glm fmt)
set_target_properties(model PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO)
target_compile_options(model PRIVATE -Wall -Wextra -Werror)