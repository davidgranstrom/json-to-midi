json-to-midi
============

`json2midi` is a command line tool for converting a json document into a midi file.

## key table 

This table contains the keys recognizable by `json2midi`

The event key is the name of the midi event:

* event
    - note
    - cc

All events are required to have an absolute time in seconds:

* `absTime` (inter-onset time)

A `note` object will be translated into a noteOn/noteOff midi event.
It is required to have the following keys:

* `midinote` (0 - 127)

Optional:

* `channel` (default 0)
* `duration` (default 1 second)
* `velocity` (default 64)

TODO: write spec for `cc`

### Example document

    {
        "tracks": [
            [
                { "type": "note", "absTime": 0, "duration": 1, "midinote": 60 },
                { "type": "note", "absTime": 1, "duration": 1, "midinote": 64 },
                { "type": "note", "absTime": 3, "duration": 1, "midinote": 67 },
            ],
            [
                { "type": "note", "absTime": 0, "duration": 3, "midinote": 60 },
            ]
        ],
        "timeSig": "4/4",
        "bpm": 120
    }
