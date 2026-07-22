# CIE 2026 香山社区 RISC-V 应用创新赛道

## 团队与作品信息

- **团队名称**：rvpeak
- **作品名称**：vdot_llm_xiangshan
- **赛题名称**：面向边缘 AI 大语言模型推理的 RISC-V 自定义 vdot 指令设计——基于"香山"昆明湖 V2 处理器架构

## 仓库说明

本仓库为参赛作品的公开交付仓库，包含设计文档、测试程序、运行报告及代码修改说明。

代码修改的完整版本见以下 fork 仓库：

- 香山 RTL 修改：[待填写，Stage 2 补充]
- NEMU 扩展：[待填写，Stage 3 补充]

## 目录结构

```
.
├── README.md
├── docs/
│   ├── stage1/          # 第一阶段：环境部署与向量加法指令分析
│   │   ├── 01_environment_setup_log.md       # 环境部署验证与操作日志
│   │   ├── 02_vector_add_execution.md        # 向量加法指令执行过程分析
│   │   └── images/                            # 波形截图与终端截图
│   ├── stage2/          # 第二阶段：vdot 设计与实现（待补充）
│   └── stage3/          # 第三阶段：协同仿真与性能分析（待补充）
├── tests/
│   └── stage1/
│       ├── vadd_test.c                       # 向量加法测试程序
│       └── Makefile                           # 编译规则
├── patches/             # 代码补丁（Stage 2/3 补充）
├── reports/             # 测试与性能报告（Stage 2/3 补充）
└── videos/              # 录屏文件
    ├── stage1_operation_recording.webm       # 环境搭建操作录屏
    ├── stage1_vaddtest_run.webm              # vadd-test 运行录屏
    └── stage1_vadd_waveform.webm             # 波形查看录屏
```

## 阶段进展

- [x] **阶段一：环境部署与验证**
  - 环境搭建：xs-env 部署、KunminghuV2Config emu 编译（约 25 小时）
  - 验证输出：`hello xiangshan, I am rvpeak, IP:10.100.173.123`
  - 向量加法指令 vadd.vv 波形分析：通过 Surfer 定位指令提交时刻，分析取指→译码→执行→写回全流程
  - 交付物：环境部署日志、指令执行分析文档、3 段操作录屏
- [ ] 阶段二：vdot 设计与实现
- [ ] 阶段三：协同仿真与评估

## 开发环境

| 项目 | 内容 |
|------|------|
| 开发仓库 | [xs-env](https://github.com/OpenXiangShan/xs-env.git) |
| 开发分支 | `kunminghu-v2`（XiangShan） |
| 仿真器配置 | `KunminghuV2Config`，`EMU_TRACE=1` |
| 工具链 | `riscv64-unknown-linux-gnu-gcc` 16.1.0 |
| Verilator | 4.210 |
| Mill | 0.12.3 |

## 团队信息

- 团队名称：rvpeak
- 参赛单位：[待填写]
- 联系邮箱：[待填写]
