/* clang-format off */
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <cuda.h>


// helpers to check for cuda errors
#define CUDA_CHECK(ans) {\
    gpuAssert((ans), __FILE__, __LINE__);\
  }\

static inline void gpuAssert(CUresult code, const char *file, int line) {
  if (code != CUDA_SUCCESS) {
    const char *prefix = "Triton Error [CUDA]: ";
    const char *str;
    cuGetErrorString(code, &str);
    char err[1024] = {0};
    strcat(err, prefix);
    strcat(err, str);
    printf("%s\\n", err);
    exit(code);
  }
}

// globals
#define CUBIN_NAME add_kernel_fp32_3fbfba5d_0123_cubin
CUmodule add_kernel_fp32_3fbfba5d_0123_mod = NULL;
CUfunction add_kernel_fp32_3fbfba5d_0123_func = NULL;
unsigned char CUBIN_NAME[10192] = { 0x7f, 0x45, 0x4c, 0x46, 0x02, 0x01, 0x01, 0x33, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0xbe, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x05, 0x50, 0x00, 0x40, 0x00, 0x38, 0x00, 0x03, 0x00, 0x40, 0x00, 0x11, 0x00, 0x01, 0x00, 0x00, 0x2e, 0x73, 0x68, 0x73, 0x74, 0x72, 0x74, 0x61, 0x62, 0x00, 0x2e, 0x73, 0x74, 0x72, 0x74, 0x61, 0x62, 0x00, 0x2e, 0x73, 0x79, 0x6d, 0x74, 0x61, 0x62, 0x00, 0x2e, 0x73, 0x79, 0x6d, 0x74, 0x61, 0x62, 0x5f, 0x73, 0x68, 0x6e, 0x64, 0x78, 0x00, 0x2e, 0x6e, 0x76, 0x2e, 0x69, 0x6e, 0x66, 0x6f, 0x00, 0x2e, 0x74, 0x65, 0x78, 0x74, 0x2e, 0x61, 0x64, 0x64, 0x5f, 0x6b, 0x65, 0x72, 0x6e, 0x65, 0x6c, 0x00, 0x2e, 0x6e, 0x76, 0x2e, 0x69, 0x6e, 0x66, 0x6f, 0x2e, 0x61, 0x64, 0x64, 0x5f, 0x6b, 0x65, 0x72, 0x6e, 0x65, 0x6c, 0x00, 0x2e, 0x6e, 0x76, 0x2e, 0x73, 0x68, 0x61, 0x72, 0x65, 0x64, 0x2e, 0x61, 0x64, 0x64, 0x5f, 0x6b, 0x65, 0x72, 0x6e, 0x65, 0x6c, 0x00, 0x2e, 0x6e, 0x76, 0x2e, 0x63, 0x6f, 0x6e, 0x73, 0x74, 0x61, 0x6e, 0x74, 0x30, 0x2e, 0x61, 0x64, 0x64, 0x5f, 0x6b, 0x65, 0x72, 0x6e, 0x65, 0x6c, 0x00, 0x2e, 0x72, 0x65, 0x6c, 0x2e, 0x6e, 0x76, 0x2e, 0x63, 0x6f, 0x6e, 0x73, 0x74, 0x61, 0x6e, 0x74, 0x30, 0x2e, 0x61, 0x64, 0x64, 0x5f, 0x6b, 0x65, 0x72, 0x6e, 0x65, 0x6c, 0x00, 0x2e, 0x64, 0x65, 0x62, 0x75, 0x67, 0x5f, 0x66, 0x72, 0x61, 0x6d, 0x65, 0x00, 0x2e, 0x64, 0x65, 0x62, 0x75, 0x67, 0x5f, 0x6c, 0x69, 0x6e, 0x65, 0x00, 0x2e, 0x72, 0x65, 0x6c, 0x2e, 0x64, 0x65, 0x62, 0x75, 0x67, 0x5f, 0x6c, 0x69, 0x6e, 0x65, 0x00, 0x2e, 0x6e, 0x76, 0x5f, 0x64, 0x65, 0x62, 0x75, 0x67, 0x5f, 0x6c, 0x69, 0x6e, 0x65, 0x5f, 0x73, 0x61, 0x73, 0x73, 0x00, 0x2e, 0x72, 0x65, 0x6c, 0x2e, 0x6e, 0x76, 0x5f, 0x64, 0x65, 0x62, 0x75, 0x67, 0x5f, 0x6c, 0x69, 0x6e, 0x65, 0x5f, 0x73, 0x61, 0x73, 0x73, 0x00, 0x2e, 0x6e, 0x76, 0x5f, 0x64, 0x65, 0x62, 0x75, 0x67, 0x5f, 0x70, 0x74, 0x78, 0x5f, 0x74, 0x78, 0x74, 0x00, 0x2e, 0x72, 0x65, 0x6c, 0x2e, 0x64, 0x65, 0x62, 0x75, 0x67, 0x5f, 0x66, 0x72, 0x61, 0x6d, 0x65, 0x00, 0x2e, 0x72, 0x65, 0x6c, 0x61, 0x2e, 0x64, 0x65, 0x62, 0x75, 0x67, 0x5f, 0x66, 0x72, 0x61, 0x6d, 0x65, 0x00, 0x2e, 0x6e, 0x76, 0x2e, 0x63, 0x61, 0x6c, 0x6c, 0x67, 0x72, 0x61, 0x70, 0x68, 0x00, 0x2e, 0x6e, 0x76, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x74, 0x79, 0x70, 0x65, 0x00, 0x2e, 0x6e, 0x76, 0x2e, 0x72, 0x65, 0x6c, 0x2e, 0x61, 0x63, 0x74, 0x69, 0x6f, 0x6e, 0x00, 0x00, 0x2e, 0x73, 0x68, 0x73, 0x74, 0x72, 0x74, 0x61, 0x62, 0x00, 0x2e, 0x73, 0x74, 0x72, 0x74, 0x61, 0x62, 0x00, 0x2e, 0x73, 0x79, 0x6d, 0x74, 0x61, 0x62, 0x00, 0x2e, 0x73, 0x79, 0x6d, 0x74, 0x61, 0x62, 0x5f, 0x73, 0x68, 0x6e, 0x64, 0x78, 0x00, 0x2e, 0x6e, 0x76, 0x2e, 0x69, 0x6e, 0x66, 0x6f, 0x00, 0x2e, 0x74, 0x65, 0x78, 0x74, 0x2e, 0x61, 0x64, 0x64, 0x5f, 0x6b, 0x65, 0x72, 0x6e, 0x65, 0x6c, 0x00, 0x2e, 0x6e, 0x76, 0x2e, 0x69, 0x6e, 0x66, 0x6f, 0x2e, 0x61, 0x64, 0x64, 0x5f, 0x6b, 0x65, 0x72, 0x6e, 0x65, 0x6c, 0x00, 0x2e, 0x6e, 0x76, 0x2e, 0x73, 0x68, 0x61, 0x72, 0x65, 0x64, 0x2e, 0x61, 0x64, 0x64, 0x5f, 0x6b, 0x65, 0x72, 0x6e, 0x65, 0x6c, 0x00, 0x2e, 0x72, 0x65, 0x6c, 0x2e, 0x6e, 0x76, 0x2e, 0x63, 0x6f, 0x6e, 0x73, 0x74, 0x61, 0x6e, 0x74, 0x30, 0x2e, 0x61, 0x64, 0x64, 0x5f, 0x6b, 0x65, 0x72, 0x6e, 0x65, 0x6c, 0x00, 0x2e, 0x6e, 0x76, 0x2e, 0x63, 0x6f, 0x6e, 0x73, 0x74, 0x61, 0x6e, 0x74, 0x30, 0x2e, 0x61, 0x64, 0x64, 0x5f, 0x6b, 0x65, 0x72, 0x6e, 0x65, 0x6c, 0x00, 0x2e, 0x64, 0x65, 0x62, 0x75, 0x67, 0x5f, 0x66, 0x72, 0x61, 0x6d, 0x65, 0x00, 0x2e, 0x64, 0x65, 0x62, 0x75, 0x67, 0x5f, 0x6c, 0x69, 0x6e, 0x65, 0x00, 0x2e, 0x72, 0x65, 0x6c, 0x2e, 0x64, 0x65, 0x62, 0x75, 0x67, 0x5f, 0x6c, 0x69, 0x6e, 0x65, 0x00, 0x2e, 0x6e, 0x76, 0x5f, 0x64, 0x65, 0x62, 0x75, 0x67, 0x5f, 0x6c, 0x69, 0x6e, 0x65, 0x5f, 0x73, 0x61, 0x73, 0x73, 0x00, 0x2e, 0x72, 0x65, 0x6c, 0x2e, 0x6e, 0x76, 0x5f, 0x64, 0x65, 0x62, 0x75, 0x67, 0x5f, 0x6c, 0x69, 0x6e, 0x65, 0x5f, 0x73, 0x61, 0x73, 0x73, 0x00, 0x2e, 0x6e, 0x76, 0x5f, 0x64, 0x65, 0x62, 0x75, 0x67, 0x5f, 0x70, 0x74, 0x78, 0x5f, 0x74, 0x78, 0x74, 0x00, 0x2e, 0x72, 0x65, 0x6c, 0x2e, 0x64, 0x65, 0x62, 0x75, 0x67, 0x5f, 0x66, 0x72, 0x61, 0x6d, 0x65, 0x00, 0x2e, 0x72, 0x65, 0x6c, 0x61, 0x2e, 0x64, 0x65, 0x62, 0x75, 0x67, 0x5f, 0x66, 0x72, 0x61, 0x6d, 0x65, 0x00, 0x2e, 0x6e, 0x76, 0x2e, 0x63, 0x61, 0x6c, 0x6c, 0x67, 0x72, 0x61, 0x70, 0x68, 0x00, 0x2e, 0x6e, 0x76, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x74, 0x79, 0x70, 0x65, 0x00, 0x2e, 0x6e, 0x76, 0x2e, 0x72, 0x65, 0x6c, 0x2e, 0x61, 0x63, 0x74, 0x69, 0x6f, 0x6e, 0x00, 0x61, 0x64, 0x64, 0x5f, 0x6b, 0x65, 0x72, 0x6e, 0x65, 0x6c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x32, 0x00, 0x00, 0x00, 0x03, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8a, 0x00, 0x00, 0x00, 0x03, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa3, 0x00, 0x00, 0x00, 0x03, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb0, 0x00, 0x00, 0x00, 0x03, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xcc, 0x00, 0x00, 0x00, 0x03, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x00, 0x00, 0x03, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2d, 0x01, 0x00, 0x00, 0x03, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x49, 0x01, 0x00, 0x00, 0x03, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x58, 0x01, 0x00, 0x00, 0x12, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x03, 0x00, 0x04, 0x7c, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x0c, 0x81, 0x80, 0x80, 0x28, 0x00, 0x08, 0xff, 0x81, 0x80, 0x28, 0x08, 0x81, 0x80, 0x80, 0x28, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x04, 0x00, 0x00, 0x00, 0x04, 0x44, 0x00, 0x00, 0x00, 0x0c, 0x81, 0x80, 0x80, 0x28, 0x00, 0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x94, 0x00, 0x00, 0x00, 0x02, 0x00, 0x5c, 0x00, 0x00, 0x00, 0x01, 0x01, 0xfb, 0x0e, 0x0a, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x2f, 0x68, 0x6f, 0x6d, 0x65, 0x2f, 0x6d, 0x69, 0x63, 0x68, 0x61, 0x65, 0x6c, 0x2f, 0x63, 0x6f, 0x64, 0x65, 0x2f, 0x53, 0x63, 0x61, 0x6c, 0x65, 0x4c, 0x4c, 0x4d, 0x2f, 0x73, 0x72, 0x63, 0x2f, 0x6b, 0x65, 0x72, 0x6e, 0x65, 0x6c, 0x73, 0x2f, 0x74, 0x72, 0x69, 0x74, 0x6f, 0x6e, 0x2f, 0x76, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x5f, 0x61, 0x64, 0x64, 0x00, 0x00, 0x6b, 0x65, 0x72, 0x6e, 0x65, 0x6c, 0x2e, 0x70, 0x79, 0x00, 0x01, 0xd5, 0xef, 0xac, 0xb5, 0x06, 0x94, 0x19, 0x00, 0x00, 0x09, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x01, 0x03, 0x08, 0x01, 0x03, 0x10, 0x02, 0x10, 0x01, 0xf4, 0x03, 0x75, 0x02, 0x30, 0x01, 0xf5, 0x03, 0x02, 0x02, 0x20, 0x01, 0xf2, 0xf0, 0xee, 0xf0, 0xf2, 0x03, 0x7e, 0x02, 0x30, 0x01, 0xf1, 0x02, 0xf0, 0x01, 0x00, 0x01, 0x01, 0x59, 0x00, 0x00, 0x00, 0x02, 0x00, 0x10, 0x00, 0x00, 0x00, 0x01, 0x01, 0xfb, 0x0e, 0x0a, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x09, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x03, 0x11, 0x01, 0x03, 0x15, 0x02, 0x10, 0x01, 0x03, 0x09, 0x02, 0x10, 0x01, 0xf3, 0x03, 0x5e, 0x02, 0x10, 0x01, 0x03, 0x0e, 0x02, 0x10, 0x01, 0x03, 0x09, 0x02, 0x10, 0x01, 0xf1, 0xf1, 0xf2, 0xf7, 0xeb, 0xf7, 0xf7, 0xed, 0x03, 0x7e, 0x02, 0x20, 0x01, 0xf7, 0x03, 0x03, 0x02, 0x20, 0x01, 0x02, 0xd0, 0x01, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x2e, 0x76, 0x65, 0x72, 0x73, 0x69, 0x6f, 0x6e, 0x20, 0x38, 0x2e, 0x34, 0x00, 0x2e, 0x74, 0x61, 0x72, 0x67, 0x65, 0x74, 0x20, 0x73, 0x6d, 0x5f, 0x38, 0x30, 0x00, 0x2e, 0x61, 0x64, 0x64, 0x72, 0x65, 0x73, 0x73, 0x5f, 0x73, 0x69, 0x7a, 0x65, 0x20, 0x36, 0x34, 0x00, 0x00, 0x00, 0x00, 0x2e, 0x76, 0x69, 0x73, 0x69, 0x62, 0x6c, 0x65, 0x20, 0x2e, 0x65, 0x6e, 0x74, 0x72, 0x79, 0x20, 0x61, 0x64, 0x64, 0x5f, 0x6b, 0x65, 0x72, 0x6e, 0x65, 0x6c, 0x28, 0x00, 0x2e, 0x70, 0x61, 0x72, 0x61, 0x6d, 0x20, 0x2e, 0x75, 0x36, 0x34, 0x20, 0x61, 0x64, 0x64, 0x5f, 0x6b, 0x65, 0x72, 0x6e, 0x65, 0x6c, 0x5f, 0x70, 0x61, 0x72, 0x61, 0x6d, 0x5f, 0x30, 0x2c, 0x00, 0x2e, 0x70, 0x61, 0x72, 0x61, 0x6d, 0x20, 0x2e, 0x75, 0x36, 0x34, 0x20, 0x61, 0x64, 0x64, 0x5f, 0x6b, 0x65, 0x72, 0x6e, 0x65, 0x6c, 0x5f, 0x70, 0x61, 0x72, 0x61, 0x6d, 0x5f, 0x31, 0x2c, 0x00, 0x2e, 0x70, 0x61, 0x72, 0x61, 0x6d, 0x20, 0x2e, 0x75, 0x36, 0x34, 0x20, 0x61, 0x64, 0x64, 0x5f, 0x6b, 0x65, 0x72, 0x6e, 0x65, 0x6c, 0x5f, 0x70, 0x61, 0x72, 0x61, 0x6d, 0x5f, 0x32, 0x2c, 0x00, 0x2e, 0x70, 0x61, 0x72, 0x61, 0x6d, 0x20, 0x2e, 0x75, 0x33, 0x32, 0x20, 0x61, 0x64, 0x64, 0x5f, 0x6b, 0x65, 0x72, 0x6e, 0x65, 0x6c, 0x5f, 0x70, 0x61, 0x72, 0x61, 0x6d, 0x5f, 0x33, 0x00, 0x29, 0x00, 0x2e, 0x6d, 0x61, 0x78, 0x6e, 0x74, 0x69, 0x64, 0x20, 0x33, 0x32, 0x2c, 0x20, 0x31, 0x2c, 0x20, 0x31, 0x00, 0x7b, 0x00, 0x2e, 0x72, 0x65, 0x67, 0x20, 0x2e, 0x70, 0x72, 0x65, 0x64, 0x20, 0x09, 0x25, 0x70, 0x3c, 0x35, 0x3e, 0x3b, 0x00, 0x2e, 0x72, 0x65, 0x67, 0x20, 0x2e, 0x62, 0x33, 0x32, 0x20, 0x09, 0x25, 0x72, 0x3c, 0x31, 0x31, 0x3e, 0x3b, 0x00, 0x2e, 0x72, 0x65, 0x67, 0x20, 0x2e, 0x66, 0x33, 0x32, 0x20, 0x09, 0x25, 0x66, 0x3c, 0x34, 0x3e, 0x3b, 0x00, 0x2e, 0x72, 0x65, 0x67, 0x20, 0x2e, 0x62, 0x36, 0x34, 0x20, 0x09, 0x25, 0x72, 0x64, 0x3c, 0x38, 0x3e, 0x3b, 0x00, 0x00, 0x24, 0x4c, 0x5f, 0x5f, 0x66, 0x75, 0x6e, 0x63, 0x5f, 0x62, 0x65, 0x67, 0x69, 0x6e, 0x30, 0x3a, 0x00, 0x00, 0x00, 0x6c, 0x64, 0x2e, 0x70, 0x61, 0x72, 0x61, 0x6d, 0x2e, 0x75, 0x36, 0x34, 0x20, 0x09, 0x25, 0x72, 0x64, 0x34, 0x2c, 0x20, 0x5b, 0x61, 0x64, 0x64, 0x5f, 0x6b, 0x65, 0x72, 0x6e, 0x65, 0x6c, 0x5f, 0x70, 0x61, 0x72, 0x61, 0x6d, 0x5f, 0x30, 0x5d, 0x3b, 0x00, 0x6c, 0x64, 0x2e, 0x70, 0x61, 0x72, 0x61, 0x6d, 0x2e, 0x75, 0x36, 0x34, 0x20, 0x09, 0x25, 0x72, 0x64, 0x35, 0x2c, 0x20, 0x5b, 0x61, 0x64, 0x64, 0x5f, 0x6b, 0x65, 0x72, 0x6e, 0x65, 0x6c, 0x5f, 0x70, 0x61, 0x72, 0x61, 0x6d, 0x5f, 0x31, 0x5d, 0x3b, 0x00, 0x24, 0x4c, 0x5f, 0x5f, 0x74, 0x6d, 0x70, 0x30, 0x3a, 0x00, 0x00, 0x00, 0x6d, 0x6f, 0x76, 0x2e, 0x75, 0x33, 0x32, 0x20, 0x25, 0x72, 0x31, 0x2c, 0x20, 0x25, 0x63, 0x74, 0x61, 0x69, 0x64, 0x2e, 0x78, 0x3b, 0x00, 0x00, 0x00, 0x73, 0x68, 0x6c, 0x2e, 0x62, 0x33, 0x32, 0x20, 0x09, 0x25, 0x72, 0x35, 0x2c, 0x20, 0x25, 0x72, 0x31, 0x2c, 0x20, 0x34, 0x3b, 0x00, 0x6c, 0x64, 0x2e, 0x70, 0x61, 0x72, 0x61, 0x6d, 0x2e, 0x75, 0x36, 0x34, 0x20, 0x09, 0x25, 0x72, 0x64, 0x36, 0x2c, 0x20, 0x5b, 0x61, 0x64, 0x64, 0x5f, 0x6b, 0x65, 0x72, 0x6e, 0x65, 0x6c, 0x5f, 0x70, 0x61, 0x72, 0x61, 0x6d, 0x5f, 0x32, 0x5d, 0x3b, 0x00, 0x6c, 0x64, 0x2e, 0x70, 0x61, 0x72, 0x61, 0x6d, 0x2e, 0x75, 0x33, 0x32, 0x20, 0x09, 0x25, 0x72, 0x36, 0x2c, 0x20, 0x5b, 0x61, 0x64, 0x64, 0x5f, 0x6b, 0x65, 0x72, 0x6e, 0x65, 0x6c, 0x5f, 0x70, 0x61, 0x72, 0x61, 0x6d, 0x5f, 0x33, 0x5d, 0x3b, 0x00, 0x00, 0x6d, 0x6f, 0x76, 0x2e, 0x75, 0x33, 0x32, 0x20, 0x09, 0x25, 0x72, 0x37, 0x2c, 0x20, 0x25, 0x74, 0x69, 0x64, 0x2e, 0x78, 0x3b, 0x00, 0x61, 0x6e, 0x64, 0x2e, 0x62, 0x33, 0x32, 0x20, 0x20, 0x09, 0x25, 0x72, 0x38, 0x2c, 0x20, 0x25, 0x72, 0x37, 0x2c, 0x20, 0x31, 0x36, 0x3b, 0x00, 0x61, 0x6e, 0x64, 0x2e, 0x62, 0x33, 0x32, 0x20, 0x20, 0x09, 0x25, 0x72, 0x39, 0x2c, 0x20, 0x25, 0x72, 0x37, 0x2c, 0x20, 0x31, 0x35, 0x3b, 0x00, 0x00, 0x6f, 0x72, 0x2e, 0x62, 0x33, 0x32, 0x20, 0x20, 0x09, 0x25, 0x72, 0x31, 0x30, 0x2c, 0x20, 0x25, 0x72, 0x35, 0x2c, 0x20, 0x25, 0x72, 0x39, 0x3b, 0x00, 0x00, 0x73, 0x65, 0x74, 0x70, 0x2e, 0x6c, 0x74, 0x2e, 0x73, 0x33, 0x32, 0x20, 0x09, 0x25, 0x70, 0x31, 0x2c, 0x20, 0x25, 0x72, 0x31, 0x30, 0x2c, 0x20, 0x25, 0x72, 0x36, 0x3b, 0x00, 0x00, 0x6d, 0x75, 0x6c, 0x2e, 0x77, 0x69, 0x64, 0x65, 0x2e, 0x73, 0x33, 0x32, 0x20, 0x09, 0x25, 0x72, 0x64, 0x37, 0x2c, 0x20, 0x25, 0x72, 0x31, 0x30, 0x2c, 0x20, 0x34, 0x3b, 0x00, 0x61, 0x64, 0x64, 0x2e, 0x73, 0x36, 0x34, 0x20, 0x09, 0x25, 0x72, 0x64, 0x31, 0x2c, 0x20, 0x25, 0x72, 0x64, 0x34, 0x2c, 0x20, 0x25, 0x72, 0x64, 0x37, 0x3b, 0x00, 0x00, 0x00, 0x6d, 0x6f, 0x76, 0x2e, 0x75, 0x33, 0x32, 0x20, 0x25, 0x72, 0x32, 0x2c, 0x20, 0x30, 0x78, 0x30, 0x3b, 0x00, 0x40, 0x25, 0x70, 0x31, 0x20, 0x6c, 0x64, 0x2e, 0x67, 0x6c, 0x6f, 0x62, 0x61, 0x6c, 0x2e, 0x62, 0x33, 0x32, 0x20, 0x7b, 0x20, 0x25, 0x72, 0x32, 0x20, 0x7d, 0x2c, 0x20, 0x5b, 0x20, 0x25, 0x72, 0x64, 0x31, 0x20, 0x2b, 0x20, 0x30, 0x20, 0x5d, 0x3b, 0x00, 0x00, 0x6d, 0x6f, 0x76, 0x2e, 0x62, 0x33, 0x32, 0x20, 0x09, 0x25, 0x66, 0x31, 0x2c, 0x20, 0x25, 0x72, 0x32, 0x3b, 0x00, 0x00, 0x61, 0x64, 0x64, 0x2e, 0x73, 0x36, 0x34, 0x20, 0x09, 0x25, 0x72, 0x64, 0x32, 0x2c, 0x20, 0x25, 0x72, 0x64, 0x35, 0x2c, 0x20, 0x25, 0x72, 0x64, 0x37, 0x3b, 0x00, 0x00, 0x00, 0x6d, 0x6f, 0x76, 0x2e, 0x75, 0x33, 0x32, 0x20, 0x25, 0x72, 0x33, 0x2c, 0x20, 0x30, 0x78, 0x30, 0x3b, 0x00, 0x40, 0x25, 0x70, 0x31, 0x20, 0x6c, 0x64, 0x2e, 0x67, 0x6c, 0x6f, 0x62, 0x61, 0x6c, 0x2e, 0x62, 0x33, 0x32, 0x20, 0x7b, 0x20, 0x25, 0x72, 0x33, 0x20, 0x7d, 0x2c, 0x20, 0x5b, 0x20, 0x25, 0x72, 0x64, 0x32, 0x20, 0x2b, 0x20, 0x30, 0x20, 0x5d, 0x3b, 0x00, 0x00, 0x6d, 0x6f, 0x76, 0x2e, 0x62, 0x33, 0x32, 0x20, 0x09, 0x25, 0x66, 0x32, 0x2c, 0x20, 0x25, 0x72, 0x33, 0x3b, 0x00, 0x00, 0x61, 0x64, 0x64, 0x2e, 0x66, 0x33, 0x32, 0x20, 0x09, 0x25, 0x66, 0x33, 0x2c, 0x20, 0x25, 0x66, 0x31, 0x2c, 0x20, 0x25, 0x66, 0x32, 0x3b, 0x00, 0x00, 0x61, 0x64, 0x64, 0x2e, 0x73, 0x36, 0x34, 0x20, 0x09, 0x25, 0x72, 0x64, 0x33, 0x2c, 0x20, 0x25, 0x72, 0x64, 0x36, 0x2c, 0x20, 0x25, 0x72, 0x64, 0x37, 0x3b, 0x00, 0x00, 0x73, 0x65, 0x74, 0x70, 0x2e, 0x65, 0x71, 0x2e, 0x73, 0x33, 0x32, 0x20, 0x09, 0x25, 0x70, 0x34, 0x2c, 0x20, 0x25, 0x72, 0x38, 0x2c, 0x20, 0x30, 0x3b, 0x00, 0x6d, 0x6f, 0x76, 0x2e, 0x62, 0x33, 0x32, 0x20, 0x09, 0x25, 0x72, 0x34, 0x2c, 0x20, 0x25, 0x66, 0x33, 0x3b, 0x00, 0x61, 0x6e, 0x64, 0x2e, 0x70, 0x72, 0x65, 0x64, 0x20, 0x20, 0x09, 0x25, 0x70, 0x33, 0x2c, 0x20, 0x25, 0x70, 0x34, 0x2c, 0x20, 0x25, 0x70, 0x31, 0x3b, 0x00, 0x00, 0x40, 0x25, 0x70, 0x33, 0x20, 0x73, 0x74, 0x2e, 0x67, 0x6c, 0x6f, 0x62, 0x61, 0x6c, 0x2e, 0x62, 0x33, 0x32, 0x20, 0x5b, 0x20, 0x25, 0x72, 0x64, 0x33, 0x20, 0x2b, 0x20, 0x30, 0x20, 0x5d, 0x2c, 0x20, 0x7b, 0x20, 0x25, 0x72, 0x34, 0x20, 0x7d, 0x3b, 0x00, 0x00, 0x00, 0x72, 0x65, 0x74, 0x3b, 0x00, 0x24, 0x4c, 0x5f, 0x5f, 0x74, 0x6d, 0x70, 0x31, 0x3a, 0x00, 0x24, 0x4c, 0x5f, 0x5f, 0x66, 0x75, 0x6e, 0x63, 0x5f, 0x65, 0x6e, 0x64, 0x30, 0x3a, 0x00, 0x00, 0x7d, 0x00, 0x00, 0x2e, 0x73, 0x65, 0x63, 0x74, 0x69, 0x6f, 0x6e, 0x09, 0x2e, 0x64, 0x65, 0x62, 0x75, 0x67, 0x5f, 0x61, 0x62, 0x62, 0x72, 0x65, 0x76, 0x00, 0x7b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7d, 0x00, 0x2e, 0x73, 0x65, 0x63, 0x74, 0x69, 0x6f, 0x6e, 0x09, 0x2e, 0x64, 0x65, 0x62, 0x75, 0x67, 0x5f, 0x69, 0x6e, 0x66, 0x6f, 0x00, 0x7b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7d, 0x00, 0x2e, 0x73, 0x65, 0x63, 0x74, 0x69, 0x6f, 0x6e, 0x09, 0x2e, 0x64, 0x65, 0x62, 0x75, 0x67, 0x5f, 0x6c, 0x6f, 0x63, 0x09, 0x7b, 0x09, 0x7d, 0x00, 0x04, 0x2f, 0x08, 0x00, 0x09, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x04, 0x12, 0x08, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x11, 0x08, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x12, 0x08, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x37, 0x04, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x01, 0x35, 0x00, 0x00, 0x04, 0x0a, 0x08, 0x00, 0x02, 0x00, 0x00, 0x00, 0x60, 0x01, 0x1c, 0x00, 0x03, 0x19, 0x1c, 0x00, 0x04, 0x17, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x18, 0x00, 0x00, 0xf0, 0x11, 0x00, 0x04, 0x17, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x10, 0x00, 0x00, 0xf0, 0x21, 0x00, 0x04, 0x17, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x08, 0x00, 0x00, 0xf0, 0x21, 0x00, 0x04, 0x17, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x21, 0x00, 0x03, 0x1b, 0xff, 0x00, 0x04, 0x1c, 0x08, 0x00, 0x10, 0x01, 0x00, 0x00, 0x30, 0x01, 0x00, 0x00, 0x04, 0x05, 0x0c, 0x00, 0x20, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xfd, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xfc, 0xff, 0xff, 0xff, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x25, 0x00, 0x05, 0x36, 0x69, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x1d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x7a, 0x01, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0xe4, 0x0f, 0x00, 0x19, 0x79, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00, 0x00, 0x22, 0x0e, 0x00, 0x35, 0x74, 0x07, 0xff, 0x04, 0x00, 0x00, 0x00, 0xff, 0x01, 0x00, 0x00, 0x00, 0xe2, 0x0f, 0x00, 0x05, 0x78, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x01, 0x00, 0x00, 0xe2, 0x0f, 0x00, 0xb9, 0x7a, 0x04, 0x00, 0x00, 0x46, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0xe2, 0x0f, 0x00, 0x19, 0x79, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x25, 0x00, 0x00, 0x00, 0x62, 0x0e, 0x00, 0x12, 0x78, 0x00, 0x06, 0x0f, 0x00, 0x00, 0x00, 0xff, 0xc0, 0x8e, 0x07, 0x00, 0xc8, 0x1f, 0x00, 0x11, 0x72, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0xff, 0x20, 0x8e, 0x07, 0x00, 0xc8, 0x2f, 0x00, 0x0c, 0x7a, 0x00, 0x00, 0x00, 0x5e, 0x00, 0x00, 0x70, 0x62, 0xf0, 0x03, 0x00, 0xe2, 0x0f, 0x04, 0x25, 0x76, 0x02, 0x00, 0x00, 0x58, 0x00, 0x00, 0x07, 0x02, 0x8e, 0x07, 0x00, 0xc8, 0x0f, 0x00, 0x25, 0x76, 0x04, 0x00, 0x00, 0x5a, 0x00, 0x00, 0x07, 0x02, 0x8e, 0x07, 0x00, 0xd0, 0x0f, 0x00, 0x81, 0x89, 0x08, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00, 0x19, 0x1e, 0x0c, 0x00, 0xa8, 0x0e, 0x00, 0x81, 0x89, 0x09, 0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x19, 0x1e, 0x0c, 0x00, 0xa2, 0x0e, 0x00, 0x12, 0x78, 0xff, 0x06, 0x10, 0x00, 0x00, 0x00, 0xff, 0xc0, 0x80, 0x07, 0x00, 0xe2, 0x0f, 0x00, 0x25, 0x76, 0x06, 0x00, 0x00, 0x5c, 0x00, 0x00, 0x07, 0x02, 0x8e, 0x07, 0x00, 0xc6, 0x0f, 0x00, 0x0c, 0x7a, 0x00, 0x00, 0x00, 0x5e, 0x00, 0x00, 0x70, 0x12, 0x70, 0x04, 0x00, 0xe2, 0x0f, 0x00, 0x21, 0x72, 0x09, 0x09, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xd8, 0x4f, 0x00, 0x4d, 0x89, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x03, 0x00, 0xea, 0x0f, 0x00, 0x86, 0x79, 0x00, 0x06, 0x09, 0x00, 0x00, 0x00, 0x04, 0x19, 0x10, 0x0c, 0x00, 0xe2, 0x0f, 0x00, 0x4d, 0x79, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x03, 0x00, 0xea, 0x0f, 0x00, 0x47, 0x79, 0x00, 0x00, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0x83, 0x03, 0x00, 0xc0, 0x0f, 0x00, 0x18, 0x79, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x0f, 0x00, 0x18, 0x79, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x0f, 0x00, 0x18, 0x79, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x0f, 0x00, 0x18, 0x79, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x0f, 0x00, 0x18, 0x79, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x0f, 0x00, 0x18, 0x79, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x0f, 0x00, 0x18, 0x79, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x0f, 0x00, 0x18, 0x79, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x0f, 0x00, 0x18, 0x79, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x0f, 0x00, 0x18, 0x79, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x0f, 0x00, 0x18, 0x79, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x58, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x98, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x63, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa3, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb0, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x98, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xcc, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x29, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x74, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x43, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa4, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2d, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x49, 0x01, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xbc, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x01, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6d, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x32, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x0c, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x40, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x80, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x40, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };


