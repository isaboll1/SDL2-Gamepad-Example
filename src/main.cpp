#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl_renderer.h"
#include "imgui/imgui_impl_sdl.h"

#include "sdl_gamepad.h"
void ImGUIStyle();

int main(int argc, char * argv[]){
    //for the sake of this example application, i'm going to initialize SDL2's controller,
    // haptics, and sensor subsystem separately from the main subsystems, as it it's a DLL

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("SDL Controller Visualizer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                        1280, 720, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Event event;
    bool running = true;
    
    // This hint should be set if you want to access the full PS4/PS5 controller features even with bluetooth enabled
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS4_RUMBLE, "1");
    // By default (from v 2.0.14), SDL2 maps Nintendo Switch Pro controllers to the layout that nintendo usually provides.
    // Since this makes button mappings slightly different, i'll disable this hint so that the layout is in the more standard
    // Xbox-like layout shared across platfoms.
    SDL_SetHint(SDL_HINT_GAMECONTROLLER_USE_BUTTON_LABELS, "0");
    // SDL2 can actually access and use Nintendo Switch Joycons without a driver needed, and allows for them to be queried with full functionality
    // regarding analog, gyro and accelerometer, and other functionality. This hint controls that.
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_JOY_CONS, "1");
    
    // SDL2 allows for Steam controllers to be queried as game controllers, which is the default for how it works. It also can support up to 8 controllers
    // on systems such as Windows and Linux, and can do so for both regular and UWP environments on windows, and from Windows 7-10.

    // Initialize all of the subsystems specific to gamepad support
    SDL_InitSubSystem(SDL_INIT_SENSOR|SDL_INIT_GAMECONTROLLER|SDL_INIT_HAPTIC);
    // The way that I load controllers and such with the class defined in "sdl_gamepad.h" was meant to be similar to the 
    // way that controllers are loaded and used typically in libraries such as Window.Gaming.Input, whihc means an array/vector for
    // storing controller instances is needed.
    std::vector<SDLGamepad *> Gamepads;

    // ImGUI stuff is initialized for the purpose of display in the example.
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGUIStyle();
    ImGui_ImplSDL2_InitForOpenGL(window, NULL);
    ImGui_ImplSDLRenderer_Init(renderer);

    while (running){
        while(SDL_PollEvent(&event)){
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT){
                running = false;
                break;
            }

            // In order to add controllers, you have to query if controllers have been added on the system.
            if (event.type == SDL_CONTROLLERDEVICEADDED){
                // If the controller is added, we create an instance. could probably modify this to
                // handle not adding controllers in the case of it already existing,
                // but we shouldn't get any error like that.
                Gamepads.push_back(new SDLGamepad(event.cdevice.which));    
            }

            if (event.type == SDL_CONTROLLERDEVICEREMOVED){
                // Remove the controller from the vector and then delete it. This can probably be handled 
                // much more efficiently if you simply maintain an array of controllers and delete at the index, or if
                // handled differently in general, but this is simply one way of doing it with this structure.
                int popped = 0;
                SDLGamepad * instance = nullptr;
                for (int i = 0; i < Gamepads.size(); i++){
                    if (Gamepads[i]->id == event.cdevice.which){
                        instance = Gamepads[i];
                        popped = i;
                        break;
                    }
                }
                Gamepads.erase(Gamepads.begin()+popped);
                delete instance;
            }
        }

        //the SDLGamepad class relies on polling each connected gamepad
        for (auto controller: Gamepads){
            controller->pollState();
        }

        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        // Down below we're gonna use ImGUI to display controller values for every connected controller. 
        // You can use it as an example of querying through SDLGamepad.
        for (auto controller: Gamepads){
            ImVec4 pressed = ImVec4(0.0, 1.0, 0.0, 1.0);
            ImGui::Begin(controller->getName().c_str());

            // Show number of touchpads if supported.
            if (controller->getTouchpadCount()){
                ImGui::Text("Number of touchpads: %i", controller->getTouchpadCount());
            }
            
            // Provide options to enable gyro and accelerometer.
            if (controller->hasSensors()){
                if (controller->hasGyroscope()){
                    ImGui::Checkbox("Gyroscope", &controller->gyroActive);
                }
                if (controller->hasAccelerometer()){
                    ImGui::Checkbox("Accelerometer", &controller->accelActive);
                }
            }

            // Allow controller rumble to be activated.
            if (controller->hasHaptics()){
                ImGui::SliderFloat("Left Motor", &controller->vibration.motor_left, 0, 1, "%.3f", 1.0f);
                ImGui::SliderFloat("Right Motor", &controller->vibration.motor_right, 0, 1,"%.3f", 1.0f);
                controller->Rumble(controller->vibration.motor_left, controller->vibration.motor_right, 100);   
            }

            // Allow controller trigger rumble to be activated.
            if (controller->hasTriggerHaptics()){
                ImGui::SliderFloat("Left Trigger Motor", &controller->vibration.trigger_left, 0, 1, "%.3f", 1.0f);
                ImGui::SliderFloat("Right Trigger Motor", &controller->vibration.trigger_right, 0, 1,"%.3f", 1.0f);
                controller->RumbleTriggers(controller->vibration.trigger_left, controller->vibration.trigger_right, 100);   
            }

            // Print the face buttons, and color them if pressed.
            // Using the class, to query buttons you check the state struct.
            if (controller->state.A){
                ImGui::TextColored(pressed, "Button A");
            }else {ImGui::Text("Button A");}

            if (controller->state.B){
                ImGui::TextColored(pressed, "Button B");
            }else {ImGui::Text("Button B");}

            if (controller->state.X){
                ImGui::TextColored(pressed, "Button X");
            }else {ImGui::Text("Button X");}

            if (controller->state.Y){
                ImGui::TextColored(pressed, "Button Y");
            }else {ImGui::Text("Button Y");}

            ImGui::NewLine();
            // Print the DPad Buttons, and color them if they are pressed.

            ImGui::End();

        }
        ImGui::Begin("SDL Game Controller Test App");
        ImGui::Text("This is an application that tests the controllers you have on your system, using SDL2 with a custom class. \n This serves to also be an example of using SDL2 with controller support.");
        ImGui::NewLine();
        ImGui::Text("Number of Controllers: %i", Gamepads.size());
        ImGui::End();
        ImGui::Render();

        SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(renderer);



    }
    ImGui_ImplSDLRenderer_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    // This function call deinitializes all SDL2 subsystems. to deinitialize a specific subsystem, call "SDL_QuitSubSystem()"
    SDL_Quit();
    return 0;
                                
}


void ImGUIStyle(){
     //Unreal Engine style coloring, with global alpha. 
    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
    colors[ImGuiCol_ChildBg]                = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImGuiCol_Border]                 = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.20f, 0.21f, 0.22f, 0.54f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.40f, 0.40f, 0.40f, 0.40f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.18f, 0.18f, 0.18f, 0.67f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.29f, 0.29f, 0.29f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
    colors[ImGuiCol_Button]                 = ImVec4(0.44f, 0.44f, 0.44f, 0.40f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.46f, 0.47f, 0.48f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.42f, 0.42f, 0.42f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.70f, 0.70f, 0.70f, 0.31f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.70f, 0.70f, 0.70f, 0.80f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.48f, 0.50f, 0.52f, 1.00f);
    colors[ImGuiCol_Separator]              = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.72f, 0.72f, 0.72f, 0.78f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.91f, 0.91f, 0.91f, 0.25f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.81f, 0.81f, 0.81f, 0.67f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.46f, 0.46f, 0.46f, 0.95f);
    colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.73f, 0.60f, 0.15f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.87f, 0.87f, 0.87f, 0.35f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight]           = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
}