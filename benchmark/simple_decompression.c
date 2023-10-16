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
#include <zstd.h>      // presumes zstd library is installed
#include "common.h"    // Helper functions, CHECK(), and CHECK_ZSTD()
#include <time.h>

static void decompress(const char* fname)
{
    size_t cSize;
    void* const cBuff = mallocAndLoadFile_orDie(fname, &cSize);
    /* Read the content size from the frame header. For simplicity we require
     * that it is always present. By default, zstd will write the content size
     * in the header when it is known. If you can't guarantee that the frame
     * content size is always written into the header, either use streaming
     * decompression, or ZSTD_decompressBound().
     */

    struct timespec enter_time = { 0 };
    clock_gettime(CLOCK_REALTIME, &enter_time);

    unsigned long long const rSize = ZSTD_getFrameContentSize(cBuff, cSize);
    CHECK(rSize != ZSTD_CONTENTSIZE_ERROR, "%s: not compressed by zstd!", fname);
    CHECK(rSize != ZSTD_CONTENTSIZE_UNKNOWN, "%s: original size unknown!", fname);

    void* const rBuff = malloc_orDie((size_t)rSize);

    const int test_iterations = 10;
    for (int i = 0; i < test_iterations; i++) {
        /* Decompress.
        * If you are doing many decompressions, you may want to reuse the context
        * and use ZSTD_decompressDCtx(). If you want to set advanced parameters,
        * use ZSTD_DCtx_setParameter().
        */
        size_t const dSize = ZSTD_decompress(rBuff, rSize, cBuff, cSize);
        CHECK_ZSTD(dSize);
        /* When zstd knows the content size, it will error if it doesn't match. */
        CHECK(dSize == rSize, "Impossible because zstd will check this condition!");
    }

    struct timespec exit_time = { 0 };
    clock_gettime(CLOCK_REALTIME, &exit_time);

    const int64_t nanos = 1000000000;
    int64_t ns =  (nanos * (exit_time.tv_sec - enter_time.tv_sec))
        + ((int64_t)(exit_time.tv_nsec - enter_time.tv_nsec));

    printf("ZLIB decompression time: %lld\n", (long long) (ns / test_iterations));

    /* success */
    // printf("%25s : %6u -> %7u \n", fname, (unsigned)cSize, (unsigned)rSize);

    free(rBuff);
    free(cBuff);
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

    struct timespec warmup_time = { 0 };
    for(int i = 0; i < 10; i++) {
        clock_gettime(CLOCK_REALTIME, &warmup_time);
        if(warmup_time.tv_nsec == 0 && warmup_time.tv_sec == 0) {
        printf("Clock not working\n");
        exit(1);
        }
    }

    decompress(argv[1]);

    // printf("%s correctly decoded (in memory). \n", argv[1]);

    return 0;
}
