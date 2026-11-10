#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
struct Preset { juce::String name; std::vector<std::pair<const char*, float>> values; };

// Startpunkte fuer harte Rage-/Underground-Beats (Oversampling: 0=Off 1=2x 2=4x 3=8x)
const std::vector<Preset>& getPresets()
{
    static const std::vector<Preset> presets = {
        Preset { "Init", {} },
        Preset { "Rage Smash", {
            { PID::inGain, 2.f }, { PID::hpf, 28.f }, { PID::compThresh, -20.f }, { PID::compRatio, 4.f },
            { PID::compAttack, 6.f }, { PID::compRelease, 80.f }, { PID::compKnee, 4.f }, { PID::compMakeup, 5.f },
            { PID::satType, 2.f }, { PID::satDrive, 14.f }, { PID::satBias, 0.15f }, { PID::satTone, 0.25f },
            { PID::clipShape, 0.f }, { PID::clipDrive, 10.f }, { PID::clipCeil, -0.3f }, { PID::clipKnee, 45.f },
            { PID::clipAsym, 8.f }, { PID::bassOn, 1.f }, { PID::bassFreq, 130.f }, { PID::bassDrive, 3.f },
            { PID::lpf, 17000.f }, { PID::oversample, 2.f } } },
        Preset { "Opium Crunch", {
            { PID::compThresh, -16.f }, { PID::compRatio, 3.f }, { PID::compAttack, 15.f }, { PID::compRelease, 140.f },
            { PID::compMakeup, 3.f }, { PID::satType, 4.f }, { PID::satDrive, 16.f }, { PID::satBias, 0.3f },
            { PID::satTone, 0.4f }, { PID::satMix, 85.f }, { PID::clipShape, 2.f }, { PID::clipDrive, 8.f },
            { PID::clipCeil, -0.5f }, { PID::clipKnee, 30.f }, { PID::lpf, 15000.f }, { PID::oversample, 2.f } } },
        Preset { "808 Slam", {
            { PID::hpf, 22.f }, { PID::compThresh, -22.f }, { PID::compRatio, 6.f }, { PID::compAttack, 3.f },
            { PID::compRelease, 60.f }, { PID::compKnee, 2.f }, { PID::compMakeup, 4.f }, { PID::satType, 1.f },
            { PID::satDrive, 9.f }, { PID::satTone, -0.2f }, { PID::clipShape, 0.f }, { PID::clipDrive, 12.f },
            { PID::clipKnee, 55.f }, { PID::bassOn, 1.f }, { PID::bassFreq, 95.f }, { PID::bassDrive, 6.f },
            { PID::oversample, 2.f } } },
        Preset { "Ghost Fold", {
            { PID::satType, 3.f }, { PID::satDrive, 20.f }, { PID::satBias, 0.2f }, { PID::satMix, 70.f },
            { PID::satTone, 0.1f }, { PID::clipShape, 3.f }, { PID::clipDrive, 6.f }, { PID::clipKnee, 70.f },
            { PID::lpf, 12000.f }, { PID::oversample, 3.f } } },
        Preset { "Plugg Glue", {
            { PID::compThresh, -14.f }, { PID::compRatio, 2.5f }, { PID::compAttack, 25.f }, { PID::compRelease, 220.f },
            { PID::compKnee, 10.f }, { PID::compMakeup, 2.f }, { PID::satType, 1.f }, { PID::satDrive, 5.f },
            { PID::clipDrive, 3.f }, { PID::clipKnee, 80.f }, { PID::oversample, 1.f } } },
        Preset { "Dark Distort", {
            { PID::satType, 2.f }, { PID::satDrive, 18.f }, { PID::satTone, -0.5f }, { PID::clipShape, 4.f },
            { PID::clipDrive, 8.f }, { PID::clipKnee, 20.f }, { PID::clipAsym, 15.f }, { PID::lpf, 8500.f },
            { PID::bassOn, 1.f }, { PID::bassFreq, 150.f }, { PID::bassDrive, 4.f }, { PID::oversample, 2.f } } },
        Preset { "Loud Master", {
            { PID::compThresh, -10.f }, { PID::compRatio, 2.f }, { PID::compAttack, 30.f }, { PID::compRelease, 150.f },
            { PID::compKnee, 8.f }, { PID::compMakeup, 1.f }, { PID::satType, 1.f }, { PID::satDrive, 3.f },
            { PID::clipShape, 1.f }, { PID::clipDrive, 5.f }, { PID::clipCeil, -0.1f }, { PID::clipKnee, 65.f },
            { PID::oversample, 3.f } } },
        Preset { "Hard Clip Bus", {
            { PID::satOn, 0.f }, { PID::clipShape, 5.f }, { PID::clipDrive, 7.f }, { PID::clipKnee, 5.f },
            { PID::oversample, 3.f } } },
    };
    return presets;
}
}

