/*******************************************************************
    Multi-Part Model Example
    raylib version with explicit matrix transformations.

      - Each object is drawn by constructing a world matrix explicitly.
      - The global spin is a parent/world transform applied to every part.
      - Rectangular solids use a red material.
      - Spheres use a green material.
      - Basic per-fragment Phong-style lighting shader is embedded.

    
********************************************************************/

#include "raylib.h"
#include "raymath.h"

static float spin = 0.0f;
static bool spinning = false;

static int lightPosLoc;
static int viewPosLoc;
static int lightColorLoc;
static int ambientColorLoc;

// We will study Shaders and GPU programming later in the semester. 
// For now, we will use a simple embedded shader to demonstrate lighting and shading.
static const char* lightingVertexShader = R"glsl(
#version 330

in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

uniform mat4 mvp;
uniform mat4 matModel;

out vec3 fragPosition;
out vec3 fragNormal;
out vec4 fragColor;

void main()
{
    vec4 worldPosition = matModel * vec4(vertexPosition, 1.0);
    fragPosition = worldPosition.xyz;

    // Correct normal transform for non-uniform scaling.
    fragNormal = normalize(mat3(transpose(inverse(matModel))) * vertexNormal);

    fragColor = vertexColor;
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
)glsl";

static const char* lightingFragmentShader = R"glsl(
#version 330

in vec3 fragPosition;
in vec3 fragNormal;
in vec4 fragColor;

uniform vec4 colDiffuse;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 ambientColor;

out vec4 finalColor;

void main()
{
    vec3 N = normalize(fragNormal);
    vec3 L = normalize(lightPos - fragPosition);
    vec3 V = normalize(viewPos - fragPosition);
    vec3 R = reflect(-L, N);

    float diff = max(dot(N, L), 0.0);
    float spec = 0.0;

    if (diff > 0.0)
    {
        spec = pow(max(dot(V, R), 0.0), 32.0);
    }

    vec3 baseColor = colDiffuse.rgb * fragColor.rgb;
    vec3 ambient = ambientColor * baseColor;
    vec3 diffuse = diff * lightColor * baseColor;
    vec3 specular = spec * lightColor * 0.35;

    finalColor = vec4(ambient + diffuse + specular, colDiffuse.a * fragColor.a);
}
)glsl";

static void AssignMaterial(Model* model, Shader shader, Color color)
{
    model->materials[0].shader = shader;
    model->materials[0].maps[MATERIAL_MAP_DIFFUSE].color = color;
}


// We will study lighting and shading later in the semester
static Shader LightingAndShading()
{
    Shader shader = LoadShaderFromMemory(lightingVertexShader, lightingFragmentShader);

    // Tell raylib which shader uniforms correspond to its internal matrices/colors.
    shader.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(shader, "mvp");
    shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(shader, "matModel");
    shader.locs[SHADER_LOC_COLOR_DIFFUSE] = GetShaderLocation(shader, "colDiffuse");

    lightPosLoc = GetShaderLocation(shader, "lightPos");
    viewPosLoc = GetShaderLocation(shader, "viewPos");
    lightColorLoc = GetShaderLocation(shader, "lightColor");
    ambientColorLoc = GetShaderLocation(shader, "ambientColor");

    Vector3 lightPos = Vector3{ -5.0f, 5.0f, 5.0f };
    float lightColor[3] = { 1.0f, 1.0f, 1.0f };
    float ambientColor[3] = { 0.18f, 0.18f, 0.18f };

    SetShaderValue(shader, lightPosLoc, &lightPos, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, lightColorLoc, lightColor, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, ambientColorLoc, ambientColor, SHADER_UNIFORM_VEC3);

    return shader;
}


// raylib's DrawModelEx internally builds transforms in this same style:
// MatrixMultiply(MatrixMultiply(scale, rotation), translation)
// For a simple scale+translate local transform, this is S*T.
static Matrix LocalScaleTranslate(Vector3 scale, Vector3 translation)
{
    Matrix S = MatrixScale(scale.x, scale.y, scale.z);
    Matrix T = MatrixTranslate(translation.x, translation.y, translation.z);
    return MatrixMultiply(S, T);
}


static void DrawModelWithWorldMatrix(Model* model, Matrix world)
{
    model->transform = world;
    DrawModel(*model, Vector3{ 0.0f, 0.0f, 0.0f }, 1.0f, WHITE);
}

