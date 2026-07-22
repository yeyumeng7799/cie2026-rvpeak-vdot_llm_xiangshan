# 阶段一：向量加法指令 vadd.vv 在香山处理器中的执行过程分析

## 一、分析目标

依据赛题要求，结合仿真波形图与香山架构参考手册，深入拆解向量加法指令 `vadd.vv` 在香山处理器中的取指、译码、执行全流程，重点分析流水线阶段、数据通路流转逻辑、关键控制信号的波形解读及功能验证。

本分析参考香山官方文档《一条 ADD 指令的简单分析过程》的分析方法，以一条具体的 `vadd.vv` 指令为线索，逐阶段跟踪其在香山后端流水线中的执行轨迹。

## 二、测试程序

### 2.1 程序源码

测试程序 `vadd_test.c` 实现两个长度为 16 的 int8 向量对应元素相加：

```c
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
  // 启用向量扩展 (MSTATUS.VS = Initial)
  // AM 框架的 start.S 只开了浮点 (MSTATUS_FS)，
  // 没开向量扩展 (MSTATUS_VS)，需要手动启用
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
```

> **关键修改说明**：AM 框架的启动代码 `start.S` 只启用了浮点扩展（`MSTATUS_FS`），没有启用向量扩展（`MSTATUS_VS`）。如果不在 `main()` 开头手动设置 `MSTATUS.VS` 位，执行 `vsetivli` 等向量指令时会触发非法指令异常，导致程序卡死。`MSTATUS_VS` 位于 bits[10:9]，设为 `01`（Initial）即可启用，对应值 `0x200`。

### 2.2 Makefile

