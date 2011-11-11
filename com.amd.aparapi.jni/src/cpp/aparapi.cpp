/*
Copyright (c) 2010-2011, Advanced Micro Devices, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions and the following
disclaimer. 

Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following
disclaimer in the documentation and/or other materials provided with the distribution. 

Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

If you use the software (in whole or in part), you shall adhere to all applicable U.S., European, and other export
laws, including but not limited to the U.S. Export Administration Regulations ("EAR"), (15 C.F.R. Sections 730 through
774), and E.U. Council Regulation (EC) No 1334/2000 of 22 June 2000.  Further, pursuant to Section 740.6 of the EAR,
you hereby certify that, except pursuant to a license granted by the United States Department of Commerce Bureau of 
Industry and Security or as otherwise permitted pursuant to a License Exception under the U.S. Export Administration 
Regulations ("EAR"), you will not (1) export, re-export or release to a national of a country in Country Groups D:1,
E:1 or E:2 any restricted technology, software, or source code you receive hereunder, or (2) export to Country Groups
D:1, E:1 or E:2 the direct product of such technology or software, if such foreign produced direct product is subject
to national security controls as identified on the Commerce Control List (currently found in Supplement 1 to Part 774
of EAR).  For the most current Country Group listings, or for additional information about the EAR or your obligations
under those regulations, please refer to the U.S. Bureau of Industry and Security�s website at http://www.bis.doc.gov/. 

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef __APPLE__
#include <malloc.h>
#endif

#include <sys/types.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#ifndef __APPLE__
#include <CL/cl.h>
#else
#include <cl.h>
#endif

#include <jni.h>

#define JNIExceptionChecker(){\
   fprintf(stderr, "line %d\n", __LINE__);\
   if ((jenv)->ExceptionOccurred()) {\
      (jenv)->ExceptionDescribe(); /* write to console */\
      (jenv)->ExceptionClear();\
   }\
}


#if defined (_WIN32)
#include "windows.h"
#define alignedMalloc(size, alignment)\
   _aligned_malloc(size, alignment)
#else
#define alignedMalloc(size, alignment)\
   memalign(alignment, size)
#endif


class MicrosecondTimer{

#if defined (_WIN32)
   private:
      __int64 freq;
      __int64 startValue;
   public:
      void start(){
         QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
         QueryPerformanceCounter((LARGE_INTEGER*)&startValue);
      }
      void end(char *msg){
         __int64 endValue;
         QueryPerformanceCounter((LARGE_INTEGER*)&endValue);
         int us = (int)((endValue-startValue)* 1000000.0 / freq);
         fprintf(stderr, "%s=%d\n", msg, us);
      }

#else
   public:
      void start(){
      }
      void end(char *msg){
      }

#endif
};

MicrosecondTimer timer;


#include "com_amd_aparapi_KernelRunner.h"

#define CHECK(condition, msg) if(condition){\
   fprintf(stderr, "!!!!!!! %s failed !!!!!!!\n", msg);\
   return 0;\
}


#define ASSERT_CL_NO_RETURN(msg) if (status != CL_SUCCESS){\
   fprintf(stderr, "!!!!!!! %s failed: %s\n", msg, CLErrString(status));\
}

#define ASSERT_CL(msg) if (status != CL_SUCCESS){\
   ASSERT_CL_NO_RETURN(msg)\
   return 0;\
}

#define PRINT_CL_ERR(status, msg) fprintf(stderr, "!!!!!!! %s failed %s\n", msg, CLErrString(status));

#define ASSERT_FIELD(id) CHECK(id##FieldID == 0, "No such field as " #id)

#define GET_DEV_INFO(deviceId, param, val, format){\
   status = clGetDeviceInfo(deviceId, param, sizeof(val), &(val), NULL);\
   ASSERT_CL_NO_RETURN( "clGetDeviceInfo().");\
   /*fprintf(stderr, #param " " format " \n", val);*/ \
}


jfieldID typeFieldID;
jfieldID nameFieldID;
jfieldID javaArrayFieldID;
jfieldID bytesPerLocalSizeFieldID;
jfieldID sizeInBytesFieldID;
jfieldID numElementsFieldID;

// we rely on these being 0 initially to detect whether we have cached the above fieldId's 
jclass clazz = (jclass)0;
jclass argClazz = (jclass)0;

static const char *CLErrString(cl_int status) {
   static struct { cl_int code; const char *msg; } error_table[] = {
      { CL_SUCCESS, "success" },
      { CL_DEVICE_NOT_FOUND, "device not found", },
      { CL_DEVICE_NOT_AVAILABLE, "device not available", },
      { CL_COMPILER_NOT_AVAILABLE, "compiler not available", },
      { CL_MEM_OBJECT_ALLOCATION_FAILURE, "mem object allocation failure", },
      { CL_OUT_OF_RESOURCES, "out of resources", },
      { CL_OUT_OF_HOST_MEMORY, "out of host memory", },
      { CL_PROFILING_INFO_NOT_AVAILABLE, "profiling not available", },
      { CL_MEM_COPY_OVERLAP, "memcopy overlaps", },
      { CL_IMAGE_FORMAT_MISMATCH, "image format mismatch", },
      { CL_IMAGE_FORMAT_NOT_SUPPORTED, "image format not supported", },
      { CL_BUILD_PROGRAM_FAILURE, "build program failed", },
      { CL_MAP_FAILURE, "map failed", },
      { CL_INVALID_VALUE, "invalid value", },
      { CL_INVALID_DEVICE_TYPE, "invalid device type", },
      { CL_INVALID_PLATFORM, "invlaid platform",},
      { CL_INVALID_DEVICE, "invalid device",},
      { CL_INVALID_CONTEXT, "invalid context",},
      { CL_INVALID_QUEUE_PROPERTIES, "invalid queue properties",},
      { CL_INVALID_COMMAND_QUEUE, "invalid command queue",},
      { CL_INVALID_HOST_PTR, "invalid host ptr",},
      { CL_INVALID_MEM_OBJECT, "invalid mem object",},
      { CL_INVALID_IMAGE_FORMAT_DESCRIPTOR, "invalid image format descriptor ",},
      { CL_INVALID_IMAGE_SIZE, "invalid image size",},
      { CL_INVALID_SAMPLER, "invalid sampler",},
      { CL_INVALID_BINARY, "invalid binary",},
      { CL_INVALID_BUILD_OPTIONS, "invalid build options",},
      { CL_INVALID_PROGRAM, "invalid program ",},
      { CL_INVALID_PROGRAM_EXECUTABLE, "invalid program executable",},
      { CL_INVALID_KERNEL_NAME, "invalid kernel name",},
      { CL_INVALID_KERNEL_DEFINITION, "invalid definition",},
      { CL_INVALID_KERNEL, "invalid kernel",},
      { CL_INVALID_ARG_INDEX, "invalid arg index",},
      { CL_INVALID_ARG_VALUE, "invalid arg value",},
      { CL_INVALID_ARG_SIZE, "invalid arg size",},
      { CL_INVALID_KERNEL_ARGS, "invalid kernel args",},
      { CL_INVALID_WORK_DIMENSION , "invalid work dimension",},
      { CL_INVALID_WORK_GROUP_SIZE, "invalid work group size",},
      { CL_INVALID_WORK_ITEM_SIZE, "invalid work item size",},
      { CL_INVALID_GLOBAL_OFFSET, "invalid global offset",},
      { CL_INVALID_EVENT_WAIT_LIST, "invalid event wait list",},
      { CL_INVALID_EVENT, "invalid event",},
      { CL_INVALID_OPERATION, "invalid operation",},
      { CL_INVALID_GL_OBJECT, "invalid gl object",},
      { CL_INVALID_BUFFER_SIZE, "invalid buffer size",},
      { CL_INVALID_MIP_LEVEL, "invalid mip level",},
      { CL_INVALID_GLOBAL_WORK_SIZE, "invalid global work size",},
      { 0, NULL },
   };
   static char unknown[25];
   int ii;

   for (ii = 0; error_table[ii].msg != NULL; ii++) {
      if (error_table[ii].code == status) {
         return error_table[ii].msg;
      }
   }
#ifdef _WIN32
   _snprintf(unknown, sizeof unknown, "unknown error %d", status);
#else
   snprintf(unknown, sizeof(unknown), "unknown error %d", status);
#endif
   return unknown;
}


class ProfileInfo{
   public:
      char label[128]; 
      cl_ulong queued;
      cl_ulong submit;
      cl_ulong start;
      cl_ulong end;
};


