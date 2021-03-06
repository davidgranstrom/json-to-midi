#include <iostream>
#include <fstream>
#include <math.h>

#include "cxxopts.hpp"
#include "json.hpp"
#include "MidiFile.h"

using json = nlohmann::json;

enum MidiTypes {
    kNOTE,
    kCC,
    kUNDEFINED
};

json parseInput(std::string input);
int writeMIDIFile(json input, std::string output, bool joinTracks); 

int main(int argc, char *argv[])
{
    std::string inputPath;
    std::string outputPath;
    bool joinTracks = false;

    try {
        cxxopts::Options options(argv[0], "json2midi - json to midi file converter");

        options.add_options()
            ("i,input", "json document input file", cxxopts::value<std::string>(), "FILE")
            ("o,output", "MIDI file output path", cxxopts::value<std::string>(), "FILE")
            ("m,merge", "Merge all tracks in output MIDI file")
            ("h,help", "Print help");

        options.parse(argc, argv);

        if (options.count("help") || !options.count("input") || !options.count("output")) {
            std::cout << options.help() << std::endl;
            exit(0);
        }

        if (options.count("input")) {
            inputPath = options["input"].as<std::string>();
        }

        if (options.count("output")) {
            outputPath = options["output"].as<std::string>();
        }

        if (options.count("merge")) {
            joinTracks = true;
        }
    } catch (const cxxopts::OptionException& e) {
        std::cout << "Error parsing options: " << e.what() << std::endl;
        exit(1);
    }

    json input = parseInput(inputPath);
    return writeMIDIFile(input, outputPath, joinTracks);
}

json parseInput(std::string input)
{
    std::ifstream i(input);
    json j;
    i >> j;
    return j;
}

MidiTypes getEnumValue(std::string const &eventType) {
    if (eventType == "note") return kNOTE;
    if (eventType == "cc") return kCC;
    return kUNDEFINED;
}

void createMidiNoteEvents(MidiFile &midifile, int track, double timeScale, json event)
{
    double dur, absTime;
    int pitch, vel, channel;

    absTime = event["absTime"].get<double>();

    if (event["midinote"].is_number()) {
        pitch = event["midinote"].get<int>();
    } else {
        std::cout << "midinote is required, skipping" << std::endl;
        return;
    }

    dur = event["duration"].is_number() ? event["duration"].get<double>() : 0.0;
    vel = event["velocity"].is_number() ? event["velocity"].get<int>() : 64;
    channel = event["channel"].is_number() ? event["channel"].get<int>() : 0;

    midifile.addNoteOn(track, nearbyint(absTime * timeScale), channel, pitch, vel);

    if (dur > 0.0) {
        midifile.addNoteOff(track, nearbyint((absTime + dur) * timeScale), channel, pitch, 0);
    } else {
        std::cout << "Warning: Event duration should be greater than zero."
            << " Did not write noteOff event." << std::endl;
    }
}

void createCCEvent(MidiFile &midifile, int track, double timeScale, json event)
{
    double absTime = event["absTime"];
    int ccNum, ccVal, channel;

    if (event["ccNum"].is_number()) {
        ccNum = event["ccNum"].get<int>();
    } else {
        std::cout << "ccNum is required, skipping" << std::endl;
        return;
    }

    if (event["ccVal"].is_number()) {
        ccVal = event["ccVal"].get<int>();
    } else {
        std::cout << "ccVal is required, skipping" << std::endl;
        return;
    }

    channel = event["channel"].is_number() ? event["channel"].get<int>() : 0;

    midifile.addController(track, nearbyint(absTime * timeScale), channel, ccNum, ccVal);
}

int writeMIDIFile(json input, std::string output, bool joinTracks)
{
    MidiFile midifile;
    // use absolute time values and convert to deltas later
    midifile.absoluteTicks();

    double bpm;
    if (!input["bpm"].is_null()) {
        bpm = input["bpm"].get<double>();
    } else {
        std::cout << "Warning: Could not find bpm key!" << " bpm has been set to 60" << std::endl;
        bpm = 60.0;
    }

    const double tpq = 960.0; // ticks per quarter note
    const double timeScale = tpq * (bpm / 60); // ticks per second

    auto &tracks = input["tracks"];
    int track = 1;

    midifile.addTrack(tracks.size());
    midifile.setTicksPerQuarterNote(tpq);

    // add meta events (track 0, tick 0) 
    /* midifile.addTrackName(0, 0, "Test output"); */
    midifile.addTempo(0, 0, bpm);

    for (auto &t : tracks) {
        for (auto &event : t) {
            if (event["absTime"].is_null()) {
                std::cout << "All events needs an absolute time (absTime), skipping.." << std::endl;
                continue;
            }

            std::string eventType = !event["eventType"].is_null() ? event["eventType"].get<std::string>() : "";

            switch (getEnumValue(eventType)) {
                case kNOTE:
                    createMidiNoteEvents(midifile, track, timeScale, event);
                    break;
                case kCC: {
                    createCCEvent(midifile, track, timeScale, event);
                    break;
                }
                case kUNDEFINED: {
                    std::cout << "Skipping undefined event" << std::endl;
                    break;
                }
            }
        }
        track++;
    }

    midifile.sortTracks();
    midifile.linkNotePairs();
    if (joinTracks) {
        midifile.joinTracks();
    }
    midifile.write(output);
    // true on success
    return !midifile.status();
}
