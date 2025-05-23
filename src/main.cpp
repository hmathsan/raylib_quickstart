#include "raylib.h"
#include "rlImGui.h"

#include "imgui.h"

int main()
{
    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "raylib basic window");
    SetTargetFPS(120);

    rlImGuiSetup(true);

    while (!WindowShouldClose())
    {
        BeginDrawing();

        ClearBackground(RAYWHITE);

        rlImGuiBegin();

        bool open = true;
        ImGui::ShowDemoWindow(&open);

        open = true;
        if (ImGui::Begin("Hello", &open)) {
            ImGui::TextUnformatted(ICON_FA_JEDI);
        }
        ImGui::End();

        DrawText("It works!", 20, 20, 20, BLACK);

        rlImGuiEnd();
        EndDrawing();
    }

    rlImGuiShutdown();
    CloseWindow();
    return 0;
}