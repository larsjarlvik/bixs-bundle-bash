#include <raylib.h>
#include "game.h"

int main() {
    // Platform-specific window setup
#ifdef PLATFORM_ANDROID
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(0, 0, "Bix's Bundle Bash");
#else
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(800, 600,  "Bix's Bundle Bash");
#endif

    init_game();
    return 0;
}