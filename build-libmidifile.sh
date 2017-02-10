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

printf "Moving files into place.. "

headers=("Binasc.h" "MidiEvent.h" "MidiEventList.h" "MidiFile.h" "MidiMessage.h")
for header in "${headers[@]}"
do
    cp ./midifile/include/"$header" ./include
done

mv ./midifile/lib/libmidifile.a ./lib

echo "Done"
