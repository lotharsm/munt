#ifndef SYNTH_ROUTE_H
#define SYNTH_ROUTE_H

#include <ctime>
#include <QtCore>
#include <mt32emu/mt32emu.h>

#include "QSynth.h"
#include "MasterClock.h"
#include "MidiRecorder.h"

class MidiSession;
class AudioStream;
class AudioDevice;

enum SynthRouteState {
	SynthRouteState_CLOSED,
	SynthRouteState_OPENING,
	SynthRouteState_OPEN,
	SynthRouteState_CLOSING
};

class SynthRoute : public QObject {
	Q_OBJECT
private:
	typedef AudioStream *(*AudioStreamFactory)(const AudioDevice *, SynthRoute &, const uint, MidiSession *midiSession);

	SynthRouteState state;
	QSynth qSynth;
	QList<MidiSession *> midiSessions;
	QMutex midiSessionsMutex;
	MidiRecorder recorder;
	bool exclusiveMidiMode;
	volatile bool multiMidiMode;

	const AudioDevice *audioDevice;
	AudioStream *audioStream; // NULL until a stream is created

	quint64 debugLastEventTimestamp;
	qint64 debugDeltaLowerLimit, debugDeltaUpperLimit;

	void setState(SynthRouteState newState);
	void disableExclusiveMidiMode();
	void mergeMidiStreams(uint renderingPassFrameLength);

public:
	SynthRoute(QObject *parent = NULL);
	~SynthRoute();
	bool open(AudioStreamFactory audioStreamFactory = NULL);
	bool close();
	void reset();
	bool enableExclusiveMidiMode(MidiSession *midiSession);
	bool isExclusiveMidiModeEnabled();
	void enableMultiMidiMode();

	const QString getPatchName(int partNum) const;
	void getPartStates(bool *partStates) const;
	void getPartialStates(MT32Emu::PartialState *partialStates) const;
	uint getPlayingNotes(unsigned int partNumber, MT32Emu::Bit8u *keys, MT32Emu::Bit8u *velocities) const;
	uint getPartialCount() const;

	void flushMIDIQueue();
	void playMIDIShortMessageNow(MT32Emu::Bit32u msg);
	void playMIDISysexNow(const MT32Emu::Bit8u *sysex, MT32Emu::Bit32u sysexLen);
	bool playMIDIShortMessage(MidiSession &midiSession, MT32Emu::Bit32u msg, quint64 timestamp);
	bool playMIDISysex(MidiSession &midiSession, const MT32Emu::Bit8u *sysex, MT32Emu::Bit32u sysexLen, quint64 timestamp);
	bool pushMIDIShortMessage(MidiSession &midiSession, MT32Emu::Bit32u msg, MasterClockNanos midiNanos);
	bool pushMIDISysex(MidiSession &midiSession, const MT32Emu::Bit8u *sysex, unsigned int sysexLen, MasterClockNanos midiNanos);
	void discardMidiBuffers();
	void render(MT32Emu::Bit16s *buffer, uint length);
	void render(float *buffer, uint length);
	void audioStreamFailed();

	void enableRealtimeMode();
	void setMasterVolume(int masterVolume);
	void setOutputGain(float outputGain);
	void setReverbOutputGain(float reverbOutputGain);
	void setReverbEnabled(bool reverbEnabled);
	void setReverbOverridden(bool reverbOverridden);
	void setReverbSettings(int reverbMode, int reverbTime, int reverbLevel);
	void setReversedStereoEnabled(bool enabled);
	void setNiceAmpRampEnabled(bool enabled);
	void resetMIDIChannelsAssignment(bool engageChannel1);
	void setInitialMIDIChannelsAssignment(bool engageChannel1);
	void setReverbCompatibilityMode(ReverbCompatibilityMode reverbCompatibilityMode);
	void setMIDIDelayMode(MT32Emu::MIDIDelayMode midiDelayMode);
	void setDACInputMode(MT32Emu::DACInputMode emuDACInputMode);
	void setAnalogOutputMode(MT32Emu::AnalogOutputMode analogOutputMode);
	void setRendererType(MT32Emu::RendererType rendererType);
	void setPartialCount(int partialCount);

	void startRecordingAudio(const QString &fileName);
	void stopRecordingAudio();
	bool isRecordingAudio() const;

	void addMidiSession(MidiSession *midiSession);
	void removeMidiSession(MidiSession *midiSession);
	void setMidiSessionName(MidiSession *midiSession, QString name);
	bool hasMIDISessions() const;
	SynthRouteState getState() const;
	void setAudioDevice(const AudioDevice *newAudioDevice);
	MidiRecorder *getMidiRecorder();
	void getSynthProfile(SynthProfile &synthProfile) const;
	void setSynthProfile(const SynthProfile &synthProfile, QString useSynthProfileName);
	void getROMImages(const MT32Emu::ROMImage *&controlROMImage, const MT32Emu::ROMImage *&pcmROMImage) const;
	bool connectSynth(const char *signal, const QObject *receiver, const char *slot) const;
	bool connectReportHandler(const char *signal, const QObject *receiver, const char *slot) const;

private slots:
	void handleQSynthState(SynthState synthState);

signals:
	void stateChanged(SynthRouteState state);
	void midiSessionAdded(MidiSession *midiSession);
	void midiSessionRemoved(MidiSession *midiSession);
	void midiSessionNameChanged(MidiSession *midiSession);
	void exclusiveMidiSessionRemoved(MidiSession *midiSession);
};

#endif
