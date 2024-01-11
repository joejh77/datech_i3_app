#!/bin/bash

make $1

#[ $? -ne 0 ] && exit 1

if [ ! -f "../$USER.env" ]; then
	echo -e "\033[31mEnv file (../$USER.env) not found.\033[0m"
	exit
else
	. ../$USER.env
fi

if [ $USE_SSHFS == 1 ]; then
	#when using SSHFS
	TARGET_CONNECTED=`mount | grep $SSHFS_MOUNT_PATH | wc -l`

	if [ $TARGET_CONNECTED == 1 ]; then

		if [ $CP_RECORDER == 1 ]; then
			echo -e "\033[31mcp recorder to /data via sshfs\033[0m"
			cp -uf recorder $SSHFS_MOUNT_PATH/data
		fi

		if [ $CP_SNIFFER == 1 ]; then
			echo -e "\033[31mcp sniffer to /data via sshfs\033[0m"
			cp -uf sniffer $SSHFS_MOUNT_PATH/data
		fi

		if [ $CP_SPLAYER == 1 ]; then
			echo -e "\033[31mcp splayer to /data via sshfs\033[0m"
			cp -uf splayer $SSHFS_MOUNT_PATH/data
		fi

		if [ $CP_MPLAYER == 1 ]; then
			echo -e "\033[31mcp mplayer to /data via sshfs\033[0m"
			cp -uf mplayer $SSHFS_MOUNT_PATH/data
		fi

		if [ $CP_DUAL == 1 ]; then
			echo -e "\033[31mcp dual to /data via sshfs\033[0m"
			cp -uf dual $SSHFS_MOUNT_PATH/data
		fi

		if [ $CP_DUAL2 == 1 ]; then
			echo -e "\033[31mcp dual2 to /data via sshfs\033[0m"
			cp -uf dual2 $SSHFS_MOUNT_PATH/data
		fi

		if [ $CP_SCREENCAP == 1 ]; then
			echo -e "\033[31mcp screencap to /data via sshfs\033[0m"
			cp -uf screencap $SSHFS_MOUNT_PATH/data
		fi

		if [ $CP_PREVIEWER == 1 ]; then
			echo -e "\033[31mcp previewer to /data via sshfs\033[0m"
			cp -uf previewer $SSHFS_MOUNT_PATH/data
		fi

		if [ $CP_PARK1FPS == 1 ]; then
			echo -e "\033[31mcp park1fps to /data via sshfs\033[0m"
			cp -uf park1fps $SSHFS_MOUNT_PATH/data
		fi

	else
		echo -e "\033[31mSSHFS Target not connected.\033[0m"
		echo -e "\033[31mRun sshfs root@타겟보드ip주소:/ $SSHFS_MOUNT_PATH -o sftp_server=\"/data/sftp-server\"\033[0m"
	fi
else
	#when using ADB
	TARGET_CONNECTED=`adb devices | grep $ADB_DEVICE_NAME | wc -l`

	if [ $TARGET_CONNECTED == 1 ]; then

		if [ $CP_RECORDER == 1 ]; then
			echo -e "\033[31mcp recorder to /data via adb\033[0m"
			adb -s $ADB_DEVICE_NAME push recorder /data | grep -Ev '\[\s.*\].*'
		fi

		if [ $CP_SNIFFER == 1 ]; then
			echo -e "\033[31mcp sniffer to /data via adb\033[0m"
			adb -s $ADB_DEVICE_NAME push sniffer /data | grep -Ev '\[\s.*\].*'
		fi

		if [ $CP_SPLAYER == 1 ]; then
			echo -e "\033[31mcp splayer to /data via adb\033[0m"
			adb -s $ADB_DEVICE_NAME push splayer /data | grep -Ev '\[\s.*\].*'
		fi

		if [ $CP_MPLAYER == 1 ]; then
			echo -e "\033[31mcp mplayer to /data via adb\033[0m"
			adb -s $ADB_DEVICE_NAME push mplayer /data | grep -Ev '\[\s.*\].*'
		fi

		if [ $CP_DUAL == 1 ]; then
			echo -e "\033[31mcp dual to /data via adb\033[0m"
			adb -s $ADB_DEVICE_NAME push dual /data | grep -Ev '\[\s.*\].*'
		fi

		if [ $CP_DUAL2 == 1 ]; then
			echo -e "\033[31mcp dual2 to /data via adb\033[0m"
			adb -s $ADB_DEVICE_NAME push dual2 /data | grep -Ev '\[\s.*\].*'
		fi

		if [ $CP_SCREENCAP == 1 ]; then
			echo -e "\033[31mcp screencap to /data via adb\033[0m"
			adb -s $ADB_DEVICE_NAME push screencap /data | grep -Ev '\[\s.*\].*'
		fi

		if [ $CP_PREVIEWER == 1 ]; then
			echo -e "\033[31mcp previewer to /data via adb\033[0m"
			adb -s $ADB_DEVICE_NAME push previewer /data | grep -Ev '\[\s.*\].*'
		fi

		if [ $CP_PARK1FPS == 1 ]; then
			echo -e "\033[31mcp park1fps to /data via adb\033[0m"
			adb -s $ADB_DEVICE_NAME push park1fps /data | grep -Ev '\[\s.*\].*'
		fi

	else
		echo -e "\033[31mADB Target not connected.\033[0m"
	fi

fi
