#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "matrix.h"

/* Independent model of the RTL: packed-byte products, low-byte accumulation. */
static unsigned char packedDotProduct(unsigned int a, unsigned int b)
{
    unsigned int lane;
    unsigned int result = 0U;

    for (lane = 0U; lane < MSIZE; ++lane) {
        const unsigned int x = (a >> (lane * 8U)) & 0xFFU;
        const unsigned int y = (b >> (lane * 8U)) & 0xFFU;
        result = (result + ((x * y) & 0xFFU)) & 0xFFU;
    }
    return (unsigned char)result;
}

static int checkProduct(const char *name, VectorArray A, VectorArray B,
                        const unsigned char expected[MSIZE][MSIZE])
{
    VectorArray actual;
    VectorArray copy;
    unsigned row;
    unsigned col;

    memset(actual, 0xA5, sizeof actual);
    multiMatrixSoft(A, B, actual);
    for (row = 0U; row < MSIZE; ++row) {
        for (col = 0U; col < MSIZE; ++col) {
            const unsigned char rtl = packedDotProduct(A[row].vect, B[col].vect);
            if (actual[row].comp[col] != expected[row][col] ||
                actual[row].comp[col] != rtl) {
                fprintf(stderr,
                        "%s mismatch [%u][%u]: actual=%u expected=%u RTL=%u\n",
                        name, row, col, (unsigned)actual[row].comp[col],
                        (unsigned)expected[row][col], (unsigned)rtl);
                return 0;
            }
        }
    }
    memcpy(copy, actual, sizeof copy);
    if (!matrixEquals(actual, copy)) {
        fprintf(stderr, "%s: equality rejected identical matrices\n", name);
        return 0;
    }
    copy[MSIZE - 1U].comp[MSIZE - 1U] ^= 1U;
    if (matrixEquals(actual, copy)) {
        fprintf(stderr, "%s: equality missed the final matrix element\n", name);
        return 0;
    }
    printf("matrix %s: PASS\n", name);
    return 1;
}

static void loadMatrix(VectorArray destination,
                       const unsigned char values[MSIZE][MSIZE])
{
    unsigned int row;
    for (row = 0U; row < MSIZE; ++row) {
        memcpy(destination[row].comp, values[row], MSIZE);
    }
}

int main(void)
{
    static const unsigned char courseExpected[MSIZE][MSIZE] = {
        {30, 30, 30, 30},
        {70, 70, 70, 70},
        {110, 110, 110, 110},
        {150, 150, 150, 150}
    };
    static const unsigned char asymmetricA[MSIZE][MSIZE] = {
        {1, 2, 3, 4}, {9, 3, 1, 2}, {0, 4, 2, 7}, {8, 1, 6, 5}
    };
    static const unsigned char asymmetricB[MSIZE][MSIZE] = {
        {2, 5, 1, 3}, {9, 0, 4, 1}, {3, 8, 2, 6}, {1, 7, 5, 4}
    };
    static const unsigned char asymmetricExpected[MSIZE][MSIZE] = {
        {27, 25, 49, 46}, {40, 87, 65, 43},
        {43, 15, 78, 66}, {42, 101, 74, 65}
    };
    static const unsigned char overflowA[MSIZE][MSIZE] = {
        {255, 255, 255, 255}, {255, 1, 128, 17},
        {128, 64, 32, 16}, {0, 0, 0, 0}
    };
    static const unsigned char overflowB[MSIZE][MSIZE] = {
        {255, 255, 255, 255}, {2, 3, 4, 5},
        {16, 17, 18, 19}, {128, 1, 255, 7}
    };
    static const unsigned char overflowExpected[MSIZE][MSIZE] = {
        {4, 242, 186, 121}, {111, 86, 68, 120},
        {16, 144, 176, 144}, {0, 0, 0, 0}
    };
    /* Keep the old non-transposed fixture as an extra arithmetic regression. */
    static const unsigned char legacyRows[MSIZE][MSIZE] = {
        {1, 1, 1, 1}, {2, 2, 2, 2}, {3, 3, 3, 3}, {4, 4, 4, 4}
    };
    static const unsigned char legacyExpected[MSIZE][MSIZE] = {
        {10, 20, 30, 40}, {26, 52, 78, 104},
        {42, 84, 126, 168}, {58, 116, 174, 232}
    };
    VectorArray localA;
    VectorArray localB;
    unsigned int row;
    unsigned int col;

    /* The prescribed initializer must fill its arguments, not hidden globals. */
    memset(aInst, 0xA5, sizeof aInst);
    memset(bTInst, 0xA5, sizeof bTInst);
    setInputMatrices(localA, localB);
    for (row = 0U; row < MSIZE; ++row) {
        for (col = 0U; col < MSIZE; ++col) {
            if (localA[row].comp[col] != row * MSIZE + col + 1U ||
                localB[row].comp[col] != col + 1U ||
                aInst[row].comp[col] != 0xA5U ||
                bTInst[row].comp[col] != 0xA5U) {
                fputs("matrix initialization/interface: FAIL\n", stderr);
                return EXIT_FAILURE;
            }
        }
    }
    setInputMatrices(aInst, bTInst);
    if (!checkProduct("course transpose", aInst, bTInst, courseExpected)) {
        return EXIT_FAILURE;
    }
    loadMatrix(localB, legacyRows);
    if (!checkProduct("previous fixture", aInst, localB, legacyExpected)) {
        return EXIT_FAILURE;
    }
    loadMatrix(localA, asymmetricA);
    loadMatrix(localB, asymmetricB);
    if (!checkProduct("non-symmetric rows", localA, localB, asymmetricExpected)) {
        return EXIT_FAILURE;
    }
    loadMatrix(localA, overflowA);
    loadMatrix(localB, overflowB);
    if (!checkProduct("modulo-256 overflow", localA, localB, overflowExpected)) {
        return EXIT_FAILURE;
    }
    puts("matrix test: PASS");
    return EXIT_SUCCESS;
}
