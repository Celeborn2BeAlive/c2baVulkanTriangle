TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    System/surfacewindow.cpp \
    System/Vulkan/instance.cpp \
    System/Vulkan/device.cpp \
    System/Vulkan/queue.cpp \
    System/contextinitializer.cpp \
    System/Vulkan/physicaldevices.cpp \
    System/loggable.cpp \
    System/Vulkan/exception.cpp \
    System/Vulkan/renderpass.cpp \
    System/Vulkan/imageview.cpp \
    System/Vulkan/framebuffer.cpp \
    System/Vulkan/commandpool.cpp \
    System/Vulkan/fence.cpp

CONFIG += c++14

LIBS += -lglfw3 -ldl -lpthread -lrt -lX11 -lXrandr -lXinerama -lXxf86vm -lXcursor -lvulkan

HEADERS += \
    System/Vulkan/exception.hpp \
    System/surfacewindow.hpp \
    System/Vulkan/instance.hpp \
    System/Vulkan/device.hpp \
    System/Vulkan/queue.hpp \
    System/contextinitializer.hpp \
    System/Vulkan/physicaldevices.hpp \
    System/loggable.hpp \
    System/Vulkan/imageview.hpp \
    System/Vulkan/renderpass.hpp \
    System/Vulkan/framebuffer.hpp \
    System/Vulkan/noncopyable.hpp \
    System/Vulkan/commandpool.hpp \
    System/Vulkan/fence.hpp