VoidClipAudioProcessor::VoidClipAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "VOIDCLIP", createLayout())
{
#define X(n) p.n = apvts.getRawParameterValue (PID::n);
    VC_PARAM_LIST(X)
#undef X
}

bool VoidClipAudioProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    return l.getMainInputChannelSet() == juce::AudioChannelSet::stereo()
        && l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void VoidClipAudioProcessor::prepareToPlay (double sr, int spb)
{
    sampleRate = sr;
    maxBlock = juce::jmax (spb, 64);
    juce::dsp::ProcessSpec spec { sr, (juce::uint32) maxBlock, 2 };

    hpf.prepare (spec); hpf.setType (juce::dsp::StateVariableTPTFilterType::highpass);
    lpf.prepare (spec); lpf.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
    comp.prepare (sr);

    for (int k = 0; k < 3; ++k)
    {
        os[k] = std::make_unique<juce::dsp::Oversampling<float>> (
            2, (size_t) (k + 1), juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple, true, true);
        os[k]->initProcessing ((size_t) maxBlock);
    }

    dryBuf.setSize (2, maxBlock);
    dryDelay.prepare (spec);

    for (auto* s : { &smSatDrive, &smSatComp, &smClipDrive, &smCeil, &smBass })
        s->setCurrentAndTargetValue (1.f);

    lastInGain  = juce::Decibels::decibelsToGain (p.inGain->load());
    lastOutGain = juce::Decibels::decibelsToGain (p.outGain->load());
    currentOs = -1;
    setOversamplingIndex (juce::jlimit (0, 3, juce::roundToInt (p.oversample->load())));
}

void VoidClipAudioProcessor::setOversamplingIndex (int idx)
{
    currentOs = idx;
    const int factor = 1 << idx;
    osRate = sampleRate * factor;

    for (int c = 0; c < 2; ++c)
    {
        dcSat[c].setSampleRate (osRate); dcClip[c].setSampleRate (osRate); tilt[c].setSampleRate (osRate);
        dcSat[c].reset(); dcClip[c].reset(); tilt[c].reset();
    }
    xover.prepare ({ osRate, (juce::uint32) (maxBlock * factor), 2 });
    xover.setType (juce::dsp::LinkwitzRileyFilterType::lowpass);

    for (auto* s : { &smSatDrive, &smSatComp, &smClipDrive, &smCeil, &smBass })
        s->reset (osRate, 0.02);

    int latency = 0;
    if (idx > 0)
    {
        os[idx - 1]->reset();
        latency = juce::roundToInt (os[idx - 1]->getLatencyInSamples());
    }
    setLatencySamples (latency);
    dryDelay.reset();
    dryDelay.setDelay ((float) latency);
}

void VoidClipAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int total = buffer.getNumSamples();
    int pos = 0;
    while (pos < total)
    {
        const int len = juce::jmin (maxBlock, total - pos);
        juce::AudioBuffer<float> chunk (buffer.getArrayOfWritePointers(), buffer.getNumChannels(), pos, len);
        processChunk (chunk);
        pos += len;
    }
}

