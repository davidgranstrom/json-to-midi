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

int main(int argc, char *argv[])
{
    std::string inputPath;
    std::string outputPath;

    try {
        cxxopts::Options options(argv[0], "json2midi - json to midi file converter");

        options.add_options()
            ("i,input", "json document input file", cxxopts::value<std::string>(), "FILE")
            ("o,output", "MIDI file output path", cxxopts::value<std::string>(), "FILE")
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

MidiTypes getEnumValue(std::string const &eventType) {
    if (eventType == "note") return nNOTE;
    if (eventType == "cc") return nCC;
    return nUNDEFINED;
}

void createMidiNoteEvents(MidiFile &midifile, int track, double timeScale, json event)
{
    double absTime = event["absTime"];
    double dur;

    int pitch, vel, channel;

    if (event["midinote"].is_number()) {
        pitch = event["midinote"].get<int>();
    } else {
        std::cout << "midinote is required, skipping" << std::endl;
        return;
    }

    dur = event["duration"].is_number() ? event["duration"].get<double>() : 0.0;
    vel = event["velocity"].is_number() ? event["velocity"].get<int>() : 64;
    channel = event["channel"].is_number() ? event["channel"].get<int>() : 0;

    midifile.addNoteOn(track, absTime * timeScale, channel, pitch, vel);

    if (dur > 0.0) {
        midifile.addNoteOff(track, (absTime + dur) * timeScale, channel, pitch, 0);
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

    midifile.addController(track, absTime * timeScale, channel, ccNum, ccVal);
}

int writeMIDIFile(json input, std::string output)
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

    const int tpq = 192; // ticks per quarter note
    const double timeScale = tpq * (bpm / 60);

    auto &tracks = input["tracks"];
    int track = 1;

    midifile.setTicksPerQuarterNote(tpq);
    midifile.addTrack(tracks.size());
    midifile.addTempo(0, 0, bpm);

    for (auto &t : tracks) {
        for (auto &event : t) {
            if (event["absTime"].is_null()) {
                std::cout << "All events needs an absolute time (absTime), skipping.." << std::endl;
                continue;
            }

            std::string eventType = !event["eventType"].is_null() ? event["eventType"].get<std::string>() : "";

            switch (getEnumValue(eventType)) {
                case nNOTE:
                    createMidiNoteEvents(midifile, track, timeScale, event);
                    break;
                case nCC: {
                    createCCEvent(midifile, track, timeScale, event);
                    break;
                }
                case nUNDEFINED: {
                    std::cout << "Skipping undefined event" << std::endl;
                    break;
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
