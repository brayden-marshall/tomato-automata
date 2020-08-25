#include <SDL2/SDL.h>

#include "../imgui/imgui.h"
#include "../imgui/imgui_sdl.h"

#include "app.h"

int main()
{
	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_Window* window = SDL_CreateWindow("SDL2 ImGui Renderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, 0);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	ImGui::CreateContext();
	ImGuiSDL::Initialize(renderer, 800, 600);

        // initialize the App with the SDL renderer
        App* app = new App(renderer);

	bool run = true;
	while (run)
	{
		ImGuiIO& io = ImGui::GetIO();

		int wheel = 0;

		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT) run = false;
			else if (e.type == SDL_WINDOWEVENT)
			{
				if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
				{
					io.DisplaySize.x = static_cast<float>(e.window.data1);
					io.DisplaySize.y = static_cast<float>(e.window.data2);
				}
			}
			else if (e.type == SDL_MOUSEWHEEL)
			{
				wheel = e.wheel.y;
			}
		}

		int mouseX, mouseY;
		const int buttons = SDL_GetMouseState(&mouseX, &mouseY);

		// Setup low-level inputs (e.g. on Win32, GetKeyboardState(), or write to those fields from your Windows message loop handlers, etc.)
		
		io.DeltaTime = 1.0f / 60.0f;
		io.MousePos = ImVec2(static_cast<float>(mouseX), static_cast<float>(mouseY));
		io.MouseDown[0] = buttons & SDL_BUTTON(SDL_BUTTON_LEFT);
		io.MouseDown[1] = buttons & SDL_BUTTON(SDL_BUTTON_RIGHT);
		io.MouseWheel = static_cast<float>(wheel);

                app->render();
	}

	ImGuiSDL::Deinitialize();

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	ImGui::DestroyContext();

	return 0;
}
