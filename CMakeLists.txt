cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CUDA_SEPARABLE_COMPILATION ON) 

project(tinyRendererG2 LANGUAGES CXX C CUDA)

set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT tinyRendererG2)

set(CONFIG_IN_DIR "${PROJECT_SOURCE_DIR}/cmake.in")
configure_file(
"${CONFIG_IN_DIR}/src_config.h.in"
${PROJECT_BINARY_DIR}/include/src_config.h)


set(GLFW_DIR "${PROJECT_SOURCE_DIR}/thirdparty/GLFW")
set(GLAD_DIR "${PROJECT_SOURCE_DIR}/thirdparty/GLAD")
set(GLM_DIR "${PROJECT_SOURCE_DIR}/thirdparty/glm")
set(EIGEN_DIR "${PROJECT_SOURCE_DIR}/thirdparty/eigen")
set(IMGUI_DIR "${PROJECT_SOURCE_DIR}/thirdparty/imgui")
set(JSON11_DIR "${PROJECT_SOURCE_DIR}/thirdparty/json11")
set(STB_DIR "${PROJECT_SOURCE_DIR}/thirdparty/stb")

set(IMGUI_HEADER_DIR "${IMGUI_DIR}")
set(GLFW_HEADER_DIR "${GLFW_DIR}/include")
set(GLAD_HEADER_DIR "${GLAD_DIR}/include")
set(GLM_HEADER_DIR "${GLM_DIR}/include")
set(EIGEN_HEADER_DIR "${EIGEN_DIR}/include")
set(JSON11_HEADER_DIR "${JSON11_DIR}/include")
set(STB_HEADER_DIR "${STB_DIR}/include")

set(GLFW_LIB_DIR "${GLFW_DIR}/lib")


FILE(GLOB GLFW_FILES
"${GLFW_DIR}/include/GLFW/*.h"
)

FILE(GLOB GLAD_FILES
"${GLAD_DIR}/include/glad/*.h"
"${GLAD_DIR}/src/*.c"
)

FILE (GLOB IMGUI_FILES
"${IMGUI_DIR}/*.cpp"
"${IMGUI_DIR}/*.h"
"${IMGUI_DIR}/backends/*.cpp"
"${IMGUI_DIR}/backends/*.h"
)

FILE(GLOB JSON11_FILES
"${JSON11_DIR}/src/*.cpp"
"${JSON11_DIR}/include/*.hpp"
)

FILE(GLOB STB_FILES
"${STB_DIR}/src/*.cpp"
"${STB_DIR}/include/*.h"
)

FILE(GLOB RENDER_SOURCE_FILES 
"${PROJECT_SOURCE_DIR}/render/src/*.cpp" 
)
FILE(GLOB RENDER_HEADER_FILES 
"${PROJECT_SOURCE_DIR}/render/include/*.h" 
"${PROJECT_SOURCE_DIR}/render/include/render_pipeline/*.h" 
)

FILE(GLOB APP_SOURCE_FILES 
"${PROJECT_SOURCE_DIR}/app/src/*.cpp" 
)
FILE(GLOB APP_HEADER_FILES 
"${PROJECT_SOURCE_DIR}/app/include/*.h" 
)

FILE(GLOB SIMULATION_SOURCE_FILES 
"${PROJECT_SOURCE_DIR}/simulation/src/*.cpp" 
)
FILE(GLOB SIMULATION_HEADER_FILES 
"${PROJECT_SOURCE_DIR}/simulation/include/*.h" 
)

FILE(GLOB UI_SOURCE_FILES 
"${PROJECT_SOURCE_DIR}/UI/src/*.cpp" 
)
FILE(GLOB UI_HEADER_FILES 
"${PROJECT_SOURCE_DIR}/UI/include/*.h" 
)

FILE(GLOB UTILITY_SOURCE_FILES 
"${PROJECT_SOURCE_DIR}/utility/src/*.cpp" 
)
FILE(GLOB UTILITY_HEADER_FILES 
"${PROJECT_SOURCE_DIR}/utility/include/*.h" 
)


