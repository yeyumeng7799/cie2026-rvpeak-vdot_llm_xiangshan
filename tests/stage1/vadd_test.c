#include <klib.h>

#define NUM_ELEMS 16

static void run_vadd_vv(int8_t *src1, int8_t *src2, int8_t *dst) {
  asm volatile("vsetivli zero, 16, e8, m1, ta, ma\n"
               "vle8.v v1, (%0)\n"
               "vle8.v v2, (%1)\n"
               "vadd.vv v3, v1, v2\n"
               "vse8.v v3, (%2)\n"
               "fence rw, rw\n"
               :
               : "r"(src1), "r"(src2), "r"(dst)
               : "v0", "v1", "v2", "v3", "memory");
}

int main() {
  asm volatile(
    "li a0, 0x200\n"
    "csrs mstatus, a0\n"
    ::: "a0"
  );

  int8_t vs1[NUM_ELEMS] __attribute__((aligned(16)));
  int8_t vs2[NUM_ELEMS] __attribute__((aligned(16)));
  int8_t vd[NUM_ELEMS] __attribute__((aligned(16)));
  int i;
  int pass = 1;

  for (i = 0; i < NUM_ELEMS; i++) {
    vs1[i] = (int8_t)(i + 1);
    vs2[i] = (int8_t)(NUM_ELEMS - i);
  }

  run_vadd_vv(vs1, vs2, vd);

  for (i = 0; i < NUM_ELEMS; i++) {
    int8_t expected = (int8_t)((i + 1) + (NUM_ELEMS - i));
    if (vd[i] != expected) {
      pass = 0;
    }
  }

  if (pass) {
    printf("vadd.vv PASSED\n");
    asm volatile("li a7, 0\n");  // 标记成功
  } else {
    printf("vadd.vv FAILED\n");
    asm volatile("li a7, 1\n");  // 标记失败
  }

  return pass ? 0 : 1;
}
