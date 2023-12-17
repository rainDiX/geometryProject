#include <SDL.h>
#include <SDL_hints.h>

#include "Application.hpp"
int main() {
#ifdef __linux__
    SDL_SetHint(SDL_HINT_VIDEO_X11_FORCE_EGL, "1");
#endif
    Application app;
    app.mainLoop();
    return 0;
}