/*******************************************************************
  Robot3D.cpp
  - Idiomatic raylib: Models + custom lighting shader
  - Hierarchy computed with matrices 

  Controls:
    r / R : robot base rotation +/-
    a / A : shoulder angle +/-
    g / G : gun angle +/-
    s     : start animation
    S     : stop animation
    TAB   : toggle wireframe helper
********************************************************************/
#include "raylib.h"
#include "raymath.h"

// ------------------------- Simple lighting shader -------------------------
static const char* lightingVs = R"(
#version 330

in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

uniform mat4 mvp;
uniform mat4 matModel;

out vec3 fragPos;
out vec3 fragNormal;
out vec4 fragColor;

void main()
{
    vec4 worldPos = matModel * vec4(vertexPosition, 1.0);
    fragPos = worldPos.xyz;

    // Correct normal transform for non-uniform scaling
    fragNormal = mat3(transpose(inverse(matModel))) * vertexNormal;

    fragColor = vertexColor;
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
)";

static const char* lightingFs = R"(
#version 330

in vec3 fragPos;
in vec3 fragNormal;
in vec4 fragColor;

out vec4 finalColor;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec4 baseColor;

uniform float ambientStrength;
uniform float specularStrength;
uniform float shininess;

void main()
{
    vec3 N = normalize(fragNormal);
    vec3 L = normalize(lightPos - fragPos);
    vec3 V = normalize(viewPos - fragPos);
    vec3 R = reflect(-L, N);

    // Ambient
    vec3 ambient = ambientStrength * baseColor.rgb;

    // Diffuse
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * baseColor.rgb;

    // Specular
    float spec = 0.0;
    if (diff > 0.0)
    {
        spec = pow(max(dot(V, R), 0.0), shininess);
    }
    vec3 specular = specularStrength * spec * vec3(1.0, 1.0, 1.0);

    vec3 result = ambient + diffuse + specular;

    finalColor = vec4(result, baseColor.a) * fragColor;
}
)";

// ------------------------- Dimensions -------------------------
static float robotBodyWidth = 4.0f;
static float robotBodyLength = 6.0f;
static float robotBodyDepth = 4.0f;

static float headWidth = 0.4f * robotBodyWidth;
static float headLength = headWidth;
static float headDepth = headWidth;

static float upperArmLength = robotBodyLength;
static float upperArmWidth = 0.125f * robotBodyWidth;

static float gunLength = upperArmLength / 4.0f;
static float gunWidth = upperArmWidth;
static float gunDepth = upperArmWidth;

static float stanchionLength = robotBodyLength;
static float stanchionRadius = 0.2f * robotBodyDepth;

static float baseWidth = 2.0f * robotBodyWidth;
static float baseLength = 0.25f * stanchionLength;

// ------------------------- Angles -------------------------
static float robotAngleDeg = 0.0f;
static float shoulderDeg = -40.0f;
static float gunDeg = -25.0f;
static float cubeDeg = 0.0f;

static bool animate = false;

// Build matrix = S * R * T 
static Matrix TRS(const Vector3& t, const Vector3& axis, float angleRad, const Vector3& s)
{
    Matrix T = MatrixTranslate(t.x, t.y, t.z);
    Matrix R = MatrixRotate(axis, angleRad);
    Matrix S = MatrixScale(s.x, s.y, s.z);
    return MatrixMultiply(MatrixMultiply(S, R), T);
}

// childWorld = parentWorld * childLocal, raylib MatrixMultiply order
static Matrix ParentChild(const Matrix& parentWorld, const Matrix& childLocal)
{
    return MatrixMultiply(childLocal, parentWorld);
}


// Draw a model using a full world matrix + shader base color uniform
static void DrawModelWithWorldMatrix(Model& model, const Matrix& world, Shader shader, Color tint = WHITE)
{
    Matrix prev = model.transform;
    model.transform = world;

    float color[4] =
    {
        tint.r / 255.0f,
        tint.g / 255.0f,
        tint.b / 255.0f,
        tint.a / 255.0f
    };

    int baseColorLoc = GetShaderLocation(shader, "baseColor");
    SetShaderValue(shader, baseColorLoc, color, SHADER_UNIFORM_VEC4);

    DrawModel(model, Vector3{ 0.0f, 0.0f, 0.0f }, 1.0f, WHITE);

    model.transform = prev;
}

