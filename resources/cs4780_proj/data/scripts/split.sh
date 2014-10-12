#!/bin/bash

# This script takes an svm-light encoded file representing a training set, splits it by label and trains models for each label using the svm_learn function
# The script parameters are the svm-light file to be learned and the number of distinct labels

SVM_FILE=$1
LABEL_COUNT=$2
PREPEND_FILE=$3

TMP_DIR=split_tmp
MODEL_DIR=models

BOTTOM_SALARY=2 #Base salary is 10k



#SVM_LEARN Parameters
BIASED=1
KERNEL_FUNCTION=0 #Linear Kernel
D_POLYNOMIAL=1 #Unused currently

#Remove old tmp dir and make new one
rm -rf $TMP_DIR 
mkdir $TMP_DIR
mkdir $TMP_DIR/$MODEL_DIR

for i in $(eval echo {$BOTTOM_SALARY..$LABEL_COUNT})
do
    cat $3 | awk -F " " -v LABEL=$i '{ if ($1 ==$LABEL) {$1="1 "; print;} else $1="-1 "; print}' > $TMP_DIR/label.$i
    grep "^$i " $SVM_FILE | awk -F " " '{$1="1 "; print}' >> $TMP_DIR/label.$i
    LINES=`wc -l $TMP_DIR/label.$i | cut -f 1 -d ' '`
    if [[ $LINES != 0 ]];
        then
        ./svm_learn -b $BIASED -t $KERNEL_FUNCTION $TMP_DIR/label.$i $TMP_DIR/$MODEL_DIR/model.$i &
    fi
done

FAIL=0
for job in `jobs -p`
do
    wait $job || let "FAIL+=1"
done

if [ "$FAIL" != "0" ];
    then
    echo "Failed jobs: $FAIL"
    exit 1
fi



