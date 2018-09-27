//
// Created by fotyev on 2018-07-07.
//

#ifndef ZENGINE_TEXTRENDER_HPP
#define ZENGINE_TEXTRENDER_HPP

#include "stb/stb_truetype.h"
#include <vector>

#include <string_view>
#include <cmath>

#include "math.hpp"

#include "util/movable.hpp"

class font
{
    stbtt_fontinfo info;

    std::vector<unsigned char> ttf; // ttf buffer

    NONCOPYABLE(font) // info holds pointer to ttf buffer

private:
    static std::vector<unsigned char> read_file(const char * filename);

public:

    // idx - font index in multifont file, 0 - first
    font(const char * filename, int idx = 0)
    {
        ttf = read_file(filename);
        if(!stbtt_InitFont(&info, ttf.data(), stbtt_GetFontOffsetForIndex(ttf.data(), idx)))
            throw std::runtime_error{"cant init font: " + std::string{filename}};
    }


    qvm::vec2 prepare_pos(float pixelh) const
    {
        float scale = stbtt_ScaleForPixelHeight(&info, pixelh);
        int ascent, descent, lineGap;
        stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);
        return qvm::vec2{ 0, ascent * scale };
    };

    float get_line_skip(float pixelh) const
    {
        int ascent, descent, lineGap;
        stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);
        return (ascent - descent + lineGap) * stbtt_ScaleForPixelHeight(&info, pixelh);
    }

    /**
     * Calls f(codepoint, pos, scale) for each glyph in string. It will take care of left side bearing if x == 0,
     * baseline if y == 0. It will advance to new line (y=0) on '\n'.
     *
     */
    template <typename Callable>
    qvm::vec2 iterate_glyphs(std::string_view string, float pixelh, const Callable& f, qvm::vec2 pos = {0, 0}) const;

    /**
     * Render multiline string to output buffer. Buffer should be zeroed.
     * @param string
     * @param pixelh font vertical height in pixels
     * @param output pointer to buffer of length out_width * out_height
     * @param out_width buffer width in pixels, should be large enough to hold string
     * @param out_height buffer height in pixels, should be large enough to hold string
     */
    void render(std::string_view string, float pixelh, unsigned char * output, unsigned out_width, unsigned out_height) const;

    qvm::vec2 get_string_size(std::string_view string, float pixelh) const;


    static auto make_buffer(qvm::vec2 size, unsigned int components = 1)
    {
        int w = (int)ceil(X(size)) /*+ 1*/;
        int h = (int)ceil(Y(size)) /*+ 1*/;
        assert(w > 0);
        assert(h > 0);
        struct {
            std::vector<unsigned char> buffer;
            unsigned width, height;
        } buf;
        buf.buffer.resize(w * h * components, 0);
        buf.width = w; buf.height = h;
        return buf;
    }

    /**
     * Make buffer and render string to it.
     * @param string
     * @param pixelh font height in pixels
     * @return anonymous struct: {.buffer, .width, .height}
     */
    auto render(std::string_view string, float pixelh) const
    {
        using namespace qvm;
        vec2 size = get_string_size(string, pixelh);
        auto buf = make_buffer(size);
        render(string, pixelh, &buf.buffer[0], buf.width, buf.height);
        return buf;
    }

    /**
     * Calculate pixel height to place [lines] lines in out_height buffer
     * @param lines
     * @param out_height
     * @return pixelh
     */
    float get_pixel_height(int lines, float out_height) const
    {
        int ascent, descent, lineGap;
        stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);
        // TODO: check
        return out_height /
                (lines + (lines - 1) * float(lineGap) / (ascent - descent));
    }

};

#include "texture.hpp"

// creates alpha texture (opengl maps color to 0,0,0)
texture render_text(const font& font, std::string_view string, float pixelh = 36);



//
texture render_text_box(const font& font, std::string_view string, qvm::vec2 size, int max_lines, int max_words = -1, std::string_view delimiters = " \t\n");


#endif //ZENGINE_TEXTRENDER_HPP
