#include <iostream>
#include <fstream>

#include "cxxopts.hpp"
#include "json.hpp"
#include "MidiFile.h"

using json = nlohmann::json;

enum MidiTypes {
    nNOTE,
    nCC,
    nUNDEFINED
};

json parseInput(std::string input);
int writeMIDIFile(json input, std::string output); 
MidiTypes getEnumValue(std::string const &type);

int main(int argc, char *argv[])
{
    std::string inputPath;
    std::string outputPath;

    try {
        cxxopts::Options options(argv[0]);

        options.add_options()
            ("i,input", "json document input file", cxxopts::value<std::string>(), "FILE")
            ("o,output", "MIDI file output path", cxxopts::value<std::string>(), "FILE")
            ("h,help", "Print help");

        options.parse(argc, argv);

        if (options.count("help")) {
            std::cout << options.help({"", "Group"}) << std::endl;
            exit(0);
        }

        if (options.count("input")) {
            inputPath = options["input"].as<std::string>();
        }

        if (options.count("output")) {
            outputPath = options["output"].as<std::string>();
        }
    } catch (const cxxopts::OptionException& e) {
        std::cout << "Error parsing options: " << e.what() << std::endl;
        exit(1);
    }

    json input = parseInput(inputPath);
    return writeMIDIFile(input, outputPath);
}

json parseInput(std::string input)
{
    std::ifstream i(input);
    json j;
    i >> j;
    return j;
}

MidiTypes getEnumValue(std::string const &type) {
    if (type == "note") return nNOTE;
    if (type == "cc") return nCC;
    return nUNDEFINED;
}

int writeMIDIFile(json input, std::string output)
{
    MidiFile midifile;
    // use absolute time values and convert to deltas later
    midifile.absoluteTicks();

    int bpm;
    if (!input["bpm"].is_null()) {
        bpm = input["bpm"].get<int>();
    } else {
        std::cout << "Warning: Could not find bpm key!" << " bpm has been set to 60" << std::endl;
        bpm = 60;
    }

    const int tpq = 192; // ticks per quarter note
    const float timeScale = tpq * (bpm / 60);

    auto &tracks = input["tracks"];
    int track = 1;

    midifile.setTicksPerQuarterNote(tpq);
    midifile.addTrack(tracks.size());
    midifile.addTempo(0, 0, bpm);

    for (auto &t : tracks) {
        for (auto &event : t) {
            std::string type = !event["type"].is_null() ? event["type"].get<std::string>() : "";

            if (event["absTime"].is_null()) {
                std::cout << "All events needs an absolute time (absTime), skipping.." << std::endl;
            } else {
                float absTime = event["absTime"].get<float>();

                switch (getEnumValue(type)) {
                    case nNOTE: {
                        std::cout << "Found note event" << std::endl;
                        int vel, pitch, channel;
                        float dur;

                        if (event["midinote"].is_number()) {
                            pitch = event["midinote"].get<int>();
                        } else {
                            std::cout << "midinote is required, skipping" << std::endl;
                            break;
                        }

                        if (event["velocity"].is_number()) {
                            vel = event["velocity"].get<int>();
                        } else {
                            vel = 64;
                        }

                        if (event["channel"].is_number()) {
                            channel = event["channel"].get<int>();
                        } else {
                            channel = 0;
                        }

                        if (event["duration"].is_number()) {
                            dur = event["duration"].get<float>();
                        } else {
                            std::cout << "duration not found, setting to 1" << std::endl;
                            dur = 1;
                        }

                        midifile.addNoteOn(track, absTime * timeScale, channel, pitch, vel);
                        midifile.addNoteOff(track, (absTime + dur) * timeScale, channel, pitch, 0);
                        break;
                    }
                    case nCC: {
                        std::cout << "Found cc event" << std::endl;
                        break;
                    }
                    case nUNDEFINED: {
                        std::cout << "Skipping undefined event" << std::endl;
                        break;
                    }
                }
            }
        }
        track++;
    }

    midifile.sortTracks();
    midifile.write(output);
    // true on success
    return !midifile.status();
}
