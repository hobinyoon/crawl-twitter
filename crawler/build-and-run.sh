#!/bin/bash

set -e
set -u

# Useful for debugging
#set -x

BIN_DIR=`cd "$( dirname "$0" )"; pwd`
pushd $BIN_DIR > /dev/null
DEF_CLASS_PATH_FILE=.dep-class-path
TARGET_JAR=target/crawl-twitter-0.2.jar

function build_src {
	need_build=false
	if [ ! -f $TARGET_JAR ]; then
		need_build=true
	else
		# compare modification time of the class path file and src directory tree
		MT_TJ=`find $TARGET_JAR -printf "%Ty%Tm%Td-%TH%TM%TS\n"`
		MT_SRC=`find src -name "*.java" -printf "%Ty%Tm%Td-%TH%TM%TS\n" | sort | tail -n 1`
		if [[ "$MT_TJ" < "$MT_SRC" ]]; then
			need_build=true
			echo "Src last modification time:        "$MT_SRC
			echo "Target jar last modification time: "$MT_TJ
		fi
	fi

	if [ "$need_build" = true ] ; then
		echo "Building package ... "
		mvn package -DskipTests
	fi
}

ARGC=$#
if [ $ARGC -eq 1 ] && [ "$1" == "build_cp" ]
then
	echo -n "Building classpath dependency ... "
	/usr/bin/time --format="%es" mvn dependency:build-classpath | grep -ve "\[" > $DEF_CLASS_PATH_FILE
	exit 0
fi

build_src

if [ $ARGC -eq 1 ] && [ "$1" == "test_usamap" ]
then
	java -cp $TARGET_JAR:`cat $DEF_CLASS_PATH_FILE` crawltwitter.UsaMap "${@:1}"
	exit 0
fi

java -cp $TARGET_JAR:`cat $DEF_CLASS_PATH_FILE` crawltwitter.Crawl "${@:1}"

popd > /dev/null
