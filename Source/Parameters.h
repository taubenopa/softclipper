#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

// X-Macro: eine Liste fuer IDs UND gecachte Pointer
#define VC_PARAM_LIST(X) \
    X(inGain) X(hpf) X(lpf) X(width) X(outGain) X(mix) X(oversample) \
    X(compOn) X(compDetect) X(compThresh) X(compRatio) X(compAttack) X(compRelease) \
    X(compKnee) X(compMakeup) X(compMix) X(compSC) \
    X(satOn) X(satType) X(satDrive) X(satBias) X(satTone) X(satMix) X(satComp) \
    X(clipOn) X(clipShape) X(clipDrive) X(clipCeil) X(clipKnee) X(clipAsym) X(safety) \
    X(bassOn) X(bassFreq) X(bassDrive)

namespace PID
{
#define X(n) inline constexpr const char* n = #n;
    VC_PARAM_LIST(X)
#undef X
}

inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
{
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> p;

    auto F = [&p](const char* id, const char* name, float lo, float hi, float def,
                  const String& unit, float skew = 1.f, int dec = 1)
    {
        auto toText = [unit, dec](float v, int) -> String
        {
            if (unit == "Hz" && v >= 1000.f) return String(v / 1000.f, 2) + " kHz";
            return String(v, dec) + (unit.isEmpty() ? String() : " " + unit);
        };
        auto fromText = [](const String& s) -> float
        {
            auto t = s.trim();
            float v = t.getFloatValue();
            if (t.containsIgnoreCase("k")) v *= 1000.f;
            return v;
        };
        p.push_back(std::make_unique<AudioParameterFloat>(
            ParameterID { id, 1 }, name, NormalisableRange<float>(lo, hi, 0.f, skew), def,
            AudioParameterFloatAttributes().withLabel(unit)
                .withStringFromValueFunction(toText).withValueFromStringFunction(fromText)));
    };
    auto B = [&p](const char* id, const char* name, bool def)
    { p.push_back(std::make_unique<AudioParameterBool>(ParameterID { id, 1 }, name, def)); };
    auto C = [&p](const char* id, const char* name, StringArray items, int def)
    { p.push_back(std::make_unique<AudioParameterChoice>(ParameterID { id, 1 }, name, items, def)); };

    // Global
    F(PID::inGain, "Input Gain", -24.f, 24.f, 0.f, "dB");
    F(PID::hpf, "Low Cut", 20.f, 500.f, 20.f, "Hz", 0.4f, 0);
    F(PID::lpf, "High Cut", 1000.f, 20000.f, 20000.f, "Hz", 0.3f, 0);
    F(PID::width, "Stereo Width", 0.f, 200.f, 100.f, "%", 1.f, 0);
    F(PID::outGain, "Output Gain", -24.f, 12.f, 0.f, "dB");
    F(PID::mix, "Dry/Wet", 0.f, 100.f, 100.f, "%", 1.f, 0);
    C(PID::oversample, "Oversampling", { "Off", "2x", "4x", "8x" }, 1);

    // Compressor
    B(PID::compOn, "Comp On", true);
    C(PID::compDetect, "Detection", { "Peak", "RMS" }, 0);
    F(PID::compThresh, "Comp Threshold", -60.f, 0.f, -12.f, "dB");
    F(PID::compRatio, "Comp Ratio", 1.f, 20.f, 4.f, ":1", 0.5f);
    F(PID::compAttack, "Comp Attack", 0.1f, 100.f, 10.f, "ms", 0.4f);
    F(PID::compRelease, "Comp Release", 10.f, 1000.f, 120.f, "ms", 0.4f, 0);
    F(PID::compKnee, "Comp Knee", 0.f, 24.f, 6.f, "dB");
    F(PID::compMakeup, "Comp Makeup", 0.f, 24.f, 0.f, "dB");
    F(PID::compMix, "Comp Mix", 0.f, 100.f, 100.f, "%", 1.f, 0);
    F(PID::compSC, "Sidechain HPF", 20.f, 400.f, 80.f, "Hz", 0.5f, 0);

    // Saturation
    B(PID::satOn, "Sat On", true);
    C(PID::satType, "Sat Type", { "Tube", "Tape", "Diode", "Fold", "Crunch", "Crush" }, 0);
    F(PID::satDrive, "Sat Drive", 0.f, 36.f, 6.f, "dB");
    F(PID::satBias, "Sat Bias", -1.f, 1.f, 0.f, "", 1.f, 2);
    F(PID::satTone, "Sat Tone", -1.f, 1.f, 0.f, "", 1.f, 2);
    F(PID::satMix, "Sat Mix", 0.f, 100.f, 100.f, "%", 1.f, 0);
    B(PID::satComp, "Sat Auto Gain", true);

    // Clipper
    B(PID::clipOn, "Clip On", true);
    C(PID::clipShape, "Clip Shape", { "Tanh", "Arctan", "Cubic", "Sine", "Exp", "Hard" }, 0);
    F(PID::clipDrive, "Clip Drive", 0.f, 30.f, 6.f, "dB");
    F(PID::clipCeil, "Ceiling", -12.f, 0.f, -0.3f, "dB");
    F(PID::clipKnee, "Clip Knee", 0.f, 100.f, 60.f, "%", 1.f, 0);
    F(PID::clipAsym, "Clip Asymmetry", -50.f, 50.f, 0.f, "%", 1.f, 0);
    B(PID::safety, "Safe Ceiling", true);

    // Low End
    B(PID::bassOn, "Low End Split", false);
    F(PID::bassFreq, "Split Freq", 40.f, 400.f, 120.f, "Hz", 0.5f, 0);
    F(PID::bassDrive, "Low Drive", -12.f, 18.f, 0.f, "dB");

    return { p.begin(), p.end() };
}
