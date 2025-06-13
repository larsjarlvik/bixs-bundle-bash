#include <raylib.h>

int main() {
    // Platform-specific window setup
#ifdef PLATFORM_ANDROID
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(0, 0, "Blue Screen Test");  // Android sets size automatically
#else
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(800, 600, "Blue Screen Test");
#endif

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLUE);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}