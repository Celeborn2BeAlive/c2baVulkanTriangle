file(
    GLOB_RECURSE
    SRC_FILES
    *.cpp *.hpp *.h *.c
)

add_definitions(-D_USE_MATH_DEFINES)

add_executable(Lava ${SRC_FILES})
target_link_libraries(Lava ${VULKAN_LIBRARIES} ${GLFW_LIBRARIES})

c2ba_copy_dll_post_build(Lava "${3RD_PARTY_DLL_FILES}")