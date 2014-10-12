#!/bin/bash

# This script takes a training file, a feature count, a label count, and a test file
# It builds a multilabel bayes classifier out of the training file and uses it to classify the test file

#LIKELY FEATURE COUNT OF 521376

PADDING_FILE=$1
TRAIN_FILE=$2
FEATURE_COUNT=$3
LABEL_COUNT=$4
TEST_FILE=$5

MAX_COUNT=500
BOTTOM_LABEL=1

TMP_TRAIN_DIR=tmp_train
TMP_OUTPUT_DIR=tmp_output

rm -rf $TMP_TRAIN_DIR
rm -rf $TMP_OUTPUT_DIR

mkdir $TMP_TRAIN_DIR
mkdir $TMP_OUTPUT_DIR

for i in $(eval echo {$BOTTOM_LABEL..$LABEL_COUNT})
do
    echo "Working on file $i"
    cat $TRAIN_FILE | awk -F " " -v LABEL=$i '{ if ($1 ==LABEL) $1="1"; else $1="-1"; print}' > $TMP_TRAIN_DIR/label.$i
    #grep --max-count=$MAX_COUNT "^$i " $PADDING_FILE | awk -F " " '{ $1 ="1"; print}'>> $TMP_TRAIN_DIR/label.$i
    
done

#Probably should multithread this
#./bayes.py $TMP_TRAIN_DIR $FEATURE_COUNT /home/ross/Projects/cs4780proj/data/parsed/mnb.pydata
./bayes.py $TMP_TRAIN_DIR $TMP_OUTPUT_DIR $FEATURE_COUNT $TEST_FILE




