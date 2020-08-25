#include "app.h"

#include <SDL2/SDL.h>
#include "imgui.h"
#include "imgui_sdl.h"

App::App(SDL_Renderer* r): renderer(r)
    {}

void App::render() {
    // render main drawing
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_Rect r;
    r.x = 50;
    r.y = 50;
    r.w = 50;
    r.h = 50;

    SDL_RenderFillRect(renderer, &r);

    // render ImGui
    ImGui::NewFrame();

    ImGui::Begin("Image");
    ImGui::Dummy(ImVec2(100,100));
    ImGui::End();

    ImGui::Render();
    ImGuiSDL::Render(ImGui::GetDrawData());

    SDL_RenderPresent(renderer);
}

void App::update(const ImGuiIO& io) {
}
