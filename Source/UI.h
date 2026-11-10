#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "Parameters.h"
#include "DSP.h"

namespace vcui
{
inline const juce::Colour bg0 { 0xff07070b }, bg1 { 0xff15121f }, panelTop { 0xff1d1b29 }, panelBot { 0xff121118 },
    edge { 0xff2e2c3d }, txt { 0xffe4e1f2 }, dim { 0xff8d89a3 }, purple { 0xffa24bff }, red { 0xffff2d55 },
    cyan { 0xff27e0ff }, lime { 0xffb6ff3b }, amber { 0xffffb020 };

inline juce::Font mkFont (float h, bool bold = true)
{
    return juce::Font (juce::FontOptions (h, bold ? juce::Font::bold : juce::Font::plain));
}

// ---------------- Look & Feel ----------------
class VoidLookAndFeel : public juce::LookAndFeel_V4
{
public:
    VoidLookAndFeel()
    {
        setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff0f0e15));
        setColour (juce::ComboBox::outlineColourId, edge);
        setColour (juce::ComboBox::textColourId, txt);
        setColour (juce::ComboBox::arrowColourId, purple);
        setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xff15141c));
        setColour (juce::PopupMenu::textColourId, txt);
        setColour (juce::PopupMenu::highlightedBackgroundColourId, purple.withAlpha (0.5f));
        setColour (juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
        setColour (juce::TextButton::buttonColourId, juce::Colour (0xff1d1c27));
        setColour (juce::TextButton::textColourOffId, txt);
        setColour (juce::Label::textColourId, dim);
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider& s) override
    {
        auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (6.f);
        const float d = juce::jmin (bounds.getWidth(), bounds.getHeight());
        const auto c = bounds.getCentre();
        const float r = d * 0.5f;
        const float angle = startAngle + pos * (endAngle - startAngle);
        const auto col = s.findColour (juce::Slider::rotarySliderFillColourId);

        float zeroAngle = startAngle;
        if (s.getMinimum() < 0.0 && s.getMaximum() > 0.0)
            zeroAngle = startAngle + (float) s.valueToProportionOfLength (0.0) * (endAngle - startAngle);

        juce::Path track;
        track.addCentredArc (c.x, c.y, r - 3.f, r - 3.f, 0.f, startAngle, endAngle, true);
        g.setColour (juce::Colour (0xff24222f));
        g.strokePath (track, juce::PathStrokeType (4.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        juce::Path val;
        val.addCentredArc (c.x, c.y, r - 3.f, r - 3.f, 0.f, zeroAngle, angle, true);
        g.setColour (col.withAlpha (0.25f));
        g.strokePath (val, juce::PathStrokeType (9.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour (col);
        g.strokePath (val, juce::PathStrokeType (4.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        const float kr = r - 11.f;
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff40404d), c.x - kr * 0.4f, c.y - kr * 0.5f,
                                                 juce::Colour (0xff101016), c.x + kr * 0.5f, c.y + kr * 0.6f, true));
        g.fillEllipse (c.x - kr, c.y - kr, kr * 2.f, kr * 2.f);
        g.setColour (juce::Colours::black.withAlpha (0.7f));
        g.drawEllipse (c.x - kr, c.y - kr, kr * 2.f, kr * 2.f, 1.5f);

        juce::Path ptr;
        ptr.addRoundedRectangle (-1.8f, -kr + 3.f, 3.6f, kr * 0.45f, 1.5f);
        g.setColour (col);
        g.fillPath (ptr, juce::AffineTransform::rotation (angle).translated (c.x, c.y));
    }

    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool, bool) override
    {
        auto r = b.getLocalBounds().toFloat().reduced (1.f);
        const bool on = b.getToggleState();
        const auto col = b.findColour (juce::ToggleButton::tickColourId);
        g.setColour (on ? col.withAlpha (0.16f) : juce::Colour (0xff1a1924));
        g.fillRoundedRectangle (r, 5.f);
        g.setColour (on ? col : edge);
        g.drawRoundedRectangle (r, 5.f, 1.2f);

        auto led = juce::Rectangle<float> (r.getX() + 6.f, r.getCentreY() - 4.f, 8.f, 8.f);
        if (on) { g.setColour (col.withAlpha (0.35f)); g.fillEllipse (led.expanded (3.f)); }
        g.setColour (on ? col : dim.darker (0.6f));
        g.fillEllipse (led);

        g.setColour (on ? txt : dim);
        g.setFont (mkFont (11.5f));
        g.drawText (b.getButtonText(), juce::Rectangle<float> (r.getX() + 18.f, r.getY(), r.getWidth() - 18.f, r.getHeight()).toNearestInt(),
                    juce::Justification::centred, false);
    }
};

// ---------------- Knob ----------------
class Knob : public juce::Component
{
public:
    Knob (juce::AudioProcessorValueTreeState& apvts, const juce::String& id, const juce::String& title, juce::Colour colour)
    {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 78, 16);
        slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.75f, true);
        slider.setColour (juce::Slider::rotarySliderFillColourId, colour);
        slider.setColour (juce::Slider::textBoxTextColourId, txt);
        slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        slider.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        addAndMakeVisible (slider);

        label.setText (title, juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        label.setFont (mkFont (11.f));
        label.setColour (juce::Label::textColourId, dim);
        addAndMakeVisible (label);

        attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, id, slider);
        if (auto* prm = apvts.getParameter (id))
            slider.setDoubleClickReturnValue (true, prm->convertFrom0to1 (prm->getDefaultValue()));
    }

    void resized() override
    {
        auto r = getLocalBounds();
        label.setBounds (r.removeFromTop (16));
        slider.setBounds (r);
    }

