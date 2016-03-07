cmake_minimum_required(VERSION 2.8)

project(c2baVulkanTriangle)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(c2ba-copy-dll-post-build)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

if(WIN32)
    if(MSVC)
        set(CMAKE_LIBRARY_PATH ${CMAKE_SOURCE_DIR}/third-party/lib/windows/x64/)
        add_definitions(-DNOMINMAX)

        set(SDL_LIBRARIES ${CMAKE_LIBRARY_PATH}/SDL2.lib ${CMAKE_LIBRARY_PATH}/SDL2main.lib)
        set(VULKAN_LIBRARIES ${CMAKE_LIBRARY_PATH}/vulkan-1.lib)
        set(GLFW_LIBRARIES ${CMAKE_LIBRARY_PATH}/glfw3.lib)
        
        set(DLL_3RD ${CMAKE_SOURCE_DIR}/third-party/bin/x64)
        file(GLOB 3RD_PARTY_DLL_FILES ${DLL_3RD}/*.dll)

        add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)
    else()
        message(FATAL_ERROR "Only MSVC 2015 supported for windows builds")
    endif()
else()
    message(FATAL_ERROR "Only windows platform supported at the moment.")
endif()

include_directories(${CMAKE_SOURCE_DIR}/third-party/include)

file(
    GLOB_RECURSE
    SRC_FILES
    src/*.cpp src/*.hpp src/*.h
)

add_executable(c2baVulkanTriangle ${SRC_FILES})
target_link_libraries(c2baVulkanTriangle ${SDL_LIBRARIES} ${VULKAN_LIBRARIES})

c2ba_copy_dll_post_build(c2baVulkanTriangle "${3RD_PARTY_DLL_FILES}")

add_subdirectory(fromSamples)
add_subdirectory(vulkanTriangleGLFW)
add_subdirectory(Lava)