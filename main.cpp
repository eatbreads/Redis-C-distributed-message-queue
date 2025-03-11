#include <iostream>
#include "Task.h"


int main() {
    // 1. 创建任务队列实例
    TaskQueue taskQueue("localhost", 6379);

    // 2. 注册任务处理器
    TaskProcessorRegistry::registerProcessor("image_processing", std::make_shared<ImageProcessingTask>());
    TaskProcessorRegistry::registerProcessor("video_transcoding", std::make_shared<VideoTranscodingTask>());

    // 3. 生产任务
    Task task1("image_processing", {{"image_path", "/path/to/image.jpg"}});
    Task task2("video_transcoding", {{"video_path", "/path/to/video.mp4"}});

    taskQueue.push(task1);
    taskQueue.push(task2);

    // 4. 启动任务消费者
    TaskConsumer consumer(taskQueue);
    consumer.start();

    // 5. 运行一段时间后停止
    std::this_thread::sleep_for(std::chrono::seconds(10));

    return 0;
}