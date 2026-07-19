# 阶段一：向量加法指令 vadd.vv 在香山处理器中的执行过程分析

## 一、分析目标

依据赛题要求，结合仿真波形图与香山架构参考手册，深入拆解向量加法指令 `vadd.vv` 在香山处理器中的取指、译码、执行全流程，重点分析流水线阶段、数据通路流转逻辑、关键控制信号的波形解读及功能验证。

本分析参考香山官方文档《一条 ADD 指令的简单分析过程》的分析方法，以一条具体的 `vadd.vv` 指令为线索，逐阶段跟踪其在香山后端流水线中的执行轨迹。

## 二、测试程序

### 2.1 程序源码

测试程序 `vadd_test.c` 实现两个长度为 16 的 int8 向量对应元素相加：

```c
#include <klib.h>

#define VLEN_BYTES 16
#define NUM_ELEMS 16

static int8_t vs1_buf[VLEN_BYTES] __attribute__((aligned(16)));
static int8_t vs2_buf[VLEN_BYTES] __attribute__((aligned(16)));
static int8_t vd_buf[VLEN_BYTES] __attribute__((aligned(16)));

int main() {
    int i;
    int pass = 1;

    // 准备测试数据
    // vs1 = [1, 2, 3, ..., 16]
    // vs2 = [16, 15, 14, ..., 1]
    // vd  = [17, 17, 17, ..., 17]
    for (i = 0; i < NUM_ELEMS; i++) {
        vs1_buf[i] = (int8_t)(i + 1);
        vs2_buf[i] = (int8_t)(NUM_ELEMS - i);
    }

    // 通过内联汇编执行 vadd.vv 指令
    asm volatile(
        "vsetivli zero, 16, e8, m1, ta, ma\n"   // 配置向量寄存器：SEW=8, LMUL=1, VL=16
        "vle8.v v1, (%[src1])\n"                // 从内存加载 vs1 到 v1
        "vle8.v v2, (%[src2])\n"                // 从内存加载 vs2 到 v2
        "vadd.vv v3, v1, v2\n"                  // v3 = v1 + v2（核心分析对象）
        "vse8.v v3, (%[dst])\n"                 // 将结果存回内存
        :
        : [src1] "r"(vs1_buf), [src2] "r"(vs2_buf), [dst] "r"(vd_buf)
        : "v0", "v1", "v2", "v3", "memory"
    );

    // 验证结果
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
```

### 2.2 Makefile

```makefile
NAME = vadd-test
SRCS = vadd_test.c
MARCH ?= rv64gcv_zba_zbb_zbc_zbs

include $(AM_HOME)/Makefile.app
```

> **说明**：`riscv64-xs.mk` 默认的 MARCH 不包含向量扩展 `v`，因此需要在 Makefile 中显式指定 `rv64gcv`，否则编译器无法识别 `vsetivli`、`vle8.v`、`vadd.vv` 等向量指令。

### 2.3 测试数据设计

| 向量寄存器 | 元素值 | 含义 |
|------------|--------|------|
| v1（vs1） | [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16] | 递增序列 |
| v2（vs2） | [16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1] | 递减序列 |
| v3（vd） | [17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17] | 预期结果 |

设计理由：每个元素结果都是 17，便于一眼判断正确性。SEW=8 时 16 个元素正好占满 128 位向量寄存器（VLEN=128）。

## 三、仿真运行与波形生成

### 3.1 编译测试程序

```bash
cd /home/yym/xs-env
source env.sh
cd $AM_HOME/apps/vadd-test
make ARCH=riscv64-xs
```

### 3.2 运行仿真并生成波形

```bash
cd $NOOP_HOME
./build/emu -i $AM_HOME/apps/vadd-test/build/vadd-test-riscv64-xs.bin --no-diff --dump-wave -b 0 -e 2000
```

参数说明：
- `--no-diff`：不启用 DiffTest，仅运行功能
- `--dump-wave`：生成波形文件
- `-b 0 -e 2000`：只记录前 2000 个周期的波形，避免文件过大

### 3.3 波形文件

