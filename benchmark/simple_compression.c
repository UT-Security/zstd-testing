/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under both the BSD-style license (found in the
 * LICENSE file in the root directory of this source tree) and the GPLv2 (found
 * in the COPYING file in the root directory of this source tree).
 * You may select, at your option, one of the above-listed licenses.
 */

#include <stdio.h>     // printf
#include <stdlib.h>    // free
#include <string.h>    // strlen, strcat, memset
#include <zstd.h>      // presumes zstd library is installed
#include "common.h"    // Helper functions, CHECK(), and CHECK_ZSTD()
#include <time.h>

static void compress_orDie(const char* fname, const char* oname)
{
    size_t fSize;
    void* const fBuff = mallocAndLoadFile_orDie(fname, &fSize);

    struct timespec enter_time = { 0 };
    clock_gettime(CLOCK_REALTIME, &enter_time);


    size_t const cBuffSize = ZSTD_compressBound(fSize);
    void* const cBuff = malloc_orDie(cBuffSize);

    const int test_iterations = 10;
    size_t cSize = 0;
    for (int i = 0; i < test_iterations; i++) {
        /* Compress.
        * If you are doing many compressions, you may want to reuse the context.
        * See the multiple_simple_compression.c example.
        */
        cSize = ZSTD_compress(cBuff, cBuffSize, fBuff, fSize, 1);
        CHECK_ZSTD(cSize);
    }

    struct timespec exit_time = { 0 };
    clock_gettime(CLOCK_REALTIME, &exit_time);

    const int64_t nanos = 1000000000;
    int64_t ns =  (nanos * (exit_time.tv_sec - enter_time.tv_sec))
        + ((int64_t)(exit_time.tv_nsec - enter_time.tv_nsec));

    printf("ZLIB compression time: %lld\n", (long long) (ns / test_iterations));

    saveFile_orDie(oname, cBuff, cSize);

    // /* success */
    // printf("%25s : %6u -> %7u - %s \n", fname, (unsigned)fSize, (unsigned)cSize, oname);

    free(fBuff);
    free(cBuff);
}

static char* createOutFilename_orDie(const char* filename)
{
    size_t const inL = strlen(filename);
    size_t const outL = inL + 5;
    void* const outSpace = malloc_orDie(outL);
    memset(outSpace, 0, outL);
    strcat(outSpace, filename);
    strcat(outSpace, ".zst");
    return (char*)outSpace;
}

int main(int argc, const char** argv)
{
    const char* const exeName = argv[0];

    if (argc!=2) {
        printf("wrong arguments\n");
        printf("usage:\n");
        printf("%s FILE\n", exeName);
        return 1;
    }

    const char* const inFilename = argv[1];

    char* const outFilename = createOutFilename_orDie(inFilename);

    struct timespec warmup_time = { 0 };
    for(int i = 0; i < 10; i++) {
        clock_gettime(CLOCK_REALTIME, &warmup_time);
        if(warmup_time.tv_nsec == 0 && warmup_time.tv_sec == 0) {
        printf("Clock not working\n");
        exit(1);
        }
    }

    compress_orDie(inFilename, outFilename);
    free(outFilename);
    return 0;
}
