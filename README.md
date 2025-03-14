# 分布式任务队列（Distributed Task Queue）

## 📌 开发目标
本项目旨在使用 **C++ 和 Redis** 构建一个 **高性能的分布式任务队列**，用于后台异步任务处理。  
核心目标包括：
- **任务队列管理**：使用 Redis 作为任务存储队列，实现生产者-消费者模式。
- **任务并行处理**：支持多个消费者同时处理任务，提高吞吐量。
- **任务状态管理**：使用 Redis 哈希表存储任务状态（`running`、`done`、`failed`）。
- **失败任务重试机制**：对于执行失败的任务，存入失败队列并定期重试。
- **实时任务通知**：使用 Redis 发布/订阅（Pub/Sub）机制，通知任务完成状态。
- **扩展性**：支持多个 Redis 实例和多个 C++ 进程，提高系统稳定性和性能。

---

## 🛠 技术栈
| 组件          | 说明 |
|--------------|------|
| **语言**     | C++（C++17 或更新版本） |
| **任务队列** | Redis（使用 `List` 结构存储任务） |
| **Redis 客户端** | hiredis（轻量级 C++ Redis 库） |
| **异步处理** | Boost.Asio（用于并发任务执行） |
| **日志管理** | spdlog（用于日志记录） |
| **任务序列化** | JSON / MsgPack（使用 `nlohmann/json`） |
| **构建工具** | CMake |
| **容器化部署** | Docker |

---

## 类介绍

1. **Task（任务）**：封装任务名称和参数，支持 JSON 序列化。  
2. **TaskQueue（任务队列）**：基于 Redis 存储任务，支持任务入队、出队、状态管理和失败任务处理。  
3. **ITaskProcessor（任务处理器接口）**：定义任务处理的标准接口。  
4. **具体任务处理器（ImageProcessingTask、VideoTranscodingTask）**：实现不同类型任务的处理逻辑。  
5. **TaskProcessorRegistry（任务处理器注册表）**：管理所有任务处理器的注册和查找。  
6. **TaskConsumer（任务消费者）**：从任务队列取出任务，并交给对应的任务处理器执行。  

整个流程类似 **快递系统**：任务进入队列（仓库），消费者（快递员）取出任务，并由相应的处理器（分拣员）执行任务。

---

## 📌 开发阶段计划

### 🔹 **阶段 1：环境搭建**
1. 安装 Redis 并配置基本环境。
2. 安装 `hiredis`、`Boost.Asio`、`spdlog` 等依赖库。
3. 创建 C++ 项目目录结构，并编写 `CMakeLists.txt`。

### 🔹 **阶段 2：实现任务生产者**
1. 连接 Redis 并初始化任务队列。
2. 任务封装为 JSON 格式，存入 Redis `task_queue`。
3. 通过 Redis 命令 `LPUSH` 实现任务入队。

### 🔹 **阶段 3：实现任务消费者**
1. 从 Redis `task_queue` 获取任务（`BRPOP`）。
2. 解析任务 JSON，模拟执行任务逻辑。
3. 任务完成后，更新 Redis `task_status` 表。

### 🔹 **阶段 4：任务状态管理**
1. 使用 Redis `HSET` 记录任务状态（`running`、`done`、`failed`）。
2. 任务执行成功后，更新状态到 `done`。
3. 任务失败时，存入 `failed_task_queue` 以便后续重试。

### 🔹 **阶段 5：失败任务处理**
1. 设定任务最大重试次数，超过次数后标记 `failed`。
2. 编写一个独立的任务重试机制，定期检查 `failed_task_queue` 重新执行任务。

### 🔹 **阶段 6：实时任务通知**
1. 任务完成时，使用 Redis `PUBLISH` 发送通知。
2. 编写一个 `subscriber` 监听 `task_update` 频道，处理任务完成消息。

### 🔹 **阶段 7：优化与扩展**
1. 使用 **Lua 脚本** 优化批量任务调度，提高 Redis 处理效率。
2. 任务完成后持久化存储到 MySQL / PostgreSQL。
3. 通过 Docker 部署 Redis 服务器及多个 C++ 任务消费者，实现负载均衡。

---

## 📌 未来扩展
- **支持 Web 管理界面**：提供任务监控 Dashboard，实时显示任务状态。
- **支持多 Redis 实例**：实现 Redis Cluster，增强任务队列的可扩展性。
- **任务优先级调度**：支持不同优先级的任务处理，确保高优先级任务优先执行。

---

## 📌 结语
本项目提供了一个 **高效、可扩展** 的 C++ 任务队列解决方案，适用于 **图片处理、日志分析、视频转码、邮件发送** 等异步任务处理场景。  

欢迎贡献和优化！🚀