void VoidClipAudioProcessor::processChunk (juce::AudioBuffer<float>& buf)
{
    using juce::Decibels;
    const int n = buf.getNumSamples();

    const int osIdx = juce::jlimit (0, 3, juce::roundToInt (p.oversample->load()));
    if (osIdx != currentOs) setOversamplingIndex (osIdx);

    // Dry-Pfad (latenzkompensiert)
    for (int c = 0; c < 2; ++c)
    {
        dryBuf.copyFrom (c, 0, buf, c, 0, n);
        auto* d = dryBuf.getWritePointer (c);
        for (int i = 0; i < n; ++i) { dryDelay.pushSample (c, d[i]); d[i] = dryDelay.popSample (c); }
    }

    // Input Gain
    const float inG = Decibels::decibelsToGain (p.inGain->load());
    buf.applyGainRamp (0, n, lastInGain, inG);
    lastInGain = inG;
    inMeter.store (juce::jmax (inMeter.load(), juce::jmax (buf.getMagnitude (0, 0, n), buf.getMagnitude (1, 0, n))));

    // Low Cut
    const float hp = p.hpf->load();
    if (hp > 20.5f)
    {
        hpf.setCutoffFrequency (hp);
        juce::dsp::AudioBlock<float> b (buf);
        hpf.process (juce::dsp::ProcessContextReplacing<float> (b));
    }

    // Kompressor
    if (p.compOn->load() > 0.5f)
    {
        vc::Compressor::Params cp;
        cp.thresh = p.compThresh->load();   cp.ratio = p.compRatio->load();
        cp.attackMs = p.compAttack->load(); cp.releaseMs = p.compRelease->load();
        cp.kneeDb = p.compKnee->load();     cp.makeupDb = p.compMakeup->load();
        cp.mix = p.compMix->load() * 0.01f; cp.scFreq = p.compSC->load();
        cp.rms = p.compDetect->load() > 0.5f;
        grMeter.store (comp.process (buf, cp));
    }
    else
    {
        comp.reset();
        grMeter.store (0.f);
    }

    // Saturation + Clipper (oversampled)
    juce::dsp::AudioBlock<float> block (buf);
    if (currentOs > 0)
    {
        auto& o = *os[currentOs - 1];
        auto up = o.processSamplesUp (block);
        processNonlinear (up);
        o.processSamplesDown (block);
    }
    else
    {
        processNonlinear (block);
    }

    // High Cut
    const float lp = p.lpf->load();
    if (lp < 19500.f)
    {
        lpf.setCutoffFrequency (lp);
        juce::dsp::AudioBlock<float> b (buf);
        lpf.process (juce::dsp::ProcessContextReplacing<float> (b));
    }

    // Stereo Width (M/S)
    const float w = p.width->load() * 0.01f;
    if (std::abs (w - 1.f) > 0.001f)
    {
        auto* l = buf.getWritePointer (0);
        auto* r = buf.getWritePointer (1);
        for (int i = 0; i < n; ++i)
        {
            const float m = (l[i] + r[i]) * 0.5f, s = (l[i] - r[i]) * 0.5f * w;
            l[i] = m + s; r[i] = m - s;
        }
    }

    // Dry/Wet
    const float mix = p.mix->load() * 0.01f;
    if (mix < 0.999f)
        for (int c = 0; c < 2; ++c)
        {
            auto* wet = buf.getWritePointer (c);
            auto* dry = dryBuf.getReadPointer (c);
            for (int i = 0; i < n; ++i) wet[i] = dry[i] + (wet[i] - dry[i]) * mix;
        }

    // Output Gain
    const float outG = Decibels::decibelsToGain (p.outGain->load());
    buf.applyGainRamp (0, n, lastOutGain, outG);
    lastOutGain = outG;
    outMeter.store (juce::jmax (outMeter.load(), juce::jmax (buf.getMagnitude (0, 0, n), buf.getMagnitude (1, 0, n))));
}

