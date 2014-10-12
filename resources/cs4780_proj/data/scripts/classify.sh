#!/bin/bash

# This script takes a directory of models and an svm-light file and evaluates the accuracy of each svm



TEST_FILE=$1
MODEL_DIR=$2
LABEL_COUNT=$3

MODEL_FILE_NAME=model

TMP_DIR=classify_tmp
OUTPUT_DIR=outputs

#Remove old tmp dir if it exists and make new one
rm -rf $TMP_DIR
mkdir $TMP_DIR
mkdir $TMP_DIR/$OUTPUT_DIR

BOTTOM_SALARY=2

for i in $(eval echo {$BOTTOM_SALARY..$LABEL_COUNT})
do
    grep "^$i " $TEST_FILE | awk -F " " '{$1="1 "; print}' > $TMP_DIR/val.$i
    LINES=`wc -l $TMP_DIR/val.$i | cut -f 1 -d ' '`
    if [[ $LINES != 0 && -f $TMP_DIR/val.$i && -f $MODEL_DIR/$MODEL_FILE_NAME.$i ]];
        then
        ./svm_classify $TMP_DIR/val.$i $MODEL_DIR/$MODEL_FILE_NAME.$i $TMP_DIR/$OUTPUT_DIR/output.$i &
    fi
done

FAIL=0
for job in `jobs -p`
do
    wait $job || let "FAIL+=1"
done

if [ "$FAIL" != "0" ]
    then
    echo "Failed jobs: $FAIL"
    exit 1
fi