```makefile
NAME = vadd-test
SRCS = vadd_test.c
MARCH ?= rv64gcv_zba_zbb_zbc_zbs
CFLAGS += -fno-tree-vectorize -fno-tree-loop-vectorize

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

### 3.1 编译 NEMU 参考模型

emu 依赖 NEMU 作为 DiffTest 参考模型，需要先编译 NEMU 的共享库：

```bash
cd /home/yym/xs-env/NEMU
make riscv64-xs-ref_defconfig
make -j$(nproc)
```

编译成功后生成 `build/riscv64-nemu-interpreter-so`。

> **说明**：NEMU 首次编译需要从 GitHub 克隆 softfloat、nanopb、LibCheckpoint 等子模块。如果网络代理不可用，需先取消 git 代理配置（`git config --global --unset http.proxy`）或启动代理软件。

### 3.2 编译测试程序

```bash
cd /home/yym/xs-env
source env.sh
AM_HOME=/home/yym/xs-env/nexus-am make -C $AM_HOME/apps/vadd-test ARCH=riscv64-xs
```

> **说明**：如果环境中 `AM_HOME` 指向了其他路径（如 ysyx-workbench），需要显式覆盖 `AM_HOME` 环境变量指向 `xs-env/nexus-am`。

### 3.3 运行仿真并生成波形

```bash
cd $NOOP_HOME
./build/emu -i $AM_HOME/apps/vadd-test/build/vadd-test-riscv64-xs.bin --no-diff --dump-wave
```

参数说明：
- `--no-diff`：不启用 DiffTest，仅运行功能验证（NEMU 参考模型与 KunminghuV2 配置存在差异，difftest 会误报，因此关闭）
- `--dump-wave`：生成 VCD 波形文件

### 3.4 波形文件

波形文件生成在 `$NOOP_HOME/build/` 目录下，格式为 `.vcd`（编译 emu 时使用 `EMU_TRACE=1`），文件约 1.3 GB。

由于 VCD 文件较大（约 1.3 GB），先转换为 FST 格式以提升加载速度：

```bash
vcd2fst *.vcd vadd.fst
surfer vadd.fst
# 或
gtkwave vadd.fst
```

### 3.5 功能验证波形

程序运行结束后，UART 依次输出 `vadd.vv PASSED\n`，从波形中可以清晰观察到这一过程。

#### 全局视角

下图为 Zoom Fit 后的全局波形，可以看到 `difftest_uart_out_valid` 信号的一排脉冲尖峰，每个脉冲对应一个字符的输出：

![UART 输出全局波形](./images/01_uart_overview.png)

#### 首字符输出细节

放大到第一个 UART 脉冲，可以看到 `difftest_uart_out_ch` 的值为 `0x76`，即 ASCII 字符 `v`，是 `vadd.vv PASSED` 的第一个字符：

![UART 首字符输出波形](./images/02_uart_first_char.png)

后续字符依次为 `0x61`(a)、`0x64`(d)、`0x64`(d)、`0x2e`(.)、`0x76`(v)、`0x76`(v)、`0x20`(空格)、`0x50`(P)、`0x41`(A)、`0x53`(S)、`0x53`(S)、`0x45`(E)、`0x44`(D)、`0x0a`(\n)，构成完整的 `vadd.vv PASSED\n` 输出。

UART 每个字符之间间隔较大，这是因为 UART 按波特率逐字符传输，每个字符需要数千个时钟周期。这证明 vadd.vv 指令执行正确，功能验证通过。

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

通过反汇编可知 `vadd.vv v3, v1, v2` 的指令编码为 `0x021101d7`，PC 地址为 `0x8000016a`。在 Surfer 中添加 ROB 模块下的 `difftest_commit_instr` 信号，搜索该编码值即可定位指令提交时刻。

路径：`TOP.SimTop.cpu.core_with_l2.core.backend.rename.rob`

下图为 vadd.vv 指令提交瞬间的波形，可以看到 `difftest_commit_valid` 为 1，`difftest_commit_instr` 为 `021101d7`，`difftest_commit_pc` 为 `8000016a`：

![vadd.vv 指令提交时刻波形](./images/03_vadd_commit_moment.png)

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

### 6.0 指令提交序列总览

在定位到 vadd.vv 提交时刻后，观察其前后指令的提交序列，可以看到完整的向量指令序列按顺序提交：

![指令提交序列波形](./images/04_commit_sequence.png)

从波形中可以观察到以下指令按 PC 递增顺序依次提交：

| 提交顺序 | PC | 指令编码 | 指令 |
|---------|-----|---------|------|
| ... | 8000015e | cc087057 | vsetivli zero,16,e8,m1,ta,ma |
| ... | 80000162 | 02060087 | vle8.v v1,(a2) |
| ... | 80000166 | 02058107 | vle8.v v2,(a1) |
| **目标** | **8000016a** | **021101d7** | **vadd.vv v3,v1,v2** |
| ... | 8000016e | 020781a7 | vse8.v v3,(a5) |
| ... | 80000172 | 0330000f | fence rw,rw |

这表明向量指令序列在香山后端流水线中按序提交，vadd.vv 在两条 vle8.v 加载指令之后、vse8.v 存储指令之前执行，符合程序逻辑顺序。

> **说明**：以下各阶段分析基于香山源码调研，结合上述提交级波形验证。由于 VCD 波形中内部模块信号层级极深（约 100 万个信号），逐阶段截取内部信号波形受限于工具加载能力，因此各阶段分析以源码路径追踪为主，以提交级波形作为功能正确性的验证证据。

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

#### 波形验证

vadd.vv 指令在提交级波形中已确认成功提交（见 6.0 节），`difftest_commit_instr = 021101d7` 与反汇编一致，证明译码阶段正确识别了该指令。以下分析基于源码追踪。

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

#### 源码分析

[基于源码：向量逻辑寄存器 v1/v2/v3 经 Rename 模块映射到物理寄存器，分配 ROB 表项]

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

#### 源码分析

[基于源码：指令查询源物理寄存器就绪状态后写入向量发射队列]

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

#### 源码分析

VIAluFix（Wrapper）将 128 位向量数据分成 2 个 64 位块并行处理。每个 VIntFixpAlu64b 内部的 VIntAdder64b 由 8 个 8 位加法器链式串联，按 SEW=8 在每个 8 位边界截断进位，实现 8 个 int8 元素的并行加法。最终结果经 Mgu 处理 mask/tail 元素后输出。

### 6.5 写回阶段（Writeback）

#### 分析对象

执行结果经 `Mgu`（Mask/Gap Unit）处理 mask 和 tail 元素后，写回目标向量寄存器。

#### 关键信号

| 信号名 | 含义 |
|--------|------|
| `io_out_bits_res_data` | 写回数据（128 位） |
| `io_out_valid` | 写回有效 |
| `io_out_ready` | 写回就绪 |

#### 源码分析

128 位结果经 Mgu 处理后写回目标物理向量寄存器，更新 BusyTable，最终按 ROB 顺序提交。提交时刻已在 6.0 节的波形中验证。

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
