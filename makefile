# 设置构建目录
BUILD_DIR = build

# 默认目标
all:
	@cmake -S . -B $(BUILD_DIR)
	@cmake --build $(BUILD_DIR)

# 清理构建目录
clean:
	@rm -rf $(BUILD_DIR)

.PHONY: all clean