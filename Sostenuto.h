#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include "IControls.h"
#include "IPlugPaths.h"


const int kNumPresets = 1;

enum EParams
{
    kGain = 0,
    kParamGain,
    kParamMode,
    kParamFreq1,
    kParamFreq2,
    kNumParams
};


enum EControlTags
{
    midiView = 0,
    keyboardGraphicView = 1
    
};


using namespace iplug;
using namespace igraphics;

class Sostenuto final : public Plugin
{
public:
    Sostenuto(const InstanceInfo& info);
    
#if IPLUG_DSP // http://bit.ly/2S64BDd
    void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
    void printMidiValues(int status, int channel, int controlChange, int noteNumber, int noteVelocity);
    void addToActiveNotes(int note);
    void removeNoteFromVector(int note);
    void fillSostenutoNotes();
    bool vectorContains(std::vector<int> vec, int note);
    void ProcessMidiMsg(const IMidiMsg& msg) override;
private:
    bool sostenuto = false;

    int pedalValue = 64;
    std::vector<int> activeNotes;
    std::vector<int> sostenutoNotes;
    std::mutex activeNoteMutex;
    
    IVKeyboardControl *noUIKeyboard;
#endif
};