void unload_add_kernel_fp32_3fbfba5d_0123(void) {
    CUDA_CHECK(cuModuleUnload(add_kernel_fp32_3fbfba5d_0123_mod));
}

// TODO: some code duplication with `runtime/backend/cuda.c`
void load_add_kernel_fp32_3fbfba5d_0123() {
    int dev = 0;
    void *bin = (void *)&CUBIN_NAME;
    int shared = 0;
    CUDA_CHECK(cuModuleLoadData(&add_kernel_fp32_3fbfba5d_0123_mod, bin));
    CUDA_CHECK(cuModuleGetFunction(&add_kernel_fp32_3fbfba5d_0123_func, add_kernel_fp32_3fbfba5d_0123_mod, "add_kernel"));
    // set dynamic shared memory if necessary
    int shared_optin;
    CUDA_CHECK(cuDeviceGetAttribute(&shared_optin, CU_DEVICE_ATTRIBUTE_MAX_SHARED_MEMORY_PER_BLOCK_OPTIN, dev));
    if (shared > 49152 && shared_optin > 49152) {
      CUDA_CHECK(cuFuncSetCacheConfig(add_kernel_fp32_3fbfba5d_0123_func, CU_FUNC_CACHE_PREFER_SHARED));
      CUDA_CHECK(cuFuncSetAttribute(add_kernel_fp32_3fbfba5d_0123_func, CU_FUNC_ATTRIBUTE_MAX_DYNAMIC_SHARED_SIZE_BYTES, shared_optin))
    }
}

/*
['BLOCK_SIZE=16', 'num_warps=1', 'num_stages=3']
*/
CUresult add_kernel_fp32_3fbfba5d_0123(CUstream stream, CUdeviceptr x_ptr, CUdeviceptr y_ptr, CUdeviceptr output_ptr, int32_t n_elements) {
    if (add_kernel_fp32_3fbfba5d_0123_func == NULL)
       load_add_kernel_fp32_3fbfba5d_0123();
    unsigned int gX = (n_elements + 15) / 16;
    unsigned int gY =  1;
    unsigned int gZ =  1;
    void *args[4] = { &x_ptr, &y_ptr, &output_ptr, &n_elements };
    // TODO: shared memory
    if(gX * gY * gZ > 0)
      return cuLaunchKernel(add_kernel_fp32_3fbfba5d_0123_func, gX, gY, gZ, 1 * 32, 1, 1, 0, stream, args, NULL);
}