int main(void)
{
    const int screenWidth = 750;
    const int screenHeight = 750;

    InitWindow(screenWidth, screenHeight, "3D Example - explicit raylib matrices");
    SetTargetFPS(60);

	// Setup the Camera - we will be studying cameras and associated transformations in upcoming lectures
    Camera3D camera = { 0 };
    camera.position = Vector3{ 0.0f, 0.0f, 10.0f };
    camera.target = Vector3{ 0.0f, 0.0f,  0.0f };
    // camera orientation
    camera.up = Vector3{ 0.0f, 1.0f,  0.0f };
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;
        
    // Create the models from a raylib sphere mesh and a raylib cube mesh
    Model cubeModel = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
    Model sphereModel = LoadModelFromMesh(GenMeshSphere(1.0f, 32, 32));

    // Lighting and Shading using Shaders - we will stidy in upcoming lectures
	Shader shader = LightingAndShading();
    AssignMaterial(&cubeModel, shader, RED);
    AssignMaterial(&sphereModel, shader, GREEN);


    while (!WindowShouldClose())
    {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            spinning = true;
        }

        if (spinning)
        {
            spin += 100.0f * GetFrameTime();
            if (spin > 360.0f) spin -= 360.0f;
        }

        SetShaderValue(shader, viewPosLoc, &camera.position, SHADER_UNIFORM_VEC3);

        BeginDrawing();
        ClearBackground(WHITE);

        BeginMode3D(camera);

        DrawGrid(10, 1.0f);

        // Parent/global modeling transform - the "parent" is the entire compund model of 4 spheres and
		// 4 stretched cubes. We want to spin the entire model about the world Z axis. 
        Matrix parentSpin = MatrixRotateZ(DEG2RAD * spin);

        // In raylib/raymath style, these local ("child") transforms are composed as S*T.
        // To apply the parent spin to every child, multiply local * parent.
        // Conceptually: object local coordinates -> scaled/translated part -> rotated whole object.

        Matrix leftBarLocal = LocalScaleTranslate(Vector3{ 0.5f, 4.0f, 0.5f }, Vector3{ -2.0f,  0.0f, 0.0f });
        Matrix rightBarLocal = LocalScaleTranslate(Vector3{ 0.5f, 4.0f, 0.5f }, Vector3{ 2.0f,  0.0f, 0.0f });
        Matrix topBarLocal = LocalScaleTranslate(Vector3{ 4.0f, 0.5f, 0.5f }, Vector3{ 0.0f,  2.0f, 0.0f });
        Matrix bottomBarLocal = LocalScaleTranslate(Vector3{ 4.0f, 0.5f, 0.5f }, Vector3{ 0.0f, -2.0f, 0.0f });

        DrawModelWithWorldMatrix(&cubeModel, MatrixMultiply(leftBarLocal, parentSpin));
        DrawModelWithWorldMatrix(&cubeModel, MatrixMultiply(rightBarLocal, parentSpin));
        DrawModelWithWorldMatrix(&cubeModel, MatrixMultiply(topBarLocal, parentSpin));
        DrawModelWithWorldMatrix(&cubeModel, MatrixMultiply(bottomBarLocal, parentSpin));

        Matrix sphereScale = MatrixScale(1.0f, 1.0f, 1.0f);

        Matrix sphere0Local = MatrixMultiply(sphereScale, MatrixTranslate(-2.0f, 2.0f, 0.0f));
        Matrix sphere1Local = MatrixMultiply(sphereScale, MatrixTranslate(2.0f, 2.0f, 0.0f));
        Matrix sphere2Local = MatrixMultiply(sphereScale, MatrixTranslate(-2.0f, -2.0f, 0.0f));
        Matrix sphere3Local = MatrixMultiply(sphereScale, MatrixTranslate(2.0f, -2.0f, 0.0f));

        DrawModelWithWorldMatrix(&sphereModel, MatrixMultiply(sphere0Local, parentSpin));
        DrawModelWithWorldMatrix(&sphereModel, MatrixMultiply(sphere1Local, parentSpin));
        DrawModelWithWorldMatrix(&sphereModel, MatrixMultiply(sphere2Local, parentSpin));
        DrawModelWithWorldMatrix(&sphereModel, MatrixMultiply(sphere3Local, parentSpin));

        EndMode3D();

        DrawText("Left mouse button: start spin", 20, 20, 20, DARKGRAY);
        DrawText("Explicit matrices: local transform * parent spin", 20, 46, 20, DARKGRAY);
        DrawText("Red rectangular solids, green spheres, basic shader lighting", 20, 72, 20, DARKGRAY);

        EndDrawing();
    }

    UnloadModel(cubeModel);
    UnloadModel(sphereModel);
    UnloadShader(shader);

    CloseWindow();
    return 0;
}
