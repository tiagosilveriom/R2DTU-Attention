#include <SDL2/SDL2_gfxPrimitives.h>
#include <common/Debug.h>
#include "RenderContext.h"
#include <unistd.h>

Texture::Texture(RenderContext &context, Format format, Access access, int width, int height) {
    sdl_texture_ = SDL_CreateTexture(context.renderer_, format, access, width, height);
}

void Texture::upload(uint8_t* data, size_t length) {
    uint8_t* pixels = nullptr;
    int pitch = 0;
    SDL_LockTexture(sdl_texture_, nullptr, (void**)&pixels, &pitch);
    memcpy(pixels, data, length);
    SDL_UnlockTexture(sdl_texture_);
}

RenderContext::RenderContext(SDL_Renderer *renderer, Vec2i offset, Vec2i size)
    : renderer_(renderer), of_(offset) {
    texture_ = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_BGR24, SDL_TEXTUREACCESS_TARGET, size.x, size.y);
}

void RenderContext::clear() {
    auto c = Color::background();
    SDL_SetRenderDrawColor(renderer_, c.x, c.y, c.z, 0x0);
    SDL_RenderClear(renderer_);
}

void RenderContext::prerender() {
    SDL_SetRenderTarget(renderer_, texture_);
    clear();
}

void RenderContext::postrender(Vec2i pos, Vec2i size) {
    SDL_Rect src = {0, 0 , size.x, size.y};
    SDL_Rect dst = {pos.x, pos.y, size.x, size.y};
    SDL_SetRenderTarget(renderer_, nullptr);
    SDL_RenderCopy(renderer_, texture_, &src, &dst);
}

void RenderContext::draw_box(Vec2i p, Vec2i s, Color c, int t) {
    auto x1 = p.x+of_.x;
    auto y1 = p.y+of_.y;
    auto x2 = p.x+s.x+of_.x;
    auto y2 = p.y+s.y+of_.y;
    thickLineRGBA(renderer_, x1, y1, x1, y2, t, c.x, c.y, c.z, 0xFF);
    thickLineRGBA(renderer_, x1, y1, x2, y1, t, c.x, c.y, c.z, 0xFF);
    thickLineRGBA(renderer_, x2, y1, x2, y2, t, c.x, c.y, c.z, 0xFF);
    thickLineRGBA(renderer_, x1, y2, x2, y2, t, c.x, c.y, c.z, 0xFF);
}

void RenderContext::fill_box(Vec2i p, Vec2i s, Color c) {
    auto x1 = p.x+of_.x;
    auto y1 = p.y+of_.y;
    auto x2 = p.x+s.x+of_.x;
    auto y2 = p.y+s.y+of_.y;
    boxRGBA(renderer_, x1, y1, x2, y2, c.x, c.y, c.z, 0xFF);
}

void RenderContext::draw_line(Vec2i p1, Vec2i p2, Color c, int t) {
    auto x1 = p1.x+of_.x;
    auto y1 = p1.y+of_.y;
    auto x2 = p2.x+of_.x;
    auto y2 = p2.y+of_.y;
    thickLineRGBA(renderer_, x1, y1, x2, y2, t, c.x, c.y, c.z, 0xFF);
}

TTF_Font* RenderContext::get_font(int size, int32_t outline_size) {
    auto& cache = outline_size > 0 ? outlined_font_sizes_ : font_sizes_;
    auto search = cache.find(size);
    if (search == cache.end()) {

        auto font = TTF_OpenFont("../resources/B612-Regular.ttf", size);
        ASSERT(font, "Failed to load font!\n");
        if (outline_size > 0) {
            TTF_SetFontOutline(font, outline_size);
        }
        cache.insert({size, font});
        return font;
    }
    return search->second;
}

void RenderContext::draw_text(const std::string& str, Vec2i p, TextAlign align, int font_size, Color c, int32_t outline_size, Color outline_c) {
    SDL_Color color = {(uint8_t) c.x, (uint8_t) c.y, (uint8_t) c.z};
    SDL_Surface* text_surface = TTF_RenderUTF8_Blended(get_font(font_size), str.c_str(), color);
    if (outline_size > 0) {
        SDL_Color outline_color = {(uint8_t) outline_c.x, (uint8_t) outline_c.y, (uint8_t) outline_c.z};
        SDL_Surface* outline = TTF_RenderUTF8_Blended(get_font(font_size, outline_size), str.c_str(), outline_color);
        SDL_Rect rect = {outline_size, outline_size, text_surface->w, text_surface->h};
        SDL_SetSurfaceBlendMode(text_surface, SDL_BLENDMODE_BLEND);
        SDL_BlitSurface(text_surface, nullptr, outline, &rect);
        SDL_FreeSurface(text_surface);
        text_surface = outline;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, text_surface);
    SDL_FreeSurface(text_surface);
    SDL_Rect dst = {p.x + of_.x, p.y + of_.y, 0, 0,};
    SDL_QueryTexture(texture, nullptr, nullptr, &dst.w, &dst.h);
    switch (align) {
        case TextAlign::LEFT: {
            break;
        }
        case TextAlign::CENTER: {
            dst.x -= dst.w / 2;
            dst.y -= dst.h / 2;
            break;
        }
        case TextAlign::RIGHT: {
            dst.x -= dst.w;
            break;
        }
    }
    SDL_RenderCopy(renderer_, texture, nullptr, &dst);
    SDL_DestroyTexture(texture);
}

std::unordered_map<int, TTF_Font*> RenderContext::font_sizes_;
std::unordered_map<int, TTF_Font*> RenderContext::outlined_font_sizes_;


void RenderContext::draw_texture(const Texture* const texture, Vec2i pos) {
    int w, h;
    SDL_QueryTexture(texture->sdl_texture_, nullptr, nullptr, &w, &h);
    SDL_Rect dst = {pos.x + of_.x, pos.y + of_.y, w, h};
    SDL_RenderCopy(renderer_, texture->sdl_texture_, nullptr, &dst);
}
