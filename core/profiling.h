#ifndef GODOT_SOURCE_PROFILING_H
#define GODOT_SOURCE_PROFILING_H

#ifdef PROFILING_ENABLED
#include "optick/optick.h"

#define PROFILE_FRAME(NAME) OPTICK_FRAME(NAME)
#define PROFILE_FUNCTION(...) OPTICK_EVENT(__VA_ARGS__)
#define PROFILE_DYNAMIC_FUNCTION(...) OPTICK_EVENT_DYNAMIC(__VA_ARGS__)
#define PROFILING_THREAD(NAME) OPTICK_THREAD(NAME)
#define PROFILING_GPU_INIT_VULKAN(DEVICES, PHYSICAL_DEVICES, CMD_QUEUES, CMD_QUEUES_FAMILY, NUM_CMD_QUEUES, FUNCTIONS) OPTICK_GPU_INIT_VULKAN(DEVICES, PHYSICAL_DEVICES, CMD_QUEUES, CMD_QUEUES_FAMILY, NUM_CMD_QUEUES, FUNCTIONS)
#define PROFILING_GPU_FLIP(SWAP_CHAIN) OPTICK_GPU_FLIP(SWAP_CHAIN)
#else
#define PROFILE_FRAME(NAME)
#define PROFILE_FUNCTION(...)
#define PROFILE_DYNAMIC_FUNCTION(...)
#define PROFILING_THREAD(NAME)
#define PROFILING_GPU_INIT_VULKAN(...)
#define PROFILING_GPU_FLIP(SWAP_CHAIN)
#endif

#endif //GODOT_SOURCE_PROFILING_H
