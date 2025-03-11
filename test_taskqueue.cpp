#include <gtest/gtest.h>
#include <spdlog/spdlog.h>
#include "Task.h"  // 假设你已经将 TaskQueue 类和其他类声明包含进来了

// 测试 Task 类的序列化和反序列化
TEST(TaskTest, TaskSerialization) {
    Task::Params params = {{"key1", "value1"}, {"key2", "value2"}};
    Task task("TestTask", params);
    
    // 序列化
    std::string json = task.toJson();
    
    // 反序列化
    Task deserializedTask = Task::fromJson(json);
    
    // 校验反序列化结果
    EXPECT_EQ(deserializedTask.getName(), "TestTask");
    EXPECT_EQ(deserializedTask.getParam("key1"), "value1");
    EXPECT_EQ(deserializedTask.getParam("key2"), "value2");
}

// 测试 TaskQueue 入队和出队
TEST(TaskQueueTest, PushPopTask) {
    TaskQueue taskQueue;
    Task::Params params = {{"image_path", "/path/to/image.png"}};
    Task task("ImageProcessing", params);
    
    // 入队
    ASSERT_TRUE(taskQueue.push(task));
    
    // 出队
    Task poppedTask;
    ASSERT_TRUE(taskQueue.pop(poppedTask));
    
    EXPECT_EQ(poppedTask.getName(), "ImageProcessing");
    EXPECT_EQ(poppedTask.getParam("image_path"), "/path/to/image.png");
}

// 测试 TaskQueue 任务状态管理
TEST(TaskQueueTest, SetGetTaskStatus) {
    TaskQueue taskQueue;
    std::string taskId = "1234";
    
    // 设置任务状态
    ASSERT_TRUE(taskQueue.setTaskStatus(taskId, "in_progress"));
    
    // 获取任务状态
    std::string status = taskQueue.getTaskStatus(taskId);
    EXPECT_EQ(status, "in_progress");
}

// 测试失败任务队列
TEST(TaskQueueTest, FailedTaskHandling) {
    TaskQueue taskQueue;
    Task::Params params = {{"video_path", "/path/to/video.mp4"}};
    Task task("VideoTranscoding", params);
    
    // 入队失败任务
    ASSERT_TRUE(taskQueue.pushFailedTask(task));
    
    // 出队失败任务
    Task failedTask;
    ASSERT_TRUE(taskQueue.popFailedTask(failedTask));
    
    EXPECT_EQ(failedTask.getName(), "VideoTranscoding");
    EXPECT_EQ(failedTask.getParam("video_path"), "/path/to/video.mp4");
}

