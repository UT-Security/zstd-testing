#include <stdio.h>
#include <stdlib.h>

#include "simple_decompression.wasm.h"
#include "uvwasi.h"
#include "uvwasi-rt.h"

int main(int argc, const char** argv) {

  /* Initialize the Wasm runtime. */
  wasm_rt_init();

  /* Declare an instance of the `inst` module. */
  w2c_simple__decompression inst = { 0 };

  uvwasi_t local_uvwasi_state = {0};

  struct w2c_wasi__snapshot__preview1 wasi_env = {
    .uvwasi = &local_uvwasi_state,
    .instance_memory = &inst.w2c_memory
  };

  uvwasi_options_t init_options;

  //pass in standard descriptors
  init_options.in = 0;
  init_options.out = 1;
  init_options.err = 2;
  init_options.fd_table_size = 3;

  //pass in args and environement
  extern const char ** environ;
  init_options.argc = argc;
  init_options.argv = argv;
  init_options.envp = (const char **) environ;

  //no sandboxing enforced, binary has access to everything user does
  init_options.preopenc = 2;
  init_options.preopens = calloc(2, sizeof(uvwasi_preopen_t));

  init_options.preopens[0].mapped_path = "/";
  init_options.preopens[0].real_path = "/";
  init_options.preopens[1].mapped_path = "./";
  init_options.preopens[1].real_path = ".";

  init_options.allocator = NULL;

  uvwasi_errno_t ret = uvwasi_init(&local_uvwasi_state, &init_options);

  if (ret != UVWASI_ESUCCESS) {
    printf("uvwasi_init failed with error %d\n", ret);
    exit(1);
  }

  /* Construct the module instance. */
  wasm2c_simple__decompression_instantiate(&inst, &wasi_env);

  /* Call `inst`, using the mangled name. */
  w2c_simple__decompression_0x5Fstart(&inst);

  /* Free the inst module. */
  wasm2c_simple__decompression_free(&inst);

  uvwasi_destroy(&local_uvwasi_state);
  /* Free the Wasm runtime state. */
  wasm_rt_free();

  return 0;
}
