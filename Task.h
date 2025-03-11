#pragma once
#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <queue>
#include <memory>
#include <mutex>
#include <hiredis/hiredis.h>
#include <spdlog/spdlog.h>

class Task {
public:
    using Params = std::unordered_map<std::string, std::string>;

    Task(const std::string& name, const Params& params) : taskName(name), parameters(params) {}
    Task() = default;
    // 获取任务名称
    std::string getName() const { return taskName; }

    // 获取任务参数
    std::string getParam(const std::string& key) const {
        auto it = parameters.find(key);
        return (it != parameters.end()) ? it->second : "";
    }

    // 序列化 Task -> JSON
    std::string toJson() const {
        nlohmann::json j;
        j["task_name"] = taskName;
        j["parameters"] = parameters;
        return j.dump();
    }

    // 反序列化 JSON -> Task
    static Task fromJson(const std::string& jsonString) {
        nlohmann::json j = nlohmann::json::parse(jsonString);
        return Task(j["task_name"].get<std::string>(), j["parameters"].get<Params>());
    }

private:
    std::string taskName;
    Params parameters;
};
class ITaskProcessor {
public:
    virtual ~ITaskProcessor() = default;
    virtual void process(const Task& task) = 0;
};


class TaskProcessorRegistry {
public:
    static void registerProcessor(const std::string& taskType, std::shared_ptr<ITaskProcessor> processor) {
        processors()[taskType] = processor;
    }

    static std::shared_ptr<ITaskProcessor> getProcessor(const std::string& taskType) {
        auto it = processors().find(taskType);
        return (it != processors().end()) ? it->second : nullptr;
    }

private:
    static std::unordered_map<std::string, std::shared_ptr<ITaskProcessor>>& processors() {
        static std::unordered_map<std::string, std::shared_ptr<ITaskProcessor>> instance;
        return instance;
    }
};  
// 任务处理器示例
class ImageProcessingTask : public ITaskProcessor {
public:
    void process(const Task& task) override {
        std::cout << "Processing Image: " << task.getParam("image_path") << std::endl;
        // 实际的图片处理逻辑
    }
};

class VideoTranscodingTask : public ITaskProcessor {
public:
    void process(const Task& task) override {
        std::cout << "Transcoding Video: " << task.getParam("video_path") << std::endl;
        // 实际的视频转码逻辑
    }
};



class TaskQueue {
public:
    TaskQueue(const std::string& redis_host = "127.0.0.1", int redis_port = 6379);
    ~TaskQueue();

    // 任务队列操作
    bool push(const Task& task);  // 任务入队
    bool pop(Task& task);         // 任务出队
    bool setTaskStatus(const std::string& taskId, const std::string& status);
    std::string getTaskStatus(const std::string& taskId);

    // 失败任务处理
    bool pushFailedTask(const Task& task);
    bool popFailedTask(Task& task);

private:
    redisContext* redis_ctx;
    std::mutex queue_mutex;
    const std::string TASK_QUEUE = "task_queue";
    const std::string TASK_STATUS = "task_status";
    const std::string FAILED_TASK_QUEUE = "failed_task_queue";
};
TaskQueue::TaskQueue(const std::string& redis_host, int redis_port) {
    redis_ctx = redisConnect(redis_host.c_str(), redis_port);
    if (redis_ctx == nullptr || redis_ctx->err) {
        spdlog::error("Redis connection failed: {}", redis_ctx ? redis_ctx->errstr : "Unknown error");
        exit(EXIT_FAILURE);
    }
    spdlog::info("Connected to Redis at {}:{}", redis_host, redis_port);
}

TaskQueue::~TaskQueue() {
    if (redis_ctx) {
        redisFree(redis_ctx);
    }
}

// 任务入队
bool TaskQueue::push(const Task& task) {
    std::lock_guard<std::mutex> lock(queue_mutex);
    std::string taskJson = task.toJson();
    
    redisReply* reply = (redisReply*)redisCommand(redis_ctx, "LPUSH %s %s", TASK_QUEUE.c_str(), taskJson.c_str());
    if (!reply) return false;

    bool success = reply->integer > 0;
    freeReplyObject(reply);
    return success;
}

// 任务出队
bool TaskQueue::pop(Task& task) {
    std::lock_guard<std::mutex> lock(queue_mutex);
    redisReply* reply = (redisReply*)redisCommand(redis_ctx, "BRPOP %s 5", TASK_QUEUE.c_str());

    if (!reply || reply->type != REDIS_REPLY_ARRAY || reply->elements < 2) {
        if (reply) freeReplyObject(reply);
        return false;
    }

    std::string taskJson = reply->element[1]->str;
    task=Task::fromJson(taskJson);
    
    freeReplyObject(reply);
    return true;
}

// 设置任务状态
bool TaskQueue::setTaskStatus(const std::string& taskId, const std::string& status) {
    redisReply* reply = (redisReply*)redisCommand(redis_ctx, "HSET %s %s %s", TASK_STATUS.c_str(), taskId.c_str(), status.c_str());
    bool success = reply && reply->integer > 0;
    freeReplyObject(reply);
    return success;
}

// 获取任务状态
std::string TaskQueue::getTaskStatus(const std::string& taskId) {
    redisReply* reply = (redisReply*)redisCommand(redis_ctx, "HGET %s %s", TASK_STATUS.c_str(), taskId.c_str());
    if (!reply || reply->type != REDIS_REPLY_STRING) {
        if (reply) freeReplyObject(reply);
        return "";
    }

    std::string status = reply->str;
    freeReplyObject(reply);
    return status;
}

// 失败任务入队
bool TaskQueue::pushFailedTask(const Task& task) {
    std::lock_guard<std::mutex> lock(queue_mutex);
    std::string taskJson = task.toJson();

    redisReply* reply = (redisReply*)redisCommand(redis_ctx, "LPUSH %s %s", FAILED_TASK_QUEUE.c_str(), taskJson.c_str());
    bool success = reply && reply->integer > 0;
    freeReplyObject(reply);
    return success;
}

// 失败任务出队
bool TaskQueue::popFailedTask(Task& task) {
    std::lock_guard<std::mutex> lock(queue_mutex);
    redisReply* reply = (redisReply*)redisCommand(redis_ctx, "BRPOP %s 5", FAILED_TASK_QUEUE.c_str());

    if (!reply || reply->type != REDIS_REPLY_ARRAY || reply->elements < 2) {
        if (reply) freeReplyObject(reply);
        return false;
    }

    std::string taskJson = reply->element[1]->str;
    task = Task::fromJson(taskJson);
    
    freeReplyObject(reply);
    return true;
}


class TaskConsumer {
public:
    TaskConsumer(TaskQueue& queue) : taskQueue(queue) {}

    void start() {
        while (true) {
            Task task; 
            taskQueue.pop(task); // 从队列获取任务
            auto processor = TaskProcessorRegistry::getProcessor(task.getName());
            if (processor) {
                processor->process(task); // 调用匹配的任务处理器
            } else {
                std::cerr << "No processor found for task: " << task.getName() << std::endl;
            }
        }
    }
private:
    TaskQueue& taskQueue;
};
