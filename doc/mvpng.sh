#!/bin/bash

cd html
for file in `ls form*.gif`
do
	convert $file ${file/.gif}.png
done
cd ..
