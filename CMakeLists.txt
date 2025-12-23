cmake_minimum_required(VERSION 3.16)
project(Capstone)

set(CMAKE_CXX_STANDARD 17)

# GLFW
add_subdirectory(external/glfw)

# GLAD
add_library(glad external/glad/src/glad.c)
target_include_directories(glad PUBLIC external/glad/include)

# ImGui
add_library(imgui
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC
    external/imgui
    external/imgui/backends
)

target_link_libraries(imgui PUBLIC glfw)

# Executable
add_executable(${PROJECT_NAME} 
    src/main.cpp
    src/App.cpp 
    src/Window.cpp 
    src/Renderer.cpp 
    src/UI.cpp 
)

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
        opengl32
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
        GL
        X11
        pthread
        Xrandr
        Xi
        dl
    )
endif()
