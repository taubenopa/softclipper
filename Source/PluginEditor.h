#pragma once
#include "PluginProcessor.h"
#include "UI.h"

class VoidClipAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit VoidClipAudioProcessorEditor (VoidClipAudioProcessor&);
    ~VoidClipAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using APVTS = juce::AudioProcessorValueTreeState;
    void timerCallback() override;
    void drawPanel (juce::Graphics&, juce::Rectangle<int>, const juce::String&, juce::Colour);

    VoidClipAudioProcessor& proc;
    vcui::VoidLookAndFeel laf;

    std::unique_ptr<vcui::Knob> kInGain, kHpf, kLpf, kWidth, kOGain, kOMix,
        kCThresh, kCRatio, kCAttack, kCRelease, kCKnee, kCSC, kCMakeup, kCMix,
        kSDrive, kSBias, kSTone, kSMix, kKDrive, kKCeil, kKKnee, kKAsym, kBFreq, kBDrive;

    juce::ComboBox presetBox, osBox, detectBox, satTypeBox, shapeBox;
    juce::TextButton prevBtn { "<" }, nextBtn { ">" };
    juce::Label osLabel, detLabel, typeLabel, shapeLabel;
    juce::ToggleButton compPower, satPower, clipPower, bassPower, satAuto, safety;

    vcui::MeterBar inBar { "IN" }, outBar { "OUT" }, grBar { "GR", true };
    vcui::CurveView curve;

    struct PanelInfo { juce::Rectangle<int> r; juce::String title; juce::Colour c; };
    std::vector<PanelInfo> panels;

    std::unique_ptr<APVTS::ComboBoxAttachment> osAtt, detAtt, typeAtt, shapeAtt;
    std::vector<std::unique_ptr<APVTS::ButtonAttachment>> btnAtts;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VoidClipAudioProcessorEditor)
};