class KernelArgRef{
   public:
      jobject javaArray;        // The java array or direct buffer that this arg is mapped to 
      bool  isArray;            // true if above is an array
      cl_uint javaArrayLength;  // the number of elements for arrays (used only when ARRAYLENGTH bit is set for this arg)
      cl_mem mem;               // the opencl buffer 
      void *addr;               // we use this temporarily whilst we pin the primitive array
      cl_uint memMask;          // the mask we used for createBuffer
      jboolean isCopy;
      jboolean isPinned;
      char memSpec[128];       // The string form of the mask we used for create buffer. for debugging
      ProfileInfo read;
      ProfileInfo write;
};
class JNIContext ; // forward reference

class KernelArg{
   public:
      char *name;        // used for debugging printfs
      jfieldID fieldID;  // The field that this arg represents in the kernel (java), used only for primitive updates
      jint type;         // a bit mask determining the type of this arg
      jint sizeInBytes;  // bytes in the array or directBuf
      jobject javaArg;   // global reference to the corresponding java KernelArg 
      union{
         cl_char c;
         cl_double d;
         cl_float f;
         cl_int i;
         cl_long j;
         KernelArgRef ref;
      } value;

      void unpinAbort(JNIEnv *jenv){
         jenv->ReleasePrimitiveArrayCritical((jarray)value.ref.javaArray, value.ref.addr,JNI_ABORT);
      }
      void unpinCommit(JNIEnv *jenv){
         jenv->ReleasePrimitiveArrayCritical((jarray)value.ref.javaArray, value.ref.addr, 0);
      }
      void unpin(JNIEnv *jenv){
         if (isWrite()){
            // we only need to commit if the buffer has been written to
            // we use mode=0 in that case (rather than JNI_COMMIT) because that frees any copy buffer if it exists
            // in most cases this array will have been pinned so this will not be an issue
            unpinCommit(jenv);
         }else {
            // fast path for a read_only buffer
            unpinAbort(jenv);
         }
         value.ref.isPinned = JNI_FALSE;
      }
      void pin(JNIEnv *jenv){
         value.ref.addr = jenv->GetPrimitiveArrayCritical((jarray)value.ref.javaArray,&value.ref.isCopy);
         value.ref.isPinned = JNI_TRUE;
      }

      int isArray(){
         return(type&com_amd_aparapi_KernelRunner_ARG_ARRAY);
      }
      int isRead(){
         return(type&com_amd_aparapi_KernelRunner_ARG_READ);
      }
      int isWrite(){
         return(type&com_amd_aparapi_KernelRunner_ARG_WRITE);
      }
      int isExplicit(){
         return(type&com_amd_aparapi_KernelRunner_ARG_EXPLICIT);
      }
      int usesArrayLength(){
         return(type&com_amd_aparapi_KernelRunner_ARG_ARRAYLENGTH);
      }
      int isExplicitWrite(){
         return(type&com_amd_aparapi_KernelRunner_ARG_EXPLICIT_WRITE);
      }
      int isImplicit(){
         return(!isExplicit());
      }
      int isPrimitive(){
         return(type&com_amd_aparapi_KernelRunner_ARG_PRIMITIVE);
      }
      int isGlobal(){
         return(type&com_amd_aparapi_KernelRunner_ARG_GLOBAL);
      }
      int isFloat(){
         return(type&com_amd_aparapi_KernelRunner_ARG_FLOAT);
      }
      int isLong(){
         return (type&com_amd_aparapi_KernelRunner_ARG_LONG);
      }
      int isInt(){
         return (type&com_amd_aparapi_KernelRunner_ARG_INT);
      }
      int isDouble(){
         return (type&com_amd_aparapi_KernelRunner_ARG_DOUBLE);
      }
      int isBoolean(){
         return (type&com_amd_aparapi_KernelRunner_ARG_BOOLEAN);
      }
      int isByte(){
         return (type&com_amd_aparapi_KernelRunner_ARG_BYTE);
      }
      int isShort(){
         return (type&com_amd_aparapi_KernelRunner_ARG_SHORT);
      }
      int isLocal(){
         return (type&com_amd_aparapi_KernelRunner_ARG_LOCAL);
      }
      int isConstant(){
         return (type&com_amd_aparapi_KernelRunner_ARG_CONSTANT);
      }
      int isAparapiBuf(){
         return (type&com_amd_aparapi_KernelRunner_ARG_APARAPI_BUF);
      }
      int isAparapiBufHasArray(){
         return (type&com_amd_aparapi_KernelRunner_ARG_APARAPI_BUF_HAS_ARRAY);
      }
      int isAparapiBufIsDirect(){
         return (type&com_amd_aparapi_KernelRunner_ARG_APARAPI_BUF_IS_DIRECT);
      }
      int isBackedByArray(){
         return ( (isArray() && isGlobal()) || ((isGlobal() || isConstant()) && isAparapiBufHasArray()));
      }
      int mustReadBuffer(){
         return(((isArray() && isGlobal())||((isAparapiBuf()&&isGlobal())))&&(isImplicit()&&isWrite()));
      }
      int mustWriteBuffer(){
         return ((isImplicit()&&isRead()&&!isConstant())||(isExplicit()&&isExplicitWrite()));
      }

};

class JNIContext{
   private: 
      jint flags;
      jboolean valid;
      cl_platform_id platform;
      cl_platform_id* platforms;
      cl_uint platformc;
   public:
      JNIEnv *jenv;
      jobject kernelObject;
      jint numProcessors;
      jint maxJTPLocalSize;
      jclass kernelClass;
      cl_uint deviceIdc;
      cl_device_id* deviceIds;
      cl_int deviceType;
      cl_context context;
      cl_command_queue* commandQueues;
      cl_program program;
      cl_kernel kernel;
      jint argc;
      KernelArg** args;
      cl_event* executeEvents;
      cl_event* readEvents;
      cl_ulong profileBaseTime;
      jint* readEventArgs;
      cl_event* writeEvents;
      jint* writeEventArgs;
      jboolean firstRun;
      ProfileInfo exec;
      FILE* profileFile;
      // these map to camelCase form of CL_DEVICE_XXX_XXX  For example CL_DEVICE_MAX_COMPUTE_UNITS == maxComputeUnits
      cl_uint maxComputeUnits;
      cl_uint maxWorkItemDimensions;
      size_t maxWorkGroupSize;
      cl_ulong globalMemSize;
      cl_ulong localMemSize;

      static JNIContext* getJNIContext(jlong jniContextHandle){
         return((JNIContext*)jniContextHandle);
      }

