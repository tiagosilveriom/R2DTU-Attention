#pragma once

#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <common/Vectors.h>
#include <common/Color.h>
#include <unordered_map>

enum class TextAlign {
    CENTER,
    LEFT,
    RIGHT
};

class RenderContext;

class Texture {
public:

    enum Format {
        RGB24 = SDL_PIXELFORMAT_RGB24,
        BGR24 = SDL_PIXELFORMAT_BGR24,
        RGBA32 = SDL_PIXELFORMAT_RGBA32
    };

    enum Access {
        STATIC = SDL_TEXTUREACCESS_STATIC,
        STREAMING = SDL_TEXTUREACCESS_STREAMING
    };

    Texture(RenderContext& context, Format format, Access access, int width, int height);

    void upload(uint8_t* pixels, size_t length);

    friend class RenderContext;

private:
    SDL_Texture* sdl_texture_;
};

class RenderContext {
public:

    RenderContext(SDL_Renderer* renderer, Vec2i offset, Vec2i size);

    void prerender();

    void postrender(Vec2i pos, Vec2i size);

    void clear();

    void draw_box(Vec2i pos, Vec2i size, Color color, int thickness);
    void fill_box(Vec2i pos, Vec2i size, Color color);

    void draw_line(Vec2i p1, Vec2i p2, Color color, int thickness);

    void draw_text(const std::string& str, Vec2i pos, TextAlign align, int font_size, Color color, int32_t outline_size=0, Color outline_color={});

    void draw_texture(const Texture* const texture, Vec2i pos);

    friend class Texture;

private:

    TTF_Font* get_font(int size, int32_t outline_size=0);


    SDL_Renderer* renderer_;
    SDL_Texture* texture_;
    Vec2i of_;
    static std::unordered_map<int, TTF_Font*> font_sizes_;
    static std::unordered_map<int, TTF_Font*> outlined_font_sizes_;
};

