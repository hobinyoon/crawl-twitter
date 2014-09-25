#!/bin/bash

set -e
set -u

BIN_DIR=`cd "$( dirname "$0" )"; pwd`
pushd $BIN_DIR > /dev/null
DEF_CLASS_PATH_FILE=.dep-class-path
TARGET_JAR=target/crawl-twitter-0.1.jar

function build_src {
	need_build=false
	if [ ! -f $TARGET_JAR ]; then
		need_build=true
	else
		# compare modification time of the class path file and src directory tree
		MT_TJ=`find $TARGET_JAR -printf "%Ay%Am%Ad-%AH%AM%AS\n"`
		MT_SRC=`find src -name "*.java" -printf "%Ay%Am%Ad-%AH%AM%AS\n" | sort | tail -n 1`
		if [[ "$MT_TJ" < "$MT_SRC" ]]; then
			need_build=true
			echo "Last modification time:     "$MT_SRC
			echo "Last target jar build time: "$MT_TJ
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

java -cp $TARGET_JAR:`cat $DEF_CLASS_PATH_FILE` crawltwitter.TestCred "${@:1}"

popd > /dev/null