private:
    juce::Slider slider;
    juce::Label label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
};

// ---------------- Meter ----------------
class MeterBar : public juce::Component
{
public:
    MeterBar (juce::String t, bool gainReduction = false) : title (std::move (t)), isGR (gainReduction) {}

    void push (float v) // dB (Level) bzw. positive dB (GR)
    {
        level = v > level ? v : juce::jmax (v, level - (isGR ? 0.8f : 1.6f));
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat();
        auto titleArea = r.removeFromBottom (16.f);
        auto valueArea = r.removeFromTop (14.f);
        g.setColour (dim);
        g.setFont (mkFont (10.5f));
        g.drawText (title, titleArea.toNearestInt(), juce::Justification::centred);
        g.setColour (txt);
        g.drawText (level <= -99.f ? "-inf" : juce::String (isGR ? -level : level, 1), valueArea.toNearestInt(), juce::Justification::centred);

        auto bar = r.reduced (8.f, 2.f);
        g.setColour (juce::Colour (0xff0b0a10));
        g.fillRoundedRectangle (bar, 4.f);

        const float frac = isGR ? juce::jlimit (0.f, 1.f, level / 24.f)
                                : juce::jlimit (0.f, 1.f, (level + 48.f) / 54.f);
        if (frac > 0.002f)
        {
            juce::Rectangle<float> fill = isGR ? bar.withHeight (bar.getHeight() * frac)
                                               : bar.withTop (bar.getBottom() - bar.getHeight() * frac);
            juce::ColourGradient grad = isGR
                ? juce::ColourGradient::vertical (purple, bar.getY(), red, bar.getBottom())
                : juce::ColourGradient::vertical (red, bar.getY(), lime, bar.getBottom());
            if (! isGR) grad.addColour (0.18, amber);
            g.setGradientFill (grad);
            g.fillRoundedRectangle (fill, 4.f);
        }
        g.setColour (juce::Colours::white.withAlpha (0.12f));
        for (int i = 1; i < 8; ++i)
            g.drawHorizontalLine ((int) (bar.getY() + bar.getHeight() * (float) i / 8.f), bar.getX(), bar.getX() + 5.f);
    }

private:
    juce::String title;
    bool isGR;
    float level = isGR ? 0.f : -100.f;
};

