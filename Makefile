UTILS =
CC = cc
CFLAGS = -Wall -Wno-comment -Os -std=gnu99
LAB_OPENCL_INC=/home/xinying/sdk/OpenCL/common/inc/CL #/opt/AMDAPP/include/

TARGET = matrixmultiplication

# Linux
LIBS = -I$(LAB_OPENCL_INC) -lOpenCL -lm

all: ${TARGET}

${TARGET}: ${TARGET}.c ${UTILS}
	${CC} ${CFLAGS} ${LIBS} ${UTILS} ${TARGET}.c -o ${TARGET}

clean:
	rm -f ${TARGET}

