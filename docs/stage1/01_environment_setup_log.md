# 阶段一：香山开发环境部署与验证操作日志

## 一、硬件与系统环境

| 项目 | 内容 |
|------|------|
| 操作系统及版本 | Ubuntu 22.04 LTS (Linux 6.8.0-124-generic) |
| CPU 型号 | x86_64 |
| CPU 核心数 | 20 |
| 内存容量 | 23 GB |
| 磁盘可用空间 | 59 GB / 99 GB |
| 团队/个人标识 | rvpeak |

## 二、依赖软件版本

| 软件 | 版本 | 查看命令 |
|------|------|----------|
| gcc | 11.4.0 | `gcc --version` |
| g++ | 11.4.0 | `g++ --version` |
| Java | OpenJDK 21.0.11 | `java -version` |
| mill | 0.12.3 | `mill --version` |
| sbt | 未安装（本项目未使用） | - |
| Verilator | 5.051 | `verilator --version` |
| Git | 2.34.1 | `git --version` |

## 三、环境部署步骤

### 3.1 克隆 xs-env

```bash
cd /home/yym/xiangshanoc
git clone https://github.com/OpenXiangShan/xs-env.git
cd xs-env
```

实际执行时间：[待填写]  
执行结果：[待填写，例如成功 / 遇到网络问题已解决]

### 3.2 安装系统依赖

```bash
sudo -s ./setup-tools.sh
```

执行结果：基础依赖安装顺利，但 `mill` 的自动下载环节受网络影响失败。