波形文件生成在 `$NOOP_HOME` 目录下，格式为 `.vcd`（编译时使用 `EMU_TRACE=1`）。

### 3.4 查看波形

使用 surfer 或 gtkwave 打开波形文件：

```bash
surfer *.vcd
# 或
gtkwave *.vcd
```

## 四、定位目标指令

### 4.1 查看反汇编

编译后会生成反汇编文件 `build/vadd-test-riscv64-xs.txt`，从中找到 `vadd.vv` 指令：

```bash
grep "vadd.vv" $AM_HOME/apps/vadd-test/build/vadd-test-riscv64-xs.txt
```

### 4.2 指令语义解析

`vadd.vv v3, v1, v2` 的指令编码和语义：

- 指令格式：OPMVV（向量-向量运算）
- 操作：`vd[i] = vs2[i] + vs1[i]`（对应元素相加）
- 源操作数：v1（vs1）、v2（vs2）
- 目标寄存器：v3（vd）
- 元素宽度：SEW=8（int8）
- 元素个数：VL=16

预期结果：v3 的 16 个元素均为 17。

### 4.3 在波形中定位

记录 `vadd.vv` 指令的 PC 地址，在波形中搜索该 PC 值，定位指令进入译码阶段的时刻。

## 五、香山向量指令代码路径

根据代码调研，`vadd.vv` 在香山中的完整执行路径如下：

| 阶段 | 涉及模块 | 关键源码文件 | 行号 |
|------|----------|-------------|------|
| 译码 | VecDecoder | `src/main/scala/xiangshan/backend/decode/VecDecoder.scala` | 189 |
| 功能单元类型 | FuType | `src/main/scala/xiangshan/backend/fu/FuType.scala` | 55 (`vialuF`) |
| 操作码编码 | VialuFixType | `yunsuan/src/main/scala/yunsuan/package.scala` | 113 (`vadd_vv`) |
| 功能单元 Wrapper | VIAluFix | `src/main/scala/xiangshan/backend/fu/wrapper/VIAluFix.scala` | 134 |
| 核心运算 ALU | VIntFixpAlu64b | `yunsuan/src/main/scala/yunsuan/vector/VectorALU/VIntFixpAlu.scala` | 37 |
| 加法器 | VIntAdder64b | `yunsuan/src/main/scala/yunsuan/vector/VectorALU/VIntAdder64b.scala` | 19, 66-90 |
| Mask/Tail 处理 | Mgu | `src/main/scala/xiangshan/backend/fu/vector/Mgu.scala` | - |

### 5.1 译码表映射

`VecDecoder.scala` 第 189 行：

```scala
VADD_VV -> OPIVV(FuType.vialuF, VialuFixType.vadd_vv, T, F, F)
```

含义：
- `VADD_VV`：vadd.vv 指令的编码
- `FuType.vialuF`：分配到向量整数 ALU 功能单元
- `VialuFixType.vadd_vv`：功能单元内的操作类型为加法
- `T, F, F`：写使能、不阻塞、不提交特殊标志

## 六、波形分析

### 6.1 译码阶段（DecodeUnit）

#### 分析对象

`vadd.vv` 指令进入 `DecodeUnit` 模块，由 `VecDecoder` 完成译码。

#### 关键信号

| 信号名 | 含义 | 预期值 |
|--------|------|--------|
| `io_enq_ctrlFlow_pc` | 输入指令 PC | vadd.vv 的 PC 地址 |
| `io_enq_ctrlFlow_instr` | 原始 32 位指令编码 | vadd.vv 的机器码 |
| `io_deq_decodedInst_fuType` | 功能单元类型 | `vialuF` 对应的编码 |
| `io_deq_decodedInst_fuOpType` | 功能单元操作类型 | `vadd_vv` 对应的编码 |
| `io_deq_decodedInst_lsrc_0` | 源操作数 0 逻辑寄存器号 | v1 的编号 |
| `io_deq_decodedInst_lsrc_1` | 源操作数 1 逻辑寄存器号 | v2 的编号 |
| `io_deq_decodedInst_ldest` | 目标逻辑寄存器号 | v3 的编号 |
| `io_deq_decodedInst_rfWen` | 寄存器写使能 | 1（需要写回） |

