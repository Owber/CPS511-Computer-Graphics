#include "raylib.h"
#include "Drone.h"
#include "raymath.h"

int main() {
    // Window dimensions
    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "CPS 511 Assignment 1");

    //Create Camera init

    Camera3D camera = {0};
    Vector3 cameraOffset = {0.0f, 5.0f, 7.0f};
    //camera.position = {8.0f, 2.0f, 0.0f};  //ONLY TO BE USED FOR EDITTING
    camera.target = (Vector3) {0.0f, 0.0f, 0.0f };
    camera.up = (Vector3) { 0.0f, 1.0f, 0.0f };
    camera.fovy = 60;
    camera.projection = CAMERA_PERSPECTIVE;

    Drone drone((Vector3){0.0f, 0.25f, 0.0f});



    SetTargetFPS(60);

    // Main Game Loop
    while (!WindowShouldClose()) {
        // --- INPUT & MOVEMENT LOGIC ---

        drone.Update(); 

        camera.target = drone.GetPosition(); // COMMENT OUT IF EDITTING
        float droneYaw = drone.GetY();

 
        Vector3 rotatedOffset = Vector3RotateByAxisAngle(cameraOffset, (Vector3){0.0f, 1.0f, 0.0f}, droneYaw * DEG2RAD); //COMMENT OUT IF EDITTING
        camera.position = Vector3Add(camera.target, rotatedOffset); //COMMENT OUT IF EDITTING
         

        // --- DRAWING ---
        BeginDrawing();
            ClearBackground(RAYWHITE);
            BeginMode3D(camera);


                drone.Draw();
                DrawGrid(50, 1.0f);

            
            EndMode3D();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}