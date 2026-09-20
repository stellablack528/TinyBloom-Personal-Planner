# TinyBloom 🌱

一个免费、开源、注重真实使用体验的个人计划管理应用。当前发布的是 **TinyBloom Desktop（Windows 桌面版）**，帮助用户把复杂目标拆分成更容易开始的小步骤。

> **Small Steps, Real Progress.**

## 🌱 普通用户直接下载

如果你只是想使用 TinyBloom，不需要点击 GitHub 页面里的 `Code`、`Compare` 或分支按钮。

👉 [直接下载 TinyBloom Desktop v0.2.0（Windows x64）](https://github.com/stellablack528/TinyBloom-Personal-Planner/releases/download/v0.2.0-desktop/TinyBloom-Desktop-v0.2.0-windows-x64.zip)

下载后解压完整文件夹，双击 `TinyBloom.exe` 即可使用。下面的构建说明主要提供给开发者。

## 📖 项目简介

TinyBloom 是一个基于 **C++20、Qt 6、QML、SQLite 和 CMake** 开发的桌面计划管理应用。

很多时候，真正困难的不是完成任务，而是开始任务。TinyBloom 希望通过任务管理、子任务拆分、平静的每日计划与本地数据存储，帮助用户把一个较大的目标拆分成多个更容易完成的小步骤，并持续看到自己的进步。

项目采用 **免费 + 开源 + Local-first** 的设计理念：

- 核心功能无需注册账号
- 无需订阅
- 不依赖网络服务
- 用户数据默认保存在本地 SQLite 数据库中
- 支持数据导入与导出

## ✨ 主要功能

### 📋 任务管理

TinyBloom 支持基础任务管理能力，包括：

- 创建任务
- 编辑任务
- 删除任务
- 完成 / 恢复任务
- 设置截止日期
- 使用日历或“今天 / 明天”快捷方式选择截止日期
- 设置优先级
- 设置预计时间
- 设置任务分类

### 🧩 子任务

用户可以将一个较大的目标拆分成多个更小的子任务，降低开始大型任务时的心理负担。

例如：

```text
学习计算机网络

✓ 阅读第一节
✓ 整理知识点
□ 完成两道练习题
□ 总结今天的内容
```

### 🌱 每日计划与快速开始

- Today 首页和当日完成进度
- Today / Tomorrow / Later 自动分类
- 快速添加任务与键盘快捷键
- All / Active / Completed 状态筛选
- 根据标题、描述和分类搜索任务

### 🎨 UI 主题

TinyBloom Desktop 支持：

- 🌿 Mint Garden
- 🌙 Midnight
- 🇨🇳 简体中文（默认）
- 🇬🇧 English

主题和语言选择都会保存在本地数据库中，并在下次启动时恢复。

### 🌿 XP 与花园

- 完成任务获得 20 XP，完成小步骤获得 5 XP
- XP 和等级永久保留，植物会随累计进度经历种子、嫩芽、花苞和盛放等阶段
- 花园活力每天会下降 15 点；长期没有行动时，植物会从精神变为疲惫并最终枯萎
- 完成任务或小步骤可恢复活力，枯萎的植物也始终可以重新养好
- 每个任务和小步骤只发放一次 XP，反复取消和勾选不会重复获得奖励

### 💾 本地数据

TinyBloom 使用 SQLite 保存本地数据，包括：

- Tasks
- Subtasks
- Settings
- Growth profile（XP、等级与花园活力）

同时支持：

- Export JSON
- Import JSON
- 导入前自动创建安全备份，避免误覆盖现有数据

## 🛠️ 技术栈

| 技术 | 用途 |
| --- | --- |
| C++20 | 核心业务逻辑 |
| Qt 6 | 桌面应用框架 |
| Qt Quick / QML | UI 与动画 |
| SQLite | 本地数据持久化 |
| Qt SQL | SQLite 数据访问 |
| CMake | 项目构建 |
| Qt Test / CTest | 自动化测试 |

## 🏗️ 项目架构

TinyBloom 采用 C++ 与 QML 分离的结构。

```text
QML UI
  ↓
Application Controller
  ↓
Managers
  ↓
Database Manager
  ↓
SQLite
```

核心模块包括：

- TaskManager
- GrowthManager
- SettingsManager
- DatabaseManager
- DataService
- TaskListModel

C++ 主要负责：

- 数据管理
- 业务逻辑
- 任务系统
- 数据库操作
- JSON 导入与导出
- 输入校验与分类筛选

QML 主要负责：

- 页面布局
- UI 组件
- 动画
- 主题
- 用户交互

## 📂 项目结构

```text
tinybloom/
├── CMakeLists.txt
├── README.md
├── LICENSE
├── docs/
│   ├── architecture.md
│   └── development-log.md
│
├── src/
│   ├── main.cpp
│   ├── models/
│   ├── managers/
│   ├── database/
│   └── services/
│
├── qml/
│   ├── Main.qml
│   ├── pages/
│   ├── components/
│   └── themes/
│
├── resources/icons/
├── translations/
└── tests/
```

## 🚀 运行方式

### 环境要求

运行和构建 TinyBloom 需要：

- C++20
- Qt 6.x
- CMake 3.20+
- 支持 C++20 的编译器
- Qt SQLite Driver

### Clone

```bash
git clone https://github.com/stellablack528/TinyBloom-Personal-Planner.git
cd TinyBloom-Personal-Planner
```

### Configure

```bash
cmake -S . -B build
```

### Build

```bash
cmake --build build --config Release
```

构建完成后，运行生成的 TinyBloom 可执行文件。

## 🧪 测试

项目重点测试以下内容：

- 任务创建与编辑
- 任务完成
- 子任务
- SQLite 数据保存与读取
- 数据导入与导出
- 主题切换
- 中英文语言设置
- 设置持久化

测试命令：

```bash
ctest --test-dir build --output-on-failure
```

## 🔐 隐私

TinyBloom 默认采用本地优先设计。

用户的任务、设置和个人数据默认保存在本地设备中。项目不要求：

- 注册账号
- 登录
- 云端数据库
- 网络连接

核心功能可以在离线环境下使用。

## 🎯 项目目标

TinyBloom 希望成为一个：

- 真正免费的个人计划工具
- 简单易用的任务管理工具
- 注重用户体验的桌面软件
- C++ / Qt 学习项目
- 开源社区项目

项目的重点不是堆叠尽可能多的功能，而是让用户真正愿意使用它。

当功能数量与使用体验发生冲突时，优先保证：

> 简单、流畅、清晰和实用。

## 🗺️ Roadmap

### v0.1

- 基础桌面框架
- 任务管理
- 子任务、搜索与筛选
- SQLite 持久化
- Today 页面
- 双主题与中英文界面
- JSON 导入与导出

### v0.1.1

- Windows 应用与可执行文件图标
- 导入确认与自动安全备份
- 日期选择器和快捷日期
- 紧凑的任务弹窗与完整的筛选按钮文字
- 深色主题可读性和键盘无障碍改进

### v0.2

- Windows 桌面版发布
- XP、等级与完成反馈
- 会成长也会枯萎的个人花园
- 活力恢复与每日衰减机制

### v0.3

- Focus Timer
- 基础统计
- 成就系统

### v0.4

- 更多植物与花园装饰
- 更丰富的成长阶段

### v0.5

- 更多主题
- 更多数据备份与恢复选项
- 个性化设置

### v1.0

- 稳定版本
- 完整测试
- 完整文档
- 跨平台支持

### Future：移动端与同步

- 独立的 TinyBloom Mobile 客户端
- 桌面端与移动端共享稳定的数据结构版本
- 在充分验证隐私与冲突处理后，再引入可选的跨端同步

## 🤖 AI-assisted Development

TinyBloom 也用于实践 AI 辅助软件开发。

开发流程：

```text
Product Idea
  ↓
Requirement Design
  ↓
Architecture Design
  ↓
C++ Implementation
  ↓
QML UI
  ↓
SQLite Integration
  ↓
Testing
  ↓
Bug Fixing
  ↓
Performance Optimization
  ↓
Documentation
```

AI 生成的代码需要经过实际编译、运行、测试和修改后，才能进入项目。

## 👤 Author

**CS Student · Backend & Systems Enthusiast**

Currently learning and experimenting with:

- Modern C++ / C++20
- Linux Network Programming
- Concurrency & Server Architecture
- Database Fundamentals
- Qt Desktop Development

Mostly learning by building small projects and experimenting with ideas I find interesting.

## 📄 License

MIT License.

See [LICENSE](LICENSE) for details.

---

🌱 **Small Steps, Real Progress.**
