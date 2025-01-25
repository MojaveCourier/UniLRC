#!/bin/bash

# 定义源文件路径
SOURCE_FILE="/users/qiliang/UniEC/small_tools/generator_sh.py"

# 定义 hosts 文件路径
HOSTS_FILE="hosts"

# 检查 hosts 文件是否存在
if [[ ! -f "$HOSTS_FILE" ]]; then
  echo "Error: hosts file not found!"
  exit 1
fi

# 读取 hosts 文件中的主机列表
HOSTS=$(cat "$HOSTS_FILE" | tr '\n' ',' | sed 's/,$//')

# 检查 pdsh 是否安装
if ! command -v pdsh &> /dev/null; then
  echo "Error: pdsh is not installed!"
  exit 1
fi

# 使用 pdsh 复制文件到所有主机
echo "Copying $SOURCE_FILE to all hosts..."
pdsh -w "$HOSTS" scp "$SOURCE_FILE" localhost:/users/qiliang/UniEC/small_tools/

# 检查文件是否成功复制
if [[ $? -eq 0 ]]; then
  echo "Successfully copied to all hosts!"
else
  echo "Failed to copy to some hosts!"
  exit 1
fi

# 使用 pdsh 在所有主机上运行 Python 脚本
echo "Running generator_sh.py on all hosts..."
pdsh -w "$HOSTS" python3 /users/qiliang/UniEC/small_tools/generator_sh.py

# 检查脚本是否成功运行
if [[ $? -eq 0 ]]; then
  echo "Successfully ran generator_sh.py on all hosts!"
else
  echo "Failed to run generator_sh.py on some hosts!"
  exit 1
fi

echo "All done!"