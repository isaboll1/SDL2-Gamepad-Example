// dear imgui (https://github.com/ocornut/imgui) Render Binding for SDL2.
// (Info: SDL2 is a cross-platform general purpose library for handling windows, inputs, graphics context creation, etc. These are bindings for the renderer it provides.)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'SDL_Texture *' SDL_Renderer texture identifier as void*/ImTextureID. Read the imgui FAQ about ImTextureID! (https://github.com/ocornut/imgui/blob/master/docs/FAQ.md#q-how-can-i-display-an-image-what-is-imtextureid-how-does-it-work)
// Missing features:
//  [ ] Renderer: Support for large meshes (64k+ vertices) with 16-bit indices.

#pragma once
//INCLUDES. (you can change these if need be)
#include "imgui.h"
#include <SDL2/SDL.h>
// _______________________________
struct ImDrawData;
struct SDL_Renderer;


// Call this to initialize the SDL renderer device that is internally used by the renderer.
IMGUI_IMPL_API void ImGui_ImplSDLRenderer_Init(SDL_Renderer* renderer);
// Call this before destroying your SDL renderer or ImGui to ensure that proper cleanup is done. This doesn't do anything critically important though,
// so if you're fine with small memory leaks at the end of your application, you can even omit this.
IMGUI_IMPL_API void ImGui_ImplSDLRenderer_Shutdown();

// Call this every frame after ImGui::Render with ImGui::GetDrawData(). This will use the SDL_Renderer provided to the interfrace with Initialize
// to draw the contents of the draw data to the screen.
IMGUI_IMPL_API void ImGui_ImplSDLRenderer_RenderDrawData(ImDrawData* drawData);