int main()
{
    const int W = 1200;
    const int H = 800;

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(W, H, "Robot3D (raylib + lighting)");

    Camera3D cam = { 0 };
    cam.position = Vector3{ 0.0f, 4.0f, 24.0f };
    cam.target = Vector3{ 0.0f, 2.0f, 0.0f };
    cam.up = Vector3{ 0.0f, 1.0f, 0.0f };
    cam.fovy = 60.0f;
    cam.projection = CAMERA_PERSPECTIVE;

    SetTargetFPS(60);

    // ---- Models ----
    Model cubeModel  = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
    Model cylModel   = LoadModelFromMesh(GenMeshCylinder(1.0f, 1.0f, 24));
    Model planeModel = LoadModelFromMesh(GenMeshPlane(32.0f, 32.0f, 16, 16));

    // ---- Shader ----
    Shader lighting = LoadShaderFromMemory(lightingVs, lightingFs);

    cubeModel.materials[0].shader = lighting;
    cylModel.materials[0].shader = lighting;
    planeModel.materials[0].shader = lighting;

    int baseColorLoc = GetShaderLocation(lighting, "baseColor");

    // ---- Colors ----
    Color bodyColor = Color{ 40, 120,  40, 255 };
    Color armColor = Color{ 200,  40,  40, 255 };
    Color gunColor = Color{ 40,  40,  40, 255 };
    Color lowerColor = Color{ 150, 150, 150, 255 };
    Color groundColor = Color{ 70, 70,  70, 255 };
    Color cubeColor = Color{ 140,  40,  40, 255 };

    // ---- Lighting uniforms ----
    Vector3 lightPos = { 20.0f, 20.0f, 20.0f };

    float ambientStrength = 0.25f;
    float specularStrength = 0.60f;
    float shininess = 200.0f;

    int lightPosLoc = GetShaderLocation(lighting, "lightPos");
    int viewPosLoc = GetShaderLocation(lighting, "viewPos");
    int ambientLoc = GetShaderLocation(lighting, "ambientStrength");
    int specularLoc = GetShaderLocation(lighting, "specularStrength");
    int shininessLoc = GetShaderLocation(lighting, "shininess");

    bool wire = false;

    while (!WindowShouldClose())
    {
        // -------- Input --------
        if (IsKeyPressed(KEY_S) && !(IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))) animate = true;
        if (IsKeyPressed(KEY_S) && (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))) animate = false;

        if (IsKeyDown(KEY_R) && !(IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))) robotAngleDeg += 2.0f;
        if (IsKeyDown(KEY_R) && (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))) robotAngleDeg -= 2.0f;

        if (IsKeyDown(KEY_A) && !(IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))) shoulderDeg += 2.0f;
        if (IsKeyDown(KEY_A) && (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))) shoulderDeg -= 2.0f;

        if (IsKeyDown(KEY_G) && !(IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))) gunDeg += 2.0f;
        if (IsKeyDown(KEY_G) && (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))) gunDeg -= 2.0f;

        if (IsKeyPressed(KEY_TAB)) wire = !wire;

        if (animate)
        {
            shoulderDeg += 1.0f;
            cubeDeg += 2.0f;
        }

        // Optional light movement for debugging
        if (IsKeyDown(KEY_LEFT))  lightPos.x -= 0.2f;
        if (IsKeyDown(KEY_RIGHT)) lightPos.x += 0.2f;
        if (IsKeyDown(KEY_UP))    lightPos.z -= 0.2f;
        if (IsKeyDown(KEY_DOWN))  lightPos.z += 0.2f;
        if (IsKeyDown(KEY_PAGE_UP))   lightPos.y += 0.2f;
        if (IsKeyDown(KEY_PAGE_DOWN)) lightPos.y -= 0.2f;

        // -------- Set shader uniforms --------
        SetShaderValue(lighting, lightPosLoc, &lightPos, SHADER_UNIFORM_VEC3);
        SetShaderValue(lighting, viewPosLoc, &cam.position, SHADER_UNIFORM_VEC3);
        SetShaderValue(lighting, ambientLoc, &ambientStrength, SHADER_UNIFORM_FLOAT);
        SetShaderValue(lighting, specularLoc, &specularStrength, SHADER_UNIFORM_FLOAT);
        SetShaderValue(lighting, shininessLoc, &shininess, SHADER_UNIFORM_FLOAT);

        // -------- Build hierarchy --------
        Matrix worldRoot = MatrixIdentity();

        // Root/body coordinate frame: position + orientation only, no scale
        Matrix bodyFrameWorld =
            ParentChild(
                worldRoot,
                TRS(
                    Vector3{ 0.0f, 0.0f, 0.0f },
                    Vector3{ 0.0f, 1.0f, 0.0f },
                    DEG2RAD * robotAngleDeg,
                    Vector3{ 1.0f, 1.0f, 1.0f }
                )
            );

        // Body mesh: child of body frame
        Matrix bodyWorld =
            ParentChild(
                bodyFrameWorld,
                TRS(
                    Vector3{ 0.0f, 0.0f, 0.0f },
                    Vector3{ 0.0f, 1.0f, 0.0f },
                    0.0f,
                    Vector3{ robotBodyWidth, robotBodyLength, robotBodyDepth }
                )
            );

        // Head: child of body frame
        Vector3 headOffset =
        {
            0,
            0.5f * robotBodyWidth + headWidth,
            0.0f
        };

        Matrix headWorld =
            ParentChild(
                bodyFrameWorld,
                TRS(
                    headOffset,
                    Vector3{ 0.0f, 1.0f, 0.0f },
                    0.0f,
                    Vector3{ headWidth, headLength, headDepth }
                )
            );

        // Left arm: child of body frame
        Vector3 leftArmOffset =
        {
            0.5f * robotBodyWidth + 0.5f * upperArmWidth,
            0.0f,
            0.0f
        };

        Matrix leftArmFrameWorld =
            ParentChild(
                bodyFrameWorld,
                TRS(
                    leftArmOffset,
                    Vector3{ 0.0f, 1.0f, 0.0f },
                    0.0f,
                    Vector3{ 1.0f, 1.0f, 1.0f }
                )
            );

        Matrix leftArmWorld =
            ParentChild(
                leftArmFrameWorld,
                TRS(
                    Vector3{ 0.0f, 0.0f, 0.0f },
                    Vector3{ 0.0f, 1.0f, 0.0f },
                    0.0f,
                    Vector3{ upperArmWidth, upperArmLength, upperArmWidth }
                )
            );

        // Right arm: child of body frame
        Vector3 rightShoulderOffset =
        {
            -(0.5f * robotBodyWidth + 0.5f * upperArmWidth),
            0.5f * upperArmLength,
            0.0f
        };

        // Right arm frame is located at shoulder pivot.
        // Rotate around local X at the shoulder.
        Matrix rightArmFrameWorld =
            ParentChild(
                bodyFrameWorld,
                TRS(
                    rightShoulderOffset,
                    Vector3{ 1.0f, 0.0f, 0.0f },
                    DEG2RAD * shoulderDeg,
                    Vector3{ 1.0f, 1.0f, 1.0f }
                )
            );

        // Arm mesh hangs downward from shoulder pivot.
        // Cube is centered, so move its center down by half arm length.
        Matrix rightArmWorld =
            ParentChild(
                rightArmFrameWorld,
                TRS(
                    Vector3{ 0.0f, -0.5f * upperArmLength, 0.0f },
                    Vector3{ 0.0f, 1.0f, 0.0f },
                    0.0f,
                    Vector3{ upperArmWidth, upperArmLength, upperArmWidth }
                )
            );

        // Gun: child of right arm frame
        Vector3 gunPivotOffset =
        {
            0.0f,
            -upperArmLength,
            0.0f
        };

        // Gun frame is located at bottom of right arm.
        // Rotate around local X at gun joint.
        Matrix gunFrameWorld =
            ParentChild(
                rightArmFrameWorld,
                TRS(
                    gunPivotOffset,
                    Vector3{ 1.0f, 0.0f, 0.0f },
                    DEG2RAD * gunDeg,
                    Vector3{ 1.0f, 1.0f, 1.0f }
                )
            );

        // Gun mesh hangs downward from gun pivot.
        Matrix gunWorld =
            ParentChild(
                gunFrameWorld,
                TRS(
                    Vector3{ 0.0f, -0.5f * gunLength, 0.0f },
                    Vector3{ 0.0f, 1.0f, 0.0f },
                    0.0f,
                    Vector3{ gunWidth, gunLength, gunDepth }
                )
            );

        // Stanchion: child of body frame
        Vector3 stanchionOffset =
        {
            0.0f,
            -0.5f * robotBodyLength - 0.5f * stanchionLength,
            0.0f
        };

        Matrix stanchionFrameWorld =
            ParentChild(
                bodyFrameWorld,
                TRS(
                    stanchionOffset,
                    Vector3{ 0.0f, 1.0f, 0.0f },
                    0.0f,
                    Vector3{ 1.0f, 1.0f, 1.0f }
                )
            );

        Matrix stanchionWorld =
            ParentChild(
                stanchionFrameWorld,
                TRS(
                    Vector3{ 0.0f, 0.0f, 0.0f },
                    Vector3{ 0.0f, 1.0f, 0.0f },
                    0.0f,
                    Vector3{ stanchionRadius, stanchionLength, stanchionRadius }
                )
            );

        // Base: child of stanchion frame
        Vector3 baseOffset =
        {
            0.0f,
            -0.2f * stanchionLength - 0.2f * baseLength,
            0.0f
        };

        Matrix baseWorld =
            ParentChild(
                stanchionFrameWorld,
                TRS(
                    baseOffset,
                    Vector3{ 0.0f, 1.0f, 0.0f },
                    0.0f,
                    Vector3{ baseWidth, baseLength, baseWidth }
                )
            );


        Matrix demoCubeWorld =
            MatrixMultiply(MatrixTranslate(8.0f, 0.0f, 3.0f),
                MatrixMultiply(MatrixRotateY(DEG2RAD * cubeDeg),
                    MatrixTranslate(-8.0f, 0.0f, -3.0f)));

        Matrix groundWorld = MatrixTranslate(0.0f, -10.0f, 0.0f);

        // -------- Draw --------
        BeginDrawing();
        
        ClearBackground(Color{ 102, 102, 102, 255 });

        BeginMode3D(cam);

        DrawModelWithWorldMatrix(cubeModel, bodyWorld, lighting, bodyColor);
        DrawModelWithWorldMatrix(cubeModel, headWorld, lighting, bodyColor);

        // Uncomment when arm transforms are ready
        DrawModelWithWorldMatrix(cubeModel, leftArmWorld, lighting, armColor);
        DrawModelWithWorldMatrix(cubeModel, rightArmWorld, lighting, armColor);
        DrawModelWithWorldMatrix(cubeModel, gunWorld, lighting, gunColor);

        DrawModelWithWorldMatrix(cylModel, stanchionWorld, lighting, cubeColor);
        DrawModelWithWorldMatrix(cubeModel, baseWorld, lighting, lowerColor);

        DrawModelWithWorldMatrix(cubeModel, demoCubeWorld, lighting, cubeColor);
        DrawModelWithWorldMatrix(planeModel, groundWorld, lighting, groundColor);

        // Show light position as a small sphere
        DrawSphere(lightPos, 0.5f, YELLOW);
        
        EndMode3D();

        DrawText("r/R robot  a/A shoulder  g/G gun  s start  S stop  TAB wire", 10, 10, 20, RAYWHITE);
        DrawText("Arrow keys/PageUp/PageDown move light", 10, 35, 18, RAYWHITE);

        EndDrawing();
    }

    UnloadShader(lighting);
    UnloadModel(cubeModel);
    UnloadModel(cylModel);
    UnloadModel(planeModel);

    CloseWindow();
    return 0;
}