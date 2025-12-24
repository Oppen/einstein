#include <SDL.h>
#include "screen.h"
#include "SDL_render.h"
#include "SDL_surface.h"
#include "SDL_video.h"
#include "exceptions.h"
#include "unicode.h"


Screen::Screen()
{
    SDL_CreateWindowAndRenderer(getWidth(), getHeight(), SDL_WINDOW_FULLSCREEN_DESKTOP, &window, &renderer);
    screen = SDL_CreateRGBSurface(0, getWidth(), getHeight(), 32,
                                            0x00FF0000,
                                            0x0000FF00,
                                            0x000000FF,
                                            0xFF000000);
    texture = SDL_CreateTexture(renderer,
                                                SDL_PIXELFORMAT_ARGB8888,
                                                SDL_TEXTUREACCESS_STREAMING,
                                                getWidth(), getHeight());
    SDL_RenderSetLogicalSize(renderer, getWidth(), getHeight());
    mouseImage = NULL;
    mouseSave = NULL;
    mouseVisible = false;
    maxRegionsList = 0;
}

Screen::~Screen()
{
    SDL_SetCursor(cursor);
    if (mouseImage) SDL_FreeSurface(mouseImage);
    if (mouseSave) SDL_FreeSurface(mouseSave);
}


int Screen::getWidth() const
{
    return 800;
}


int Screen::getHeight() const
{
    return 600;
}

void Screen::setFullScreen(bool enable) {
    SDL_SetWindowFullscreen(window, (int)enable * SDL_WINDOW_FULLSCREEN_DESKTOP);
}

void Screen::centerMouse()
{
    SDL_WarpMouseGlobal(getWidth() / 2, getHeight() / 2);
}

void Screen::setMouseImage(SDL_Surface *image)
{
	/*
    if (mouseImage) {
        SDL_FreeSurface(mouseImage);
        mouseImage = NULL;
    }
    if (mouseSave) {
        SDL_FreeSurface(mouseSave);
        mouseSave = NULL;
    }

    if (! image) return;

    mouseImage = SDL_DisplayFormat(image);
    if (! mouseImage)
        throw Exception(L"Error creating surface");
    //mouseSave = SDL_DisplayFormat(image);
    mouseSave = SDL_CreateRGBSurface(0,
            image->w, image->h, screen->format->BitsPerPixel,
            screen->format->Rmask, screen->format->Gmask,
            screen->format->Bmask, screen->format->Amask);
    if (! mouseSave) {
        SDL_FreeSurface(mouseImage);
        throw Exception(L"Error creating buffer surface");
    }
    SDL_SetColorKey(mouseImage, SDL_TRUE,
            SDL_MapRGB(mouseImage->format, 0, 0, 0));
	    */
}


void Screen::hideMouse()
{
    /*
    if (! mouseVisible)
        return;

    if (! niceCursor) {
        mouseVisible = false;
        return;
    }

    if (mouseSave) {
        SDL_Rect src = { 0, 0, mouseSave->w, mouseSave->h };
        SDL_Rect dst = { saveX, saveY, mouseSave->w, mouseSave->h };
        if (src.w > 0) {
            SDL_BlitSurface(mouseSave, &src, screen, &dst);
            addRegionToUpdate(dst.x, dst.y, dst.w, dst.h);
        }
    }
    mouseVisible = false;
    */
}

void Screen::showMouse()
{
    if (mouseVisible)
        return;

    if (! niceCursor) {
        mouseVisible = true;
        return;
    }

    if (mouseImage && mouseSave) {
        int x, y;
        SDL_GetMouseState(&x, &y);
        saveX = x;
        saveY = y;
        SDL_Rect src = { 0, 0, mouseSave->w, mouseSave->h };
        SDL_Rect dst = { x, y, mouseImage->w, mouseImage->h };
        if (src.w > 0) {
            SDL_BlitSurface(screen, &dst, mouseSave, &src);
            SDL_BlitSurface(mouseImage, &src, screen, &dst);
        }
    }
    mouseVisible = true;
}

void Screen::updateMouse()
{
    hideMouse();
    showMouse();
}

void Screen::flush()
{
    Uint32 format;
    int access, w, h;
    SDL_UpdateTexture(texture, NULL, screen->pixels, screen->pitch);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void Screen::setPixel(int x, int y, int r, int g, int b)
{
    SDL_LockSurface(screen);
    int bpp = screen->format->BytesPerPixel;
    Uint32 pixel = SDL_MapRGB(screen->format, r, g, b);
    /* Here p is the address to the pixel we want to set */
    Uint8 *p = (Uint8*)screen->pixels + y * screen->pitch + x * bpp;

    switch(bpp) {
        case 1:
            *p = pixel;
            break;

        case 2:
            *(Uint16 *)p = pixel;
            break;

        case 3:
            if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                p[0] = (pixel >> 16) & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = pixel & 0xff;
            } else {
                p[0] = pixel & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = (pixel >> 16) & 0xff;
            }
            break;

        case 4:
            *(Uint32 *)p = pixel;
            break;
    }
    SDL_UnlockSurface(screen);
}


void Screen::draw(int x, int y, SDL_Surface *tile)
{
    SDL_Rect src = { 0, 0, tile->w, tile->h };
    SDL_Rect dst = { x, y, tile->w, tile->h };
    SDL_BlitSurface(tile, &src, screen, &dst);
}

void Screen::setCursor(bool nice)
{
    if (nice == niceCursor)
        return;

    bool oldVisible = mouseVisible;
    if (mouseVisible)
        hideMouse();
    niceCursor = nice;

    if (niceCursor)
        SDL_SetCursor(emptyCursor);
    else
        SDL_SetCursor(cursor);

    if (oldVisible)
        showMouse();
}

void Screen::initCursors()
{
    cursor = SDL_GetCursor();
    Uint8 t = 0;
    emptyCursor = SDL_CreateCursor(&t, &t, 8, 1, 0, 0);
}

void Screen::doneCursors()
{
    if (niceCursor)
        SDL_SetCursor(cursor);
    SDL_FreeCursor(emptyCursor);
}

SDL_Surface* Screen::createSubimage(int x, int y, int width, int height)
{
    SDL_Surface *s = SDL_CreateRGBSurface(0,
            width, height, screen->format->BitsPerPixel,
            screen->format->Rmask, screen->format->Gmask,
            screen->format->Bmask, screen->format->Amask);
    if (! s)
        throw Exception(L"Error creating buffer surface");
    SDL_Rect src = { x, y, width, height };
    SDL_Rect dst = { 0, 0, width, height };
    SDL_BlitSurface(screen, &src, s, &dst);
    return s;
}