source_group(TREE "${PROJECT_SOURCE_DIR}/app" PREFIX App FILES 
    ${APP_SOURCE_FILES} 
    ${APP_HEADER_FILES}
)
source_group(TREE "${PROJECT_SOURCE_DIR}/render" PREFIX Render FILES 
    ${RENDER_SOURCE_FILES} 
    ${RENDER_HEADER_FILES}
)
source_group(TREE "${PROJECT_SOURCE_DIR}/simulation" PREFIX Simulation FILES 
    ${SIMULATION_SOURCE_FILES}
    ${SIMULATION_HEADER_FILES}
)
source_group(TREE "${PROJECT_SOURCE_DIR}/UI" PREFIX UI FILES 
    ${UI_SOURCE_FILES}
    ${UI_HEADER_FILES}
) 
source_group(TREE "${PROJECT_SOURCE_DIR}/utility" PREFIX Utility FILES 
    ${UTILITY_SOURCE_FILES}
    ${UTILITY_HEADER_FILES}
) 
source_group(
    TREE "${PROJECT_SOURCE_DIR}/thirdparty" PREFIX ThirdParty FILES
    ${GLAD_FILES} 
    ${GLFW_FILES}
    ${IMGUI_FILES}
    ${JSON11_FILES}
    ${STB_FILES}
 )



add_executable(${CMAKE_PROJECT_NAME}
"${GLAD_FILES}"
"${GLFW_FILES}"
"${IMGUI_FILES}"
"${JSON11_FILES}"
"${STB_FILES}"

"${APP_HEADER_FILES}" 
"${APP_SOURCE_FILES}"
"${SIMULATION_HEADER_FILES}" 
"${SIMULATION_SOURCE_FILES}" 
"${RENDER_HEADER_FILES}" 
"${RENDER_SOURCE_FILES}" 
"${UI_HEADER_FILES}" 
"${UI_SOURCE_FILES}" 
"${UTILITY_HEADER_FILES}" 
"${UTILITY_SOURCE_FILES}" 
)

# set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT tinyRendererG2)

target_include_directories(${CMAKE_PROJECT_NAME} 
PUBLIC "${PROJECT_SOURCE_DIR}/render/include"
PUBLIC "${PROJECT_SOURCE_DIR}/render/include/render_pipeline"
PUBLIC "${PROJECT_SOURCE_DIR}/simulation/include"
PUBLIC "${PROJECT_SOURCE_DIR}/UI/include"
PUBLIC "${PROJECT_SOURCE_DIR}/utility/include"
PUBLIC "${PROJECT_SOURCE_DIR}/app/include"

PUBLIC "${PROJECT_BINARY_DIR}/include"

PUBLIC "${IMGUI_HEADER_DIR}"
PUBLIC "${GLFW_HEADER_DIR}"
PUBLIC "${GLAD_HEADER_DIR}"
PUBLIC "${GLM_HEADER_DIR}"
PUBLIC "${EIGEN_HEADER_DIR}"
PUBLIC "${JSON11_HEADER_DIR}"
PUBLIC "${STB_HEADER_DIR}"
)


target_link_libraries(${CMAKE_PROJECT_NAME} 
PUBLIC "${GLFW_LIB_DIR}/Release/glfw3.lib"
)

set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC $<$<COMPILE_LANGUAGE:CUDA>:--extended-lambda>)
target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC $<$<COMPILE_LANGUAGE:CUDA>:--expt-relaxed-constexpr>)
target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE "/MP") 


FILE(GLOB SHADER_FILES
"${PROJECT_SOURCE_DIR}/asset/shader/*.glsl"
"${PROJECT_SOURCE_DIR}/asset/shader/*.frag"
"${PROJECT_SOURCE_DIR}/asset/shader/*.vert"
"${PROJECT_SOURCE_DIR}/asset/shader/*.geom"
"${PROJECT_SOURCE_DIR}/asset/shader/*.vs"
"${PROJECT_SOURCE_DIR}/asset/shader/*.fs"
)

FILE(GLOB MATERIAL_FILES
"${PROJECT_SOURCE_DIR}/asset/material/*.json"
)

FILE(GLOB SCENE_FILES
"${PROJECT_SOURCE_DIR}/asset/scene/*.json"
)

source_group(TREE "${PROJECT_SOURCE_DIR}/asset/shader" PREFIX SHADER FILES ${SHADER_FILES})
source_group(TREE "${PROJECT_SOURCE_DIR}/asset/material" PREFIX MATERIAL FILES ${MATERIAL_FILES})
source_group(TREE "${PROJECT_SOURCE_DIR}/asset/scene" PREFIX SCENE FILES ${SCENE_FILES})

add_custom_target(Asset
    SOURCES ${SHADER_FILES} ${MATERIAL_FILES} ${SCENE_FILES}
)

