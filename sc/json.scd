(
Event.addEventType(\midiCCEvent, {arg server;
    var freqs, lag, strum, sustain;
    var bndl, addAction, sendGate, ids;
    var msgFunc, instrumentName, offset, strumOffset;

    msgFunc = ~getMsgFunc.valueEnvir;
    instrumentName = ~synthDefName.valueEnvir;
    // sendGate == false turns off releases
    sendGate = ~sendGate ? ~hasGate;

    ~freq = freqs;
    ~amp = ~amp.value;
    ~sustain = sustain = ~sustain.value;
    lag = ~lag;
    offset = ~timingOffset;
    strum = ~strum;
    ~server = server;
    ~isPlaying = true;

    ~id = server.nextNodeID;

    bndl = (
        event: \ccOn,
        instrument: instrumentName,
        id: ~id,
        freq: ~freq,
        amp: ~amp,
        control: ~control.clip(0, 127)
    );

    ~schedBundleArray.(lag, offset, server, bndl);

    if (sendGate) {
        bndl = bndl.copy;
        bndl.gate = 0;
        bndl.event = \ccOff;

        ~schedBundleArray.(
            lag,
            sustain + offset,
            server,
            bndl
        );
    }
});
)

(
    q = q ? ();

    q.serialize = {arg self, pattern, duration=1, timeOffset=0.0, protoEvent;
        self.makeScore(pattern.asStream, duration, protoEvent, timeOffset);
    };

	q.makeScore = {|self, stream, duration=1, event, timeOffset=0|
		var ev, startTime, proto;
        var bundleList = [];

        var beats = 0;
        var tempo, maxTime;

        var add = {|beats, args|
            bundleList = bundleList.add([beats min: maxTime, args]);
        };

		proto = (
			// server: this,
            play: {
                var tempo, server;

                "play".postln;
                // ~nrtMidiEvent.postln;

                ~finish.value;

                server = ~server ?? { Server.default };

                tempo = ~tempo;
                if (tempo.notNil) {
                    thisThread.clock.tempo = tempo;
                };

                if(currentEnvironment.isRest.not) { ~eventTypes[~type].value(server) };
            },

            // TODO: Port additional events, note, monoNote etc.

			schedBundle: { | lag, offset, server ...bundle |
				add.(offset * tempo + lag + beats, bundle)
			},
			schedBundleArray: { | lag, offset, server, bundle |
				add.(offset * tempo + lag + beats, bundle)
			}
		);

		event = event ? Event.default;
		event = event.copy.putAll(proto);
		beats = timeOffset;
		tempo = 1;
		// bundleList = [];
		maxTime = timeOffset + duration;
		Routine {
			// thisThread.clock = this;
			while ({
				thisThread.beats = beats;
				ev = stream.next(event.copy);
				(maxTime >= beats) && ev.notNil
			}, {
				ev.putAll(proto);
				ev.play;
				beats = ev.delta * tempo + beats
			})
		}.next;

		bundleList = bundleList.sort({ | a, b | b[0] >= a[0] });
		if ((startTime = bundleList[0][0]) < 0 ) {
			timeOffset = timeOffset - startTime;
		};
		bundleList.do { | b | b[0] = b[0] + timeOffset };
        bundleList;
		// ^Score(bundleList.add([duration+timeOffset, [\c_set, 0, 0]]) );
	};

    p = Pbind(
        \type, \midiCCEvent,
        \control, Pwhite(0, 127), 
        \dur, Pseq([ 1, Rest(2) ], inf),
    );

    x = Pbind(
        \type, \midiCCEvent,
        \control, Pwhite(0, 127), 
        \dur, Pseq([ 0.5, 1 ], inf)
    );

    a = Ppar([ p, x ]);
    a = Pfindur(10, a);

    v = q.serialize(a, 10);
)

v.do(_.postln);""
