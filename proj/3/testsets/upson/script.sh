#!/bin/bash
i=0
for file in *.jpg
do
  i=$((i + 1))
  mv "$file" "upson$i.jpg"
done

