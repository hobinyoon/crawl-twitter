#!/bin/bash

set -e
set -u

BIN_DIR=`cd "$( dirname "$0" )"; pwd`
pushd $BIN_DIR > /dev/null
DEF_CLASS_PATH_FILE=.dep-class-path

function build_src {
	need_build=false
	if [ ! -f $DEF_CLASS_PATH_FILE ]; then
		need_build=true
	else
		# compare modification time of the class path file and src directory tree
		MT_CP=`find $DEF_CLASS_PATH_FILE -printf "%Ay%Am%Ad-%AH%AM%AS\n"`
		MT_SRC=`find src -name "*.java" -printf "%Ay%Am%Ad-%AH%AM%AS\n" | sort | tail -n 1`
		if [[ "$MT_CP" < "$MT_SRC" ]]; then
			need_build=true
			echo "Last modification time:    "$MT_SRC
			echo "Last classpath build time: "$MT_CP
		fi
	fi

	if [ "$need_build" = true ] ; then
		echo "Building package ... "
		mvn package -DskipTests
		echo -n "Building classpath dependency file ... "
		/usr/bin/time --format="%es" mvn dependency:build-classpath | grep -ve "\[" > $DEF_CLASS_PATH_FILE
	fi
}

build_src

java -cp target/crawl-twitter-0.1.jar:`cat $DEF_CLASS_PATH_FILE` crawltwitter.Crawl "${@:1}"

popd > /dev/null
