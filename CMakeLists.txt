cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CUDA_SEPARABLE_COMPILATION ON) 

project(tinyRendererG2 LANGUAGES CXX C CUDA)

# add_compile_options("/MP")
# set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT tinyRendererG2)

set(GLFW_DIR "${PROJECT_SOURCE_DIR}/thirdparty/GLFW")
set(GLAD_DIR "${PROJECT_SOURCE_DIR}/thirdparty/GLAD")
set(GLM_DIR "${PROJECT_SOURCE_DIR}/thirdparty/glm")
set(EIGEN_DIR "${PROJECT_SOURCE_DIR}/thirdparty/eigen")
set(IMGUI_DIR "${PROJECT_SOURCE_DIR}/thirdparty/imgui")

set(IMGUI_HEADER_DIR "${IMGUI_DIR}")
set(GLFW_HEADER_DIR "${GLFW_DIR}/include")
set(GLAD_HEADER_DIR "${GLAD_DIR}/include")
set(GLM_HEADER_DIR "${GLM_DIR}/include")
set(EIGEN_HEADER_DIR "${EIGEN_DIR}/include")
set(GLFW_LIB_DIR "${GLFW_DIR}/lib")


FILE(GLOB GLAD_SOURCE_FILES
"${GLAD_DIR}/src/*.c"
)

FILE(GLOB SOURCE_FILES 
"${PROJECT_SOURCE_DIR}/src/*.cpp" 
)

FILE (GLOB IMGUI_FILES
"${IMGUI_DIR}/*.cpp"
"${IMGUI_DIR}/*.h"
"${IMGUI_DIR}/backends/*.cpp"
"${IMGUI_DIR}/backends/*.h"
)

FILE(GLOB CU_SOURCE_FILES 
"${PROJECT_SOURCE_DIR}/src/*.cu" 
)

FILE(GLOB SIMULATION_FILES
"${PROJECT_SOURCE_DIR}/simulation/*.h" 
"${PROJECT_SOURCE_DIR}/simulation/*.cpp" 
)

FILE(GLOB HEADER_FILES
"${PROJECT_SOURCE_DIR}/include/*.h" 
"${PROJECT_SOURCE_DIR}/include/*.hpp" 
)

FILE(GLOB SHADER_FILES
"${PROJECT_SOURCE_DIR}/resource/shader/*.glsl"
"${PROJECT_SOURCE_DIR}/resource/shader/*.frag"
"${PROJECT_SOURCE_DIR}/resource/shader/*.vert"
"${PROJECT_SOURCE_DIR}/resource/shader/*.geom"
"${PROJECT_SOURCE_DIR}/resource/shader/*.vs"
"${PROJECT_SOURCE_DIR}/resource/shader/*.fs"
)

FILE(GLOB MATERIAL_FILES
"${PROJECT_SOURCE_DIR}/resource/material/*.json"
)

FILE(GLOB SCENE_FILES
"${PROJECT_SOURCE_DIR}/resource/scene/*.json"
)

source_group(TREE "${PROJECT_SOURCE_DIR}/simulation" PREFIX SIMULATION FILES ${SIMULATION_FILES})
source_group(TREE "${PROJECT_SOURCE_DIR}/include" PREFIX HEADER FILES ${HEADER_FILES})
source_group(TREE "${PROJECT_SOURCE_DIR}/src" PREFIX SOURCE FILES ${SOURCE_FILES})
source_group(TREE "${PROJECT_SOURCE_DIR}/src" PREFIX CUDA_SOURCE FILES ${CU_SOURCE_FILES})
source_group(
    TREE "${PROJECT_SOURCE_DIR}/thirdparty" PREFIX ThirdParty FILES
    ${GLAD_SOURCE_FILES} 
    ${IMGUI_FILES}
 )
source_group(TREE "${PROJECT_SOURCE_DIR}/resource/shader" PREFIX SHADER FILES ${SHADER_FILES})
source_group(TREE "${PROJECT_SOURCE_DIR}/resource/material" PREFIX MATERIAL FILES ${MATERIAL_FILES})
source_group(TREE "${PROJECT_SOURCE_DIR}/resource/scene" PREFIX SCENE FILES ${SCENE_FILES})

add_executable(${CMAKE_PROJECT_NAME}
"${CU_SOURCE_FILES}"
"${GLAD_SOURCE_FILES}"
"${SIMULATION_FILES}" 
"${HEADER_FILES}" 
"${SOURCE_FILES}"
"${IMGUI_FILES}"
)

set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

target_include_directories(${CMAKE_PROJECT_NAME} 
    PUBLIC "${PROJECT_SOURCE_DIR}/include"
    PUBLIC "${PROJECT_SOURCE_DIR}/simulation"
    PUBLIC "${GLFW_HEADER_DIR}"
    PUBLIC "${GLAD_HEADER_DIR}"
    PUBLIC "${GLM_HEADER_DIR}"
    PUBLIC "${EIGEN_HEADER_DIR}"
    PUBLIC "${IMGUI_HEADER_DIR}"
)

target_link_libraries(${CMAKE_PROJECT_NAME} 
    PUBLIC "${GLFW_LIB_DIR}/Release/glfw3.lib"
)

target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC $<$<COMPILE_LANGUAGE:CUDA>:--extended-lambda>)
target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC $<$<COMPILE_LANGUAGE:CUDA>:--expt-relaxed-constexpr>)

add_custom_target(Asset
    SOURCES ${SHADER_FILES} ${MATERIAL_FILES} ${SCENE_FILES} ${BACKUP_FILES} 
)