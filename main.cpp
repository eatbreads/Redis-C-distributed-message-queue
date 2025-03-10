#include <iostream>
#include <hiredis/hiredis.h>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

int main() {
    // 测试 spdlog
    spdlog::info("🚀 依赖库测试开始！");

    // 测试 hiredis 连接 Redis
    redisContext *context = redisConnect("127.0.0.1", 6379);
    if (context == nullptr || context->err) {
        spdlog::error("❌ Redis 连接失败！");
        return -1;
    } else {
        spdlog::info("✅ 成功连接到 Redis！");
        redisFree(context);
    }

    // 测试 Boost.Asio 创建 IO 上下文
    boost::asio::io_context io_context;
    spdlog::info("✅ Boost.Asio 运行正常！");

    // 测试 nlohmann-json
    nlohmann::json test_json = {{"message", "Hello, Redis Task Queue!"}, {"status", "OK"}};
    spdlog::info("✅ JSON 解析成功: {}", test_json.dump());

    spdlog::info("🎉 所有库都可用！");

    return 0;
}
