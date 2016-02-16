file(
    GLOB
    SRC_FILES
    *.cpp *.hpp *.h
)

add_definitions(-D_USE_MATH_DEFINES)

add_executable(c2baVulkanTriangleFromSamples ${SRC_FILES})
target_link_libraries(c2baVulkanTriangleFromSamples ${VULKAN_LIBRARIES})

c2ba_copy_dll_post_build(c2baVulkanTriangleFromSamples "${3RD_PARTY_DLL_FILES}")