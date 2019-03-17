#!/bin/bash
sizes="1 4 512 1024 4096 8192"
counts="3000 6000"
DIR="test_tmp"

#build
cmake .
make

#clean and create build directory
rm -r ${DIR}
mkdir ${DIR}
cd ${DIR}

#tests
for size in $sizes; do
	for count in $counts; do
		echo "Comparing for count: ${count} and size: ${size}"
		sysfile="test_${count}_${size}_sys"
		libfile="test_${count}_${size}_lib"
		../fo generate ${sysfile} ${count} ${size} > /dev/null
		cp ${sysfile} ${libfile}
		echo "Sys"
		../fo copy ${sysfile} cp_${sysfile} ${count} ${size} sys
		../fo sort ${sysfile} ${count} ${size} sys
		echo "Lib"
		../fo copy ${libfile} cp_${libfile} ${count} ${size} lib
		../fo sort ${libfile} ${count} ${size} lib
	done
done