      JNIContext(JNIEnv *_jenv, jobject _kernelObject, jint _flags, jint _numProcessors, jint _maxJTPLocalSize): 
         jenv(_jenv),
         kernelObject(jenv->NewGlobalRef(_kernelObject)),
         kernelClass((jclass)jenv->NewGlobalRef(jenv->GetObjectClass(_kernelObject))), 
         flags(_flags),
         numProcessors(_numProcessors),
         maxJTPLocalSize(_maxJTPLocalSize),
         platform(NULL),
         profileBaseTime(0),
         deviceType(((flags&com_amd_aparapi_KernelRunner_JNI_FLAG_USE_GPU)==com_amd_aparapi_KernelRunner_JNI_FLAG_USE_GPU)?CL_DEVICE_TYPE_GPU:CL_DEVICE_TYPE_CPU),
         profileFile(NULL), 
         valid(JNI_FALSE){
            cl_int status = CL_SUCCESS;

            // create the context using the mechanism described here
            // http://developer.amd.com/support/KnowledgeBase/Lists/KnowledgeBase/DispForm.aspx?ID=71
            
            // We may have one or more available platforms so determine how many we have 
            status = clGetPlatformIDs(0, NULL, &platformc);

            if (status == CL_SUCCESS && platformc >0) {
               platforms = new cl_platform_id[platformc];
               status = clGetPlatformIDs(platformc, platforms, NULL);
               if (status == CL_SUCCESS){
                  // iterate through platforms looking for an OpenCL 1.1 platform supporting required device (GPU|CPU)
                  // note that we exit the loop when we find a match so we find the first match
                  for (unsigned i = 0; platform == NULL && i < platformc; ++i) {
                     char platformVendorName[512];  
                     char platformVersionName[512];
                     status = clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, sizeof(platformVendorName), platformVendorName, NULL);
                     status = clGetPlatformInfo(platforms[i], CL_PLATFORM_VERSION, sizeof(platformVersionName), platformVersionName, NULL);
                     if (isVerbose()){
                        fprintf(stderr, "platform name    %d %s\n", i, platformVendorName); 
                        fprintf(stderr, "platform version %d %s\n", i, platformVersionName); 
                     }
                     // platformVendorName = "Advanced Micro Devices, Inc."||"NVIDIA Corporation"
                     // platformVersionName = "OpenCL 1.1 AMD-APP-SDK-v2.5 (684.213)"|"OpenCL 1.1 CUDA 4.0.1"
#ifndef __APPLE__
                     // Here we check if the platformVersionName starts with "OpenCL 1.1" (10 chars!) 
                     if (!strncmp(platformVersionName, "OpenCL 1.1", 10)) {
#else 
                     // Here we check if the platformVersionName starts with "OpenCL 1.1" or "OpenCL 1.0" (10 chars!) 
                     if (!strncmp(platformVersionName, "OpenCL 1.1", 10) || !strncmp(platformVersionName, "OpenCL 1.0", 10)) {
#endif
                        // Get the # of devices
                        status = clGetDeviceIDs(platforms[i], deviceType, 0, NULL, &deviceIdc);
                        // now check if this platform supports the requested device type (GPU or CPU)
                        if (status == CL_SUCCESS && deviceIdc >0 ){
                           platform = platforms[i];
                           if (isVerbose()){
                              fprintf(stderr, "platform %s supports requested device type\n", platformVendorName);
                           }

                           deviceIds = new cl_device_id[deviceIdc];
                           status = clGetDeviceIDs(platform, deviceType, deviceIdc, deviceIds, NULL);
                           if (status == CL_SUCCESS){
                              ASSERT_CL_NO_RETURN("clGetDeviceIDs()"); 
      
                              GET_DEV_INFO(deviceIds[0], CL_DEVICE_MAX_COMPUTE_UNITS, maxComputeUnits, "%d");
                              GET_DEV_INFO(deviceIds[0], CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, maxWorkItemDimensions, "%d");
                              GET_DEV_INFO(deviceIds[0], CL_DEVICE_MAX_WORK_GROUP_SIZE, maxWorkGroupSize, "%d");
                              GET_DEV_INFO(deviceIds[0], CL_DEVICE_GLOBAL_MEM_SIZE, globalMemSize, "%d");
                              GET_DEV_INFO(deviceIds[0], CL_DEVICE_LOCAL_MEM_SIZE, localMemSize, "%d");
                              if (isVerbose()){
      
                                 fprintf(stderr, "device[%p]: Type: ", deviceIds[0]);
                                 if (deviceType & CL_DEVICE_TYPE_DEFAULT) {
                                    //  deviceType &= ~CL_DEVICE_TYPE_DEFAULT;
                                    fprintf(stderr, "Default ");
                                 }else if (deviceType & CL_DEVICE_TYPE_CPU) {
                                    // deviceType &= ~CL_DEVICE_TYPE_CPU;
                                    fprintf(stderr, "CPU ");
                                 }else if (deviceType & CL_DEVICE_TYPE_GPU) {
                                    // deviceType &= ~CL_DEVICE_TYPE_GPU;
                                    fprintf(stderr, "GPU ");
                                 }else if (deviceType & CL_DEVICE_TYPE_ACCELERATOR) {
                                    // deviceType &= ~CL_DEVICE_TYPE_ACCELERATOR;
                                    fprintf(stderr, "Accelerator ");
                                 }else{
                                    fprintf(stderr, "Unknown (0x%llx) ", deviceType);
                                 }
                                 fprintf(stderr, "\n");
                              }
                              cl_context_properties cps[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };
                              cl_context_properties* cprops = (NULL == platform) ? NULL : cps;
                              context = clCreateContextFromType( cprops, deviceType, NULL, NULL, &status);
                              ASSERT_CL_NO_RETURN("clCreateContextFromType()");
                              if (status == CL_SUCCESS){
      
                                 valid = JNI_TRUE;
                              }
                           }
                        }else{
                           if (isVerbose()){
                              fprintf(stderr, "platform %s does not support requested device type skipping!\n", platformVendorName);
                           }
                        }

                     }else{
                         if (isVerbose()){
#ifndef __APPLE__
                            fprintf(stderr, "platform %s version %s is not OpenCL 1.1 skipping!\n", platformVendorName, platformVersionName);
#else
                            fprintf(stderr, "platform %s version %s is neither OpenCL 1.1 or OpenCL 1.0 skipping!\n", platformVendorName, platformVersionName);
#endif

                         }
                     }
                  }

               } 
            }else{
               if (isVerbose()){
                  fprintf(stderr, "no opencl platforms available!\n");
               }
            }

         }

      jboolean isValid(){
         return(valid);
      }
      jboolean isProfilingEnabled(){
         return((flags&com_amd_aparapi_KernelRunner_JNI_FLAG_ENABLE_PROFILING)==com_amd_aparapi_KernelRunner_JNI_FLAG_ENABLE_PROFILING?JNI_TRUE:JNI_FALSE);
      }
      jboolean isUsingGPU(){
         return((flags&com_amd_aparapi_KernelRunner_JNI_FLAG_USE_GPU)==com_amd_aparapi_KernelRunner_JNI_FLAG_USE_GPU?JNI_TRUE:JNI_FALSE);
      }
      jboolean isVerbose(){
         return((flags&com_amd_aparapi_KernelRunner_JNI_FLAG_ENABLE_VERBOSE_JNI)==com_amd_aparapi_KernelRunner_JNI_FLAG_ENABLE_VERBOSE_JNI?JNI_TRUE:JNI_FALSE);
      }

      ~JNIContext(){
         cl_int status = CL_SUCCESS;
         jenv->DeleteGlobalRef(kernelObject);
         jenv->DeleteGlobalRef(kernelClass);
         if (context != 0){
            status = clReleaseContext(context);
            ASSERT_CL_NO_RETURN("clReleaseContext()");
            context = (cl_context)0;
         }
         if (commandQueues){
            for (int dev=0; dev<deviceIdc; dev++){
               status = clReleaseCommandQueue((cl_command_queue)commandQueues[dev]);
               ASSERT_CL_NO_RETURN("clReleaseCommandQueue()");
               commandQueues[dev] = (cl_command_queue)0;
            }
            delete[] commandQueues; commandQueues = NULL;
         }
         if (program != 0){
            status = clReleaseProgram((cl_program)program);
            ASSERT_CL_NO_RETURN("clReleaseProgram()");
            program = (cl_program)0;
         }
         if (kernel != 0){
            status = clReleaseKernel((cl_kernel)kernel);
            ASSERT_CL_NO_RETURN("clReleaseKernel()");
            kernel = (cl_kernel)0;
         }
         if (platforms){
            delete []platforms; platforms=NULL;
         }
         if (deviceIds){
            delete [] deviceIds; deviceIds=NULL;
         }
         if (argc> 0){
            for (int i=0; i< argc; i++){
               KernelArg *arg = args[i];
               if (!arg->isPrimitive()){
                  if (arg->value.ref.mem != 0){
                     status = clReleaseMemObject((cl_mem)arg->value.ref.mem);
                     ASSERT_CL_NO_RETURN("clReleaseMemObject()");
                     arg->value.ref.mem = (cl_mem)0;
                  }
                  if (arg->value.ref.javaArray != NULL)  {
                     jenv->DeleteWeakGlobalRef((jweak) arg->value.ref.javaArray);
                  }
               }
               if (arg->name != NULL){
                  free(arg->name); arg->name = NULL;
               }
               if (arg->javaArg != NULL ) {
                  jenv->DeleteGlobalRef((jobject) arg->javaArg);
               }
               delete arg; arg=args[i]=NULL;
            }
            delete[] args; args=NULL;

            delete []readEvents; readEvents =NULL;
            delete []writeEvents; writeEvents = NULL;
            delete []executeEvents; executeEvents = NULL;

            if (isProfilingEnabled()) {
               if (profileFile != NULL && profileFile != stderr) {
                  fclose(profileFile);
               }
               delete[] readEventArgs; readEventArgs=0;
               delete[] writeEventArgs; writeEventArgs=0;
            } 
         }
      }

      /*
         Release JNI critical pinned arrays before returning to java code
         */
      void unpinAll() {
         for (int i=0; i< argc; i++){
            KernelArg *arg = args[i];
            if (arg->isBackedByArray()) {
               arg->unpin(jenv);
            }
         }
      }


};

