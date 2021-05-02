#include "sdl_gamepad.h"

int main(int * argc, char ** argv){
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

    // Initialize all of the subsystems specific to gamepad support
    SDL_InitSubSystem(SDL_INIT_SENSOR||SDL_INIT_HAPTIC||SDL_INIT_GAMECONTROLLER);
    // The way that I load controllers and such with the class defined in "sdl_gamepad.h" was meant to be similar to the 
    // way that controllers are loaded and used typically in libraries such as Window.Gaming.Input, whihc means an array/vector for
    // storing controller instances is needed.
    std::vector<SDLGamepad *> Gamepads;

    while (running){
        while(SDL_PollEvent(&event)){
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

        // Down below we're gonna use ImGUI to display controller values for every connected controller. 
        // You can use it as an example of querying through SDLGamepad.

        for (auto controller: Gamepads){
            controller->state.A;
        }
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    // This function call deinitializes all SDL2 subsystems. to deinitialize a specific subsystem, call "SDL_QuitSubSystem()"
    SDL_Quit();
    return 0;
                                
}