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
get_ini() {
  local section="$1" key="$2"
  awk -F'=' -v S="[$section]" -v K="$key" '
    $0 ~ S { f=1; next }
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

for (( c=0; c<CLUSTER_NUM; c++ )); do
  if [ "$USE_LOCALHOST" = 1 ]; then
    NODE_IP="127.0.0.1"
  else
    NODE_IP="${PREFIX}$((FIRST_OCTET + c))"
  fi
  PROXY_PORT=$((FIRST_PORT + c))

  REMOTE_CMD="cd $REMOTE_REPO && pkill -9 run_datanode 2>/dev/null; pkill -9 run_proxy 2>/dev/null; sleep 1; "
  for (( d=0; d<DN_PER; d++ )); do
    DP=$((DN_PORT_START + c * DN_PER + d))
    REMOTE_CMD+="$BUILD/run_datanode ${NODE_IP}:${DP} & "
  done
  REMOTE_CMD+="sleep 2 && $BUILD/run_proxy ${NODE_IP}:${PROXY_PORT} ${COORD_IP} &"

  echo "Cluster $c: ${SSH_USER}@${NODE_IP} (proxy ${NODE_IP}:${PROXY_PORT})"
  ssh -o ConnectTimeout=5 "${SSH_USER}@${NODE_IP}" "$REMOTE_CMD" || {
    echo "Failed: cluster $c at $NODE_IP"
  }
done
echo "Done."