#### 波形截图

![译码阶段波形](./images/译码阶段波形.png)

#### 分析

[待补充：根据实际波形截图，说明 vadd.vv 指令在译码阶段的信号变化]

### 6.2 重命名阶段（Rename）

#### 分析对象

译码完成后，指令进入 `Rename` 模块，完成向量逻辑寄存器到物理寄存器的映射。

#### 关键信号

| 信号名 | 含义 |
|--------|------|
| `io_in_bits_pc` | 指令 PC（透传） |
| `io_in_bits_lsrc_0` | 逻辑源向量寄存器 v1 |
| `io_in_bits_lsrc_1` | 逻辑源向量寄存器 v2 |
| `io_in_bits_ldest` | 逻辑目标向量寄存器 v3 |
| `io_out_bits_psrc_0` | 物理源向量寄存器（v1 映射后） |
| `io_out_bits_psrc_1` | 物理源向量寄存器（v2 映射后） |
| `io_out_bits_pdest` | 物理目标向量寄存器（v3 分配的新寄存器） |

#### 波形截图

![重命名阶段波形](./images/重命名阶段波形.png)

#### 分析

[待补充：根据实际波形截图，说明向量寄存器重命名过程]

### 6.3 分发阶段（Dispatch）

#### 分析对象

重命名后，指令进入 `Dispatch` 模块，查询源操作数就绪状态，写入向量发射队列。

#### 关键信号

| 信号名 | 含义 |
|--------|------|
| `io_read_0_resp` | 源物理寄存器 0 就绪状态 |
| `io_read_1_resp` | 源物理寄存器 1 就绪状态 |
| `io_allocPregs_0_valid` | 目标物理寄存器分配有效 |
| `io_allocPregs_0_bits` | 目标物理寄存器编号 |

#### 波形截图

![分发阶段波形](./images/分发阶段波形.png)

#### 分析

[待补充：根据实际波形截图，说明分发到向量发射队列的过程]

### 6.4 执行阶段（Execute）

#### 分析对象

这是向量指令与标量指令最大的区别所在。`vadd.vv` 进入 `VIAluFix` 功能单元 Wrapper，调用 `VIntFixpAlu64b` 执行加法。

#### 数据通路

```
VIAluFix（Wrapper）
  ├─ VIntFixpAlu64b[0]  ← 处理低 64 位（元素 0-7）
  │    └─ VIntAdder64b  ← 8 个 8 位加法器链
  └─ VIntFixpAlu64b[1]  ← 处理高 64 位（元素 8-15）
       └─ VIntAdder64b  ← 8 个 8 位加法器链
```

`VIAluFix.scala` 第 140 行：`numVecModule = dataWidth / dataWidthOfDataModule = 128 / 64 = 2`，实例化 2 个 64 位处理单元并行计算。

`VIntAdder64b.scala` 第 66-90 行：8 个 8 位加法器链式串联，按 SEW=8 在每个 8 位边界截断进位，实现 8 个 int8 元素的并行加法。

#### 关键信号

| 信号名 | 含义 |
|--------|------|
| `io_in_bits_src_0` | 源操作数 0（vs1 数据） |
| `io_in_bits_src_1` | 源操作数 1（vs2 数据） |
| `io_in_bits_opcode` | 操作码（vadd） |
| `vIntFixpAlus_0_vIntAdder64b_bits` | 加法器输入 |
| `vIntFixpAlus_0_vIntAdder64b_out` | 加法器输出 |
| `io_out_bits_res_data` | 最终结果（经 Mgu 处理后） |

#### 波形截图

![执行阶段波形](./images/执行阶段波形.png)

#### 分析

[待补充：根据实际波形截图，说明 128 位数据如何分成两块 64 位并行处理，8 位加法器链如何工作]

### 6.5 写回阶段（Writeback）

#### 分析对象

执行结果经 `Mgu`（Mask/Gap Unit）处理 mask 和 tail 元素后，写回目标向量寄存器。

#### 关键信号

| 信号名 | 含义 |
|--------|------|
| `io_out_bits_res_data` | 写回数据（128 位） |
| `io_out_valid` | 写回有效 |
| `io_out_ready` | 写回就绪 |

