#ifndef __SCREEN_H__
#define __SCREEN_H__


#include "SDL.h"
#include "SDL_render.h"
#include "SDL_video.h"
#include <cassert>
#include <vector>
#include <list>


class Screen
{
    private:
        SDL_Surface *screen;
        SDL_Texture *texture;
        SDL_Renderer *renderer;
        SDL_Window *window;
        bool fullScreen;
        SDL_Surface *mouseImage;
        SDL_Surface *mouseSave;
        bool mouseVisible;
        int maxRegionsList;
        int saveX, saveY;
        bool niceCursor;
        SDL_Cursor *cursor, *emptyCursor;

    public:
        Screen();
        ~Screen();

    public:
        int getWidth() const;
        int getHeight() const;
        void setFullScreen(bool enable);
        void centerMouse();
        void setMouseImage(SDL_Surface *image);
        void hideMouse();
        void showMouse();
        void updateMouse();
        void flush();
        void setPixel(int x, int y, int r, int g, int b);
        SDL_Surface* getSurface() { return screen; };
        void draw(int x, int y, SDL_Surface *surface);
        void setCursor(bool nice);
        void initCursors();
        void doneCursors();
        SDL_Surface* createSubimage(int x, int y, int width, int height);
};


#endif
