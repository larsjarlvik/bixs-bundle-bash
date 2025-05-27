#include "raylib.h"

int main() {
    InitWindow(800, 600, "Bix's Bundle Bash!");

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
