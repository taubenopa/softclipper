#include "PluginEditor.h"

using namespace vcui;

VoidClipAudioProcessorEditor::VoidClipAudioProcessorEditor (VoidClipAudioProcessor& p)
    : AudioProcessorEditor (&p), proc (p), curve (p.apvts)
{
    setLookAndFeel (&laf);

    auto mk = [this] (std::unique_ptr<Knob>& k, const char* id, const char* name, juce::Colour c)
    { k = std::make_unique<Knob> (proc.apvts, id, name, c); addAndMakeVisible (*k); };

    mk (kInGain, PID::inGain, "INPUT", cyan);   mk (kHpf, PID::hpf, "LOW CUT", cyan);
    mk (kLpf, PID::lpf, "HIGH CUT", cyan);      mk (kWidth, PID::width, "WIDTH", cyan);
    mk (kOGain, PID::outGain, "OUTPUT", cyan);  mk (kOMix, PID::mix, "DRY / WET", cyan);

    mk (kCThresh, PID::compThresh, "THRESH", purple);   mk (kCRatio, PID::compRatio, "RATIO", purple);
    mk (kCAttack, PID::compAttack, "ATTACK", purple);   mk (kCRelease, PID::compRelease, "RELEASE", purple);
    mk (kCKnee, PID::compKnee, "KNEE", purple);         mk (kCSC, PID::compSC, "SC HPF", purple);
    mk (kCMakeup, PID::compMakeup, "MAKEUP", purple);   mk (kCMix, PID::compMix, "MIX", purple);

    mk (kSDrive, PID::satDrive, "DRIVE", amber);  mk (kSBias, PID::satBias, "BIAS", amber);
    mk (kSTone, PID::satTone, "TONE", amber);     mk (kSMix, PID::satMix, "MIX", amber);

    mk (kKDrive, PID::clipDrive, "DRIVE", red);   mk (kKCeil, PID::clipCeil, "CEILING", red);
    mk (kKKnee, PID::clipKnee, "KNEE", red);      mk (kKAsym, PID::clipAsym, "ASYM", red);

    mk (kBFreq, PID::bassFreq, "SPLIT FREQ", lime);  mk (kBDrive, PID::bassDrive, "LOW DRIVE", lime);

    auto setupCombo = [this] (juce::ComboBox& box, const char* id, std::unique_ptr<APVTS::ComboBoxAttachment>& att)
    {
        if (auto* cp = dynamic_cast<juce::AudioParameterChoice*> (proc.apvts.getParameter (id)))
            box.addItemList (cp->choices, 1);
        addAndMakeVisible (box);
        att = std::make_unique<APVTS::ComboBoxAttachment> (proc.apvts, id, box);
    };
    setupCombo (osBox, PID::oversample, osAtt);
    setupCombo (detectBox, PID::compDetect, detAtt);
    setupCombo (satTypeBox, PID::satType, typeAtt);
    setupCombo (shapeBox, PID::clipShape, shapeAtt);

    auto mkLabel = [this] (juce::Label& l, const char* t)
    {
        l.setText (t, juce::dontSendNotification);
        l.setFont (mkFont (10.5f));
        l.setColour (juce::Label::textColourId, dim);
        addAndMakeVisible (l);
    };
    mkLabel (osLabel, "OVERSAMPLING"); mkLabel (detLabel, "DETECTION");
    mkLabel (typeLabel, "TYPE");       mkLabel (shapeLabel, "SHAPE");
    osLabel.setJustificationType (juce::Justification::centredRight);

    auto mkToggle = [this] (juce::ToggleButton& b, const char* id, const char* text, juce::Colour c)
    {
        b.setButtonText (text);
        b.setColour (juce::ToggleButton::tickColourId, c);
        addAndMakeVisible (b);
        btnAtts.push_back (std::make_unique<APVTS::ButtonAttachment> (proc.apvts, id, b));
    };
    mkToggle (compPower, PID::compOn, "ACTIVE", purple);
    mkToggle (satPower, PID::satOn, "ACTIVE", amber);
    mkToggle (clipPower, PID::clipOn, "ACTIVE", red);
    mkToggle (bassPower, PID::bassOn, "SPLIT", lime);
    mkToggle (satAuto, PID::satComp, "AUTO GAIN", amber);
    mkToggle (safety, PID::safety, "SAFE CEIL", red);

    for (int i = 0; i < proc.getNumPrograms(); ++i) presetBox.addItem (proc.getProgramName (i), i + 1);
    presetBox.setSelectedItemIndex (proc.getCurrentProgram(), juce::dontSendNotification);
    presetBox.onChange = [this] { proc.setCurrentProgram (presetBox.getSelectedItemIndex()); };
    addAndMakeVisible (presetBox);
    auto step = [this] (int dir)
    {
        const int n = proc.getNumPrograms();
        presetBox.setSelectedItemIndex ((presetBox.getSelectedItemIndex() + dir + n) % n, juce::sendNotificationAsync);
    };
    prevBtn.onClick = [step] { step (-1); };
    nextBtn.onClick = [step] { step (1); };
    addAndMakeVisible (prevBtn);
    addAndMakeVisible (nextBtn);

    addAndMakeVisible (inBar);
    addAndMakeVisible (outBar);
    addAndMakeVisible (grBar);
    addAndMakeVisible (curve);

    setSize (1040, 606);
    startTimerHz (30);
}

