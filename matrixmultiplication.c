/************************************************************************
OpenCL Exercise: Matrix Multiplication
*************************************************************************/

/* 2013/09/30 
 * Set TileSize same as block size to skip or load entire block
 * Xinying Zeng
*/

// Standard headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/stat.h>
#include <math.h>

// OpenCL headers
#include <CL/opencl.h>

// Exercise utility
#include "util.h"


// Multiply two matrices A * B = C
//#define NSIZE 512 
#define WA NSIZE
#define HA NSIZE
#define WB NSIZE
#define HB WA
#define WC WB
#define HC HA
//#define BLOCKSIZE 16 
#define TILESIZE 16 
#define SKIP 2
//#define PERCENTAGE 0.3
/////////////////////////////////////////////////////////
// Program main
/////////////////////////////////////////////////////////
//
//


void PrintEventInfo(cl_event evt, float actualPercentage, int id)
{
   cl_int error;
   cl_ulong cl_start_time, cl_end_time, queued, submitted;

   error = clGetEventProfilingInfo(evt, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &cl_start_time, NULL);
   OpenCL_CheckError(error, "clGetEventProfilingInfo Error");

   error = clGetEventProfilingInfo(evt, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &cl_end_time, NULL);
   OpenCL_CheckError(error, "clGetEventProfilingInfo Error");

   error = clGetEventProfilingInfo(evt, CL_PROFILING_COMMAND_QUEUED, sizeof(cl_ulong), &queued, NULL);
   OpenCL_CheckError(error, "clGetEventProfilingInfo Error");

   error = clGetEventProfilingInfo(evt, CL_PROFILING_COMMAND_SUBMIT, sizeof(cl_ulong), &submitted, NULL);
   OpenCL_CheckError(error, "clGetEventProfilingInfo Error");
   /*
   printf("submit->queued: %f ms\n", (submitted-queued)*1e-6);
   printf("queued->start: %f ms\n",  (cl_start_time-submitted)*1e-6);
   printf("start->end: %f ms\n",     (cl_end_time-cl_start_time)*1e-6);
   */
   printf("%d,%d,%d,%f,%f,", WA, BLOCKSIZE, id, PERCENTAGE, actualPercentage);
   printf("%f,%f,%f\n", (submitted-queued)*1e-6, (cl_start_time-submitted)*1e-6, (cl_end_time-cl_start_time)*1e-6); 

}
 