jclass cacheKernelArgFields(JNIEnv *jenv, jobject jobj){
   jclass c = jenv->GetObjectClass(jobj); 
   nameFieldID = jenv->GetFieldID(c, "name", "Ljava/lang/String;"); ASSERT_FIELD(name);
   typeFieldID = jenv->GetFieldID(c, "type", "I"); ASSERT_FIELD(type);
   javaArrayFieldID = jenv->GetFieldID(c, "javaArray", "Ljava/lang/Object;"); ASSERT_FIELD(javaArray);
   bytesPerLocalSizeFieldID = jenv->GetFieldID(c, "bytesPerLocalSize", "I"); ASSERT_FIELD(bytesPerLocalSize);
   sizeInBytesFieldID = jenv->GetFieldID(c, "sizeInBytes", "I"); ASSERT_FIELD(sizeInBytes);
   numElementsFieldID = jenv->GetFieldID(c, "numElements", "I"); ASSERT_FIELD(numElements);
   return(c);
}

JNIEXPORT jint JNICALL Java_com_amd_aparapi_KernelRunner_disposeJNI(JNIEnv *jenv, jobject jobj, jlong jniContextHandle) {
   cl_int status = CL_SUCCESS;
   JNIContext* jniContext = JNIContext::getJNIContext(jniContextHandle);
   if (jniContext != NULL){
      delete jniContext;//free(jniContext);
      jniContext = NULL;
   }
   return(status);
}

void idump(char *str, void *ptr, int size){
   int * iptr = (int *)ptr;
   for (int i=0; i<size/sizeof(int); i++){
      fprintf(stderr, "%s%4d %d\n", str, i, iptr[i]);
   }
}

void fdump(char *str, void *ptr, int size){
   float * fptr = (float *)ptr;
   for (int i=0; i<size/sizeof(float); i++){
      fprintf(stderr, "%s%4d %6.2f\n", str, i, fptr[i]);
   }
}


jint writeProfileInfo(JNIContext* jniContext){
   cl_ulong currSampleBaseTime = -1;
   int pos = 1;

   if (jniContext->firstRun) {
      fprintf(jniContext->profileFile, "# PROFILE Name, queued, submit, start, end (microseconds)\n");
   }       

   // A read by a user kernel means the OpenCL layer wrote to the kernel and vice versa
   for (int i=0; i< jniContext->argc; i++){
      KernelArg *arg=jniContext->args[i];
      if (arg->isBackedByArray() && arg->isRead()){

         // Initialize the base time for this sample
         if (currSampleBaseTime == -1) {
            currSampleBaseTime = arg->value.ref.write.queued;
         } 

         if (jniContext->profileBaseTime == 0){
            jniContext->profileBaseTime = arg->value.ref.write.queued;

            // Write the base time as the first item in the csv
            //fprintf(jniContext->profileFile, "%llu,", jniContext->profileBaseTime);
         }

         fprintf(jniContext->profileFile, "%d write %s,", pos++, arg->name);

         fprintf(jniContext->profileFile, "%lu,%lu,%lu,%lu,",  
               (arg->value.ref.write.queued - currSampleBaseTime)/1000, 
               (arg->value.ref.write.submit - currSampleBaseTime)/1000, 
               (arg->value.ref.write.start - currSampleBaseTime)/1000, 
               (arg->value.ref.write.end - currSampleBaseTime)/1000);
      }
   }

   if (jniContext->profileBaseTime == 0){
      jniContext->profileBaseTime = jniContext->exec.queued;

      // Write the base time as the first item in the csv
      //fprintf(jniContext->profileFile, "%llu,", jniContext->profileBaseTime);
   }

   // Initialize the base time for this sample if necessary
   if (currSampleBaseTime == -1) {
      currSampleBaseTime = jniContext->exec.queued;
   } 

   // exec 
   fprintf(jniContext->profileFile, "%d exec,", pos++);

   fprintf(jniContext->profileFile, "%lu,%lu,%lu,%lu,",  
         (jniContext->exec.queued - currSampleBaseTime)/1000, 
         (jniContext->exec.submit - currSampleBaseTime)/1000, 
         (jniContext->exec.start - currSampleBaseTime)/1000, 
         (jniContext->exec.end - currSampleBaseTime)/1000);

   // 
   if ( jniContext->argc == 0 ) {
      fprintf(jniContext->profileFile, "\n");
   } else { 
      for (int i=0; i< jniContext->argc; i++){
         KernelArg *arg=jniContext->args[i];
         if (arg->isBackedByArray() && arg->isWrite()){
            if (jniContext->profileBaseTime == 0){
               jniContext->profileBaseTime = arg->value.ref.read.queued;

               // Write the base time as the first item in the csv
               //fprintf(jniContext->profileFile, "%llu,", jniContext->profileBaseTime);               
            }

            // Initialize the base time for this sample
            if (currSampleBaseTime == -1) {
               currSampleBaseTime = arg->value.ref.read.queued;
            }

            fprintf(jniContext->profileFile, "%d read %s,", pos++, arg->name);

            fprintf(jniContext->profileFile, "%lu,%lu,%lu,%lu,",  
                  (arg->value.ref.read.queued - currSampleBaseTime)/1000, 
                  (arg->value.ref.read.submit - currSampleBaseTime)/1000, 
                  (arg->value.ref.read.start - currSampleBaseTime)/1000, 
                  (arg->value.ref.read.end - currSampleBaseTime)/1000);
         }
      }
   }
   fprintf(jniContext->profileFile, "\n");
   return(0);
}

// Should failed profiling abort the run and return early?
cl_int profile(ProfileInfo *profileInfo, cl_event *event){
   cl_int status = CL_SUCCESS;
   status = clGetEventProfilingInfo(*event, CL_PROFILING_COMMAND_QUEUED, sizeof(profileInfo->queued), &(profileInfo->queued), NULL);
   ASSERT_CL( "clGetEventProfiliningInfo() QUEUED");
   status = clGetEventProfilingInfo(*event, CL_PROFILING_COMMAND_SUBMIT, sizeof(profileInfo->submit), &(profileInfo->submit), NULL);
   ASSERT_CL( "clGetEventProfiliningInfo() SUBMIT");
   status = clGetEventProfilingInfo(*event, CL_PROFILING_COMMAND_START, sizeof(profileInfo->start), &(profileInfo->start), NULL);
   ASSERT_CL( "clGetEventProfiliningInfo() START");
   status = clGetEventProfilingInfo(*event, CL_PROFILING_COMMAND_END, sizeof(profileInfo->end), &(profileInfo->end), NULL);
   ASSERT_CL( "clGetEventProfiliningInfo() END");
   return status;
}




jint updateKernel(JNIEnv *jenv, jobject jobj, JNIContext* jniContext) {
   cl_int status = CL_SUCCESS;
   if (jniContext != NULL){
      // we need to step through the array of KernelArg's to create the info required to create the cl_mem buffers.
      for (jint i=0; i<jniContext->argc; i++){ 
         KernelArg *arg=jniContext->args[i];

         arg->type = jenv->GetIntField(arg->javaArg, typeFieldID);
         if (jniContext->isVerbose()){
            fprintf(stderr, "got type for %s: %08x\n", arg->name, arg->type);
         }
         if (!arg->isPrimitive()) {
            // Following used for all primitive arrays, object arrays  and nio Buffers
            jarray newRef = (jarray)jenv->GetObjectField(arg->javaArg, javaArrayFieldID);
            if (jniContext->isVerbose()){

               fprintf(stderr, "testing for Resync javaArray %s: old=%p, new=%p\n", arg->name, arg->value.ref.javaArray, newRef);         
            }

            jboolean isSame = jenv->IsSameObject( newRef, arg->value.ref.javaArray);
            if (isSame == JNI_FALSE) {
               if (jniContext->isVerbose()){
                  fprintf(stderr, "Resync javaArray for %s: %p  %p\n", arg->name, newRef, arg->value.ref.javaArray);         
               }
               // Free previous ref if any
               if (arg->value.ref.javaArray != NULL) {
                  jenv->DeleteWeakGlobalRef((jweak) arg->value.ref.javaArray);
                  if (jniContext->isVerbose()){
                     fprintf(stderr, "DeleteWeakGlobalRef for %s: %p\n", arg->name, arg->value.ref.javaArray);         
                  }
               }

               // need to free opencl buffers, run will reallocate later
               if (arg->value.ref.mem != 0) {
                  //fprintf(stderr, "-->releaseMemObject[%d]\n", i);
                  status = clReleaseMemObject((cl_mem)arg->value.ref.mem);
                  //fprintf(stderr, "<--releaseMemObject[%d]\n", i);
                  ASSERT_CL("clReleaseMemObject()");
                  arg->value.ref.mem = (cl_mem)0;
               }

               arg->value.ref.mem = (cl_mem) 0;
               arg->value.ref.addr = NULL;

               // Capture new array ref from the kernel arg object

               if (newRef != NULL) {
                  arg->value.ref.javaArray = (jarray)jenv->NewWeakGlobalRef((jarray)newRef);
                  if (jniContext->isVerbose()){
                     fprintf(stderr, "NewWeakGlobalRef for %s, set to %p\n", arg->name,
                           arg->value.ref.javaArray);         
                  }
               } else {
                  arg->value.ref.javaArray = NULL;
               }
               arg->value.ref.isArray = !arg->isAparapiBufIsDirect();

               // Save the sizeInBytes which was set on the java side
               arg->sizeInBytes = jenv->GetIntField(arg->javaArg, sizeInBytesFieldID);

               if (jniContext->isVerbose()){
                  fprintf(stderr, "updateKernel, args[%d].sizeInBytes=%d\n", i, arg->sizeInBytes);
               }
            } // !is_same
         }
      } // for each arg
   } // if jniContext != NULL

   return(status);
}



