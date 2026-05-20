################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Add inputs and outputs from these tool invocations to the build variables 
CMD_SRCS += \
../src_device/f28003x_flash_lib_is.cmd \
../src_device/f28003x_headers_nonbios.cmd 

LIB_SRCS += \
../src_device/driverlib.lib 

ASM_SRCS += \
../src_device/f28003x_codestartbranch.asm 

C_SRCS += \
../src_device/device.c \
../src_device/f28003x_globalvariabledefs.c 

C_DEPS += \
./src_device/device.d \
./src_device/f28003x_globalvariabledefs.d 

OBJS += \
./src_device/device.obj \
./src_device/f28003x_codestartbranch.obj \
./src_device/f28003x_globalvariabledefs.obj 

ASM_DEPS += \
./src_device/f28003x_codestartbranch.d 

OBJS__QUOTED += \
"src_device\device.obj" \
"src_device\f28003x_codestartbranch.obj" \
"src_device\f28003x_globalvariabledefs.obj" 

C_DEPS__QUOTED += \
"src_device\device.d" \
"src_device\f28003x_globalvariabledefs.d" 

ASM_DEPS__QUOTED += \
"src_device\f28003x_codestartbranch.d" 

C_SRCS__QUOTED += \
"../src_device/device.c" \
"../src_device/f28003x_globalvariabledefs.c" 

ASM_SRCS__QUOTED += \
"../src_device/f28003x_codestartbranch.asm" 


