#!/bin/bash
for file in *.jpg
do
  convert $file -resize 10% $(basename $file .jpg).rsz.jpg
done
 