JNIEXPORT jint JNICALL Java_com_amd_aparapi_KernelRunner_runKernelJNI(JNIEnv *jenv,
      jobject jobj, jlong jniContextHandle, jint globalSize, jint localSize, jboolean needSync,
      jboolean useNullForLocalSize, jint passes) {

   cl_int status = CL_SUCCESS;
   JNIContext* jniContext = JNIContext::getJNIContext(jniContextHandle);
   if (jniContext->isVerbose()){
      timer.start();
   }

   // Need to capture array refs
   if (jniContext->firstRun || needSync) {
      updateKernel(jenv, jobj, jniContext );
      if (jniContext->isVerbose()){
         fprintf(stderr, "back from updateKernel\n");
      }
   }

   int writeEventCount = 0;

   // kernelArgPos is used to keep track of the kernel arg position, it can 
   // differ from "i" due to insertion of javaArrayLength args which are not
   // fields read from the kernel object.
   int kernelArgPos = 0;

   for (int i=0; i< jniContext->argc; i++){
      KernelArg *arg = jniContext->args[i];
      // TODO: see if we can get rid of this read
      arg->type = jenv->GetIntField(arg->javaArg, typeFieldID);
      if (jniContext->isVerbose()){
         fprintf(stderr, "got type for arg %d, %s, type=%08x\n", i, arg->name, arg->type);
      }
      if (!arg->isPrimitive() && !arg->isLocal()) {
         // pin the arrays so that GC does not move them during the call

         // get the C memory address for the region being transferred
         // this uses different JNI calls for arrays vs. directBufs
         void * prevAddr =  arg->value.ref.addr;
         if (arg->value.ref.isArray) {
            arg->pin(jenv);
         } else if (arg->isAparapiBufIsDirect()) {
            // different call used for directbuffers
            arg->value.ref.addr = jenv->GetDirectBufferAddress(arg->value.ref.javaArray);
         }

         if (jniContext->isVerbose()){
            fprintf(stderr, "runKernel: arrayOrBuf ref %p, oldAddr=%p, newAddr=%p, ref.mem=%p, isArray=%d\n",
                  arg->value.ref.javaArray, 
                  prevAddr,
                  arg->value.ref.addr,
                  arg->value.ref.mem,
                  arg->value.ref.isArray );
            fprintf(stderr, "at memory addr %p, contents: ", arg->value.ref.addr);
            unsigned char *pb = (unsigned char *) arg->value.ref.addr;
            for (int k=0; k<8; k++) {
               fprintf(stderr, "%02x ", pb[k]);
            }
            fprintf(stderr, "\n" );
         }
         // record whether object moved 
         // if we see that isCopy was returned by getPrimitiveArrayCritical, treat that as a move
         bool objectMoved = (arg->value.ref.addr != prevAddr) || arg->value.ref.isCopy;

#ifdef VERBOSE_EXPLICIT
         if (arg->isExplicit() && arg->isExplicitWrite()){
            fprintf(stderr, "explicit write of %s\n",  arg->name);
         }
#endif

         if (jniContext->firstRun || (arg->value.ref.mem == 0) || objectMoved ){
            // if either this is the first run or user changed input array
            // or gc moved something, then we create buffers/args
            cl_uint mask = CL_MEM_USE_HOST_PTR;
            if (arg->isRead() && arg->isWrite()) mask |= CL_MEM_READ_WRITE;
            else if (arg->isRead() && !arg->isWrite()) mask |= CL_MEM_READ_ONLY;
            else if (arg->isWrite()) mask |= CL_MEM_WRITE_ONLY;
            arg->value.ref.memMask = mask;
            if (jniContext->isVerbose()){
               strcpy(arg->value.ref.memSpec,"CL_MEM_USE_HOST_PTR");
               if (mask & CL_MEM_READ_WRITE) strcat(arg->value.ref.memSpec,"|CL_MEM_READ_WRITE");
               if (mask & CL_MEM_READ_ONLY) strcat(arg->value.ref.memSpec,"|CL_MEM_READ_ONLY");
               if (mask & CL_MEM_WRITE_ONLY) strcat(arg->value.ref.memSpec,"|CL_MEM_WRITE_ONLY");

               fprintf(stderr, "%s %d clCreateBuffer(context, %s, size=%08x bytes, address=%08x, &status)\n", arg->name, 
                     i, arg->value.ref.memSpec, arg->sizeInBytes, arg->value.ref.addr);
            }
            arg->value.ref.mem = clCreateBuffer(jniContext->context, arg->value.ref.memMask, 
                  arg->sizeInBytes, arg->value.ref.addr, &status);

            if (status != CL_SUCCESS) {
               PRINT_CL_ERR(status, "clCreateBuffer");
               jniContext->unpinAll();
               return status;
            }

            status = clSetKernelArg(jniContext->kernel, kernelArgPos++, sizeof(cl_mem), (void *)&(arg->value.ref.mem));                  
            if (status != CL_SUCCESS) {
               PRINT_CL_ERR(status, "clSetKernelArg (array)");
               jniContext->unpinAll();
               return status;
            }

            // Add the array length if needed
            if (arg->usesArrayLength()){
               arg->value.ref.javaArrayLength = jenv->GetIntField(arg->javaArg, numElementsFieldID);
               status = clSetKernelArg(jniContext->kernel, kernelArgPos++, sizeof(jint), &(arg->value.ref.javaArrayLength));

               if (jniContext->isVerbose()){
                  fprintf(stderr, "runKernel arg %d %s, javaArrayLength = %d\n", i, arg->name, arg->value.ref.javaArrayLength);
               }
               if (status != CL_SUCCESS) {
                  PRINT_CL_ERR(status, "clSetKernelArg (array length)");
                  jniContext->unpinAll();
                  return status;
               }
            }
         } else {
            // Keep the arg position in sync if no updates were required
            kernelArgPos++;
            if (arg->usesArrayLength()){
               kernelArgPos++;
            }
         }

         // we only enqueue a write if we know the kernel actually reads the buffer or if there is an explicit write pending
         // the default behavior for Constant buffers is also that there is no write enqueued unless explicit

         if (arg->mustWriteBuffer()){
#ifdef VERBOSE_EXPLICIT
            if (arg->isExplicit() && arg->isExplicitWrite()){
               fprintf(stderr, "writing explicit buffer %d %s\n", i, arg->name);
            }
#endif
            if (jniContext->isVerbose()){
               fprintf(stderr, "%s writing buffer %d %s\n",  (arg->isExplicit() ? "explicitly" : ""), 
                     i, arg->name);
            }
            if (jniContext->isProfilingEnabled()) {
               jniContext->writeEventArgs[writeEventCount]=i;
            }

            status = clEnqueueWriteBuffer(jniContext->commandQueues[0], arg->value.ref.mem, CL_FALSE, 0, 
                  arg->sizeInBytes, arg->value.ref.addr, 0, NULL, &(jniContext->writeEvents[writeEventCount++]));
            if (status != CL_SUCCESS) {
               PRINT_CL_ERR(status, "clEnqueueWriteBuffer");
               jniContext->unpinAll();
               return status;
            }
            if (arg->isExplicit() && arg->isExplicitWrite()){
               arg->type &= ~com_amd_aparapi_KernelRunner_ARG_EXPLICIT_WRITE;
#ifdef VERBOSE_EXPLICIT
               fprintf(stderr, "clearing explicit buffer bit %d %s\n", i, arg->name);
#endif
               jenv->SetIntField(arg->javaArg, typeFieldID,arg->type );
            }
         }
      } else if (arg->isLocal()){
         if (jniContext->firstRun){
            // must multiply perlocalByteSize by localSize to get real opencl buffer size
            int bytesPerLocalSize = jenv->GetIntField(arg->javaArg, bytesPerLocalSizeFieldID);
            int adjustedLocalBufSize = bytesPerLocalSize * localSize;

            if (jniContext->isVerbose()){
               fprintf(stderr, "ISLOCAL, clSetKernelArg(jniContext->kernel, %d, %d, NULL);\n", i, adjustedLocalBufSize);
            }
            status = clSetKernelArg(jniContext->kernel, kernelArgPos++, adjustedLocalBufSize, NULL);
            if (status != CL_SUCCESS) {
               PRINT_CL_ERR(status, "clSetKernelArg() (local)");
               jniContext->unpinAll();
               return status;
            }
         } else {
            // Keep the arg position in sync if no updates were required
            kernelArgPos++;
            if (arg->usesArrayLength()){
               kernelArgPos++;
            }
         }
      }else{  // primitive arguments

         // we need to reflectively sync the value out of the kernel object
         if (arg->isFloat()){
            arg->value.f = jenv->GetFloatField(jniContext->kernelObject, arg->fieldID);
            //fprintf(stderr, "float arg %d\n", arg->value.f); 
         }else if (arg->isInt()){
            arg->value.i = jenv->GetIntField(jniContext->kernelObject, arg->fieldID);
            //fprintf(stderr, "int arg %d\n", arg->value.i); 
         }else if (arg->isBoolean()){
            arg->value.c = jenv->GetBooleanField(jniContext->kernelObject, arg->fieldID);
            //fprintf(stderr, "boolean arg %d\n", arg->value.c); 
         }else if (arg->isByte()){
            arg->value.c = jenv->GetByteField(jniContext->kernelObject, arg->fieldID);
            //fprintf(stderr, "byte arg %d\n", arg->value.c); 
         }else if (arg->isLong()){
            arg->value.j = jenv->GetLongField(jniContext->kernelObject, arg->fieldID);
            //fprintf(stderr, "long arg %d\n", arg->value.c); 
         }else if (arg->isDouble()){
            arg->value.d = jenv->GetDoubleField(jniContext->kernelObject, arg->fieldID);
            //fprintf(stderr, "double arg %d\n", arg->value.c); 
         }

         if (jniContext->isVerbose()){
            fprintf(stderr, "clSetKernelArg %s: %d %d %d 0x%08x\n", arg->name, i, kernelArgPos, 
                  arg->sizeInBytes, arg->value);         
         }
         status = clSetKernelArg(jniContext->kernel, kernelArgPos++, arg->sizeInBytes, &(arg->value));
         if (status != CL_SUCCESS) {
            PRINT_CL_ERR(status, "clSetKernelArg() (value)");
            jniContext->unpinAll();
            return status;
         }
      }
   }  // for each arg

   size_t globalSizeAsSizeT = (globalSize /jniContext->deviceIdc);
   size_t localSizeAsSizeT = localSize;

   // To support multiple passes we add a 'secret' final arg called 'passid' and just schedule multiple enqueuendrange kernels.  Each of which having a separate value of passid

   for (int passid=0; passid<passes; passid++){
      for (int dev =0; dev < jniContext->deviceIdc; dev++){
         size_t offset = (size_t)((globalSize/jniContext->deviceIdc)*dev);
         status = clSetKernelArg(jniContext->kernel, kernelArgPos, sizeof(passid), &(passid));
         if (status != CL_SUCCESS) {
            PRINT_CL_ERR(status, "clSetKernelArg() (passid)");
            jniContext->unpinAll();
            return status;
         }

         // four options here due to passid
         if (passid == 0 && passes==1){
            //fprintf(stderr, "setting passid to %d of %d first and last\n", passid, passes);
            // there is one pass and this is it
            // enqueue depends on write enqueues 
            // we don't block but and we populate the executeEvents
            status = clEnqueueNDRangeKernel(jniContext->commandQueues[dev], jniContext->kernel, 1, &offset, &globalSizeAsSizeT,
                  useNullForLocalSize ? NULL : &localSizeAsSizeT,
                  writeEventCount, writeEventCount?jniContext->writeEvents:NULL, &jniContext->executeEvents[dev]);
         }else if (passid == 0){
            //fprintf(stderr, "setting passid to %d of %d first not last\n", passid, passes);
            // this is the first of multiple passes
            // enqueue depends on write enqueues 
            // we block but do not populate executeEvents (only the last pass does this)
            status = clEnqueueNDRangeKernel(jniContext->commandQueues[dev], jniContext->kernel, 1, &offset, &globalSizeAsSizeT,
                  useNullForLocalSize ? NULL : &localSizeAsSizeT,
                  writeEventCount, writeEventCount?jniContext->writeEvents:NULL, &jniContext->executeEvents[dev]);

         }else if (passid < passes-1){
            // we are in some middle pass (neither first or last) 
            // we don't depend on write enqueues
            // we block and do not supply executeEvents (only the last pass does this)
            //fprintf(stderr, "setting passid to %d of %d not first not last\n", passid, passes);
            status = clEnqueueNDRangeKernel(jniContext->commandQueues[dev], jniContext->kernel, 1, &offset, &globalSizeAsSizeT,
                  useNullForLocalSize ? NULL : &localSizeAsSizeT, 0, NULL, &jniContext->executeEvents[dev]);
         }else{
            // we are the last pass of >1
            // we don't depend on write enqueues
            // we block and supply executeEvents
            //fprintf(stderr, "setting passid to %d of %d  last\n", passid, passes);
            status = clEnqueueNDRangeKernel(jniContext->commandQueues[dev], jniContext->kernel, 1, &offset, &globalSizeAsSizeT,
                  useNullForLocalSize ? NULL : &localSizeAsSizeT, 0, NULL, &jniContext->executeEvents[dev]);
         }


         if (status != CL_SUCCESS) {
            PRINT_CL_ERR(status, "clEnqueueNDRangeKernel()");
            fprintf(stderr, "after clEnqueueNDRangeKernel, globalSize=%d localSize=%d usingNull=%d\n", (int)globalSizeAsSizeT, (int)localSizeAsSizeT, useNullForLocalSize);
            jniContext->unpinAll();
            return status;
         }
      }
      if (passid < passes-1){
         // we need to wait for the executions to complete...
         status = clWaitForEvents(jniContext->deviceIdc,  jniContext->executeEvents);
         if (status != CL_SUCCESS) {
            PRINT_CL_ERR(status, "clWaitForEvents() execute events mid pass");
            jniContext->unpinAll();
            return status;
         }

         for (int dev = 0; dev < jniContext->deviceIdc; dev++){
            status = clReleaseEvent(jniContext->executeEvents[dev]);
            if (status != CL_SUCCESS) {
               PRINT_CL_ERR(status, "clReleaseEvent() read event");
               jniContext->unpinAll();
               return status;
            }
         }
      }
   }

   int readEventCount = 0;

   for (int i=0; i< jniContext->argc; i++){
      KernelArg *arg = jniContext->args[i];

      if (arg->mustReadBuffer()){
         if (jniContext->isProfilingEnabled()) {
            jniContext->readEventArgs[readEventCount]=i;
         }
         if (jniContext->isVerbose()){
            fprintf(stderr, "reading buffer %d %s\n", i, arg->name);
         }

         status = clEnqueueReadBuffer(jniContext->commandQueues[0], arg->value.ref.mem, CL_FALSE, 0, 
               arg->sizeInBytes,arg->value.ref.addr , jniContext->deviceIdc, jniContext->executeEvents, &(jniContext->readEvents[readEventCount++]));
         if (status != CL_SUCCESS) {
            PRINT_CL_ERR(status, "clEnqueueReadBuffer()");
            jniContext->unpinAll();
            return status;
         }
      }
   }

   // don't change the order here
   // We wait for the reads which each depend on the execution, which depends on the writes ;)
   // So after the reads have completed, we can release the execute and writes.

   if (readEventCount >0){
      status = clWaitForEvents(readEventCount, jniContext->readEvents);
      if (status != CL_SUCCESS) {
         PRINT_CL_ERR(status, "clWaitForEvents() read events");
         jniContext->unpinAll();
         return status;
      }

      for (int i=0; i< readEventCount; i++){
         if (jniContext->isProfilingEnabled()) {
            status = profile(&jniContext->args[jniContext->readEventArgs[i]]->value.ref.read, &jniContext->readEvents[i]);
            if (status != CL_SUCCESS) {
               jniContext->unpinAll();
               return status;
            }
         }
         status = clReleaseEvent(jniContext->readEvents[i]);
         if (status != CL_SUCCESS) {
            PRINT_CL_ERR(status, "clReleaseEvent() read event");
            jniContext->unpinAll();
            return status;
         }
      }
   } else {
      // if readEventCount == 0 then we don't need any reads so we just wait for the executions to complete
      status = clWaitForEvents(jniContext->deviceIdc, jniContext->executeEvents);
      if (status != CL_SUCCESS) {
         PRINT_CL_ERR(status, "clWaitForEvents() execute event");
         jniContext->unpinAll();
         return status;
      }
   }

   if (jniContext->isProfilingEnabled()) {
      status = profile(&jniContext->exec, &jniContext->executeEvents[0]);
      if (status != CL_SUCCESS) {
         jniContext->unpinAll();
         return status;
      }
   }

   // extract the execution status from the executeEvent
   cl_int executeStatus;
   status = clGetEventInfo(jniContext->executeEvents[0], CL_EVENT_COMMAND_EXECUTION_STATUS, sizeof(cl_int), &executeStatus, NULL);
   if (status != CL_SUCCESS) {
      PRINT_CL_ERR(status, "clGetEventInfo() execute event");
      jniContext->unpinAll();
      return status;
   }
   if (executeStatus != CL_SUCCESS) {
      // it should definitely not be negative, but since we did a wait above, it had better be CL_COMPLETE==CL_SUCCESS
      PRINT_CL_ERR(executeStatus, "Execution status of execute event");
      jniContext->unpinAll();
      return executeStatus;
   }

   for (int dev=0; dev<jniContext->deviceIdc; dev++){

      status = clReleaseEvent(jniContext->executeEvents[dev]);
      if (status != CL_SUCCESS) {
         PRINT_CL_ERR(status, "clReleaseEvent() execute event");
         jniContext->unpinAll();
         return status;
      }
   }

   for (int i=0; i< writeEventCount; i++){
      if (jniContext->isProfilingEnabled()) {
         profile(&jniContext->args[jniContext->writeEventArgs[i]]->value.ref.write, &jniContext->writeEvents[i]);
      }
      status = clReleaseEvent(jniContext->writeEvents[i]);
      if (status != CL_SUCCESS) {
         PRINT_CL_ERR(status, "clReleaseEvent() write event");
         jniContext->unpinAll();
         return status;
      }
   }

   jniContext->unpinAll();

   if (jniContext->isProfilingEnabled()) {
      writeProfileInfo(jniContext);
   }

   jniContext->firstRun = false;
   if (jniContext->isVerbose()){
      timer.end("elapsed");
   }

   //fprintf(stderr, "About to return %d from exec\n", status);
   return(status);
}


