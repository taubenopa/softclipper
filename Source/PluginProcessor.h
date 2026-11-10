#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "Parameters.h"
#include "DSP.h"

class VoidClipAudioProcessor : public juce::AudioProcessor
{
public:
    VoidClipAudioProcessor();
    ~VoidClipAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "VoidClip"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    // Meter (Audio-Thread schreibt, GUI liest)
    std::atomic<float> inMeter { 0.f }, outMeter { 0.f }, grMeter { 0.f }, clipMeter { 0.f };

private:
    void processChunk (juce::AudioBuffer<float>&);
    void processNonlinear (juce::dsp::AudioBlock<float>&);
    void setOversamplingIndex (int idx);

    struct ParamPtrs
    {
#define X(n) std::atomic<float>* n = nullptr;
        VC_PARAM_LIST(X)
#undef X
    } p;

    using MulSmooth = juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative>;

    double sampleRate = 44100.0, osRate = 88200.0;
    int maxBlock = 512, currentOs = -1, currentProgram = 0;

    juce::dsp::StateVariableTPTFilter<float> hpf, lpf;
    vc::Compressor comp;
    std::unique_ptr<juce::dsp::Oversampling<float>> os[3];
    juce::dsp::LinkwitzRileyFilter<float> xover;
    vc::DCBlocker dcSat[2], dcClip[2];
    vc::Tilt tilt[2];
    MulSmooth smSatDrive, smSatComp, smClipDrive, smCeil, smBass;

    juce::AudioBuffer<float> dryBuf;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::None> dryDelay { 4096 };
    float lastInGain = 1.f, lastOutGain = 1.f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VoidClipAudioProcessor)
};