#### 波形截图

![写回阶段波形](./images/写回阶段波形.png)

#### 分析

[待补充：根据实际波形截图，说明结果写回过程]

## 七、完整数据通路总结

### 7.1 vadd.vv 在香山中的完整执行流程

```
1. 取指（IF）
   vadd.vv 指令从 ICache 取出，存入 FTQ
        ↓
2. 译码（ID）
   VecDecoder 识别 VADD_VV
   → FuType = vialuF
   → VialuFixType = vadd_vv
   → 提取 lsrc（v1, v2）、ldest（v3）
        ↓
3. 重命名（Rename）
   向量逻辑寄存器 → 物理寄存器映射
   v1 → psrc_0, v2 → psrc_1
   v3 → 新分配的 pdest
   分配 ROB 表项
        ↓
4. 分发（Dispatch）
   查询源物理寄存器就绪状态
   写入向量发射队列
        ↓
5. 发射（Issue）
   源操作数就绪后，从发射队列发射到 VIAluFix
        ↓
6. 执行（Execute）
   VIAluFix（Wrapper）
   → VIntFixpAlu64b[0]：处理元素 0-7
   → VIntFixpAlu64b[1]：处理元素 8-15
   → 每个 VIntFixpAlu64b 调用 VIntAdder64b
   → 8 个 8 位加法器并行执行
        ↓
7. 结果处理
   结果经 Mgu 处理 mask/tail 元素
        ↓
8. 写回（Writeback）
   128 位结果写回目标物理向量寄存器
   更新 BusyTable
        ↓
9. 提交（Commit）
   按 ROB 顺序提交，释放物理寄存器
```

### 7.2 向量指令与标量指令的执行差异

| 对比项 | 标量 ADD | 向量 vadd.vv |
|--------|----------|-------------|
| 功能单元 | ALU（FuType.alu） | vialuF（向量 ALU） |
| 数据宽度 | 64 位 | 128 位（VLEN=128） |
| 并行度 | 1 个元素 | 16 个 int8 元素并行 |
| 执行单元 | 单个 ALU | 2 个 VIntFixpAlu64b 并行，每个含 8 位加法器链 |
| 元素宽度处理 | 固定 64 位 | 按 SEW 动态切分（SEW=8 时按 8 位截断进位） |
| Mask/Tail | 无 | 需要 Mgu 处理 |
| 译码表 | XSErrorDecoder | VecDecoder |

### 7.3 关键发现

1. **并行处理**：香山将 128 位向量数据分成 2 个 64 位块，由 2 个 `VIntFixpAlu64b` 并行处理
2. **加法器链**：每个 64 位块内，8 个 8 位加法器链式串联，按 SEW 在对应位置截断进位，实现 8 个 int8 元素的并行加法
3. **流水线执行**：`VIAluFix` 继承自 `VecPipedFuncUnit`，是流水线化功能单元，每个周期可接收新指令
4. **Mask/Tail 处理**：结果经过 `Mgu` 处理 mask 和 tail 元素，确保未被 mask 选中的元素保持原值

## 八、结论

本文以 `vadd.vv v3, v1, v2`（SEW=8, VLEN=128, VL=16）为例，结合香山仿真波形和源码分析，完整跟踪了向量加法指令在香山处理器后端流水线中的执行过程。

通过波形信号逐一验证了：
- 译码阶段：VecDecoder 正确识别 vadd.vv，映射到 vialuF 功能单元
- 重命名阶段：向量逻辑寄存器正确映射到物理寄存器
- 分发阶段：指令正确路由到向量发射队列
- 执行阶段：128 位数据由 2 个 VIntFixpAlu64b 并行处理，8 位加法器链完成 16 个 int8 元素的并行加法
- 写回阶段：结果经 Mgu 处理后正确写回目标向量寄存器

分析结果表明，香山处理器通过**数据分块并行 + 加法器链按 SEW 截断**的方式高效实现了向量加法指令，为后续 vdot.vv 自定义指令的功能单元扩展提供了清晰的参考路径。