// we return the JNIContext from here 
JNIEXPORT jlong JNICALL Java_com_amd_aparapi_KernelRunner_initJNI(JNIEnv *jenv, jclass clazz, jobject kernelObject, 
      jint flags, jint numProcessors,
      jint maxJTPLocalSize) {
   cl_int status = CL_SUCCESS;
   JNIContext* jniContext = new JNIContext(jenv, kernelObject, flags, numProcessors, maxJTPLocalSize);
   if (jniContext->isValid()){
      return((jlong)jniContext);
   }else{
      return(0L);
   }
}


JNIEXPORT jlong JNICALL Java_com_amd_aparapi_KernelRunner_buildProgramJNI(JNIEnv *jenv, jobject jobj, jlong jniContextHandle, jstring source) {
   JNIContext* jniContext = JNIContext::getJNIContext(jniContextHandle);
   if (jniContext == NULL){
      return 0;
   }

   cl_int status = CL_SUCCESS;
   const char *sourceChars = jenv->GetStringUTFChars(source, NULL);
   CHECK(sourceChars == NULL, "jenv->GetStringUTFChars() returned null" );

   size_t sourceSize[] = { strlen(sourceChars) };
   jniContext->program = clCreateProgramWithSource( jniContext->context, 1, &sourceChars, sourceSize, &status); 
   jenv->ReleaseStringUTFChars(source, sourceChars);
   ASSERT_CL("clCreateProgramWithSource()");

   status = clBuildProgram(jniContext->program, jniContext->deviceIdc, jniContext->deviceIds, NULL, NULL, NULL);

   if(status == CL_BUILD_PROGRAM_FAILURE) {
      cl_int logStatus;
      size_t buildLogSize = 0;
      status = clGetProgramBuildInfo(jniContext->program, jniContext->deviceIds[0], 
            CL_PROGRAM_BUILD_LOG, buildLogSize, NULL, &buildLogSize);
      ASSERT_CL("clGetProgramBuildInfo()");
      char * buildLog = new char[buildLogSize];
      CHECK(buildLog == NULL, "Failed to allocate host memory. (buildLog)");
      memset(buildLog, 0, buildLogSize);
      status = clGetProgramBuildInfo (jniContext->program, jniContext->deviceIds[0], 
            CL_PROGRAM_BUILD_LOG, buildLogSize, buildLog, NULL);
      ASSERT_CL("clGetProgramBuildInfo()");

      fprintf(stderr, "clBuildProgram failed");
      fprintf(stderr, "\n************************************************\n");
      fprintf(stderr, "%s", buildLog);
      fprintf(stderr, "\n************************************************\n\n\n");
      delete []buildLog;
      return(0);
   }

   jniContext->kernel = clCreateKernel(jniContext->program, "run", &status);
   ASSERT_CL("clCreateKernel()");

   cl_command_queue_properties queue_props = 0;
   if (jniContext->isProfilingEnabled()) {
      queue_props |= CL_QUEUE_PROFILING_ENABLE;
   }

   jniContext->commandQueues= new cl_command_queue[jniContext->deviceIdc];
   for (int dev=0; dev < jniContext->deviceIdc; dev++){
      jniContext->commandQueues[dev]=clCreateCommandQueue(jniContext->context, (cl_device_id)jniContext->deviceIds[dev],
            queue_props,
            &status);
      ASSERT_CL("clCreateCommandQueue()");
   }

   if (jniContext->isProfilingEnabled()) {
      // compute profile filename
#if defined (_WIN32)
      jint pid = GetCurrentProcessId();
#else
      pid_t pid = getpid();
#endif
      // indicate cpu or gpu
      // timestamp
      // kernel name

      jclass classMethodAccess = jenv->FindClass("java/lang/Class"); 
      jmethodID getNameID=jenv->GetMethodID(classMethodAccess,"getName","()Ljava/lang/String;");
      jstring className = (jstring)jenv->CallObjectMethod(jniContext->kernelClass, getNameID);
      const char *classNameChars = jenv->GetStringUTFChars(className, NULL);

#define TIME_STR_LEN 200

      char timeStr[TIME_STR_LEN];
      struct tm *tmp;
      time_t t = time(NULL);
      tmp = localtime(&t);
      if (tmp == NULL) {
         perror("localtime");
      }
      //strftime(timeStr, TIME_STR_LEN, "%F.%H%M%S", tmp);  %F seemed to cause a core dump
      strftime(timeStr, TIME_STR_LEN, "%H%M%S", tmp);

      char* fnameStr = new char[strlen(classNameChars) + strlen(timeStr) + 128];

      //sprintf(fnameStr, "%s.%s.%d.%llx\n", classNameChars, timeStr, pid, jniContext);
      sprintf(fnameStr, "aparapiprof.%s.%d.%016lx", timeStr, pid, (unsigned long)jniContext);
      jenv->ReleaseStringUTFChars(className, classNameChars);

      FILE* profileFile = fopen(fnameStr, "w");
      if (profileFile != NULL) {
         jniContext->profileFile = profileFile;
      } else {
         jniContext->profileFile = stderr;
         fprintf(stderr, "Could not open profile data file %s, reverting to stderr\n", fnameStr);
      }
      delete []fnameStr;
   }

   return((jlong)jniContext);
}


