# 阶段一：香山开发环境部署与验证操作日志

## 一、硬件与系统环境

| 项目 | 内容 |
|------|------|
| 操作系统及版本 | [待填写，例如 Ubuntu 22.04 LTS] |
| CPU 型号 | [待填写] |
| CPU 核心数 | [待填写] |
| 内存容量 | [待填写] |
| 磁盘可用空间 | [待填写] |
| 团队/个人标识 | [待填写，例如 RVPeak] |

## 二、依赖软件版本

| 软件 | 版本 | 查看命令 |
|------|------|----------|
| gcc | [待填写] | `gcc --version` |
| g++ | [待填写] | `g++ --version` |
| Java | [待填写] | `java -version` |
| sbt / mill | [待填写] | `sbt --version` / `mill --version` |
| Verilator | [待填写] | `verilator --version` |
| Git | [待填写] | `git --version` |

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
make emu CONFIG=MinimalConfig EMU_TRACE=1 -j$(nproc)
```

编译耗时：[待填写]  
编译结果：[待填写，例如 build/emu 已生成]

## 四、遇到的问题及解决方案

| 问题描述 | 解决方法 |
|----------|----------|
| [待填写] | [待填写] |
| [待填写] | [待填写] |

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
[待粘贴运行输出]
```

### 5.2 赛题要求验证

修改 `$AM_HOME/apps/hello/` 源码，输出：

```
hello xiangshan, I am [团队/个人标识], IP address
```

修改内容：

```c
// [待填写修改后的 hello 程序源码]
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
[待粘贴运行输出]
```

截图/录屏：[待补充]

## 六、当前环境状态

| 项目 | 路径/Commit |
|------|-------------|
| xs-env 路径 | [待填写] |
| xs-env 最新 commit | [待填写] |
| XiangShan 最新 commit | [待填写] |
| NEMU 最新 commit | [待填写] |
| nexus-am 最新 commit | [待填写] |

## 七、结论

[待填写，例如：]  
环境部署成功，可正常编译香山仿真器并运行 Hello XiangShan 程序。已按赛题要求输出指定验证信息，具备进入第二阶段开发的条件。
