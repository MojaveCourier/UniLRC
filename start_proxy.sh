#!/bin/bash

HOSTS_FILE="proxy_hosts"

USER="root"

REMOTE_COMMAND="cd /users/qiliang/UniLRC && sh run_proxy_datanode.sh"

PARALLEL=50

echo "Running command on all nodes..."
sudo pdsh -R ssh -w ^$HOSTS_FILE -l $USER -f $PARALLEL "$REMOTE_COMMAND"

if [ $? -eq 0 ]; then
	echo "Command executed successfully on all nodes."
else
	echo "Failed to execute command on some nodes."
fi