// this is called once when the arg list is first determined for this kernel
JNIEXPORT jint JNICALL Java_com_amd_aparapi_KernelRunner_setArgsJNI(JNIEnv *jenv, jobject jobj, jlong jniContextHandle, jobjectArray argArray, jint argc) {
   JNIContext* jniContext = JNIContext::getJNIContext(jniContextHandle);
   cl_int status = CL_SUCCESS;
   if (jniContext != NULL){      
      jniContext->argc = argc;
      jniContext->args = new KernelArg*[jniContext->argc];
      jniContext->firstRun = true;

      // Step through the array of KernelArg's to capture the type data for the Kernel's data members.
      for (jint i=0; i<jniContext->argc; i++){ 
         KernelArg* arg = jniContext->args[i] = new KernelArg;


         jobject argObj = jenv->GetObjectArrayElement(argArray, i);
         if (argClazz == 0){
            argClazz = cacheKernelArgFields(jenv, argObj);
         }
         arg->javaArg = jenv->NewGlobalRef(argObj);   // save a global ref to the java Arg Object

         arg->type = jenv->GetIntField(argObj, typeFieldID);
         jstring name  = (jstring)jenv->GetObjectField(argObj, nameFieldID);
         const char *nameChars = jenv->GetStringUTFChars(name, NULL);
         arg->name=strdup(nameChars);
         jenv->ReleaseStringUTFChars(name, nameChars);
#ifdef VERBOSE_EXPLICIT
         if (arg->isExplicit()){
            fprintf(stderr, "%s is explicit!\n", arg->name);
         }
#endif

         if (arg->isPrimitive()) {
            // for primitives, we cache the fieldID for that field in the kernel's arg object
            if (arg->isFloat()){
               arg->fieldID = jenv->GetFieldID(jniContext->kernelClass, arg->name, "F");
               arg->sizeInBytes = sizeof(jfloat);
            }else if (arg->isInt()){
               arg->fieldID = jenv->GetFieldID(jniContext->kernelClass, arg->name, "I");
               arg->sizeInBytes = sizeof(jint);
            }else if (arg->isByte()){
               arg->fieldID = jenv->GetFieldID(jniContext->kernelClass, arg->name, "B");
               arg->sizeInBytes = sizeof(jbyte);
            }else if (arg->isBoolean()){
               arg->fieldID = jenv->GetFieldID(jniContext->kernelClass, arg->name, "Z");
               arg->sizeInBytes = sizeof(jboolean);
            }else if (arg->isLong()){
               arg->fieldID = jenv->GetFieldID(jniContext->kernelClass, arg->name, "J");
               arg->sizeInBytes = sizeof(jlong);
            }else if (arg->isDouble()){
               arg->fieldID = jenv->GetFieldID(jniContext->kernelClass, arg->name, "D");
               arg->sizeInBytes = sizeof(jdouble);
            }
         }else{ // we will use an array
            arg->value.ref.mem = (cl_mem) 0;
            arg->value.ref.javaArray = 0;
            arg->sizeInBytes = 0;
         }
         if (jniContext->isVerbose()){
            fprintf(stderr, "in setArgs arg %d %s type %08x\n", i, arg->name, arg->type);
         }

      }
      // we will need an executeEvent buffer for all devices
      jniContext->executeEvents = new cl_event[jniContext->deviceIdc];

      // We will need *at most* jniContext->argc read/write events
      jniContext->readEvents = new cl_event[jniContext->argc];
      if (jniContext->isProfilingEnabled()) {
         jniContext->readEventArgs = new jint[jniContext->argc];
      }
      jniContext->writeEvents = new cl_event[jniContext->argc];
      if (jniContext->isProfilingEnabled()) {
         jniContext->writeEventArgs = new jint[jniContext->argc];
      }
   }
   return(status);
}

