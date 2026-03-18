#!/bin/bash

HOSTS_FILE="hosts"

USER="root"

REMOTE_COMMAND="cd /users/Fengming/UniLRC && sh run_proxy_datanode.sh"
read -p "请输入要远程执行的命令(默认: cd /users/Fengming/UniLRC && sh run_proxy_datanode.sh): " input_command
if [ -z "$input_command" ]; then
    REMOTE_COMMAND="cd /users/Fengming/UniLRC && sh run_proxy_datanode.sh"
else
    REMOTE_COMMAND="$input_command"
fi

PARALLEL=50

echo "Running command on all nodes..."
sudo pdsh -R ssh -w ^$HOSTS_FILE -l $USER -f $PARALLEL "$REMOTE_COMMAND"

if [ $? -eq 0 ]; then
	echo "Command executed successfully on all nodes."
else
	echo "Failed to execute command on some nodes."
fi
