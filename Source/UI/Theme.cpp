#include "Theme.h"
#include "BinaryData.h"

namespace theme
{
void setAccent (float hue, float saturation, float brightness)
{
    accentHue = hue; accentSat = saturation; accentBright = brightness;
    violet       = recolour (juce::Colour (base::violet));
    violetDeep   = recolour (juce::Colour (base::violetDeep));
    pink         = recolour (juce::Colour (base::pink));
    coral        = recolour (juce::Colour (base::coral));
    blue         = recolour (juce::Colour (base::blue));
    cyan         = recolour (juce::Colour (base::cyan));
    lavender     = recolour (juce::Colour (base::lavender));
    rose         = recolour (juce::Colour (base::rose));
    lilac        = recolour (juce::Colour (base::lilac));
    amber        = recolour (juce::Colour (base::amber));
    warn         = recolour (juce::Colour (base::warn));
    keyOnTop     = recolour (juce::Colour (base::keyOnTop));
    keyOnBottom  = recolour (juce::Colour (base::keyOnBottom));
    blackOnTop   = recolour (juce::Colour (base::blackOnTop));
    blackOnBottom = recolour (juce::Colour (base::blackOnBottom));
}

void setGradient (bool on, juce::Colour a, juce::Colour b)
{
    gradientOn = on;
    gradA = a; gradB = b;
    if (! on) { setAccent (accentHue, accentSat, accentBright); return; } // restore the hue-rotation palette
    violet        = a.interpolatedWith (b, 0.5f);
    violetDeep    = violet.withMultipliedBrightness (0.82f).withMultipliedSaturation (1.1f);
    pink          = a;
    coral         = a.interpolatedWith (b, 0.22f);
    blue          = b;
    cyan          = a.interpolatedWith (b, 0.78f);
    lavender      = violet.brighter (0.35f);
    rose          = a.interpolatedWith (b, 0.4f).withMultipliedSaturation (0.85f);
    lilac         = violet.brighter (0.55f);
    amber         = b.brighter (0.2f);
    warn          = a.brighter (0.2f);
    keyOnTop      = violet.withMultipliedBrightness (0.9f);
    keyOnBottom   = lavender;
    blackOnTop    = violetDeep.withMultipliedBrightness (0.6f);
    blackOnBottom = violetDeep;
}

void setTheme (bool light)
{
    lightMode = light;
    if (light)
    {
        bg0 = juce::Colour (0xffcdd0d6); bg1 = juce::Colour (0xffeceef2); bg2 = juce::Colour (0xffe4e6eb);
        bg3 = juce::Colour (0xffd4d7de); line = juce::Colour (0xffbfc2cb);
        text = juce::Colour (0xff1b1c20); textDim = juce::Colour (0xff5c5e68); textFaint = juce::Colour (0xff9a9ca6);
        header = juce::Colour (0xffe4e6eb); headerBox = juce::Colour (0xffeceef2); stage = juce::Colour (0xffe8eaef);
        highlight = juce::Colour (0xfff4f6fa); panel = juce::Colour (0xffe0e3e9); knobBody = juce::Colour (0xffe8eaee);
        menuBg = juce::Colour (0xfff5f6f9); menuBg2 = juce::Colour (0xffe4e6ea); menuSel = juce::Colour (0xffdcdee5);
        menuTab = juce::Colour (0xffe9ebf0); well = juce::Colour (0xffd8dae1);
        keyOffTop = juce::Colour (0xffc3c5cc); keyOffBottom = juce::Colour (0xffb0b2ba); blackOff = juce::Colour (0xff6c6e76);
        labelWhite = juce::Colour (0xff888a92); labelBlack = juce::Colour (0xff4a4c54); greyText = juce::Colour (0xff86888f);
    }
    else
    {
        bg0 = juce::Colour (0xff0d0e10); bg1 = juce::Colour (0xff121316); bg2 = juce::Colour (0xff17181c);
        bg3 = juce::Colour (0xff24262d); line = juce::Colour (0xff2e3038);
        text = juce::Colour (0xfff2f1f7); textDim = juce::Colour (0xffa09eac); textFaint = juce::Colour (0xff5f5d6b);
        header = juce::Colour (0xff17181c); headerBox = juce::Colour (0xff121316); stage = juce::Colour (0xff121316);
        highlight = juce::Colour (0xff1e2026); panel = juce::Colour (0xff1e2026); knobBody = juce::Colour (0xff18191c);
        menuBg = juce::Colour (0xff1c1c1e); menuBg2 = juce::Colour (0xff121213); menuSel = juce::Colour (0xff252528);
        menuTab = juce::Colour (0xff232327); well = juce::Colour (0xff141416);
        keyOffTop = juce::Colour (0xff8b8b8e); keyOffBottom = juce::Colour (0xff7c7c80); blackOff = juce::Colour (0xff262628);
        labelWhite = juce::Colour (0xff5d5d60); labelBlack = juce::Colour (0xff19191a); greyText = juce::Colour (0xff7d808a);
    }
}

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

void drawSevenSeg (juce::Graphics& g, const juce::String& value, juce::Rectangle<float> area, int cells, juce::Colour on, juce::Colour ghost, bool centre)
{
    // Split into digit glyphs and the glyph after which a decimal point sits.
    juce::Array<juce::juce_wchar> glyphs;
    int dotAfter = -1;
    for (auto ch : value)
    {
        if (ch == '.') dotAfter = glyphs.size() - 1;
        else glyphs.add (ch);
    }
    // Centred: draw just the value's digits, centred (no ghost padding). Otherwise: right-aligned in `cells`.
    const int n = centre ? glyphs.size() : cells;
    const int lead = centre ? 0 : juce::jmax (0, cells - glyphs.size());
    const float h = area.getHeight(), dw = h * 0.50f, pitch = dw * 1.34f;
    const float total = pitch * float (n) - (pitch - dw);
    float x = area.getCentreX() - total * 0.5f;
    for (int i = 0; i < n; ++i, x += pitch)
    {
        const int gi = i - lead;
        drawDigit (g, (gi >= 0 && gi < glyphs.size()) ? segmentsFor (glyphs[gi]) : 0, { x, area.getY(), dw, h }, on, ghost);
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