JNIEXPORT jint JNICALL Java_com_amd_aparapi_KernelRunner_getLocalSizeJNI(JNIEnv *jenv, jobject jobj, jlong jniContextHandle, jint globalSize, jint localBytesPerLocalId) {
   size_t kernelMaxWorkGroupSize = 0;
   size_t kernelWorkGroupSizeMultiple = 0;
   cl_int status = CL_SUCCESS;
   JNIContext* jniContext = JNIContext::getJNIContext(jniContextHandle);
   if (jniContext != NULL){
      clGetKernelWorkGroupInfo(jniContext->kernel, jniContext->deviceIds[0], CL_KERNEL_WORK_GROUP_SIZE, sizeof(kernelMaxWorkGroupSize), &kernelMaxWorkGroupSize, NULL);
      ASSERT_CL("clGetKernelWorkGroupInfo()");
      // starting value depends on device type
      // not sure why the CPU has a different starting size, but it does
      int startLocalSize = (jniContext->deviceType == CL_DEVICE_TYPE_GPU ? kernelMaxWorkGroupSize : globalSize/(jniContext->numProcessors*4));

      if (startLocalSize == 0) startLocalSize = 1;
      if (startLocalSize > kernelMaxWorkGroupSize) startLocalSize = kernelMaxWorkGroupSize;
      if (startLocalSize > globalSize) startLocalSize = globalSize;
      // if the kernel uses any local memory, determine our max local memory size so we can possibly limit localsize
      cl_ulong devLocalMemSize;
      if (localBytesPerLocalId > 0) {
         status = clGetDeviceInfo(jniContext->deviceIds[0], CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong), &devLocalMemSize, NULL);
         cl_uint localSizeLimitFromLocalMem = devLocalMemSize/localBytesPerLocalId;
         if (startLocalSize > localSizeLimitFromLocalMem) startLocalSize = localSizeLimitFromLocalMem;
         if (jniContext->isVerbose()){
            fprintf(stderr, "localBytesPerLocalId=%d, device localMemMax=%d, localSizeLimitFromLocalMem=%d\n",
                  localBytesPerLocalId, (cl_uint) devLocalMemSize, localSizeLimitFromLocalMem);
         }

      }

      // then iterate down until we find a localSize that divides globalSize equally
      for (int localSize = startLocalSize; localSize>0; localSize--) {
         if (globalSize % localSize == 0) {
            if (jniContext->isVerbose()){
               fprintf(stderr, "for globalSize=%d, stepping localSize from %d, returning localSize=%d\n", globalSize, startLocalSize, localSize);
            }
            return localSize;
         }
      }
   }
   // should never get this far
   return 0;
}

JNIEXPORT jstring JNICALL Java_com_amd_aparapi_KernelRunner_getExtensions(JNIEnv *jenv, jobject jobj, jlong jniContextHandle) {
   jstring jextensions = NULL;
   JNIContext* jniContext = JNIContext::getJNIContext(jniContextHandle);
   if (jniContext != NULL){
      size_t retvalsize = 0;
      cl_int status = CL_SUCCESS;
      status = clGetDeviceInfo(jniContext->deviceIds[0], CL_DEVICE_EXTENSIONS, 0, NULL, &retvalsize);
      ASSERT_CL("clGetDeviceInfo()");
      char* extensions = new char[retvalsize];
      clGetDeviceInfo(jniContext->deviceIds[0], CL_DEVICE_EXTENSIONS, retvalsize, extensions, NULL);
      jextensions = jenv->NewStringUTF(extensions);
      delete [] extensions;
   }
   return jextensions;
}

// Called as a result of Kernel.get(someArray)
JNIEXPORT jint JNICALL Java_com_amd_aparapi_KernelRunner_getJNI(JNIEnv *jenv, jobject jobj, jlong jniContextHandle, jobject buffer) {
   cl_int status = CL_SUCCESS;
   JNIContext* jniContext = JNIContext::getJNIContext(jniContextHandle);
   if (jniContext != NULL){
      jboolean foundArg = false;
      for (jint i=0; i<jniContext->argc; i++){ 
         KernelArg *arg= jniContext->args[i];
         if (arg->isArray()){
            jboolean isSame = jenv->IsSameObject(buffer, arg->value.ref.javaArray);
            // only do this if the array that we are passed is indeed an arg we are tracking
            if (isSame){
               foundArg = true;
               //fprintf(stderr, "get of %s\n", arg->name);

#ifdef VERBOSE_EXPLICIT
               fprintf(stderr, "explicitly reading buffer %d %s\n", i, arg->name);
#endif
               arg->pin(jenv);

               status = clEnqueueReadBuffer(jniContext->commandQueues[0], arg->value.ref.mem, CL_FALSE, 0, 
                     arg->sizeInBytes,arg->value.ref.addr , 0, NULL, &jniContext->readEvents[0]);
               if (status != CL_SUCCESS) {
                  PRINT_CL_ERR(status, "clEnqueueReadBuffer()");
                  return status;
               }
               status = clWaitForEvents(1, jniContext->readEvents);
               if (status != CL_SUCCESS) {
                  PRINT_CL_ERR(status, "clWaitForEvents");
                  return status;
               }
               clReleaseEvent(jniContext->readEvents[0]);
               if (status != CL_SUCCESS) {
                  PRINT_CL_ERR(status, "clReleaseEvent() read event");
                  return status;
               }
               // since this is an explicit buffer get, we expect the buffer to have changed so we commit
               arg->unpinCommit(jenv);
            }
         }
      }
      if (!foundArg){
         if (jniContext->isVerbose()){
            fprintf(stderr, "attempt to request to get a buffer that does not appear to be referenced from kernel\n");
         }
      }
   }
   return 0;
}



