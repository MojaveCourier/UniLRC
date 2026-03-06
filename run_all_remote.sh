#!/bin/bash
# B 方案：在 client 端执行，通过 SSH 在各 proxy/datanode 节点上启动进程
# 配置文件默认：项目根目录下 project/config/cluster.ini

set -e
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
CONFIG="${1:-$SCRIPT_DIR/project/config/cluster.ini}"

if [ ! -f "$CONFIG" ]; then
  echo "Config not found: $CONFIG"
  echo "Usage: $0 [cluster.ini]"
  exit 1
fi

# 读 INI：取 [section] 下 key 的值（去掉首尾空格）
# 使用字面匹配 [section]，避免在 awk 正则中 [cluster] 被当作字符类
get_ini() {
  local section="$1" key="$2"
  local section_regex='\['"$section"'\]'
  awk -F'=' -v S="$section_regex" -v K="$key" '
    $0 ~ "^[ \t]*" S "[ \t]*$" { f=1; next }
    f && /^[ \t]*\[/ { f=0; next }
    f && /^[ \t]*#/ { next }
    f && NF >= 2 { gsub(/^[ \t]+|[ \t]+$/,"",$1); if ($1 == K) { gsub(/^[ \t]+|[ \t]+$/,"",$2); print $2; exit } }
  ' "$CONFIG"
}

CLUSTER_NUM=$(get_ini cluster cluster_num)
DN_PER=$(get_ini cluster datanode_per_cluster)
FIRST_IP=$(get_ini cluster first_proxy_ip)
FIRST_PORT=$(get_ini cluster first_proxy_port)
DN_PORT_START=$(get_ini cluster datanode_port_start)
COORD_IP=$(get_ini cluster coordinator_ip)
SSH_USER=$(get_ini ssh user)
REMOTE_REPO="$SCRIPT_DIR"

# 校验
[ -n "$CLUSTER_NUM" ] && [ -n "$DN_PER" ] && [ -n "$FIRST_IP" ] && [ -n "$FIRST_PORT" ] || {
  echo "Missing required [cluster] keys in $CONFIG"
  exit 1
}
[ -n "$SSH_USER" ] || {
  echo "Missing required [ssh] user in $CONFIG"
  exit 1
}

# 127.0.0.1 则所有节点 IP 均为 127.0.0.1
if [ "$FIRST_IP" = "127.0.0.1" ]; then
  USE_LOCALHOST=1
else
  USE_LOCALHOST=0
  PREFIX="${FIRST_IP%.*}."
  FIRST_OCTET="${FIRST_IP##*.}"
fi

BUILD="$REMOTE_REPO/project/cmake/build"

# 本机运行时只在最开始 kill 一次，避免每次循环都杀掉已启动的进程
if [ "$USE_LOCALHOST" = 1 ]; then
  pkill -9 run_datanode 2>/dev/null || true
  pkill -9 run_proxy 2>/dev/null || true
  sleep 1
fi

c=0
while [ "$c" -lt "$CLUSTER_NUM" ]; do
  if [ "$USE_LOCALHOST" = 1 ]; then
    NODE_IP="127.0.0.1"
  else
    NODE_IP="${PREFIX}$((FIRST_OCTET + c))"
  fi
  PROXY_PORT=$((FIRST_PORT + c))

  if [ "$USE_LOCALHOST" = 1 ]; then
    REMOTE_CMD="cd $REMOTE_REPO && "
  else
    REMOTE_CMD="cd $REMOTE_REPO && pkill -9 run_datanode 2>/dev/null; pkill -9 run_proxy 2>/dev/null; sleep 1; "
  fi
  d=0
  while [ "$d" -lt "$DN_PER" ]; do
    DP=$((DN_PORT_START + c * DN_PER + d))
    REMOTE_CMD="$REMOTE_CMD$BUILD/run_datanode ${NODE_IP}:${DP} & "
    d=$((d + 1))
  done
  REMOTE_CMD="$REMOTE_CMD sleep 2 && $BUILD/run_proxy ${NODE_IP}:${PROXY_PORT} ${COORD_IP} &"

  echo "Cluster $c: ${SSH_USER}@${NODE_IP} (proxy ${NODE_IP}:${PROXY_PORT})"
  if [ "$NODE_IP" = "127.0.0.1" ]; then
    bash -c "$REMOTE_CMD" || { echo "Failed: cluster $c at $NODE_IP"; }
  else
    ssh -o ConnectTimeout=5 "${SSH_USER}@${NODE_IP}" "$REMOTE_CMD" || {
      echo "Failed: cluster $c at $NODE_IP"
    }
  fi
  c=$((c + 1))
done
echo "Done."
