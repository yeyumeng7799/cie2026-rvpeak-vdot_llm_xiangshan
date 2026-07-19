#include <klib.h>

#define VLEN_BYTES 16
#define NUM_ELEMS 16

static int8_t vs1_buf[VLEN_BYTES] __attribute__((aligned(16)));
static int8_t vs2_buf[VLEN_BYTES] __attribute__((aligned(16)));
static int8_t vd_buf[VLEN_BYTES] __attribute__((aligned(16)));

int main() {
    int i;
    int pass = 1;

    for (i = 0; i < NUM_ELEMS; i++) {
        vs1_buf[i] = (int8_t)(i + 1);
        vs2_buf[i] = (int8_t)(NUM_ELEMS - i);
    }

    asm volatile(
        "vsetivli zero, 16, e8, m1, ta, ma\n"
        "vle8.v v1, (%[src1])\n"
        "vle8.v v2, (%[src2])\n"
        "vadd.vv v3, v1, v2\n"
        "vse8.v v3, (%[dst])\n"
        :
        : [src1] "r"(vs1_buf), [src2] "r"(vs2_buf), [dst] "r"(vd_buf)
        : "v0", "v1", "v2", "v3", "memory"
    );

    for (i = 0; i < NUM_ELEMS; i++) {
        int8_t expected = (int8_t)((i + 1) + (NUM_ELEMS - i));
        if (vd_buf[i] != expected) {
            printf("FAIL: vd[%d]=%d, expected=%d\n", i, vd_buf[i], expected);
            pass = 0;
        }
    }

    if (pass) {
        printf("vadd.vv test PASSED\n");
        printf("vs1 = [1,2,3,...,16]\n");
        printf("vs2 = [16,15,14,...,1]\n");
        printf("vd  = [17,17,17,...,17]\n");
        for (i = 0; i < NUM_ELEMS; i++) {
            printf("vd[%d] = %d\n", i, vd_buf[i]);
        }
    }

    return 0;
}