void VoidClipAudioProcessor::processNonlinear (juce::dsp::AudioBlock<float>& b)
{
    using juce::Decibels;
    const int ns = (int) b.getNumSamples();
    float* d[2] = { b.getChannelPointer (0), b.getChannelPointer (1) };

    const bool satOn  = p.satOn->load()  > 0.5f;
    const bool clipOn = p.clipOn->load() > 0.5f;
    const bool bassOn = p.bassOn->load() > 0.5f;
    const bool safety = p.safety->load() > 0.5f;
    const int satType = juce::roundToInt (p.satType->load());
    const int shape   = juce::roundToInt (p.clipShape->load());
    const float satMix = p.satMix->load() * 0.01f;
    const float bias   = p.satBias->load() * 0.6f;
    const float tone   = p.satTone->load();
    const float tLow   = Decibels::decibelsToGain (-6.f * tone);
    const float tHigh  = Decibels::decibelsToGain ( 6.f * tone);
    const float knee   = juce::jmax (0.001f, p.clipKnee->load() * 0.01f);
    const float asym   = p.clipAsym->load() * 0.01f;
    const bool useAsym = std::abs (asym) > 0.0005f;
    const float satG   = Decibels::decibelsToGain (p.satDrive->load());

    smSatDrive.setTargetValue (satG);
    smSatComp.setTargetValue (p.satComp->load() > 0.5f ? std::pow (satG, -0.6f) : 1.f);
    smClipDrive.setTargetValue (Decibels::decibelsToGain (p.clipDrive->load()));
    smCeil.setTargetValue (Decibels::decibelsToGain (p.clipCeil->load()));
    smBass.setTargetValue (Decibels::decibelsToGain (p.bassDrive->load()));
    xover.setCutoffFrequency (p.bassFreq->load());

    float preMax = 0.f;

    for (int i = 0; i < ns; ++i)
    {
        const float sDrive = smSatDrive.getNextValue();
        const float sComp  = smSatComp.getNextValue();
        const float cDrive = smClipDrive.getNextValue();
        const float ceil   = smCeil.getNextValue();
        const float bassG  = smBass.getNextValue();

        for (int c = 0; c < 2; ++c)
        {
            float x = d[c][i];

            if (satOn)
            {
                float s = vc::saturate (satType, x * sDrive, bias);
                s = dcSat[c].process (s);
                s = tilt[c].process (s, tLow, tHigh) * sComp;
                x += (s - x) * satMix;
            }

            if (clipOn)
            {
                float y;
                if (bassOn)
                {
                    float lo, hi;
                    xover.processSample (c, x, lo, hi);
                    const float lg = cDrive * bassG;
                    preMax = juce::jmax (preMax, std::abs (lo * lg), std::abs (hi * cDrive));
                    y = vc::clipAsym (lo * lg, shape, knee, asym) + vc::clipAsym (hi * cDrive, shape, knee, asym);
                }
                else
                {
                    preMax = juce::jmax (preMax, std::abs (x * cDrive));
                    y = vc::clipAsym (x * cDrive, shape, knee, asym);
                }
                y *= ceil;
                if (useAsym) y = dcClip[c].process (y);
                if (safety)
                {
                    const float lim = ceil * (1.f + std::abs (asym));
                    y = juce::jlimit (-lim, lim, y);
                }
                x = y;
            }
            d[c][i] = x;
        }
    }
    clipMeter.store (juce::jmax (clipMeter.load(), preMax));
}

int VoidClipAudioProcessor::getNumPrograms() { return (int) getPresets().size(); }
const juce::String VoidClipAudioProcessor::getProgramName (int i)
{
    return juce::isPositiveAndBelow (i, getNumPrograms()) ? getPresets()[(size_t) i].name : juce::String();
}

void VoidClipAudioProcessor::setCurrentProgram (int index)
{
    if (! juce::isPositiveAndBelow (index, getNumPrograms())) return;
    currentProgram = index;
    for (auto* prm : getParameters())
        if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (prm))
            rp->setValueNotifyingHost (rp->getDefaultValue());
    for (auto& kv : getPresets()[(size_t) index].values)
        if (auto* rp = apvts.getParameter (kv.first))
            rp->setValueNotifyingHost (rp->convertTo0to1 (kv.second));
}

void VoidClipAudioProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    auto state = apvts.copyState();
    if (auto xml = state.createXml()) copyXmlToBinary (*xml, dest);
}

void VoidClipAudioProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor* VoidClipAudioProcessor::createEditor() { return new VoidClipAudioProcessorEditor (*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new VoidClipAudioProcessor(); }
