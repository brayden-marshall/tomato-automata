#ifndef APP_H
#define APP_H

#include <SDL2/SDL.h>

#include "imgui.h"

class App {
    public:
        // members
        SDL_Renderer* renderer;

        // constructor
        App(SDL_Renderer* r);

        // functions
        void render();
        void update(const ImGuiIO& io);
};

#endif
