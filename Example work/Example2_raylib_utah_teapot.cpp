/*******************************************************************
    Example2.cpp
    3D Geometric Transformations: Orbiting Utah Teapots
    With Basic Lighting and Shading

    Assumes this file exists beside the executable:

        utah_teapot.obj

    Notes:
    - raylib does not have fixed-function OpenGL lighting.
    - This example uses a small embedded vertex/fragment shader.
    - It keeps the same transformation idea as the GLUT original:
        parent orbit around origin
        local spin for the earth teapot
        small moon teapot orbiting around the earth teapot
********************************************************************/

#include "raylib.h"
#include "raymath.h"

static float spin = 0.0f;   // local rotation angle
static float theta = 0.0f;  // orbital rotation angle

static const char* lightingVertexShader = R"(
#version 330

in vec3 vertexPosition;
in vec3 vertexNormal;
in vec2 vertexTexCoord;

uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;

out vec3 fragPosition;
out vec3 fragNormal;
out vec2 fragTexCoord;

void main()
{
    vec4 worldPosition = matModel * vec4(vertexPosition, 1.0);

    fragPosition = worldPosition.xyz;
    fragNormal = normalize((matNormal * vec4(vertexNormal, 0.0)).xyz);
    fragTexCoord = vertexTexCoord;

    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
)";

static const char* lightingFragmentShader = R"(
#version 330

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexCoord;

out vec4 finalColor;

uniform vec3 materialColor;
uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform vec3 viewPosition;
uniform float ambientStrength;
uniform float specularStrength;
uniform float shininess;

void main()
{
    vec3 N = normalize(fragNormal);
    vec3 L = normalize(lightPosition - fragPosition);
    vec3 V = normalize(viewPosition - fragPosition);
    vec3 R = reflect(-L, N);

    vec3 ambient = ambientStrength * materialColor;

    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * materialColor;

    float spec = pow(max(dot(V, R), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 color = (ambient + diffuse) * lightColor + specular;

    finalColor = vec4(color, 1.0);
}
)";

int main()
{
    const int screenWidth = 750;
    const int screenHeight = 750;

    InitWindow(screenWidth, screenHeight, "raylib Utah teapot orbit - basic lighting");
    SetTargetFPS(60);

    Camera3D camera = { 0 };
    camera.position = Vector3{ 0.0f, 0.0f, 10.0f };
    camera.target = Vector3{ 0.0f, 0.0f, 0.0f };
    camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    Shader shader = LoadShaderFromMemory(lightingVertexShader, lightingFragmentShader);

    int materialColorLoc = GetShaderLocation(shader, "materialColor");
    int lightPositionLoc = GetShaderLocation(shader, "lightPosition");
    int lightColorLoc = GetShaderLocation(shader, "lightColor");
    int viewPositionLoc = GetShaderLocation(shader, "viewPosition");
    int ambientStrengthLoc = GetShaderLocation(shader, "ambientStrength");
    int specularStrengthLoc = GetShaderLocation(shader, "specularStrength");
    int shininessLoc = GetShaderLocation(shader, "shininess");

    Vector3 lightPosition = { -4.0f, 6.0f, 6.0f };
    Vector3 lightColor = { 1.0f, 1.0f, 1.0f };
    float ambientStrength = 0.18f;
    float specularStrength = 0.45f;
    float shininess = 32.0f;

    SetShaderValue(shader, lightPositionLoc, &lightPosition, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, lightColorLoc, &lightColor, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, ambientStrengthLoc, &ambientStrength, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, specularStrengthLoc, &specularStrength, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, shininessLoc, &shininess, SHADER_UNIFORM_FLOAT);

    // Assumes utah_teapot.obj is in the working directory.
    Model earthTeapot = LoadModel("utah_teapot.obj");
    Model moonTeapot = LoadModel("utah_teapot.obj");

    // Apply shader to all materials in case the OBJ has more than one material.
    for (int i = 0; i < earthTeapot.materialCount; i++)
    {
        earthTeapot.materials[i].shader = shader;
    }

    for (int i = 0; i < moonTeapot.materialCount; i++)
    {
        moonTeapot.materials[i].shader = shader;
    }

    bool animate = false;

    while (!WindowShouldClose())
    {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            animate = !animate;
        }

        if (animate)
        {
            float dt = GetFrameTime();

            spin += 100.0f * dt;
            theta += 125.0f * dt;

            if (spin > 360.0f)  spin -= 360.0f;
            if (theta > 360.0f) theta -= 360.0f;
        }

        // Keep camera position uniform current.
        SetShaderValue(shader, viewPositionLoc, &camera.position, SHADER_UNIFORM_VEC3);

        // ------------------------------------------------------------------
        // Parent transform:
        //
        // OpenGL original:
        //   glRotatef(theta, 0, 0, 1);
        //   glTranslatef(3, 0, 0);
        //
        // Conceptually:
        //   M_parent = Rz(theta) * T(3,0,0)
        // ------------------------------------------------------------------
        Matrix Rz_theta = MatrixRotateZ(DEG2RAD * theta);
        Matrix T_earth = MatrixTranslate(3.0f, 0.0f, 0.0f);

        // raymath order used in previous examples:
        // local first, then parent.
        Matrix earthParent = MatrixMultiply(T_earth, Rz_theta);

        // ------------------------------------------------------------------
        // Earth teapot:
        //
        // M_earth = Rz(theta) * T(3,0,0) * Ry(spin)
        // ------------------------------------------------------------------
        Matrix Ry_spin = MatrixRotateY(DEG2RAD * spin);
        Matrix earthWorld = MatrixMultiply(Ry_spin, earthParent);

        // ------------------------------------------------------------------
        // Moon teapot:
        //
        // M_moon = Rz(theta) * T(3,0,0)
        //        * Ry(2theta) * T(2,0,0) * S(0.25)
        // ------------------------------------------------------------------
        Matrix Ry_moon = MatrixRotateY(DEG2RAD * (2.0f * theta));
        Matrix T_moon = MatrixTranslate(2.0f, 0.0f, 0.0f);
        Matrix S_moon = MatrixScale(0.25f, 0.25f, 0.25f);

        Matrix moonLocal = MatrixMultiply(S_moon, T_moon);
        moonLocal = MatrixMultiply(moonLocal, Ry_moon);

        Matrix moonWorld = MatrixMultiply(moonLocal, earthParent);

        earthTeapot.transform = earthWorld;
        moonTeapot.transform = moonWorld;

        BeginDrawing();
        ClearBackground(WHITE);

        BeginMode3D(camera);

        // Draw the earth teapot with a warm shaded material.
        Vector3 earthColor = { 1.0f, 0.55f, 0.12f };
        SetShaderValue(shader, materialColorLoc, &earthColor, SHADER_UNIFORM_VEC3);
        DrawModel(earthTeapot, Vector3Zero(), 0.75f, WHITE);


        // Draw the moon teapot with a cool shaded material.
        Vector3 moonColor = { 0.25f, 0.65f, 1.0f };
        SetShaderValue(shader, materialColorLoc, &moonColor, SHADER_UNIFORM_VEC3);
        DrawModel(moonTeapot, Vector3Zero(), 1.0f, WHITE);

        EndMode3D();

        EndDrawing();
    }

    UnloadModel(earthTeapot);
    UnloadModel(moonTeapot);
    UnloadShader(shader);

    CloseWindow();
    return 0;
}
