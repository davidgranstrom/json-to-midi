#!/bin/bash

echo "Checking existance of libmidifile.."

target=./lib/libmidifile.a

if [ -f "$target" ]
then
    printf "Found: %s\n" "$target"
    exit 0
fi

pushd ./midifile > /dev/null
make library
popd > /dev/null

echo "Copying files"

mv ./midifile/lib/libmidifile.a ./lib
cp ./midifile/include/* ./include

echo "Done"
