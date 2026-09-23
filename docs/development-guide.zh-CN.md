# TinyBloom Desktop 开发文档

> 面向 TinyBloom 维护者、贡献者，以及希望通过真实项目学习 C++、Qt 6、QML、SQLite 和并发编程的开发者。

TinyBloom Desktop 是一个本地优先的 Windows 个人计划工具。它不仅提供普通待办事项，还通过长期任务地图、XP、花园成长、活力衰减和专注自习室，帮助用户把现实中的持续行动转化为可见的正反馈。

本文档描述 `main` 分支当前开发版。已发布安装包可能暂时落后于本文档，请以 Git 标签和 Release 页面为准。

## 目录

- [1. 设计目标](#1-设计目标)
- [2. 功能概览](#2-功能概览)
- [3. 技术栈](#3-技术栈)
- [4. 开发环境](#4-开发环境)
- [5. 构建与运行](#5-构建与运行)
- [6. 项目结构](#6-项目结构)
- [7. 总体架构](#7-总体架构)
- [8. 数据模型与 SQLite](#8-数据模型与-sqlite)
- [9. 任务模型与界面刷新](#9-任务模型与界面刷新)
- [10. 长期任务地图](#10-长期任务地图)
- [11. XP、活力与花园](#11-xp活力与花园)
- [12. 线程池与回调](#12-线程池与回调)
- [13. 图形渲染与兼容模式](#13-图形渲染与兼容模式)
- [14. 国际化](#14-国际化)
- [15. 数据导入导出](#15-数据导入导出)
- [16. 自动测试与界面验收](#16-自动测试与界面验收)
- [17. Windows 打包与发布](#17-windows-打包与发布)
- [18. 开发约定](#18-开发约定)
- [19. 常见问题](#19-常见问题)
- [20. 后续演进方向](#20-后续演进方向)

## 1. 设计目标

TinyBloom 遵循以下原则：

1. **Local-first**：核心功能离线可用，用户数据默认只保存在本机。
2. **先帮助用户行动**：功能的目标不是堆砌选项，而是降低开始一件事的心理成本。
3. **正反馈与适度督促并存**：XP 永久保留；活力会随长期不行动而下降，但用户始终可以重新养好花朵。
4. **普通任务保持轻量**：日常事项不强迫用户建立复杂结构；只有大目标才使用长期任务地图。
5. **明确线程边界**：QML、SQLite 和后台工作遵守 Qt 的线程归属规则，不以“多线程”为名引入随机崩溃。
6. **可迁移、可恢复**：数据库升级需要兼容旧数据；导入前必须创建安全备份。
7. **真实发布标准**：代码必须经过编译、自动测试和实际窗口验收，才能进入发布流程。

## 2. 功能概览

### 2.1 普通任务

- 创建、编辑、完成、恢复和删除任务。
- 设置说明、截止日期、优先级、预计时长和分类。
- 将任务拆分为多个小步骤。
- 按全部、进行中、今天、明天、稍后和已完成筛选。
- 按标题、说明和分类搜索。
- 使用 `Ctrl+N` 新建任务，使用 `Ctrl+F` 搜索。

### 2.2 长期任务

- 单独的大任务页面。
- 大任务包含愿景和目标日期。
- 使用“大任务 → 阶段 → 行动步骤”的目标地图。
- 阶段和步骤均可新增、改名、完成和删除。
- 自动计算叶子步骤完成率。
- 所有最终步骤完成后，大任务自动完成。

### 2.3 花园自习室

- 两个独立花位。
- 支持小粉花、向日葵、郁金香和玫瑰。
- 花朵根据播种后的 XP 独立成长。
- 支持重新播种、成长动画和活力状态。
- 内置 25 分钟专注计时器，可开始、暂停和重置。

### 2.4 个性化与数据

- 简体中文默认，支持 English。
- Mint Garden 和 Midnight 主题。
- 减少动画与兼容渲染模式。
- SQLite 本地存储。
- JSON 导入、导出和导入前安全备份。
- Windows 图标、便携包和安装程序。

## 3. 技术栈

| 技术 | 用途 |
| --- | --- |
| C++20 | 业务逻辑、模型、数据库和后台服务 |
| Qt 6.5+ | 桌面应用框架；当前主要在 Qt 6.10.1 上验证 |
| Qt Quick / QML | 页面、组件、动画和状态绑定 |
| Qt Quick Controls 2 | Button、Dialog、ListView、Layout 等控件 |
| Qt SQL / SQLite | 本地持久化 |
| `QAbstractListModel` | 向 QML 提供增量任务列表 |
| `QThreadPool` | 后台 JSON 序列化和文件写入 |
| `QMetaObject::invokeMethod` | 从工作线程回调 UI 线程 |
| `QPointer` | 防止异步回调访问已销毁对象 |
| `QSaveFile` | 原子式导出和备份写入 |
| Qt Linguist | 中英文翻译、TS/QM 资源 |
| CMake 3.21+ | 配置、构建、资源和测试 |
| Qt Test / CTest | 核心行为与持久化测试 |
| Inno Setup | Windows 安装程序 |

TinyBloom 当前不需要 MySQL、Redis 或云服务器。只有未来加入可选的账号与跨设备同步后，才需要重新设计服务端数据模型、认证、冲突解决和加密策略。

## 4. 开发环境

### 4.1 必需组件

- Windows 10 或 Windows 11。
- Qt 6.5 或更高版本，安装以下模块：
  - Qt Quick
  - QML
  - Qt Quick Controls 2
  - Qt SQL 和 SQLite Driver
  - Qt Linguist Tools
  - Qt Test
- 支持 C++20 的编译器。
- CMake 3.21 或更高版本。
- Ninja（推荐）。
- Git。

仓库当前主要使用 Qt 6.10.1 与 MinGW 13.1 进行验证。编译器、Qt 库和部署工具必须来自同一套工具链，不要混用 MSVC Qt 与 MinGW 编译器。

### 4.2 推荐方式：Qt Creator

1. 用 Qt Creator 打开仓库根目录的 `CMakeLists.txt`。
2. 选择 `Desktop Qt 6.x MinGW 64-bit` Kit。
3. 使用独立构建目录，例如 `build/Desktop_Qt_6_x_MinGW_64_bit-Debug`。
4. 点击 Configure Project。
5. 构建并运行 `TinyBloom`。

### 4.3 命令行环境

PowerShell 示例，路径需要替换成自己的 Qt 安装位置：

```powershell
$env:Path = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\6.10.1\mingw_64\bin;$env:Path"
cmake -S . -B build-debug -G Ninja `
  -DCMAKE_BUILD_TYPE=Debug `
  -DCMAKE_PREFIX_PATH=C:\Qt\6.10.1\mingw_64
```

如果 CMake 提示编译器探测失败，优先检查 MinGW 的 `bin` 目录是否已经加入 `PATH`。只传入 `g++.exe` 路径并不够，因为编译器还需要调用同目录下的辅助程序。

## 5. 构建与运行

### 5.1 Debug 构建

```powershell
cmake --build build-debug
```

生成的可执行文件通常位于：

```text
build-debug/TinyBloom.exe
```

### 5.2 Release 构建

```powershell
cmake -S . -B build-release -G Ninja `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_PREFIX_PATH=C:\Qt\6.10.1\mingw_64
cmake --build build-release
```

### 5.3 数据隔离运行

正常运行时，数据库位置由 `QStandardPaths::AppDataLocation` 决定。开发和测试时可以指定独立数据库，避免污染真实数据：

```powershell
$env:TINYBLOOM_DATABASE_PATH = "$PWD\work\dev.sqlite3"
& .\build-debug\TinyBloom.exe
```

### 5.4 强制软件渲染

显卡驱动不稳定时，可以从命令行启用兼容模式：

```powershell
$env:TINYBLOOM_SOFTWARE_RENDERING = "1"
& .\build-debug\TinyBloom.exe
```

也可以在应用设置中打开“兼容渲染模式”，重启后生效。

## 6. 项目结构

```text
TinyBloom/
├── CMakeLists.txt
├── README.md
├── LICENSE
├── docs/
│   ├── architecture.md
│   ├── development-guide.zh-CN.md
│   └── development-log.md
├── src/
│   ├── main.cpp
│   ├── database/
│   │   └── DatabaseManager.*
│   ├── managers/
│   │   ├── TaskManager.*
│   │   ├── GrowthManager.*
│   │   └── SettingsManager.*
│   ├── models/
│   │   ├── Task.*
│   │   ├── Subtask.h
│   │   └── TaskListModel.*
│   └── services/
│       └── DataService.*
├── qml/
│   ├── Main.qml
│   ├── pages/
│   ├── components/
│   └── themes/
├── resources/
│   └── icons/
├── translations/
│   └── tinybloom_zh_CN.ts
├── tests/
│   └── tst_core.cpp
└── packaging/windows/
    ├── TinyBloom.iss
    └── build-installer.ps1
```

## 7. 总体架构

```text
┌──────────────────────────────────────┐
│ QML Presentation                    │
│ Pages / Components / Theme / Dialog │
└──────────────────┬───────────────────┘
                   │ 属性、信号、Q_INVOKABLE
┌──────────────────▼───────────────────┐
│ Application Managers                │
│ Task / Growth / Settings            │
└───────────────┬───────────┬──────────┘
                │           │
┌───────────────▼──────┐ ┌──▼────────────────┐
│ Models               │ │ DataService       │
│ QAbstractListModel   │ │ JSON / ThreadPool │
└───────────────┬──────┘ └──┬────────────────┘
                │           │
┌───────────────▼───────────▼──────────┐
│ DatabaseManager / SQLite             │
└──────────────────────────────────────┘
```

### 7.1 QML 层

QML 只负责表现和短生命周期的界面状态，例如当前页面、选中的筛选项、弹窗是否打开。QML 不执行 SQL，也不直接操作数据库文件。

重要页面：

- `TodayPage.qml`：今日计划与进度。
- `TasksPage.qml`：任务搜索、筛选与列表。
- `LongTermPage.qml`：大任务列表与目标地图。
- `GardenPage.qml`：花园、自习室和专注计时器。
- `SettingsPage.qml`：语言、主题、渲染和数据操作。

### 7.2 Manager 层

- `TaskManager`：任务校验、CRUD、筛选、长期任务树和自动完成逻辑。
- `GrowthManager`：XP、活力、播种和植物阶段。
- `SettingsManager`：设置读取、持久化与变更通知。

Manager 的修改流程应遵循：

```text
校验输入 → 写入 SQLite → 更新内存快照 → 刷新模型 → 发出信号
```

数据库写入失败时，不应该先修改内存状态。

### 7.3 Service 层

`DataService` 负责跨模块的数据工作：

- JSON 导入导出。
- 导入前备份。
- 后台文件写入。
- 操作成功或失败通知。

## 8. 数据模型与 SQLite

TinyBloom 使用一个 `DatabaseManager` 和一个 SQLite 连接。启动时启用：

```sql
PRAGMA foreign_keys = ON;
```

### 8.1 `tasks`

保存普通任务和长期任务根节点。

关键字段：

| 字段 | 含义 |
| --- | --- |
| `id` | 自增主键 |
| `title` | 标题，1–160 字符 |
| `description` | 说明 |
| `completed` | 是否完成 |
| `priority` | 0 低、1 中、2 高 |
| `due_date` | ISO 日期，可空 |
| `estimated_minutes` | 预计时长 |
| `category` | 分类 |
| `created_at` / `updated_at` | UTC ISO 8601 时间 |
| `completed_at` | 完成时间 |
| `experience_awarded` | 是否已经发放一次性 XP |
| `long_term` | 是否为长期任务 |

### 8.2 `subtasks`

普通任务的小步骤和长期任务树节点共用此表。

| 字段 | 含义 |
| --- | --- |
| `task_id` | 所属任务，删除任务时级联删除 |
| `parent_id` | 0 表示一级阶段；非 0 表示所属阶段 |
| `title` | 节点名称 |
| `completed` | 是否完成 |
| `experience_awarded` | 是否已经发放一次性 XP |

长期任务当前限制为两层步骤。这是有意的产品约束：目标地图需要帮助用户行动，而不是演变成无限层级的知识图谱。

### 8.3 `settings`

键值表，当前保存：

- `theme`
- `language`
- `reduceAnimations`
- `compatibilityRendering`
- `defaultTaskDuration`
- 两个花位的品种和播种时 XP 基线

### 8.4 `growth_profile`

单行表，保存：

- 累计 XP。
- 有进展的天数。
- 当天 XP。
- 最近进展日期。
- 当前活力。
- 上次活力结算日期。

### 8.5 Schema 演进

`createTables()` 可以重复调用，并通过 `ensureColumn()` 为旧数据库增加字段。添加字段时必须：

1. 为新安装更新 `CREATE TABLE`。
2. 为旧安装增加 `ensureColumn()`。
3. 为读取、插入、更新、导出和导入路径补充字段。
4. 添加旧数据库迁移测试。
5. 更新 JSON `schemaVersion`。

不要要求用户删除数据库来解决迁移问题。

## 9. 任务模型与界面刷新

`TaskManager` 在内存中保存一份 `QVector<Task>`，多个 `TaskListModel` 共享它并提供不同视图：

- 全部普通任务。
- 今天。
- 明天。
- 稍后。
- 长期任务。

模型内部使用稳定的任务 ID，而不是保存易失效的数组下标。刷新时比较 ID，发出：

- `rowsInserted`
- `rowsRemoved`
- `rowsMoved`
- `dataChanged`

不要在普通任务修改时使用 `beginResetModel()`。全量重置会让 QML `ListView` 重新创建所有 delegate，导致用户勾选或删除任务后列表滚回顶部。

新增模型角色时，需要同时更新：

1. `TaskListModel::Role`。
2. `data()`。
3. `roleNames()`。
4. 相关 QML delegate。
5. 模型测试。

## 10. 长期任务地图

长期任务仍然使用 `Task` 作为根节点，通过 `longTerm=true` 与普通任务区分。

```text
Task(longTerm=true)
├── Subtask(parentId=0)              阶段
│   ├── Subtask(parentId=阶段 ID)    行动步骤
│   └── Subtask(parentId=阶段 ID)    行动步骤
└── Subtask(parentId=0)              可直接完成的独立步骤
```

完成率只统计叶子节点：

- 有子节点的阶段不计入分母。
- 阶段没有子节点时，它本身就是可执行叶子节点。
- 所有叶子节点完成后，根任务自动完成。
- 后续添加未完成叶子节点时，根任务会恢复为进行中。

删除一级阶段时，`TaskManager` 先删除其子节点，再删除阶段节点，最后重新计算完成状态。

目标地图的 UI 保持可横向滚动，以兼容较小窗口；默认窗口宽度优先保证根目标、阶段和行动步骤同时可见。

## 11. XP、活力与花园

当前奖励常量：

| 行为 | XP | 活力恢复 |
| --- | ---: | ---: |
| 完成任务 | 20 | 28 |
| 完成小步骤/行动步骤 | 5 | 10 |

每条任务和步骤只有第一次完成时发放奖励。数据库中的 `experience_awarded` 防止用户通过反复取消和勾选刷 XP。

活力与 XP 是两种不同信号：

- XP 表示永久积累，不会下降。
- 活力表示近期照料状态，每个没有行动的自然日下降 15 点。

植物品种和播种基线保存在设置表中。每个花位的成长值是：

```text
当前累计 XP - 该花位播种时的 XP
```

因此两株花可以独立播种和成长，重新播种只重置对应花位的成长基线，不会删除用户已获得的总 XP。

## 12. 线程池与回调

### 12.1 为什么引入线程池

大型 JSON 的格式化和文件写入不应该阻塞 UI。`DataService` 使用 `QThreadPool` 执行后台导出，并将最大工作线程数限制为 2：

```cpp
const int availableWorkers = qMax(1, QThread::idealThreadCount() - 1);
m_threadPool.setMaxThreadCount(qMin(2, availableWorkers));
```

限制线程数可以给 GUI 线程、Qt Quick 渲染线程和操作系统保留资源。线程越多并不必然越快。

### 12.2 线程边界

```text
UI / 数据库线程
  1. 从 SQLite 生成 QJsonObject 快照
  2. 设置 busy=true
  3. 提交到 QThreadPool
            ↓
工作线程
  4. JSON 序列化
  5. QSaveFile 写入与 commit
            ↓
Queued callback
  6. QMetaObject::invokeMethod 回到 UI 线程
  7. 设置 busy=false
  8. 发出成功或失败信号
```

SQLite 连接不能直接跨线程使用。当前方案只把不可变的 `QJsonObject` 快照交给工作线程，从而避免数据库线程归属问题。

### 12.3 回调安全

后台 lambda 捕获 `QPointer<DataService>`。如果应用在任务完成前关闭，`QPointer` 会自动变为空，工作线程不会访问已销毁对象。

完成回调使用：

```cpp
QMetaObject::invokeMethod(service, callback, Qt::QueuedConnection);
```

不要从工作线程直接修改 QML 属性或发起界面操作。

### 12.4 生命周期

`DataService` 析构时调用 `waitForDone()`，保证线程池中的文件任务结束。新增后台任务时必须继续考虑：

- 应用关闭期间是否安全。
- 是否可以取消。
- 回调目标是否仍然存在。
- 是否会与下一次导入/导出冲突。

## 13. 图形渲染与兼容模式

Qt Quick 默认使用硬件加速的 Scene Graph/RHI 渲染路径。Qt 自己已经管理 GUI 线程和渲染线程，因此增加业务线程池不能修复显卡驱动崩溃。

TinyBloom 提供独立的软件渲染回退：

```cpp
QQuickWindow::setGraphicsApi(QSGRendererInterface::Software);
```

必须在创建第一个 `QQuickWindow` 前设置图形 API。当前启动流程在读取设置后、加载 QML 引擎前完成该操作。

可以通过两种方式启用：

1. 设置页打开“兼容渲染模式”，重启应用。
2. 设置环境变量 `TINYBLOOM_SOFTWARE_RENDERING=1`。

兼容模式更稳定，但复杂动画可能比硬件渲染慢。排查图形问题时应分别验证：

- 默认硬件渲染。
- 软件渲染。
- 减少动画。
- 显卡驱动版本和远程桌面环境。

## 14. 国际化

英文源文本写在 C++ 的 `tr()` 和 QML 的 `qsTr()` 中，简体中文翻译位于：

```text
translations/tinybloom_zh_CN.ts
```

更新翻译：

```powershell
lupdate qml src -ts translations\tinybloom_zh_CN.ts
```

然后使用 Qt Linguist 编辑，或谨慎修改 TS 文件。发布前必须确认不存在未完成翻译：

```powershell
Select-String translations\tinybloom_zh_CN.ts 'type="unfinished"'
```

CMake 的 `qt_add_translations()` 会生成 QM 文件并嵌入资源。运行时通过 `QTranslator` 安装中文翻译，通过 `QQmlApplicationEngine::retranslate()` 实时刷新 QML。

新增用户可见文本时，不要直接硬编码中文作为唯一界面文本；应使用英文源文本并补充中文翻译。测试场景中的示例用户数据不属于界面文案，可以直接使用确定的测试字符串。

## 15. 数据导入导出

导出 JSON 包含：

- `application`
- `platform`
- `version`
- `schemaVersion`
- `exportedAt`
- `tasks`
- `subtasks`
- `settings`
- `growth`

应用版本和数据结构版本是两个概念：

- `version` 用于描述产品版本。
- `schemaVersion` 用于判断数据结构兼容性。

导入流程：

1. 读取并解析 JSON。
2. 检查必需字段。
3. 检查重复任务 ID、重复步骤 ID和无效父子关系。
4. 在数据库旁创建带时间戳的安全备份。
5. 开启 SQLite 事务。
6. 重建任务、步骤、设置和成长数据。
7. 任意步骤失败时回滚，保留原数据。
8. 提交后重新加载 Manager。

未来移动端同步不能直接复用“导入覆盖”语义。同步需要单独设计全局 ID、修改时间、删除墓碑和冲突解决策略。

## 16. 自动测试与界面验收

### 16.1 运行核心测试

```powershell
ctest --test-dir build-debug --output-on-failure
```

如果 Windows 报 `0xc0000135`，通常不是测试逻辑失败，而是 Qt/MinGW DLL 不在 `PATH`。先加入对应 Qt 和 MinGW `bin` 目录再运行。

当前核心测试覆盖：

- 任务 CRUD 与持久化。
- 子任务与级联删除。
- 长期任务树、自动完成和备份恢复。
- 搜索、日期分类和输入校验。
- 列表增量更新，不触发 `modelReset`。
- 设置持久化。
- XP 一次性奖励和活力衰减。
- 两个花位的独立成长。
- 旧数据库自动迁移。
- JSON 导入导出与失败保护。
- 线程池后台导出和 UI 回调。

### 16.2 截图验收钩子

以下环境变量仅用于隔离测试：

| 变量 | 用途 |
| --- | --- |
| `TINYBLOOM_DATABASE_PATH` | 指定临时数据库 |
| `TINYBLOOM_LANGUAGE_OVERRIDE` | 强制测试语言 |
| `TINYBLOOM_SCREENSHOT_SCENARIO` | 准备稳定的界面状态 |
| `TINYBLOOM_SCREENSHOT_PATH` | 保存窗口截图后退出 |
| `TINYBLOOM_SOFTWARE_RENDERING` | 强制软件渲染 |

可用场景包括：

- `tasks`
- `tasks-demo`
- `long-term-demo`
- `garden`
- `garden-showcase`
- `garden-seeds`
- `settings-midnight`
- `task-dialog`
- `task-dialog-advanced`
- `date-picker`

截图只能补充自动测试，不能替代真实鼠标、键盘、滚动和安装包体验。

## 17. Windows 打包与发布

### 17.1 生成便携目录

先完成 Release 构建，然后准备目录：

```text
dist/TinyBloom-Desktop-vX.Y.Z-windows-x64/
```

将 `TinyBloom.exe` 放入目录，再使用与编译器匹配的 `windeployqt`：

```powershell
windeployqt --release --qmldir qml `
  dist\TinyBloom-Desktop-vX.Y.Z-windows-x64\TinyBloom.exe
```

在干净目录或另一台电脑上运行便携版，确认 Qt DLL、平台插件、SQLite Driver 和 QML 模块齐全。

### 17.2 生成安装程序

安装 Inno Setup 6 或 7，然后运行：

```powershell
powershell -ExecutionPolicy Bypass `
  -File packaging\windows\build-installer.ps1 `
  -Version X.Y.Z
```

脚本会生成：

- Windows x64 安装程序。
- SHA-256 校验文件。

### 17.3 发布前版本同步

发布版本号必须同时检查：

- `CMakeLists.txt` 的 `project(... VERSION ...)`。
- `src/main.cpp` 的应用版本。
- `qml/pages/SettingsPage.qml` 的版本显示。
- `src/database/DatabaseManager.cpp` 的导出产品版本。
- `packaging/windows/build-installer.ps1` 默认版本。
- `packaging/windows/TinyBloom.iss` 默认版本。
- README 下载链接和 Roadmap。

### 17.4 发布检查表

- [ ] Debug 与 Release 均能构建。
- [ ] CTest 全部通过。
- [ ] TS 文件没有 unfinished 翻译。
- [ ] 默认中文和 English 均人工检查。
- [ ] Mint 与 Midnight 均人工检查。
- [ ] 普通任务、长期任务、花园和设置页实际操作通过。
- [ ] 默认渲染和软件渲染至少各启动一次。
- [ ] 全新安装、覆盖安装和卸载通过。
- [ ] 便携版在完整解压后可运行。
- [ ] 导出、导入和安全备份通过。
- [ ] 安装包和 ZIP 名称明确包含 Desktop、版本和架构。
- [ ] 生成 SHA-256。
- [ ] 创建中文 Release Notes、Git 标签和 Pre-release/Stable 标记。

## 18. 开发约定

### 18.1 C++

- 使用 C++20。
- QObject 派生类明确父对象和生命周期。
- 对只读返回值使用 `[[nodiscard]]`。
- SQL 使用参数绑定，不拼接用户输入。
- 用户可见错误应本地化，技术细节写入日志。
- 数据库成功后再修改内存状态。
- 不跨线程共享 `QSqlDatabase` 连接。

### 18.2 QML

- 页面放在 `qml/pages/`。
- 可复用控件放在 `qml/components/`。
- 颜色来自 `Theme.qml`，避免散落硬编码主题色。
- 用户可见字符串使用 `qsTr()`。
- 自定义交互控件补充 `Accessible.name` 和键盘操作。
- 长列表使用模型和 delegate，不在 QML 手工复制数据。
- 动画时长使用主题的 `animationDuration`，尊重减少动画设置。

### 18.3 Git

推荐使用 Conventional Commits：

```text
feat: 新增长期任务目标地图
fix: 修复连续完成任务时列表跳回顶部
docs: 补充线程池与渲染兼容开发说明
test: 增加旧数据库迁移测试
refactor: 拆分数据导出后台任务
```

提交前至少运行：

```powershell
git diff --check
ctest --test-dir build-debug --output-on-failure
```

不要把本地数据库、构建目录、截图验收数据或个人文档混入功能提交。

## 19. 常见问题

### CMake 找不到 Qt

确认 `CMAKE_PREFIX_PATH` 指向包含 `lib/cmake/Qt6` 的 Qt Kit 根目录，而不是 Qt Creator 或 Tools 目录。

### MinGW 编译器检测失败但没有清楚报错

将 MinGW 的完整 `bin` 目录加入 `PATH`。编译器需要找到 `cc1plus`、汇编器和链接器等辅助程序。

### 程序能在开发机运行，发给同学却打不开

只发送一个 EXE 不够。必须运行 `windeployqt`，并发送完整便携目录或使用 Inno Setup 安装程序。

### 测试返回 `0xc0000135`

Qt 或 MinGW DLL 不在测试进程的 `PATH`。这通常是运行环境问题，不是断言失败。

### 勾选任务后列表跳到顶部

检查是否对模型执行了全量 reset。普通 CRUD 应继续使用稳定 ID 和行级信号。

### QML 编译成功但程序启动时页面不可用

QML AOT 编译不能代替真实运行。查看 `QQmlApplicationEngine failed to load component` 后面的具体文件和行号，并运行截图场景进行窗口验收。

### 显卡或远程桌面环境中渲染异常

先启用兼容渲染模式并重启，再尝试减少动画。线程池不会修复显卡驱动问题。

### 为什么没有 MySQL 和 Redis

TinyBloom 当前是单用户、本地优先桌面应用。SQLite 更易安装、备份和离线使用。Redis 缓存对当前数据规模没有收益，只会增加部署成本。

## 20. 后续演进方向

### 近期适合继续完善

- 长期任务拖拽排序。
- 阶段折叠和完成历史。
- 专注记录持久化与统计。
- 更完整的键盘操作和屏幕阅读器测试。
- 安装包自动化构建。
- 崩溃日志与可选诊断导出。

### 移动端与同步前置工作

在加入云同步前，需要先完成：

- 稳定的跨端 schema 版本策略。
- UUID，而不是依赖本地自增 ID。
- 每条记录的修改时间和删除墓碑。
- 冲突解决规则。
- 端到端认证与传输加密。
- 用户可理解的数据导出和账号删除机制。

不要因为未来可能有移动端，就提前把当前离线桌面应用强行改造成依赖服务器的系统。

## 推荐博客选题

本项目适合拆成以下系列文章：

1. 用 C++20、Qt 6 与 QML 搭建 Local-first 桌面应用。
2. QML 如何调用 C++ Manager，以及属性、信号和 `Q_INVOKABLE` 的作用。
3. 使用 `QAbstractListModel` 增量更新，修复 ListView 自动跳动。
4. SQLite 外键、事务和兼容旧版本的 Schema 迁移。
5. 用邻接表设计可执行的长期任务地图。
6. XP 永久成长与活力衰减为什么要分成两个系统。
7. `QThreadPool`、lambda、队列回调和 `QPointer` 的真实项目用法。
8. 为什么多线程不能直接修复 Qt Quick 图形崩溃。
9. Qt Linguist 中英文运行时切换。
10. 从一个 EXE 到真正可分发的 Windows 安装程序。

---

TinyBloom 的工程目标不是为了展示用了多少技术，而是让每项技术都解决一个真实问题：数据必须可靠、界面必须流畅、后台工作不能卡住用户、显卡异常必须有回退路径，而每一个完成的小步骤都应该让用户感受到进展。