void LaunchOpenCL()
{
   // set seed for rand()
   srand(2012);
   
   // 
   unsigned int BlockSize = BLOCKSIZE;
   unsigned int BlockNumByEdge = WA / BlockSize;
   unsigned int BlockNum = BlockNumByEdge * BlockNumByEdge;
   /*
   unsigned int TileSize = TILESIZE;
   unsigned int TileNumByBlockEdge = BlockSize / TileSize;
   unsigned int TileNumByBlock = TileNumByBlockEdge * TileNumByBlockEdge;
   */

   // 1. allocate host memory for matrices A and B
   unsigned int size_A = WA * HA;
   unsigned int mem_size_A = sizeof(float) * size_A;
   float* h_A = (float*) malloc(mem_size_A);
 
   unsigned int size_B = WB * HB;
   unsigned int mem_size_B = sizeof(float) * size_B;
   float* h_B = (float*) malloc(mem_size_B);
   
   // allocate host memory for mask matrix
   unsigned int size_BL = BlockNumByEdge * BlockNumByEdge;
   unsigned int mem_size_BL = sizeof(int) * size_BL;
   int* h_BL = (int*)malloc(mem_size_BL);

 
   // 2. initialize host memory
   InitArrayFloat(h_A, size_A);
   InitArrayFloat(h_B, size_B);
   float skip_percentage = PERCENTAGE;
   float actualPercentage;
   InitBinaryMaskArrayPercentile(h_BL, size_BL, skip_percentage, &actualPercentage);
   /*
   int x;
   for (x = 0; x < size_BL; x++)
   {printf("%d \t", h_BL[x]);}
   */
   //printf("mask: %s", h_BL);
   // unsigned int skipstep = SKIP;
   //InitBinaryMaskArraySkipN(h_BL, size_BL, skipstep);

   // 4. allocate host memory for the result C
   unsigned int size_C = WC * HC;
   unsigned int mem_size_C = sizeof(float) * size_C;
   float* h_C = (float*) malloc(mem_size_C);
 
   // 5. Initialize OpenCL
   // OpenCL specific variables
   cl_context clContext;
   cl_command_queue clCommandQueue;
   cl_program clProgram;
   cl_kernel clKernel;
   cl_platform_id    cpPlatform; // OpenCL platform
  
   cl_int errcode;
 
   // OpenCL device memory for matrices
   cl_mem d_A;
   cl_mem d_B;
   cl_mem d_C;
   cl_mem d_BL;

   /*****************************************/
   /* Initialize OpenCL */
   /*****************************************/
   // Check Platform
   errcode = clGetPlatformIDs(1, &cpPlatform, NULL);
   if (errcode != CL_SUCCESS)
      FATAL("Failed to find an opencl platform!",errcode);

   // Get the devices
   cl_device_id device_id;
   errcode =clGetDeviceIDs(cpPlatform, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);
   if (errcode != CL_SUCCESS)
      FATAL("Failed to create a device group!",errcode);

   // Create a compute context 
   if(!(clContext = clCreateContext(0, 1, &device_id, NULL, NULL, &errcode)))
      FATAL("Failed to create a compute context!",errcode);
 
   //Create a command-queue
   clCommandQueue = clCreateCommandQueue(clContext, device_id, CL_QUEUE_PROFILING_ENABLE, &errcode);
   OpenCL_CheckError(errcode, "clCreateCommandQueue");
  
   // Setup device memory
   d_C = clCreateBuffer(clContext, CL_MEM_READ_WRITE, mem_size_A, NULL, &errcode);
   d_A = clCreateBuffer(clContext, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, mem_size_A, h_A, &errcode);
   d_B = clCreateBuffer(clContext, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, mem_size_B, h_B, &errcode);
   d_BL = clCreateBuffer(clContext, CL_MEM_READ_ONLY| CL_MEM_COPY_HOST_PTR, mem_size_BL, h_BL, &errcode);
   // 6. Load and build OpenCL kernel
   // Open the ptx file and load it
   // into a char* buffer

   //  Create program and kernel
   // Load Program source
   char *source = OpenCL_LoadProgramSource("matrixmultiplication.cl");
   if(!source)
      FATAL("Error: Failed to load compute program from file!\n",0);

   char clOptions[128]; 
   // propagate the LENGTH preprocessor symbol to the kernel compilation
   sprintf(clOptions, "-DBLOCK_SIZE=%d", BLOCKSIZE);

   // Create the compute program from the source buffer
   if(!(clProgram = clCreateProgramWithSource(clContext, 1, (const char **) & source, NULL, &errcode)))
        FATAL("Failed to create compute program!",errcode);

   // Build the program executable
   errcode = clBuildProgram(clProgram, 0, NULL, clOptions, NULL, NULL);
   if (errcode != CL_SUCCESS)
   {
        size_t len=2048;
        char buffer[2048];

        printf("Error: Failed to build program executable!\n");
        clGetProgramBuildInfo(clProgram, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);

        printf("%s \n", buffer);
        FATAL("Failed: program build", errcode);
   }

   clKernel = clCreateKernel(clProgram, "matrixMul", &errcode);
   OpenCL_CheckError(errcode, "clCreateKernel");
 
   // 7. Launch OpenCL kernel
   size_t localWorkSize[2], globalWorkSize[2];
 
   int wA = WA;
   int wC = WC;
   errcode = clSetKernelArg(clKernel, 0, sizeof(cl_mem), (void *)&d_C);
   errcode |= clSetKernelArg(clKernel, 1, sizeof(cl_mem), (void *)&d_A);
   errcode |= clSetKernelArg(clKernel, 2, sizeof(cl_mem), (void *)&d_B);
   errcode |= clSetKernelArg(clKernel, 3, sizeof(cl_mem), (void *)&d_BL);
   errcode |= clSetKernelArg(clKernel, 4, sizeof(int), (void *)&wA);
   errcode |= clSetKernelArg(clKernel, 5, sizeof(int), (void *)&wC);
   errcode |= clSetKernelArg(clKernel, 6, sizeof(int), (void *)&BlockSize);
   //errcode |= clSetKernelArg(clKernel, 7, sizeof(int), (void *)&TileSize);
   //errcode |= clSetKernelArg(clKernel, 8, sizeof(int), (void *)&TileNumByBlockEdge);
   OpenCL_CheckError(errcode, "clSetKernelArg");
 
   localWorkSize[0] = BLOCKSIZE;
   localWorkSize[1] = BLOCKSIZE;
   globalWorkSize[0] = WA;
   globalWorkSize[1] = WA;
 
   cl_event             event;
   errcode = clEnqueueNDRangeKernel(clCommandQueue, 
              clKernel, 2, NULL, globalWorkSize, 
              localWorkSize, 0, NULL, &event);
   errcode =  clWaitForEvents(1, &event);
   OpenCL_CheckError(errcode, "clEnqueueNDRangeKernel");
    
   PrintEventInfo(event, actualPercentage, (int) 1);

   errcode = clEnqueueNDRangeKernel(clCommandQueue,
   		clKernel, 2, NULL, globalWorkSize,
		localWorkSize, 0, NULL, &event);
   errcode =  clWaitForEvents(1, &event);
   OpenCL_CheckError(errcode, "clEnqueueNDRangeKernel");

   PrintEventInfo(event, actualPercentage, (int) 2);

   errcode = clEnqueueNDRangeKernel(clCommandQueue,
   		clKernel, 2, NULL, globalWorkSize,
		localWorkSize, 0, NULL, &event);
   errcode =  clWaitForEvents(1, &event);
   OpenCL_CheckError(errcode, "clEnqueueNDRangeKernel");
   PrintEventInfo(event, actualPercentage, (int)3);

   // 8. Retrieve result from device
   errcode = clEnqueueReadBuffer(clCommandQueue, 
              d_C, CL_TRUE, 0, mem_size_C, 
              h_C, 0, NULL, NULL);
   OpenCL_CheckError(errcode, "clEnqueueReadBuffer");
  
   // 10. clean up memory
   free(h_A);
   free(h_B);
   free(h_C);
   free(h_BL);
 
   clReleaseMemObject(d_A);
   clReleaseMemObject(d_C);
   clReleaseMemObject(d_B);
   clReleaseMemObject(d_BL);
 
   clReleaseContext(clContext);
   clReleaseKernel(clKernel);
   clReleaseProgram(clProgram);
   clReleaseCommandQueue(clCommandQueue);

}




// Top level routine
// 
int main (int argc, char **argv)
{
  OpenCL_ParseArguments(argc, argv);

  LaunchOpenCL();
}

