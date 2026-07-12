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
| sbt / mill | sbt 2.0.1 / [待填写] | `sbt --version` / `mill --version` |
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

执行结果：[待填写]

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

编译耗时：[待填写]  
编译结果：[待填写，例如 build/emu 已生成]

> **说明**：本环境使用 `riscv64-linux-gnu-` 工具链编译 nexus-am 应用，因此编译测试程序时需要加上 `LINUX_GNU_TOOLCHAIN=1` 参数。

## 四、遇到的问题及解决方案

| 问题描述 | 解决方法 |
|----------|----------|
| [待填写] | [待填写] |
| [待填写] | [待填写] |

## 五、环境验证

### 5.1 Hello XiangShan 原始验证

```bash
cd $AM_HOME/apps/hello/
make ARCH=riscv64-xs LINUX_GNU_TOOLCHAIN=1
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
make ARCH=riscv64-xs LINUX_GNU_TOOLCHAIN=1
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

## 七、结论

环境部署成功，可正常编译香山仿真器并运行 Hello XiangShan 程序。已按赛题要求修改并运行 hello 程序，成功输出 `hello xiangshan, I am rvpeak, IP address`。当前 xs-env 中各子模块版本稳定，具备进入第二阶段 vdot 指令设计与实现的条件。

后续待补充：
- 3.1–3.5 各步骤的具体执行时间与详细结果
- 部署过程中遇到的实际问题及解决方案
- 第一阶段操作过程录屏
