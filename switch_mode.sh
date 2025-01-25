#!/bin/bash

# 定义配置文件目录
CONFIG_DIR="/users/qiliang/UniEC/project/config"

# 定义 hosts 文件路径
HOSTS_FILE="hosts"

# 检查 hosts 文件是否存在
if [[ ! -f "$HOSTS_FILE" ]]; then
  echo "Error: hosts file not found!"
  exit 1
fi

# 读取 hosts 文件中的主机列表
HOSTS=$(cat "$HOSTS_FILE" | tr '\n' ',' | sed 's/,$//')



# 提示用户输入字符串 XXX
read -p "Enter the string XXX: " XXX

# 检查输入的字符串是否为空
if [[ -z "$XXX" ]]; then
  echo "Error: Input string cannot be empty!"
  exit 1
fi

# 定义源文件和目标文件路径
SOURCE_FILE="$CONFIG_DIR/$XXX.xml"
TARGET_FILE="$CONFIG_DIR/parameterConfiguration.xml"

# 使用 pdsh 在所有主机上复制文件
echo "Copying $SOURCE_FILE to $TARGET_FILE on all hosts..."
pdsh -w "$HOSTS" cp "$SOURCE_FILE" "$TARGET_FILE"

# 检查是否成功复制
if [[ $? -eq 0 ]]; then
  echo "Successfully copied $SOURCE_FILE to $TARGET_FILE on all hosts!"
else
  echo "Failed to copy $SOURCE_FILE to $TARGET_FILE on some hosts!"
  exit 1
fi

echo "All done!"