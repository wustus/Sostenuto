#include <iostream>

#include "Sostenuto.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"
#include "IPlugPaths.h"


Sostenuto::Sostenuto(const InstanceInfo& info) : Plugin(info, MakeConfig(kNumParams, kNumPresets)) {
    GetParam(kGain)->InitDouble("Gain", 0., 0., 100.0, 0.01, "%");
    
#if IPLUG_EDITOR // http://bit.ly/2S64BDd
    mMakeGraphicsFunc = [&]() {
        return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
    };
    
    mLayoutFunc = [&](IGraphics* pGraphics) {
        pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
        pGraphics->AttachPanelBackground(COLOR_GRAY);
        pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
        
        const IVStyle style {
            true, // Show label
            true, // Show value
            {
                DEFAULT_BGCOLOR, // Background
                DEFAULT_FGCOLOR, // Foreground
                DEFAULT_PRCOLOR, // Pressed
                COLOR_BLACK, // Frame
                DEFAULT_HLCOLOR, // Highlight
                DEFAULT_SHCOLOR, // Shadow
                COLOR_BLACK, // Extra 1
                DEFAULT_X2COLOR, // Extra 2
                DEFAULT_X3COLOR  // Extra 3
            }, // Colors
            IText(20.f, EAlign::Center) // Label text
        };
        
        const IRECT b = pGraphics->GetBounds();
        IRECT wideCell;
        
        pGraphics->AttachControl(new ITextControl(b.GetFromTop(50),
                                    "Channel: - \t Note Number: - \t Velocity: - \t Control Change: -",
                                    IText(15), COLOR_LIGHT_GRAY), midiView, "misccontrols");
        
        auto setMidiValue = [&](IControl* pCaller) {
            pedalValue = (int) pCaller->As<IVNumberBoxControl>()->GetRealValue();
        };
        
        
        pGraphics->AttachControl(new ITextControl(b.GetFromTop(175), "Pedal Value", IText(20)));
        
        pGraphics->AttachControl(new IVNumberBoxControl(b.GetPadded(-100, -100, -100, -150), kNoParameter, setMidiValue,
                                                        "", style, 64., -1., 256.));
        
        pGraphics->AttachControl(new IVKeyboardControl(IRECT(0,0,600,300).GetPadded(0, -200, 0, 0), 21, 108), keyboardGraphicView);
        GetUI()->GetControlWithTag(keyboardGraphicView)->As<IVKeyboardControl>()->SetWantsMidi(false);
        
                    
    };
#endif
}

#if IPLUG_DSP
void Sostenuto::ProcessBlock(sample** inputs, sample** outputs, int nFrames) {
    const int nChansOut = NOutChansConnected();
    
    for (auto s = 0; s < nFrames; s++) {
        for (auto c = 0; c < nChansOut; c++) {
            outputs[c][s] = outputs[c][s] * 1;
        }
    }
}


void Sostenuto::printMidiValues(int status, int channel, int controlChange, int noteNumber, int noteVelocity) {
    switch(status) {
        case 8:
            std::cout << "Note Off: {" << std::endl;
        case 9:
            std::cout << "Note On: {" << std::endl;
        case 11:
            std::cout << "Control Change: {" << std::endl;
        default:
            assert("error");
    }
    
    std::cout << "\t Status: " << status << std::endl;
    std::cout << "\t Channel: " << channel << std::endl;
    std::cout << "\t Control Change: " << controlChange << std::endl;
    std::cout << "\t Note Number: " << noteNumber << std::endl;
    std::cout << "\t Note Velocity: " << noteVelocity << std::endl;
    std::cout << "}" << std::endl;
    std::cout << std::endl;
}


void Sostenuto::removeNoteFromVector(int note) {
    int h = 0;
    
    std::lock_guard<std::mutex> lock(activeNoteMutex);
    
    for (int i=0; i!=activeNotes.size(); i++) {
        if (activeNotes[i] == note) {
            h = i;
            break;
        }
    }
    
    activeNotes.erase(activeNotes.begin()+h);
}


