#!/bin/bash
#Tom Ben-Dor

if [ "$#" -lt 2 ]; then
    echo "Not enough parameters"
    exit 1
fi

home=$(pwd)
cd $1

rm -f *.out
for f in *.c; do
	if grep -q -i "\b$2\b" $f; then
		gcc -w "$f" -o "${f%.*}".out
	fi
done

cd $home

if [ "$3" = "-r" ]; then
	for dir in "$1"/*; do
        if [ -d "$dir" ]; then
            $0 "$dir" "$2" "-r"
        fi
    done
fi