VoidClipAudioProcessorEditor::~VoidClipAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void VoidClipAudioProcessorEditor::timerCallback()
{
    auto toDb = [] (float g) { return juce::Decibels::gainToDecibels (g, -100.f); };
    inBar.push (toDb (proc.inMeter.exchange (0.f)));
    outBar.push (toDb (proc.outMeter.exchange (0.f)));
    grBar.push (-proc.grMeter.load());
    proc.clipMeter.exchange (0.f);
    curve.repaint();
}

void VoidClipAudioProcessorEditor::drawPanel (juce::Graphics& g, juce::Rectangle<int> r, const juce::String& title, juce::Colour c)
{
    auto f = r.toFloat();
    g.setGradientFill (juce::ColourGradient (panelTop, f.getX(), f.getY(), panelBot, f.getX(), f.getBottom(), false));
    g.fillRoundedRectangle (f, 8.f);
    g.setColour (edge);
    g.drawRoundedRectangle (f.reduced (0.5f), 8.f, 1.f);
    g.setColour (c);
    g.fillRoundedRectangle (f.getX() + 12.f, f.getY() + 8.f, 3.f, 14.f, 1.5f);
    g.setColour (txt);
    g.setFont (mkFont (13.f));
    g.drawText (title, (int) f.getX() + 22, (int) f.getY() + 5, 220, 20, juce::Justification::centredLeft);
}

void VoidClipAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.setGradientFill (juce::ColourGradient (bg1, 0.f, 0.f, bg0, 0.f, (float) getHeight(), false));
    g.fillAll();

    g.setColour (txt);
    g.setFont (mkFont (30.f));
    g.drawText ("VOID", 16, 8, 90, 34, juce::Justification::centredLeft);
    g.setColour (red);
    g.drawText ("CLIP", 98, 8, 90, 34, juce::Justification::centredLeft);
    g.setColour (dim);
    g.setFont (mkFont (10.f, false));
    g.drawText ("SOFT CLIPPER  /  SATURATOR  /  COMPRESSOR", 18, 40, 320, 14, juce::Justification::centredLeft);

    for (auto& pn : panels) drawPanel (g, pn.r, pn.title, pn.c);
}

