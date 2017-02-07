json-to-midi
============

`json2midi` is a command line tool for converting a json document into a midi file.

## Usage

    ./json2midi --input input.json --output output.mid

## Documentation

## Document structure

The MIDI file is parsed from a json document.

    {
        bpm: 128,
        timeSig: "4/4",
        tracks: [ [track-1-events .. ], [track-2-events] ],
    }


`tracks` contains event lists, a new MIDI track will be created for each event list.  
`bpm` will set bpm value for track 0 (meta track)
`timeSig` (not implemented yet)

## Event types

The `eventType` key specifies the midi event:

* eventType
    - `note`
    - `cc` (not implemented yet)

All events are required to have an absolute time in seconds:

* `absTime` (inter-onset time)

### note

A `note` event will be translated into noteOn/noteOff midi event pairs. The `duration` key specifies when to send the noteOff event.

* `midinote` (0 - 127)
* `channel` (default 0)
* `duration` (default 1 second)
* `velocity` (default 64)

### cc

TODO: write spec

## Example document

    {
        "timeSig": "4/4",
        "bpm": 120,
        "tracks": [
            [
                { "eventType": "note", "absTime": 0, "duration": 1, "midinote": 60 },
                { "eventType": "note", "absTime": 1, "duration": 1, "midinote": 64 },
                { "eventType": "note", "absTime": 3, "duration": 1, "midinote": 67 },
            ],
            [
                { "eventType": "note", "absTime": 0, "duration": 3, "midinote": 60 },
            ]
        ]
    }