void Sostenuto::addToActiveNotes(int note) {
    if (!activeNotes.empty()) {
        if (note > activeNotes.front()) activeNotes.push_back(note);
        else activeNotes.insert(activeNotes.begin(), note);
    } else {
        activeNotes.push_back(note);
    }
}


void Sostenuto::fillSostenutoNotes() {
    for (int i : activeNotes) {
        sostenutoNotes.push_back(i);
    }
}


bool Sostenuto::vectorContains(std::vector<int> vec, int note) {
    for (int i : vec) {
        if (i == note) {
            return true;
        }
    }
    
    return false;
}


void Sostenuto::ProcessMidiMsg(const IMidiMsg& msg) {
    TRACE;
    
    bool ui = true;
    
    int status = msg.StatusMsg();
    IMidiMsg noteMsg;
    
    int channel = msg.Channel();
    
    int controlChange = msg.ControlChangeIdx();
    int noteNumber = msg.NoteNumber();
    int noteVelocity = msg.Velocity();
    
    IVKeyboardControl *keyboardGraphic;
    
    if (GetUI() != nil) {
        WDL_String str;
        str.SetFormatted(256, "Channel: %d \t Note Number: %d \t Velocity: %d \t Control Change: %d",
                         channel, noteNumber, noteVelocity, controlChange);
        GetUI()->GetControlWithTag(midiView)->As<ITextControl>()->SetStr(str.Get());
        keyboardGraphic = GetUI()->GetControlWithTag(keyboardGraphicView)->As<IVKeyboardControl>();
    } else {
        if (noUIKeyboard == nil) {
            noUIKeyboard = new IVKeyboardControl(IRECT());
            noUIKeyboard->SetWantsMidi(false);
        }
    }
    
    switch (status) {
        case 9:
            goto handleNoteOn;
            
        case 8:
            goto handleNoteOff;
        
        case 11:
            goto handleControlChange;
            
            
        default:
            return;
    }
    
    handleNoteOn:
    {
        if (noteVelocity == 0) {
            status = 8;
            goto handleNoteOff;
        }
        
        addToActiveNotes(noteNumber);
        
        if (sostenuto && vectorContains(sostenutoNotes, noteNumber)) {
            noteMsg.MakeNoteOffMsg(noteNumber, 0);
            SendMidiMsg(noteMsg);
            
            noteMsg.MakeNoteOnMsg(noteNumber, noteVelocity, 0);
            SendMidiMsg(noteMsg);
            
            return;
        }
        
        if (ui) keyboardGraphic->SetNoteFromMidi(noteNumber, true);
        
        noteMsg.MakeNoteOnMsg(noteNumber, noteVelocity, 0);
        SendMidiMsg(noteMsg);
        
        return;
    }
        
    handleNoteOff:
    {
        if (vectorContains(activeNotes, noteNumber)) removeNoteFromVector(noteNumber);
        
        if (sostenuto && vectorContains(sostenutoNotes, noteNumber)) return;
     
        if (ui) keyboardGraphic->SetNoteFromMidi(noteNumber, false);
        
        noteMsg.MakeNoteOffMsg(noteNumber, 0);
        SendMidiMsg(noteMsg);
        
        return;
    }
        
    handleControlChange:
    {
        if (controlChange == pedalValue) {
            sostenuto = !sostenuto;
            if (sostenuto) {
                fillSostenutoNotes();
              
            } else {
                for (int i : sostenutoNotes) {
                    if (!vectorContains(activeNotes, i)) {
                        if (ui) keyboardGraphic->SetNoteFromMidi(i, false);
                            
                        noteMsg.MakeNoteOffMsg(i, 0);
                        SendMidiMsg(noteMsg);
                    }
                }
                sostenutoNotes.clear();
            }
        }
        
        return;
    }
}
#endif
