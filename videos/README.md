# 视频与截图目录

本目录存放第一阶段提交所需的录屏和截图文件。

## 文件说明

| 文件名 | 用途 | 对应赛题要求 |
|--------|------|--------------|
| `Screencast from 2026年07月13日 14时54分49秒.webm` | 第一阶段完整操作过程录屏 | 系统视频提交位置 ① |
| `hello_xiangshan_screenshot.png` | hello 程序运行结果截图（待补充） | 环境部署验证佐证 |

## 录制建议

### 录屏内容应包含

1. 打开终端
2. 进入 xs-env 目录：`cd /home/yym/xs-env`
3. 配置环境：`source env.sh`
4. 编译香山仿真器：`cd $NOOP_HOME && make emu CONFIG=MinimalConfig EMU_TRACE=1 -j$(nproc)`
   - 注：首次编译耗时较长，若已编译过可跳过此步；录屏时如需展示，可剪辑或仅展示开始与结束片段。
5. 编译 hello 程序：`cd $AM_HOME/apps/hello && make ARCH=riscv64-xs`
6. 运行 hello 程序：`cd $NOOP_HOME && ./build/emu -i $AM_HOME/apps/hello/build/hello-riscv64-xs.bin --no-diff`
7. 清晰显示输出：`hello xiangshan, I am rvpeak, IP address`

### 截图内容应包含

- 终端中 hello 程序的运行输出
- 关键信息：`hello xiangshan, I am rvpeak, IP address` 和 `HIT GOOD TRAP`