void VoidClipAudioProcessorEditor::resized()
{
    auto b = getLocalBounds().reduced (8);
    auto header = b.removeFromTop (52);
    b.removeFromTop (6);

    // Header
    auto hr = header;
    osBox.setBounds (hr.removeFromRight (80).reduced (0, 10));
    osLabel.setBounds (hr.removeFromRight (110).reduced (0, 10));
    auto mid = juce::Rectangle<int> (0, header.getY() + 10, 290, 32).withCentre ({ getWidth() / 2, header.getCentreY() });
    prevBtn.setBounds (mid.removeFromLeft (30));
    nextBtn.setBounds (mid.removeFromRight (30));
    presetBox.setBounds (mid.reduced (4, 0));

    auto body = b;
    inBar.setBounds (body.removeFromLeft (38));
    outBar.setBounds (body.removeFromRight (38));
    body.reduce (8, 0);

    auto r1 = body.removeFromTop (172); body.removeFromTop (8);
    auto r2 = body.removeFromTop (172); body.removeFromTop (8);
    auto r3 = body.removeFromTop (172);

    auto content = [] (juce::Rectangle<int> r) { return r.reduced (8).withTrimmedTop (22); };
    auto layoutKnobs = [] (juce::Rectangle<int> area, std::initializer_list<juce::Component*> list)
    {
        const int w = area.getWidth() / (int) list.size();
        for (auto* c : list) c->setBounds (area.removeFromLeft (w));
    };
    auto powerBtn = [] (juce::Component& c, juce::Rectangle<int> panel)
    { c.setBounds (panel.getRight() - 88, panel.getY() + 5, 78, 20); };

    panels.clear();

    // Compressor
    panels.push_back ({ r1, "COMPRESSOR", purple });
    powerBtn (compPower, r1);
    {
        auto c = content (r1);
        auto right = c.removeFromRight (180);
        grBar.setBounds (right.removeFromRight (40));
        right.removeFromRight (8);
        detLabel.setBounds (right.removeFromTop (16));
        detectBox.setBounds (right.removeFromTop (26));
        layoutKnobs (c, { kCThresh.get(), kCRatio.get(), kCAttack.get(), kCRelease.get(),
                          kCKnee.get(), kCSC.get(), kCMakeup.get(), kCMix.get() });
    }

    // Saturation | Clipper
    const int half = (r2.getWidth() - 8) / 2;
    auto satR = r2.removeFromLeft (half); r2.removeFromLeft (8);
    auto clipR = r2;
    panels.push_back ({ satR, "SATURATION", amber });
    panels.push_back ({ clipR, "SOFT CLIPPER", red });
    powerBtn (satPower, satR);
    powerBtn (clipPower, clipR);
    {
        auto c = content (satR);
        auto left = c.removeFromLeft (108);
        typeLabel.setBounds (left.removeFromTop (16));
        satTypeBox.setBounds (left.removeFromTop (26));
        left.removeFromTop (10);
        satAuto.setBounds (left.removeFromTop (24));
        layoutKnobs (c, { kSDrive.get(), kSBias.get(), kSTone.get(), kSMix.get() });
    }
    {
        auto c = content (clipR);
        auto left = c.removeFromLeft (108);
        shapeLabel.setBounds (left.removeFromTop (16));
        shapeBox.setBounds (left.removeFromTop (26));
        left.removeFromTop (10);
        safety.setBounds (left.removeFromTop (24));
        layoutKnobs (c, { kKDrive.get(), kKCeil.get(), kKKnee.get(), kKAsym.get() });
    }

    // Row 3
    auto lowR = r3.removeFromLeft (190); r3.removeFromLeft (8);
    auto filtR = r3.removeFromLeft (350); r3.removeFromLeft (8);
    auto outR = r3.removeFromLeft (190); r3.removeFromLeft (8);
    auto curveR = r3;
    panels.push_back ({ lowR, "LOW END CLIP", lime });
    panels.push_back ({ filtR, "INPUT / TONE", cyan });
    panels.push_back ({ outR, "OUTPUT", cyan });
    panels.push_back ({ curveR, "TRANSFER", red });
    powerBtn (bassPower, lowR);
    layoutKnobs (content (lowR), { kBFreq.get(), kBDrive.get() });
    layoutKnobs (content (filtR), { kInGain.get(), kHpf.get(), kLpf.get(), kWidth.get() });
    layoutKnobs (content (outR), { kOGain.get(), kOMix.get() });
    curve.setBounds (curveR.reduced (8).withTrimmedTop (22));
}
