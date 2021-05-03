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
                                        1280, 720, SDL_WINDOW_SHOWN|SDL_WINDOW_RESIZABLE);
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Event event;
    bool running = true;

    // Initialize all of the subsystems specific to gamepad support
    SDL_InitSubSystem(SDL_INIT_SENSOR|SDL_INIT_GAMECONTROLLER|SDL_INIT_HAPTIC);
    
    // This hint should be set if you want to access the full PS4 controller features even with bluetooth enabled.
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS4_RUMBLE, "1");
    // This hint should be set if you want to access the full PS5 controller features even with bluetooth enabled.
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS5_RUMBLE, "1");
    // By default (from v 2.0.14), SDL2 maps Nintendo Switch Pro controllers to the layout that nintendo usually provides.
    // Since this makes button mappings slightly different, i'll disable this hint so that the layout is in the more standard
    // Xbox-like layout shared across platfoms.
    SDL_SetHint(SDL_HINT_GAMECONTROLLER_USE_BUTTON_LABELS, "0");
    // SDL2 can actually access and use Nintendo Switch Joycons without a driver needed, and allows for them to be queried with full functionality
    // regarding analog, gyro and accelerometer, and other functionality. This hint controls that.
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_JOY_CONS, "1");
    
    // SDL2 allows for Steam controllers to be queried as game controllers, which is the default for how it works. It also can support up to 8 controllers
    // on systems such as Windows and Linux, and can do so for both regular and UWP environments on windows, and from Windows 7-10.

    
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
    int count = 0;
    int show_controller[8] {0,0,0,0,0,0,0,0};
    ImGuiID child_id = 0;

    while (running){
        while (SDL_PollEvent(&event)){
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT){
                running = false;
                break;
            }

            // In order to add controllers, you have to query if controllers have been added on the system.
            if (event.type == SDL_CONTROLLERDEVICEADDED){
                // If the controller is added, we create an instance. modified this to
                // handle not adding controllers in the case of it already existing,
                bool add_device = true;
                for (int i = 0; i < int(Gamepads.size()); i++){
                    if (Gamepads[i]->id == event.cdevice.which){
                        add_device = false;
                        break;
                    }
                }
                if (add_device){
                    Gamepads.push_back(new SDLGamepad(event.cdevice.which));
                }         
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

        SDL_RenderClear(renderer);
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();
        count = 0;
        ImVec4 color = {.500, .500, .500, 1.0};
   
        // Down below we're gonna use ImGUI to display controller values for every connected controller. 
        // You can use it as an example of querying through SDLGamepad.
        for (auto controller: Gamepads){
            count += 1;
            if (show_controller[count-1]){
                ImGui::Begin((controller->getName()+": #" + std::to_string(count)).c_str());
                ImVec4 pressed = ImVec4(0.0, 1.0, 0.0, 1.0);

                //Set Controller LED (If supported)
                if (controller->hasLED()){
                    std::vector<float> LED_Color = {float(controller->led_color.r) / float(255),
                                                    float(controller->led_color.g) / float(255), 
                                                    float(controller->led_color.b) / float(255), 
                                                        1.0f};

                    ImGui::ColorEdit3("LED Color", LED_Color.data());
                    controller->led_color.r = LED_Color[0] * 255;
                    controller->led_color.g = LED_Color[1] * 255;
                    controller->led_color.b = LED_Color[2] * 255;

                    controller->SetLED(controller->led_color.r, controller->led_color.g, controller->led_color.b);
                }

                ImGui::NewLine();
                // Show number of touchpads if supported.
                if (controller->getTouchpadCount()){
                    ImGui::Text("Number of touchpads: %i", controller->getTouchpadCount());
                }
                
                // Provide options to enable gyro and accelerometer.
                if (controller->hasSensors()){
                    controller->sensorEnabled = true;
                    if (controller->hasGyroscope()){
                        ImGui::Checkbox("Gyroscope", &controller->gyroActive);
                        controller->setSensor(SDL_SENSOR_GYRO, (SDL_bool)controller->gyroActive);
                    }
                    if (controller->hasAccelerometer()){
                        ImGui::Checkbox("Accelerometer", &controller->accelActive);
                        controller->setSensor(SDL_SENSOR_ACCEL, (SDL_bool)controller->accelActive);
                    }
                }

                if (controller->getTouchpadCount()){
                    ImGui::Checkbox("Touchpad Polling", &controller->queryTouchpads);
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


                ImGui::NewLine();
                // Print the face buttons, and color them if pressed.
                // Using the class, to query buttons you check the state struct.
                ImGui::TextColored(color, "Face Buttons");
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
                // Using the class, to query buttons you check the state struct.
                ImGui::TextColored(color, "DPAD Buttons");
                if (controller->state.DPadUp){
                    ImGui::TextColored(pressed, "Up");
                }else {ImGui::Text("Up");}

                if (controller->state.DPadDown){
                    ImGui::TextColored(pressed, "Down");
                }else {ImGui::Text("Down");}

                if (controller->state.DPadLeft){
                    ImGui::TextColored(pressed, "Left");
                }else {ImGui::Text("Left");}

                if (controller->state.DPadRight){
                    ImGui::TextColored(pressed, "Right");
                }else {ImGui::Text("Right");}


                ImGui::NewLine();
                // Print the DPad Buttons, and color them if they are pressed.   
                // Using the class, to query buttons you check the state struct.
                ImGui::TextColored(color, "Shoulder Buttons and Stick Clicks");
                if (controller->state.LeftShoulder){
                    ImGui::TextColored(pressed, "Left Shoulder");
                }else {ImGui::Text("Left Shoulder");}

                if (controller->state.RightShoulder){
                    ImGui::TextColored(pressed, "Right Shoulder");
                }else {ImGui::Text("Right Shoulder");}

                if (controller->state.LeftStickClick){
                    ImGui::TextColored(pressed, "Left Stick");
                }else {ImGui::Text("Left Stick");}

                if (controller->state.RightStickClick){
                    ImGui::TextColored(pressed, "Right Stick");
                }else {ImGui::Text("Right Stick");}


                ImGui::NewLine();
                // Print the DPad Buttons, and color them if they are pressed.   
                // Using the class, to query buttons you check the state struct.
                ImGui::TextColored(color, "Start, Back, Guide");
                if (controller->state.Start){
                    ImGui::TextColored(pressed, "Start");
                }else {ImGui::Text("Start");}

                if (controller->state.Back){
                    ImGui::TextColored(pressed, "Back");
                }else {ImGui::Text("Back");}

                if (controller->state.Guide){
                    ImGui::TextColored(pressed, "Guide");
                }else {ImGui::Text("Guide");}


                ImGui::NewLine();
                // Print the Axis values for the Triggers
                ImGui::TextColored(color, "Left Trigger and Right Trigger");
                ImGui::Text("Left Trigger: %.3f ,  Right Trigger: %.3f", controller->state.LeftTrigger, controller->state.RightTrigger);


                ImGui::NewLine();
                // Print the Axis values for the Sticks.
                ImGui::TextColored(color, "Left Stick and Right Stick");
                ImGui::Text("Left Stick (x: %.3f ,  y: %.3f)", controller->state.LeftStick.x, controller->state.LeftStick.y);
                ImGui::Text("Right Stick (x: %.3f ,  y: %.3f)", controller->state.RightStick.x, controller->state.RightStick.y);


                if (controller->sensorEnabled){
                    ImGui::NewLine();
                    // Print the Axis values for the Sticks.
                    ImGui::TextColored(color, "Gyro and/or Accelerometer");
                    if (controller->hasGyroscope()){
                        ImGui::Text("Gyroscope (x: %.3f ,  y: %.3f, z: %.3f)", controller->sensor_state.Gyroscope[0],
                                                                        controller->sensor_state.Gyroscope[1],
                                                                        controller->sensor_state.Gyroscope[2]);
                    }
                    if (controller->hasAccelerometer()){
                        ImGui::Text("Accelerometer (x: %.3f ,  y: %.3f, z: %.3f)", controller->sensor_state.Accelerometer[0],
                                                                        controller->sensor_state.Accelerometer[1],
                                                                        controller->sensor_state.Accelerometer[2]);
                    }
                    
                }

                ImGui::NewLine();
                // Print the Paddle Buttons
                ImGui::TextColored(color, "Paddle Buttons");
                if (controller->state.Paddle1){
                    ImGui::TextColored(pressed, "Paddle1");
                }else {ImGui::Text("Paddle1");}

                if (controller->state.Paddle2){
                    ImGui::TextColored(pressed, "Paddle2");
                }else {ImGui::Text("Paddle2");}

                if (controller->state.Paddle3){
                    ImGui::TextColored(pressed, "Paddle3");
                }else {ImGui::Text("Paddle3");}

                if (controller->state.Paddle4){
                    ImGui::TextColored(pressed, "Paddle4");
                }else {ImGui::Text("Paddle4");}


                ImGui::NewLine();
                // Print the Touchpad, and Misc button (Capture, Mic, and Share button respectively)
                ImGui::TextColored(color, "Touchpad and Misc");
                if (controller->state.Touchpad){
                    ImGui::TextColored(pressed, "Touchpad");
                }else {ImGui::Text("Touchpad");}

                if (controller->state.Misc){
                    ImGui::TextColored(pressed, "Misc");
                }else {ImGui::Text("Misc");}


                ImGui::NewLine();
                // Show Touchpad coordinates.
                if (controller->getTouchpadCount() && controller->queryTouchpads){
                    if (ImGui::CollapsingHeader("Touchpads")){
                        for (int i = 0; i < controller->touchpads.size(); i++){
                            if (ImGui::CollapsingHeader(("Touchpad: "+ std::to_string(i)).c_str())){
                                if (ImGui::BeginTable("Fingers", 1)){
                                    for (int j = 0; j < controller->touchpads[i].fingers.size(); j++){
                                        ImGui::TableNextColumn();
                                        ImGui::Text("Finger %i: (x: %f, y: %f, pressure: %f, state: %d)", j, 
                                                            controller->touchpads[i].fingers[j].x,
                                                            controller->touchpads[i].fingers[j].y,
                                                            controller->touchpads[i].fingers[j].pressure,
                                                            controller->touchpads[i].fingers[j].state);
                                    }
                                    
                                    ImGui::EndTable();
                                }
                            }
                        }
                   }
                }
                ImGui::End();
            }
            
            
        }
        ImGui::Begin("SDL Game Controller Test/Example App");
        ImGui::Text("This is an application that tests the controllers you have on your system, using SDL2 with a custom class. \nThis serves to also be an example of using SDL2 with controller support.");
        ImGui::NewLine();
        ImGui::Text("Number of Controllers: %i", Gamepads.size());
        for (int i = 0; i < Gamepads.size(); i++ ){
            if (ImGui::Button((Gamepads[i]->getName()+" (Index: "+ std::to_string(i) +")").c_str())){
                if (show_controller[i]){
                    show_controller[i] = false;
                }
                else{
                    show_controller[i] = true;
                }
            }
        }
        ImGui::End();
        ImGui::EndFrame();
        ImGui::Render();

        SDL_SetRenderTarget(renderer, NULL);
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