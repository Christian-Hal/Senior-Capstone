cmake_minimum_required(VERSION 3.16)
project(MockUp)

set(CMAKE_CXX_STANDARD 17)

# GLFW
add_subdirectory(external/glfw)

# GLAD
add_library(glad external/glad/src/glad.c)
target_include_directories(glad PUBLIC external/glad/include)

# miniz
add_library(miniz STATIC external/miniz/miniz.c)
target_include_directories(miniz PUBLIC external/miniz)

# tinyxml2
add_library(tinyxml2 STATIC external/tinyxml2/tinyxml2.cpp)
target_include_directories(tinyxml2 PUBLIC external/tinyxml2)

# ImGui
add_library(imgui
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
    external/imgui/misc/cpp/imgui_stdlib.cpp 
 )

target_include_directories(imgui PUBLIC
    external/imgui
    external/imgui/backends
    external/imgui/misc/cpp
)

target_link_libraries(imgui PUBLIC glfw)

# ImSequencer 
add_library(imsequencer STATIC
    external/imsequencer/ImSequencer.h 
    external/imsequencer/ImSequencer.cpp
)

target_include_directories(imsequencer PUBLIC external/imsequencer)

target_link_libraries(imsequencer PUBLIC imgui) 



# Executable
file(GLOB_RECURSE PROJECT_SOURCES
     CONFIGURE_DEPENDS
     src/*.cpp

     src/*.c
)

if(WIN32)
    add_executable(${PROJECT_NAME} WIN32)
    target_link_options(${PROJECT_NAME} PRIVATE "/ENTRY:mainCRTStartup")

else()
    add_executable(${PROJECT_NAME})

endif()

target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES})


target_include_directories(${PROJECT_NAME} PUBLIC
    external/glm
    include/
)

# Link libraries differently depending on OS
if(WIN32)
    target_link_libraries(${PROJECT_NAME}
        glfw
        glad
        imgui
        imsequencer
        opengl32
	tinyxml2
	miniz
    )
elseif(APPLE)
    find_library(COCOA Cocoa REQUIRED)
    find_library(IOKIT IOKit REQUIRED)
    find_library(COREVIDEO CoreVideo REQUIRED)
    find_library(OPENGL OpenGL REQUIRED)

    target_link_libraries(${PROJECT_NAME}
        glfw
        glad
        imgui
	tinyxml2
	miniz
        ${OPENGL}
        ${COCOA}
        ${IOKIT}
        ${COREVIDEO}
    )
else()
    # Linux
    target_link_libraries(${PROJECT_NAME}
        glfw
        glad
        imgui
	tinyxml2
	miniz
        GL
        X11
        pthread
        Xrandr
        Xi
        dl
    )
endif()

# Copy assets folder to build directory
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})
