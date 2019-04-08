#!/usr/bin/env bash

i=0
while read test; do
	echo "($i, $test)"
	i=$((i+1))
done
echo "done"