// ---------------- Transfer-Kurve ----------------
class CurveView : public juce::Component
{
public:
    explicit CurveView (juce::AudioProcessorValueTreeState& a) : apvts (a) {}

    void paint (juce::Graphics& g) override
    {
        auto gv = [this] (const char* id) { return apvts.getRawParameterValue (id)->load(); };
        auto r = getLocalBounds().toFloat().reduced (2.f);
        g.setColour (juce::Colour (0xff0c0b12));
        g.fillRoundedRectangle (r, 6.f);

        g.setColour (juce::Colours::white.withAlpha (0.06f));
        for (int i = 1; i < 4; ++i)
        {
            g.drawHorizontalLine ((int) (r.getY() + r.getHeight() * (float) i / 4.f), r.getX(), r.getRight());
            g.drawVerticalLine ((int) (r.getX() + r.getWidth() * (float) i / 4.f), r.getY(), r.getBottom());
        }

        const bool satOn = gv (PID::satOn) > 0.5f, clipOn = gv (PID::clipOn) > 0.5f;
        const int type = juce::roundToInt (gv (PID::satType)), shape = juce::roundToInt (gv (PID::clipShape));
        const float satG = juce::Decibels::decibelsToGain (gv (PID::satDrive));
        const float satComp = gv (PID::satComp) > 0.5f ? std::pow (satG, -0.6f) : 1.f;
        const float bias = gv (PID::satBias) * 0.6f, satMix = gv (PID::satMix) * 0.01f;
        const float cg = juce::Decibels::decibelsToGain (gv (PID::clipDrive));
        const float ceil = juce::Decibels::decibelsToGain (gv (PID::clipCeil));
        const float knee = juce::jmax (0.001f, gv (PID::clipKnee) * 0.01f), asym = gv (PID::clipAsym) * 0.01f;

        auto mapY = [&] (float v) { return juce::jmap (juce::jlimit (-1.25f, 1.25f, v), 1.25f, -1.25f, r.getY(), r.getBottom()); };

        juce::Path diag;
        diag.startNewSubPath (r.getX(), mapY (-1.f)); diag.lineTo (r.getRight(), mapY (1.f));
        g.setColour (dim.withAlpha (0.4f));
        g.strokePath (diag, juce::PathStrokeType (1.f));

        juce::Path ps, pc;
        const int steps = juce::jmax (2, (int) r.getWidth());
        for (int i = 0; i <= steps; ++i)
        {
            const float x = juce::jmap ((float) i, 0.f, (float) steps, -1.f, 1.f);
            float s = x;
            if (satOn) { const float w = vc::saturate (type, x * satG, bias) * satComp; s = x + (w - x) * satMix; }
            float y = s;
            if (clipOn) y = vc::clipAsym (s * cg, shape, knee, asym) * ceil;
            const float px = r.getX() + r.getWidth() * (float) i / (float) steps;
            if (i == 0) { ps.startNewSubPath (px, mapY (s)); pc.startNewSubPath (px, mapY (y)); }
            else        { ps.lineTo (px, mapY (s));          pc.lineTo (px, mapY (y)); }
        }
        if (satOn) { g.setColour (purple.withAlpha (0.8f)); g.strokePath (ps, juce::PathStrokeType (1.5f)); }
        g.setColour (red.withAlpha (0.3f));
        g.strokePath (pc, juce::PathStrokeType (6.f));
        g.setColour (red);
        g.strokePath (pc, juce::PathStrokeType (2.f));

        g.setColour (dim);
        g.setFont (mkFont (10.f, false));
        g.drawText ("IN", r.toNearestInt().reduced (6, 4), juce::Justification::bottomRight);
        g.drawText ("OUT", r.toNearestInt().reduced (6, 4), juce::Justification::topLeft);
    }

private:
    juce::AudioProcessorValueTreeState& apvts;
};
} // namespace vcui
