#ifndef WASM_RT_FILEWASI_H_
#define WASM_RT_FILEWASI_H_

/* A minimum wasi implementation supporting only stdin, stdout, stderr, argv
 * (upto 100 args) and clock functions. */

#include <stdbool.h>
#include <stdint.h>

#include "wasm-rt.h"

#define WASM2C_WASI_MAX_FDS 32

#ifdef __cplusplus
extern "C"
{
#endif

  typedef struct w2c_wasi__snapshot__preview1
  {
    wasm_rt_memory_t* instance_memory;

    uint32_t main_argc;
    const char** main_argv;

    uint32_t env_count;
    const char** env;

    void* clock_data;

    int wasm_fd_to_native[WASM2C_WASI_MAX_FDS];
    uint32_t next_wasm_fd;

  } w2c_wasi__snapshot__preview1;

  bool filewasi_init();
  bool filewasi_init_instance(w2c_wasi__snapshot__preview1* wasi_data);
  void filewasi_cleanup_instance(w2c_wasi__snapshot__preview1* wasi_data);

#ifdef __cplusplus
}
#endif

#endif
