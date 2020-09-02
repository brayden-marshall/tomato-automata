#include <SDL2/SDL.h>
#include <iostream>
#include <chrono>
using namespace std::chrono;

using std::cout;
using std::endl;

#include "../imgui/imgui.h"
#include "../imgui/imgui_sdl.h"

#include "app.h"

#define WINDOW_SIZE 900

int main() {
    // initialize SDL
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window* window = SDL_CreateWindow(
        "Tomato Automata", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_SIZE, WINDOW_SIZE, 0
    );
    SDL_Renderer* renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_ACCELERATED
    );

    // enable VSync
    SDL_GL_SetSwapInterval(1);

    ImGui::CreateContext();
    ImGuiSDL::Initialize(renderer, WINDOW_SIZE, WINDOW_SIZE);

    // initialize the App with the SDL renderer
    App* app = new App(renderer);

    auto start_time = high_resolution_clock::now();

    bool running = true;
    while (running) {
        auto now = high_resolution_clock::now();
        auto delta_time_ms = duration_cast<milliseconds>(now - start_time);
        start_time = now;

        ImGuiIO& io = ImGui::GetIO();

        int wheel = 0;

        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT: {
                    running = false;
                } break;
                case SDL_WINDOWEVENT: {
                    if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                        io.DisplaySize.x = static_cast<float>(e.window.data1);
                        io.DisplaySize.y = static_cast<float>(e.window.data2);
                    }
                } break;
                case SDL_MOUSEWHEEL: {
                    wheel = e.wheel.y;
                } break;
                case SDL_KEYDOWN: {
                    switch (e.key.keysym.sym) {
                        case SDLK_SPACE:
                            app->paused = !app->paused;
                            break;
                        case SDLK_ESCAPE:
                            running = false;
                            break;
                    }
                } break;
            }
        }

        int mouseX, mouseY;
        const int buttons = SDL_GetMouseState(&mouseX, &mouseY);

        // Setup low-level inputs (e.g. on Win32, GetKeyboardState(), or write to those fields from your Windows message loop handlers, etc.)
        
        //io.DeltaTime = delta_time.count();
        io.MousePos = ImVec2(static_cast<float>(mouseX), static_cast<float>(mouseY));
        io.MouseDown[0] = buttons & SDL_BUTTON(SDL_BUTTON_LEFT);
        io.MouseDown[1] = buttons & SDL_BUTTON(SDL_BUTTON_RIGHT);
        io.MouseWheel = static_cast<float>(wheel);

        app->render(io);
        app->update(io, delta_time_ms.count());
    }

    ImGuiSDL::Deinitialize();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    ImGui::DestroyContext();

    return 0;
}
