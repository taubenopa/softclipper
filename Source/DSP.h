#pragma once
#include <juce_dsp/juce_dsp.h>
#include <cmath>

namespace vc
{
// ---------- Clipper-Kurven: f(0)=0, f'(0)=1, f(inf)=1 ----------
inline float softShape (int shape, float x) noexcept
{
    constexpr float halfPi = juce::MathConstants<float>::halfPi;
    switch (shape)
    {
        case 0:  return std::tanh (x);
        case 1:  return (2.f / juce::MathConstants<float>::pi) * std::atan (halfPi * x);
        case 2:  return x >= 1.5f ? 1.f : x - (4.f / 27.f) * x * x * x;
        case 3:  return std::sin (juce::jmin (x, halfPi));
        case 4:  return 1.f - std::exp (-x);
        default: return juce::jmin (x, 1.f);
    }
}

// Symmetrisch, Schwelle 1.0, k = Knie (0 = hart, 1 = komplett weich)
inline float clipOne (float x, int shape, float k) noexcept
{
    const float a = std::abs (x);
    const float t = 1.f - k;
    const float y = a <= t ? a : t + k * softShape (shape, (a - t) / k);
    return x < 0.f ? -y : y;
}

inline float clipAsym (float x, int shape, float k, float asym) noexcept
{
    return x >= 0.f ? clipOne (x / (1.f + asym), shape, k) * (1.f + asym)
                    : clipOne (x / (1.f - asym), shape, k) * (1.f - asym);
}

// ---------- Saturation ----------
inline float saturate (int type, float x, float bias) noexcept
{
    const float v = x + bias;
    switch (type)
    {
        case 0: return std::tanh (v) - std::tanh (bias);                                   // Tube
        case 1: return (2.f / juce::MathConstants<float>::pi) * (std::atan (1.6f * v) - std::atan (1.6f * bias)); // Tape
        case 2: { auto f = [] (float a) { return a >= 0.f ? 1.f - std::exp (-a)
                                                          : -(1.f - std::exp (1.5f * a)) / 1.5f; };
                  return f (v) - f (bias); }                                               // Diode
        case 3: return std::sin (v) - std::sin (bias);                                     // Wavefolder
        case 4: return v >= 0.f ? std::tanh (1.5f * v) : juce::jmax (-0.6f, 1.2f * std::tanh (v)); // Crunch
        default: return std::round (std::tanh (v) * 12.f) / 12.f;                          // Crush
    }
}

struct DCBlocker
{
    float x1 = 0.f, y1 = 0.f, R = 0.995f;
    void setSampleRate (double sr) { R = 1.f - (juce::MathConstants<float>::twoPi * 8.f / (float) sr); }
    void reset() { x1 = y1 = 0.f; }
    float process (float x) noexcept { const float y = x - x1 + R * y1; x1 = x; y1 = y; return y; }
};

struct Tilt
{
    float lp = 0.f, a = 0.1f;
    void setSampleRate (double sr) { a = 1.f - std::exp (-juce::MathConstants<float>::twoPi * 700.f / (float) sr); }
    void reset() { lp = 0.f; }
    float process (float x, float gLow, float gHigh) noexcept { lp += a * (x - lp); return lp * gLow + (x - lp) * gHigh; }
};

// ---------- Kompressor (stereo-linked, Soft-Knee, Peak/RMS, Sidechain-HPF, Parallel-Mix) ----------
class Compressor
{
public:
    struct Params { float thresh, ratio, attackMs, releaseMs, kneeDb, makeupDb, mix, scFreq; bool rms; };

    void prepare (double sr)
    {
        sampleRate = sr;
        juce::dsp::ProcessSpec spec { sr, 8192, 2 };
        scHP.prepare (spec);
        scHP.setType (juce::dsp::StateVariableTPTFilterType::highpass);
        smMakeup.reset (sr, 0.03);
        smMix.reset (sr, 0.03);
        smMakeup.setCurrentAndTargetValue (1.f);
        smMix.setCurrentAndTargetValue (1.f);
        reset();
    }
    void reset() { scHP.reset(); gr = 0.f; rmsState = 0.f; }

    // gibt maximale Gain Reduction (dB, <= 0) des Blocks zurueck
    float process (juce::AudioBuffer<float>& buf, const Params& p)
    {
        const float sr = (float) sampleRate;
        const float attC = std::exp (-1.f / (0.001f * p.attackMs * sr));
        const float relC = std::exp (-1.f / (0.001f * p.releaseMs * sr));
        const float rmsC = std::exp (-1.f / (0.010f * sr));
        scHP.setCutoffFrequency (p.scFreq);
        smMakeup.setTargetValue (juce::Decibels::decibelsToGain (p.makeupDb));
        smMix.setTargetValue (p.mix);

        float* ch[2] = { buf.getWritePointer (0), buf.getWritePointer (1) };
        const int n = buf.getNumSamples();
        float minGr = 0.f;

        for (int i = 0; i < n; ++i)
        {
            float lvl = 0.f, sq = 0.f;
            for (int c = 0; c < 2; ++c)
            {
                const float s = scHP.processSample (c, ch[c][i]);
                lvl = juce::jmax (lvl, std::abs (s));
                sq += s * s;
            }
            if (p.rms) { rmsState = rmsC * rmsState + (1.f - rmsC) * (sq * 0.5f); lvl = std::sqrt (rmsState); }

            const float lv = juce::Decibels::gainToDecibels (lvl, -100.f);
            const float over = lv - p.thresh;
            const float k = p.kneeDb;
            float out;
            if (2.f * over < -k)                          out = lv;
            else if (2.f * over > k || k <= 0.001f)       out = p.thresh + over / p.ratio;
            else { const float t = over + k * 0.5f;       out = lv + (1.f / p.ratio - 1.f) * t * t / (2.f * k); }

            const float target = out - lv; // <= 0
            const float coef = target < gr ? attC : relC;
            gr = coef * gr + (1.f - coef) * target;
            minGr = juce::jmin (minGr, gr);

            const float g = juce::Decibels::decibelsToGain (gr) * smMakeup.getNextValue();
            const float mix = smMix.getNextValue();
            for (int c = 0; c < 2; ++c)
            {
                const float dry = ch[c][i];
                ch[c][i] = dry + (dry * g - dry) * mix;
            }
        }
        return minGr;
    }

private:
    double sampleRate = 44100.0;
    juce::dsp::StateVariableTPTFilter<float> scHP;
    juce::SmoothedValue<float> smMakeup, smMix;
    float gr = 0.f, rmsState = 0.f;
};
} // namespace vc