> **实际经历**：`setup-tools.sh` 会尝试从网络下载 `mill` 构建工具。由于国内代理网络不稳定，GitHub 连接经常会被中断或卡掉，无论是直接访问还是通过国内镜像中转代理，都无法稳定完成 `mill` 的自动下载。此前学习 Chisel 时曾经历过类似问题，当时采用 `sbt` 替代 `mill` 完成构建。本次尝试直接使用 `mill`，但自动下载仍然失败。
>
> **解决方法**：手动前往 [GitHub mill releases](https://github.com/com-lihaoyi/mill/releases) 页面，下载与香山环境匹配版本的 `mill` 可执行文件，将其重命名为 `mill` 后放置到系统 `PATH` 目录（例如 `/usr/local/bin/`）中，并赋予可执行权限。完成手动安装后，执行 `mill --version` 确认可用。
>
> ```bash
> # 示例：手动安装 mill
> sudo cp /path/to/downloaded/mill /usr/local/bin/mill
> sudo chmod +x /usr/local/bin/mill
> mill --version
> ```

### 3.3 配置环境变量

```bash
source setup.sh
source env.sh
```

执行结果：[待填写]

### 3.4 初始化香山项目

```bash
cd $NOOP_HOME
make init
```

执行结果：[待填写]

### 3.5 编译仿真器

```bash
cd $NOOP_HOME
make emu CONFIG=MinimalConfig EMU_TRACE=1 -j$(nproc)
```

编译耗时：首次编译约 **5 小时**，第二次重新编译约 **13 分钟**。  
编译结果：`build/emu` 可执行文件生成成功。

> **实际经历**：香山仿真器的首次编译涉及两大耗时环节：一是 Chisel 代码生成 Verilog，二是 Verilator 将 Verilog 编译为可仿真 C++ 程序并链接成可执行文件。在 20 核 CPU 上使用 `-j18` 并行编译，首次完整编译总共耗时约 5 小时。这属于正常现象，主要耗时来自 Verilator 对大规模 RTL 的编译和优化。
>
> 在代码没有大幅改动的情况下，第二次重新编译由于增量编译和缓存机制，耗时大幅缩短至约 13 分钟。因此建议在环境搭建时预留充足的首次编译时间，避免误以为环境配置失败。

> **说明**：
> - `CONFIG=MinimalConfig`：使用香山最小配置进行编译，移除了 L2/L3 缓存等复杂模块，可降低首次编译复杂度，适合环境验证。
> - `EMU_TRACE=1`：启用仿真跟踪功能，生成波形文件（如 `.vcd` 或 `.fst`），便于后续分析指令执行过程。
> - 安装 `riscv64-unknown-linux-gnu-gcc` 后，编译 nexus-am 测试程序时无需再加 `LINUX_GNU_TOOLCHAIN=1` 参数。

## 四、遇到的问题及解决方案

| 问题描述 | 解决方法 |
|----------|----------|
| `mill` 自动下载失败 | 由于 GitHub 网络不稳定，国内代理和中转镜像均无法可靠下载 `mill`。改为手动从 mill 官方 release 页面下载对应版本，重命名后放入 `PATH`，验证 `mill --version` 可用。 |
| 香山仿真器首次编译耗时极长 | 首次编译需要完成 Chisel → Verilog → Verilator C++ → 可执行文件的完整流程，在 20 核机器上耗时约 5 小时。预留足够时间，确认进程未中断即可。第二次编译借助增量编译缩短至约 13 分钟。 |
| nexus-am 默认工具链不匹配 | 环境默认调用 `riscv64-unknown-linux-gnu-gcc`，但本机最初只安装了 `riscv64-linux-gnu-gcc`。临时解决方案是编译测试程序时添加 `LINUX_GNU_TOOLCHAIN=1` 参数；最终解决方案是从 RISC-V GNU Toolchain 官方 release 下载并安装真正的 `riscv64-unknown-linux-gnu-gcc`。 |

### 环境搭建心路历程

本次环境搭建是整个赛题中最基础但也最容易卡住的环节。最大的挑战并非操作步骤本身，而是**国内网络环境对 GitHub 资源访问的不稳定性**。

在运行 `setup-tools.sh` 时，`mill` 的自动下载多次失败。尝试切换不同代理和中转镜像后，问题依然存在，GitHub 连接会在下载过程中被中断。此前学习 Chisel 时已经遇到过类似情况，当时选择用 `sbt` 绕过 `mill`。本次比赛希望直接按照香山官方流程使用 `mill`，因此改为手动下载安装：找到对应 release 版本的可执行文件，改名后放入系统 `PATH`，最终验证通过。

另一个深刻印象是**香山仿真器的编译规模**。虽然官方文档提到首次编译较慢，但实际在 20 核机器上运行 `-j18` 仍然花费了约 5 小时才完成从 Chisel 到 Verilator 可执行文件的完整流程。期间需要保持终端稳定，避免因为编译时间过长而误判为卡死。第二次重新编译时，由于增量构建，时间缩短到约 13 分钟，说明环境一旦搭好，后续迭代效率很高。

这些经历让我对工业级处理器开发环境有了更真实的认识：环境搭建本身就是工程能力的一部分，处理网络、依赖、编译时间问题是后续所有开发工作的基础。

## 五、环境验证

### 5.1 Hello XiangShan 原始验证

```bash
cd $AM_HOME/apps/hello/
make ARCH=riscv64-xs
cd $NOOP_HOME
./build/emu -i $AM_HOME/apps/hello/build/hello-riscv64-xs.bin --no-diff
```

实际输出：

```
hello xiangshan, I am rvpeak, IP address
Core 0: HIT GOOD TRAP at pc = 0x8000014c
Core-0 instrCnt = 1,053, cycleCnt = 6,698, IPC = 0.157211
```

### 5.2 赛题要求验证

修改 `$AM_HOME/apps/hello/hello.c` 源码，将输出改为：

```
hello xiangshan, I am rvpeak, IP address
```

修改内容：

```c
#include <klib.h>

int main()
{
    printf("hello xiangshan, I am rvpeak, IP address\n");
    return 0;
}
```

编译运行命令：

```bash
cd $AM_HOME/apps/hello/
make ARCH=riscv64-xs
cd $NOOP_HOME
./build/emu -i $AM_HOME/apps/hello/build/hello-riscv64-xs.bin --no-diff
```

实际输出：

```
The image is /home/yym/xs-env/nexus-am/apps/hello/build/hello-riscv64-xs.bin
hello xiangshan, I am rvpeak, IP address
Core 0: HIT GOOD TRAP at pc = 0x8000014c
Core-0 instrCnt = 1,053, cycleCnt = 6,698, IPC = 0.157211
Seed=0 Guest cycle spent: 6,702
Host time spent: 9,548ms
```

截图/录屏：[待补充]

## 六、当前环境状态

| 项目 | 路径/Commit |
|------|-------------|
| xs-env 路径 | `/home/yym/xs-env` |
| xs-env 最新 commit | `d0e08fb` |
| XiangShan 最新 commit | `16ae9ddcd` |
| NEMU 最新 commit | `c5b49241` |
| nexus-am 最新 commit | `92b36da1` |

## 七、RISC-V 交叉编译工具链安装与配置

### 5.1 nexus-am 默认工具链

nexus-am 的 `am/arch/isa/riscv64.mk` 中默认配置如下：

```makefile
ifeq ($(LINUX_GNU_TOOLCHAIN),1)
CROSS_COMPILE := riscv64-linux-gnu-
else
CROSS_COMPILE := riscv64-unknown-linux-gnu-
endif
```

默认情况下，nexus-am 会调用 `riscv64-unknown-linux-gnu-gcc`。

### 5.2 工具链版本差异

RISC-V GNU 工具链按目标库和 vendor 字段有多种变体：

| 工具链 | 目标环境 | 默认 C 库 | 用途 |
|--------|----------|-----------|------|
| `riscv64-unknown-linux-gnu-gcc` | Linux ABI | glibc（本环境下载的预编译版） | RISC-V GNU 工具链官方命名，香山 nexus-am 默认使用 |
| `riscv64-linux-gnu-gcc` | Linux ABI | glibc | Ubuntu/Debian 发行版官方包，功能等价 |
| `riscv64-unknown-elf-gcc` | 裸机 ELF | newlib | 用于没有操作系统的嵌入式程序 |

### 5.3 初始环境与问题

环境初始化后，本机已安装 `riscv64-linux-gnu-gcc`（gcc 11.4.0）和 `riscv64-unknown-elf-gcc`（gcc 10.2.0），但缺少 nexus-am 默认调用的 `riscv64-unknown-linux-gnu-gcc`。因此最初编译 hello 时必须加参数：

```bash
make ARCH=riscv64-xs LINUX_GNU_TOOLCHAIN=1
```

虽然这种方式可以工作，但每次输入较长，且与官方默认流程不一致，因此决定安装真正的 `riscv64-unknown-linux-gnu-gcc`。

### 5.4 安装 `riscv64-unknown-linux-gnu-gcc`

从 [RISC-V GNU Toolchain 官方 Release 页面](https://github.com/riscv-collab/riscv-gnu-toolchain/releases) 下载适用于 Ubuntu 22.04 的预编译版本：

```text
文件：riscv64-glibc-ubuntu-22.04-gcc.tar.xz
大小：668 MB
sha256：79f4aba41bda21262f154c22172e3473566927da7a78d5e2555b6a820083b8e4
下载地址：https://github.com/riscv-collab/riscv-gnu-toolchain/releases/download/2026.07.12/riscv64-glibc-ubuntu-22.04-gcc.tar.xz
```

> **说明**：`glibc` 版本提供 Linux ABI 工具链，包含 `riscv64-unknown-linux-gnu-gcc`，符合 nexus-am 默认需求。选择 `elf` 版本则会得到 `riscv64-unknown-elf-gcc`，不适用于 `ARCH=riscv64-xs`。

安装步骤：

```bash
# 1. 解压到 /home/yym/riscv/
mkdir -p /home/yym/riscv
cd /home/yym/riscv
tar -xJf /path/to/riscv64-glibc-ubuntu-22.04-gcc.tar.xz

# 2. 添加到 PATH（写入 ~/.bashrc），确保新终端中可用
echo 'export PATH=/home/yym/riscv/riscv/bin:$PATH' >> ~/.bashrc

# 3. 重新加载 ~/.bashrc 或打开新终端
source ~/.bashrc

# 4. 验证工具链可用
which riscv64-unknown-linux-gnu-gcc
riscv64-unknown-linux-gnu-gcc --version
```

### 5.5 验证安装

```bash
riscv64-unknown-linux-gnu-gcc --version
```

输出示例：

```text
riscv64-unknown-linux-gnu-gcc (g6afcc4f6d) 16.1.0
Copyright (C) 2026 Free Software Foundation, Inc.
```

### 5.6 编译验证

安装完成后，无需再加 `LINUX_GNU_TOOLCHAIN=1`：

```bash
cd $AM_HOME/apps/hello/
make ARCH=riscv64-xs
cd $NOOP_HOME
./build/emu -i $AM_HOME/apps/hello/build/hello-riscv64-xs.bin --no-diff
```

输出：

```text
hello xiangshan, I am rvpeak, IP address
Core 0: HIT GOOD TRAP at pc = 0x8000014c
```

## 八、结论

环境部署成功，可正常编译香山仿真器并运行 Hello XiangShan 程序。已按赛题要求修改并运行 hello 程序，成功输出 `hello xiangshan, I am rvpeak, IP address`。当前 xs-env 中各子模块版本稳定，具备进入第二阶段 vdot 指令设计与实现的条件。

后续待补充：
- 3.1、3.3、3.4 各步骤的具体执行时间与结果（需要你补充）
- 第一阶段操作过程录屏（需要你录制后上传）
