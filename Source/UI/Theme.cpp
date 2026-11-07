#include "Theme.h"
#include "BinaryData.h"

namespace theme
{
juce::Font font (float height, bool semibold)
{
    // Embedded Inter (SIL OFL): identical rendering regardless of fonts present in the Wine prefix.
    static const auto regular = juce::Typeface::createSystemTypefaceFor (BinaryData::InterRegular_ttf, BinaryData::InterRegular_ttfSize);
    static const auto bold = juce::Typeface::createSystemTypefaceFor (BinaryData::InterSemiBold_ttf, BinaryData::InterSemiBold_ttfSize);
    return juce::Font (juce::FontOptions (semibold ? bold : regular).withHeight (height));
}

// Segment bits: a=top b=upper-right c=lower-right d=bottom e=lower-left f=upper-left g=middle.
static int segmentsFor (juce::juce_wchar ch)
{
    switch (ch)
    {
        case '0': return 0b0111111; case '1': return 0b0000110; case '2': return 0b1011011; case '3': return 0b1001111;
        case '4': return 0b1100110; case '5': return 0b1101101; case '6': return 0b1111101; case '7': return 0b0000111;
        case '8': return 0b1111111; case '9': return 0b1101111; case '-': return 0b1000000;
        default: return 0;
    }
}

static void drawDigit (juce::Graphics& g, int bits, juce::Rectangle<float> r, juce::Colour on, juce::Colour ghost)
{
    const float w = r.getWidth(), h = r.getHeight(), t = w * 0.19f, gap = t * 0.18f, ht = t * 0.5f;
    const float x0 = r.getX(), y0 = r.getY();
    auto horiz = [&] (float y) {
        const float l = x0 + gap + ht, rr = x0 + w - gap - ht;
        juce::Path p;
        p.startNewSubPath (l, y); p.lineTo (l + ht, y - ht); p.lineTo (rr - ht, y - ht);
        p.lineTo (rr, y); p.lineTo (rr - ht, y + ht); p.lineTo (l + ht, y + ht); p.closeSubPath();
        return p;
    };
    auto vert = [&] (float x, float ya, float yb) {
        const float a = ya + gap, b = yb - gap;
        juce::Path p;
        p.startNewSubPath (x, a); p.lineTo (x + ht, a + ht); p.lineTo (x + ht, b - ht);
        p.lineTo (x, b); p.lineTo (x - ht, b - ht); p.lineTo (x - ht, a + ht); p.closeSubPath();
        return p;
    };
    const float xl = x0 + ht, xr = x0 + w - ht, yt = y0 + ht, yb = y0 + h - ht, ym = y0 + h * 0.5f;
    const juce::Path seg[7] { horiz (yt), vert (xr, yt, ym), vert (xr, ym, yb), horiz (yb), vert (xl, ym, yb), vert (xl, yt, ym), horiz (ym) };
    for (int i = 0; i < 7; ++i)
    {
        g.setColour ((bits >> i) & 1 ? on : ghost);
        g.fillPath (seg[i]);
    }
}

void drawSevenSeg (juce::Graphics& g, const juce::String& value, juce::Rectangle<float> area, int cells, juce::Colour on, juce::Colour ghost)
{
    // Split into digit glyphs and the glyph after which a decimal point sits.
    juce::Array<juce::juce_wchar> glyphs;
    int dotAfter = -1;
    for (auto ch : value)
    {
        if (ch == '.') dotAfter = glyphs.size() - 1;
        else glyphs.add (ch);
    }
    const int lead = juce::jmax (0, cells - glyphs.size());
    const float h = area.getHeight(), dw = h * 0.50f, pitch = dw * 1.34f;
    const float total = pitch * float (cells) - (pitch - dw);
    float x = area.getCentreX() - total * 0.5f;
    for (int i = 0; i < cells; ++i, x += pitch)
    {
        const int gi = i - lead;
        drawDigit (g, gi >= 0 ? segmentsFor (glyphs[gi]) : 0, { x, area.getY(), dw, h }, on, ghost);
        if (gi >= 0 && gi == dotAfter)
        {
            const float d = dw * 0.19f;
            g.setColour (on);
            g.fillRect (x + dw + (pitch - dw) * 0.5f - d * 0.5f, area.getBottom() - d, d, d);
        }
    }
}

juce::Path bolt (juce::Rectangle<float> area)
{
    juce::Path p;
    p.startNewSubPath (0.62f, 0.0f); p.lineTo (0.10f, 0.58f); p.lineTo (0.46f, 0.58f);
    p.lineTo (0.34f, 1.0f); p.lineTo (0.90f, 0.38f); p.lineTo (0.54f, 0.38f); p.closeSubPath();
    p.applyTransform (p.getTransformToScaleToFit (area, true));
    return p;
}
} // namespace theme
