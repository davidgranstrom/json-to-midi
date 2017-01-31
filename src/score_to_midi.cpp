#include <iostream>
#include <fstream>

#include "json.hpp"
#include "MidiFile.h"

using namespace std;
using json = nlohmann::json;

int main(int argc, char** argv)
{
    std::string input_json = argv[1]; 
    // read a JSON file
    ifstream i(input_json);
    json j;
    i >> j;

    MidiFile midifile;

    midifile.absoluteTicks(); // use absolute time values and convert to deltas later
    midifile.addTrack();

    int bpm = 120;
    int tpq = 128; // ticks per quarter note
    float timeScale = tpq * (bpm / 60);

    midifile.setTicksPerQuarterNote(tpq);

    int channel = 0;
    int track = 0;

    for (auto& element : j) {
        /* std::cout << element << '\n'; */
        int pitch = element["midiNote"];
        float absTime = element["absTime"];

        if (element["action"] == "noteOn") {
            midifile.addNoteOn(track, absTime * timeScale, channel, pitch, 64);
        } else if(element["action"] == "noteOff") {
            midifile.addNoteOff(track, absTime * timeScale, channel, pitch, 0);
        }
    }

    midifile.sortTracks();
    midifile.joinTracks();
    midifile.write("test.mid");

    return 0;
}
