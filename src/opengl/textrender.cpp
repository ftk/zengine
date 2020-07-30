//
// Created by fotyev on 2018-07-16.
//
#define STB_RECT_PACK_IMPLEMENTATION
#include "stb/stb_rect_pack.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "textrender.hpp"
#include <fstream>

#define CUTE_UTF_IMPLEMENTATION
#include "cute_headers/cute_utf.h"

#include "util/assert.hpp"
#include "opengl.hpp"

std::vector<unsigned char> font::read_file(const char * filename)
{
    std::ifstream t(filename, std::ios::binary);

    if(!t)
        throw std::runtime_error{"cant open file: " + std::string{filename}};

    t.seekg(0, std::ios::end);
    std::vector<unsigned char> buf;
    buf.reserve(t.tellg());
    t.seekg(0, std::ios::beg);

    buf.assign((std::istreambuf_iterator<char>(t)),
               std::istreambuf_iterator<char>());
    return buf;
}

template <typename Callable>
qvm::vec2 font::iterate_glyphs(std::string_view string, float pixelh, const Callable& f, qvm::vec2 pos) const
{
    using namespace qvm;

    // move to baseline
    if(Y(pos) == 0)
        Y(pos) = Y(prepare_pos(pixelh));

    float scale = stbtt_ScaleForPixelHeight(&info, pixelh);
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);

    const char * text = string.data();
    const char * text_end = string.data() + string.size();
    int codepoint;
    text = cu_decode8(text, &codepoint);
    while(true)
    {
        int advance, lsb;
        stbtt_GetCodepointHMetrics(&info, codepoint, &advance, &lsb);
        if(X(pos) == 0)
            X(pos) = lsb * scale;

        f(codepoint, pos, scale);


        X(pos) += advance * scale;

        if(text >= text_end)
            break;
        int next;
        text = cu_decode8(text, &next);

        bool nl = false;
        while(next == '\n')
        {
            X(pos) = 0;
            Y(pos) += scale * (ascent - descent + lineGap);

            f(next, pos, scale);

            if(text >= text_end)
                break;
            text = cu_decode8(text, &next);

            nl = true;
        }
        if(!nl)
            X(pos) += scale * stbtt_GetCodepointKernAdvance(&info, codepoint, next);
        codepoint = next;
    }
    return pos;
}

void font::render(std::string_view string, float pixelh, unsigned char * output, unsigned out_width,
                  unsigned out_height) const
{

    iterate_glyphs(string, pixelh, [output, out_width, out_height, this] (int code, qvm::vec2 pos, float scale) {
        using namespace qvm;
        if(code == '\n' /*|| code == 0*/)
            return;
        const auto get_subpixel = [](float x) { return x - int(x); };

        int x0,y0,x1,y1;

        stbtt_GetCodepointBitmapBoxSubpixel(&info, code,
                                            scale,scale,
                                            get_subpixel(X(pos)), get_subpixel(Y(pos)),
                                            &x0,&y0,&x1,&y1);

        int x = int(X(pos));
        int y = int(Y(pos));

        // check if can place in buffer
        assume(x + x0 >= 0);
        if(x + x0 >= (int)out_width)
            x0 = (int)out_width - x - 1;
        assume(y + y0 >= 0);
        if(y + y0 >= (int)out_height)
            y0 = (int)out_height - y - 1;

        if(x + x1 >= (int)out_width)
            x1 = (int)out_width - x - 1;
        assume(x + x1 < (int)out_width);
        if(y + y1 >= (int)out_height)
            y1 = (int)out_height - y - 1;
        assume(y + y1 < (int)out_height);

        stbtt_MakeCodepointBitmapSubpixel(&info,
                                          &output[(y + y0) * out_width + x + x0],
                                          x1-x0, y1-y0,
                                          out_width,
                                          scale,scale,
                                          get_subpixel(X(pos)), get_subpixel(Y(pos)),
                                          code);
        // note that this stomps the old data, so where character boxes overlap (e.g. 'lj') it's wrong
        // because this API is really for baking character bitmaps into textures. if you want to render
        // a sequence of characters, you really need to render each bitmap to a temp buffer, then
        // "alpha blend" that into the working buffer
    });
}

qvm::vec2 font::get_string_size(std::string_view string, float pixelh) const
{
    float scale = stbtt_ScaleForPixelHeight(&info, pixelh);
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);

    using namespace qvm;
    vec2 size {0, 0};
    iterate_glyphs(string, pixelh, [&size, this] (int code, qvm::vec2 pos, float scale) {
        int advance, lsb;
        stbtt_GetCodepointHMetrics(&info, code, &advance, &lsb);

        X(pos) += scale * advance;

        X(size) = std::max(X(size), X(pos));
        Y(size) = std::max(Y(size), Y(pos));
    });
    Y(size) -= descent * scale;
    X(size) = ceil(X(size));
    Y(size) = ceil(Y(size));
    return size;
}

texture make_text(const font& font, std::string_view string, float pixelh)
{
    auto buf = font.render(string, pixelh);
    auto tex = texture{buf.buffer.data(), (unsigned) buf.buffer.size(), buf.width, buf.height, GL_ALPHA};
    gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    return tex;
}

texture make_text_box(const font& font, std::string_view string, qvm::vec2 size, int max_lines, int max_words,
                      std::string_view delimiters)
{
    using namespace qvm;
    {
        const float pixelh = font.get_pixel_height(max_lines, Y(size));

        std::string newstring;
        vec2 curpos{0,0};


        std::size_t pos = 0, prev_pos;
        --pos;
        int words = -1;
        do
        {
            words++;
            prev_pos = pos;
            pos = string.find_first_of(delimiters, prev_pos + 1);

            bool newline = false;
            if(!newstring.empty() && string[prev_pos] == '\n')
            {
                newstring.pop_back();
                //prev_pos++;
                newline = true;
            }

            auto substr = string.substr(prev_pos + 1, (pos == std::string_view::npos) ? pos : (pos - prev_pos));

            float substr_len = X(font.get_string_size(substr, pixelh));
            if(!substr.empty())
                X(curpos) += substr_len;
            if((X(curpos) > X(size) || newline) && --max_lines)
            {
                // wrap
                newstring += '\n';
                X(curpos) = substr_len;
                Y(curpos) += font.get_line_skip(pixelh);
                if(Y(curpos) >= Y(size))
                {
                    newstring.pop_back();
                    break;
                }
            }
            if(words == max_words)
                break;
            newstring += substr;
        }
        while(pos != std::string_view::npos);

        auto buf = font::make_buffer(size);
        font.render(newstring, pixelh, buf.buffer.data(), buf.width, buf.height);


        texture tex = texture{buf.buffer.data(), (unsigned)buf.buffer.size(), buf.width, buf.height, GL_ALPHA};
        gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        return tex;
    }
}
