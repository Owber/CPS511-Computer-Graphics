#include "Drone.h"
#include "MathUtils.h"

Drone:: Drone(Vector3 startPos)
: body(LoadModelFromMesh(GenMeshCube(3.0f, 0.5f, 4.0f))),
head(LoadModelFromMesh(GenMeshSphere(0.25f, 16, 16))),
position(startPos), axis(Vector3 {0,0,0}), 
xRot(0.0f), yRot(0.0f), zRot(0.0f), //pitch, yaw, roll init.
speed(2.0f), liftSpeed(3.5f), 
turnSpeed(20.0f), rotSpeed(15.0f), retSpeed(5.0f),
legModel(LoadModelFromMesh(GenMeshCube(0.25f, 1.0f, 0.25f))),
branchModel(LoadModelFromMesh(GenMeshCube(1.0f, 0.25f, 2.5f))),
rotorModel(LoadModelFromMesh(GenMeshCylinder(0.25f, 0.75f, 24))),
rotorModel2(LoadModelFromMesh(GenMeshCylinder(0.25f, 0.25f, 24))),
cwRotor(LoadModelFromMesh(GenMeshCylinder(0.25f, 0.25f, 24))),
ccwRotor(LoadModelFromMesh(GenMeshCylinder(0.25f, 0.25f, 24))),
rotorHousing(LoadModelFromMesh(GenMeshTorus(0.05f, 1.5f, 16, 16))),
housingBranch(LoadModelFromMesh(GenMeshCube(0.25f, 0.1f, 0.6f))),
rotorBlade(LoadModelFromMesh(GenMeshCube(0.25f, 0.1f, 0.4f))){}

bool motorKill = false;
float propRotation = 0.0f;

void Drone::Update(){
    float deltaTime = GetFrameTime();

    bool isMovingX = false;
    bool isMovingZ = false;





    if (position.y <= 0.5f && (!IsKeyDown(KEY_R))) { 
        position.y = (position.y - 1.0f *deltaTime <= 0.25f) ? position.y : position.y - 5.0f * deltaTime;
        motorKill = false;
    }
    else { propRotation += 180 * deltaTime; } //propellor speed change if want slower. 180 deg/s
    if(IsKeyPressed(KEY_SPACE)) motorKill = !motorKill;

    if (propRotation >= 360.0f) propRotation -= 360.f;

    //falling
    if (motorKill){
        propRotation -= 180 * deltaTime;
        position.y = (position.y - 5.0f *deltaTime <= 0.25f) ? position.y : position.y - 5.0f * deltaTime;
    }

    //get local coordinates for the drone.
    float yawRad = yRot * DEG2RAD;
    Vector3 localForward = Vector3RotateByAxisAngle({0.0f,0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, yawRad);
    Vector3 localRight = Vector3RotateByAxisAngle({1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, yawRad);

    if (!motorKill){
        //move forward and backward
        if(IsKeyDown(KEY_W)) {
            xRot = (xRot - rotSpeed* deltaTime<= -8.0f || position.y <= 0.5) ? xRot : xRot - rotSpeed* deltaTime;
            position = (position.y <= 0.5) ? position : Vector3Add(position, Vector3Scale(localForward, speed * deltaTime)); //move forward
            isMovingZ = true;
        }
        if(IsKeyDown(KEY_S)) {
            xRot = (xRot + rotSpeed*deltaTime >= 8.0f || position.y <= 0.5) ? xRot : xRot + rotSpeed * deltaTime;
            position = (position.y <= 0.5) ? position : Vector3Subtract(position, Vector3Scale(localForward, speed * deltaTime)); //move backward
            isMovingZ = true;
        }
        //move left and right
        if(IsKeyDown(KEY_A)) {
            zRot = (zRot + rotSpeed * deltaTime >= 8.0f || position.y <= 0.5) ? zRot : zRot + rotSpeed * deltaTime;
            position = (position.y <= 0.5) ? position : Vector3Subtract(position, Vector3Scale(localRight, speed * deltaTime)); // move left
            isMovingX = true;
        }
        if(IsKeyDown(KEY_D)) {
            zRot = (zRot - rotSpeed * deltaTime<= -8.0f || position.y <= 0.5) ? zRot : zRot - rotSpeed * deltaTime;
            position = (position.y <= 0.5) ? position : Vector3Add(position, Vector3Scale(localRight, speed * deltaTime)); // move right
            isMovingX = true;
        }
        //lift up and down
        if(IsKeyDown(KEY_R)) {
            motorKill = false;
            propRotation += 360.0f *deltaTime;
            position.y += liftSpeed *deltaTime; //lift up
        }
        if(IsKeyDown(KEY_F)) {
            propRotation -= 45.0f * deltaTime;
            position.y = (position.y - liftSpeed *deltaTime <= 0.25f) ? position.y : position.y - liftSpeed * deltaTime; //lift down
        }
        //spin left and right
        if(IsKeyDown(KEY_Q)) yRot = (position.y <= 0.5f) ? yRot : yRot + turnSpeed* deltaTime;
        if(IsKeyDown(KEY_E)) yRot = (position.y <= 0.5f) ? yRot : yRot - turnSpeed * deltaTime;
        //return to steady
        if (!isMovingZ) xRot = Lerp(xRot, 0.0f, retSpeed*deltaTime);
        if (!isMovingX) zRot = Lerp(zRot, 0.0f, retSpeed*deltaTime);  
    }

}

void Drone :: Draw(){

    Matrix mainBody = MathUtils :: TRS(position, axis, xRot, yRot, zRot, {1.0f, 1.0f, 1.0f});

    body.transform = mainBody;

    DrawModel(body, {0.0f, 1.0f, 0.0f}, 1.0f, GRAY);
    DrawModelWires(body,{0.0f, 1.0f, 0.0f}, 1.0f, BLACK);

    Matrix headLocal = MathUtils :: TRS({0.0f, 0.25f, -2.0f}, {0.0f, 1.0f, 0.0f}, 0.0f, 0.0f, 0.0f, {1.0f, 1.0f, 1.0f});
    Matrix headWorld = MathUtils :: ParentChild(mainBody, headLocal);

    head.transform = headWorld;
    DrawModel(head, {0.0f, 1.0f, 0.0f}, 1.0f, YELLOW);
    

    Matrix frontLeftLegLocal = MathUtils :: TRS ({-1.25f, -0.75f, -1.75f}, {0.0f, 1.0f, 0.0f} , 0.0f, 0.0f, 0.0f, {1.0f, 1.0f, 1.0f});
    Matrix backLeftLegLocal = MathUtils :: TRS({-1.25f, -0.75f, 1.75f}, {0.0f, 1.0f, 0.0f}, 0.0f, 0.0f, 0.0f, {1.0f, 1.0f, 1.0f});
    Matrix frontRightLegLocal = MathUtils :: TRS({1.25f, -0.75f, -1.75f}, {0.0f, 1.0f, 0.0f}, 0.0f, 0.0f, 0.0f, {1.0f, 1.0f, 1.0f});
    Matrix backRightLegLocal = MathUtils :: TRS({1.25f, -0.75f, 1.75f}, {0.0f, 1.0f, 0.0f}, 0.0f, 0.0f, 0.0f, {1.0f, 1.0f, 1.0f});

    Matrix frontLeftBranchLocal = MathUtils :: TRS({-1.25f, 0.0f, -2.0f}, {0.0f, 1.0f, 0.0f}, 0.0f, 38.0f, 0.0f, {1.0f, 1.0f, 1.0f});
    Matrix backLeftBranchLocal = MathUtils :: TRS({-1.25f, 0.0f, 2.0f}, {0.0f, 1.0f, 0.0f}, 0.0f, -38.0f, 0.0f, {1.0f, 1.0f, 1.0f});
    Matrix frontRightBranchLocal = MathUtils :: TRS({1.25f, 0.0f, -2.0f}, {0.0f, 1.0f, 0.0f}, 0.0f, -38.0f, 0.0f, {1.0f, 1.0f, 1.0f});
    Matrix backRightBranchLocal = MathUtils :: TRS({1.25f, 0.0f, 2.0f}, {0.0f, 1.0f, 0.0f}, 0.0f, 38.0f, 0.0f, {1.0f, 1.0f, 1.0f});

    Matrix frontLeftLegWorld = MathUtils :: ParentChild(mainBody, frontLeftLegLocal);
    Matrix backLeftLegWorld = MathUtils :: ParentChild(mainBody, backLeftLegLocal);
    Matrix backRightLegWorld = MathUtils :: ParentChild(mainBody, backRightLegLocal);
    Matrix frontRightLegWorld = MathUtils :: ParentChild(mainBody, frontRightLegLocal);

    Matrix frontLeftBranchWorld = MathUtils :: ParentChild(mainBody, frontLeftBranchLocal);
    Matrix backLeftBranchWorld = MathUtils :: ParentChild(mainBody, backLeftBranchLocal);
    Matrix frontRightBranchWorld = MathUtils :: ParentChild(mainBody, frontRightBranchLocal);
    Matrix backRightBranchWorld = MathUtils :: ParentChild(mainBody, backRightBranchLocal);


    legModel.transform = frontLeftLegWorld;
    DrawModel(legModel, {0.0f, 1.0f, 0.0f}, 1.0f, GRAY);
    DrawModelWires(legModel, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);
    legModel.transform = backLeftLegWorld;
    DrawModel(legModel, {0.0f, 1.0f, 0.0f}, 1.0f, GRAY);
    DrawModelWires(legModel, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);
    legModel.transform = backRightLegWorld;
    DrawModel(legModel, {0.0f, 1.0f, 0.0f}, 1.0f, GRAY);
    DrawModelWires(legModel, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);
    legModel.transform = frontRightLegWorld;
    DrawModel(legModel, {0.0f, 1.0f, 0.0f}, 1.0f, GRAY);
    DrawModelWires(legModel, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);

    branchModel.transform = frontLeftBranchWorld;
    DrawModel(branchModel, {0.0f, 1.0f, 0.0f}, 1.0f, GRAY);
    DrawModelWires(branchModel, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);
    branchModel.transform = backLeftBranchWorld;
    DrawModel(branchModel, {0.0f, 1.0f, 0.0f}, 1.0f, GRAY);
    DrawModelWires(branchModel, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);
    branchModel.transform = frontRightBranchWorld;
    DrawModel(branchModel, {0.0f, 1.0f, 0.0f}, 1.0f, GRAY);
    DrawModelWires(branchModel, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);
    branchModel.transform = backRightBranchWorld;
    DrawModel(branchModel, {0.0f, 1.0f, 0.0f}, 1.0f, GRAY);
    DrawModelWires(branchModel, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);
    

    Matrix frontRotorLocal = MathUtils :: TRS({-0.0f, 0.0f, -0.8f}, {0.0f, 1.0f, 0.0f}, 0.0f, 0.0f, 0.0f, {1.0f, 1.0f, 1.0f});
    Matrix backRotorLocal = MathUtils :: TRS({0.0f, 0.0f, 0.8f}, {0.0f, 1.0f, 0.0f}, 0.0f, 0.0f, 0.0f, {1.0f, 1.0f, 1.0f});
    //change lower roters to be CW and CCW.
    Matrix cwRotor1Local = MathUtils :: TRS ({0.0f, 0.75f, -0.8f }, {0.0f, 1.0f, 0.0f}, 0.0f, propRotation, 0.0f, {1.0f, 1.0f, 1.0f});
    Matrix cwRotor2Local = MathUtils :: TRS ({0.0f, 0.75f, 0.8f}, {0.0f, 1.0f, 0.0f}, 0.0f, propRotation, 0.0f, {1.0f, 1.0f, 1.0f});
    Matrix ccwRotor1Local = MathUtils :: TRS ({0.0f, 0.75f, -0.8f}, {0.0f, 1.0f, 0.0f}, 0.0f, -propRotation, 0.0f, {1.0f, 1.0f, 1.0f});
    Matrix ccwRotor2Local = MathUtils :: TRS ({0.0f, 0.75f, 0.8f}, {0.0f, 1.0f, 0.0f}, 0.0f, -propRotation, 0.0f, {1.0f, 1.0f, 1.0f});


    Matrix frontLeftRotorWorld = MathUtils :: ParentChild(frontLeftBranchWorld, frontRotorLocal);
    Matrix backLeftRotorWorld = MathUtils :: ParentChild(backLeftBranchWorld, backRotorLocal);
    Matrix frontRightRotorWorld = MathUtils :: ParentChild(frontRightBranchWorld, frontRotorLocal);
    Matrix backRightRotorWorld = MathUtils :: ParentChild(backRightBranchWorld, backRotorLocal);
    
    //Spinning motor
    Matrix frontLeftRotor2World = MathUtils :: ParentChild(frontLeftBranchWorld, cwRotor1Local);
    Matrix backLeftRotor2World = MathUtils :: ParentChild(backLeftBranchWorld, ccwRotor2Local);
    Matrix frontRightRotor2World = MathUtils :: ParentChild(frontRightBranchWorld, ccwRotor1Local);
    Matrix backRightRotor2World = MathUtils :: ParentChild(backRightBranchWorld, cwRotor2Local);

    rotorModel.transform = frontLeftRotorWorld;
    DrawModel(rotorModel, {0.0f, 1.0f, 0.0f}, 1.0f, GRAY);
    DrawModelWires(rotorModel, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);
    rotorModel.transform = backLeftRotorWorld;
    DrawModel(rotorModel, {0.0f, 1.0f, 0.0f}, 1.0f, GRAY);
    DrawModelWires(rotorModel, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);
    rotorModel.transform = frontRightRotorWorld;
    DrawModel(rotorModel, {0.0f, 1.0f, 0.0f}, 1.0f, GRAY);
    DrawModelWires(rotorModel, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);
    rotorModel.transform = backRightRotorWorld;
    DrawModel(rotorModel, {0.0f, 1.0f, 0.0f}, 1.0f, GRAY);
    DrawModelWires(rotorModel, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);
    
    rotorModel2.transform = frontLeftRotor2World;
    DrawModel(rotorModel2, {0.0f, 1.0f, 0.0f}, 1.0f, BLUE);
    DrawModelWires(rotorModel2, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);
    rotorModel2.transform = backLeftRotor2World;
    DrawModel(rotorModel2, {0.0f, 1.0f, 0.0f}, 1.0f, BLUE);
    DrawModelWires(rotorModel2, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);
    rotorModel2.transform = frontRightRotor2World;
    DrawModel(rotorModel2, {0.0f, 1.0f, 0.0f}, 1.0f, BLUE);
    DrawModelWires(rotorModel2, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);
    rotorModel2.transform = backRightRotor2World;
    DrawModel(rotorModel2, {0.0f, 1.0f, 0.0f}, 1.0f, BLUE);
    DrawModelWires(rotorModel2, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);
    
    Matrix housingLocal = MathUtils :: TRS ({0.0f, 0.75f, 0.0f}, {0.0f, 1.0f, 0.0f}, 90.0f, 0.0f, 0.0f, {1.0f, 1.0f, 3.0f});

    Matrix housing1World = MathUtils :: ParentChild(frontLeftRotorWorld, housingLocal);
    Matrix housing2World = MathUtils :: ParentChild(frontRightRotorWorld, housingLocal);
    Matrix housing3World = MathUtils :: ParentChild(backLeftRotorWorld, housingLocal);
    Matrix housing4World = MathUtils :: ParentChild(backRightRotorWorld, housingLocal);
    
    rotorHousing.transform = housing1World;
    DrawModel(rotorHousing, {0.0f, 1.0f, 0.0f}, 1.0f, GRAY);
    DrawModelWires(rotorHousing, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);
    rotorHousing.transform = housing2World;
    DrawModel(rotorHousing, {0.0f, 1.0f, 0.0f}, 1.0f, GRAY);
    DrawModelWires(rotorHousing, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);
    rotorHousing.transform = housing3World;
    DrawModel(rotorHousing, {0.0f, 1.0f, 0.0f}, 1.0f, GRAY);
    DrawModelWires(rotorHousing, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);
    rotorHousing.transform = housing4World;
    DrawModel(rotorHousing, {0.0f, 1.0f, 0.0f}, 1.0f, GRAY);
    DrawModelWires(rotorHousing, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);


    Matrix housingBranch1Local = MathUtils :: TRS ({0.0f, 0.65f, -0.4f}, {0.0f, 1.0f, 0.0f}, 0.0f, 0.0f, 0.0f, {1.0f, 1.0f, 1.0f});
    Matrix housingBranch2Local = MathUtils :: TRS ({0.0f, 0.65f, 0.4f}, {0.0f, 1.0f, 0.0f}, 0.0f, 0.0f, 0.0f, {1.0f, 1.0f, 1.0f});
    Matrix housingBranch3Local = MathUtils :: TRS ({0.4f, 0.65f, -0.0f}, {0.0f, 1.0f, 0.0f}, 0.0f, 90.0f, 0.0f, {1.0f, 1.0f, 1.0f});
    Matrix housingBranch4Local = MathUtils :: TRS ({-0.4f, 0.65f, -0.0f}, {0.0f, 1.0f, 0.0f}, 0.0f, 90.0f, 0.0f, {1.0f, 1.0f, 1.0f});

    //front left motor housing.
    Matrix housingBranch1World = MathUtils :: ParentChild(frontLeftRotorWorld, housingBranch1Local);
    Matrix housingBranch2World = MathUtils :: ParentChild(frontLeftRotorWorld, housingBranch2Local);
    Matrix housingBranch3World = MathUtils :: ParentChild(frontLeftRotorWorld, housingBranch3Local);
    Matrix housingBranch4World = MathUtils :: ParentChild(frontLeftRotorWorld, housingBranch4Local);

    //front right motor housing
    Matrix housingBranch5World = MathUtils :: ParentChild(frontRightRotorWorld, housingBranch1Local);
    Matrix housingBranch6World = MathUtils :: ParentChild(frontRightRotorWorld, housingBranch2Local);
    Matrix housingBranch7World = MathUtils :: ParentChild(frontRightRotorWorld, housingBranch3Local);
    Matrix housingBranch8World = MathUtils :: ParentChild(frontRightRotorWorld, housingBranch4Local);

    //back left motor housing
    Matrix housingBranch9World = MathUtils :: ParentChild(backLeftRotorWorld, housingBranch1Local);
    Matrix housingBranch10World = MathUtils :: ParentChild(backLeftRotorWorld, housingBranch2Local);
    Matrix housingBranch11World = MathUtils :: ParentChild(backLeftRotorWorld, housingBranch3Local);
    Matrix housingBranch12World = MathUtils :: ParentChild(backLeftRotorWorld, housingBranch4Local);

    //back right motor housing
    Matrix housingBranch13World = MathUtils :: ParentChild(backRightRotorWorld, housingBranch1Local);
    Matrix housingBranch14World = MathUtils :: ParentChild(backRightRotorWorld, housingBranch2Local);
    Matrix housingBranch15World = MathUtils :: ParentChild(backRightRotorWorld, housingBranch3Local);
    Matrix housingBranch16World = MathUtils :: ParentChild(backRightRotorWorld, housingBranch4Local);

    //front left motor transform
    housingBranch.transform = housingBranch1World;
    DrawModel(housingBranch, {0.0f, 1.0f, 0.0f}, 1.0f, GRAY);
    DrawModelWires(housingBranch, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);
    housingBranch.transform = housingBranch2World;
    DrawModel(housingBranch, {0.0f, 1.0f, 0.0f}, 1.0f, GRAY);
    DrawModelWires(housingBranch, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);
    housingBranch.transform = housingBranch3World;
    DrawModel(housingBranch, {0.0f, 1.0f, 0.0f}, 1.0f, GRAY);
    DrawModelWires(housingBranch, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);
    housingBranch.transform = housingBranch4World;
    DrawModel(housingBranch, {0.0f, 1.0f, 0.0f}, 1.0f, GRAY);
    DrawModelWires(housingBranch, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);

    //fornt right motor transform
    housingBranch.transform = housingBranch5World;
    DrawModel(housingBranch, {0.0f, 1.0f, 0.0f}, 1.0f, GRAY);
    DrawModelWires(housingBranch, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);
    housingBranch.transform = housingBranch6World;
    DrawModel(housingBranch, {0.0f, 1.0f, 0.0f}, 1.0f, GRAY);
    DrawModelWires(housingBranch, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);
    housingBranch.transform = housingBranch7World;
    DrawModel(housingBranch, {0.0f, 1.0f, 0.0f}, 1.0f, GRAY);
    DrawModelWires(housingBranch, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);
    housingBranch.transform = housingBranch8World;
    DrawModel(housingBranch, {0.0f, 1.0f, 0.0f}, 1.0f, GRAY);
    DrawModelWires(housingBranch, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);

    //back left motor transform
    housingBranch.transform = housingBranch9World;
    DrawModel(housingBranch, {0.0f, 1.0f, 0.0f}, 1.0f, GRAY);
    DrawModelWires(housingBranch, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);
    housingBranch.transform = housingBranch10World;
    DrawModel(housingBranch, {0.0f, 1.0f, 0.0f}, 1.0f, GRAY);
    DrawModelWires(housingBranch, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);
    housingBranch.transform = housingBranch11World;
    DrawModel(housingBranch, {0.0f, 1.0f, 0.0f}, 1.0f, GRAY);
    DrawModelWires(housingBranch, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);
    housingBranch.transform = housingBranch12World;
    DrawModel(housingBranch, {0.0f, 1.0f, 0.0f}, 1.0f, GRAY);
    DrawModelWires(housingBranch, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);

    //back right motor transform
    housingBranch.transform = housingBranch13World;
    DrawModel(housingBranch, {0.0f, 1.0f, 0.0f}, 1.0f, GRAY);
    DrawModelWires(housingBranch, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);
    housingBranch.transform = housingBranch14World;
    DrawModel(housingBranch, {0.0f, 1.0f, 0.0f}, 1.0f, GRAY);
    DrawModelWires(housingBranch, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);
    housingBranch.transform = housingBranch15World;
    DrawModel(housingBranch, {0.0f, 1.0f, 0.0f}, 1.0f, GRAY);
    DrawModelWires(housingBranch, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);
    housingBranch.transform = housingBranch16World;
    DrawModel(housingBranch, {0.0f, 1.0f, 0.0f}, 1.0f, GRAY);
    DrawModelWires(housingBranch, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);  

    Matrix ccwBlade1Local = MathUtils :: TRS({0.0f, 0.1f, 0.4f}, {0.0f, 1.0f, 0.0f}, 0.0f, 0.0f, 30.0f, {1.0f, 1.0f, 1.0f});
    Matrix ccwBlade2Local = MathUtils :: TRS({0.0f, 0.1f, -0.4f}, {0.0f, 1.0f, 0.0f}, 0.0f, 180.0f, 30.0f, {1.0f, 1.0f, 1.0f});
    Matrix ccwBlade3Local = MathUtils :: TRS({0.4f, 0.1f, 0.0f}, {0.0f, 1.0f, 0.0f}, 0.0f, 90.0f, 30.0f, {1.0f, 1.0f, 1.0f});
    Matrix ccwBlade4Local = MathUtils :: TRS({-0.4f, 0.1f, 0.0f}, {0.0f, 1.0f, 0.0f}, 0.0f, 270.0f, 30.0f, {1.0f, 1.0f, 1.0f});

    Matrix cwBlade1Local = MathUtils :: TRS({0.0f, 0.1f, 0.4f}, {0.0f, 1.0f, 0.0f}, 0.0f, 0.0f, -30.0f, {1.0f, 1.0f, 1.0f});
    Matrix cwBlade2Local = MathUtils :: TRS({0.0f, 0.1f, -0.4f}, {0.0f, 1.0f, 0.0f}, 0.0f, 180.0f, -30.0f, {1.0f, 1.0f, 1.0f});
    Matrix cwBlade3Local = MathUtils :: TRS({0.4f, 0.1f, 0.0f}, {0.0f, 1.0f, 0.0f}, 0.0f, 90.0f, -30.0f, {1.0f, 1.0f, 1.0f});
    Matrix cwBlade4Local = MathUtils :: TRS({-0.4f, 0.1f, 0.0f}, {0.0f, 1.0f, 0.0f}, 0.0f, 270.0f, -30.0f, {1.0f, 1.0f, 1.0f});

    Matrix ccwBlade1World = MathUtils :: ParentChild(frontLeftRotor2World, ccwBlade1Local);
    Matrix ccwBlade2World = MathUtils :: ParentChild(frontLeftRotor2World, ccwBlade2Local);
    Matrix ccwBlade3World = MathUtils :: ParentChild(frontLeftRotor2World, ccwBlade3Local);
    Matrix ccwBlade4World = MathUtils :: ParentChild(frontLeftRotor2World, ccwBlade4Local);

    Matrix ccwBlade5World = MathUtils :: ParentChild(backRightRotor2World, ccwBlade1Local);
    Matrix ccwBlade6World = MathUtils :: ParentChild(backRightRotor2World, ccwBlade2Local);
    Matrix ccwBlade7World = MathUtils :: ParentChild(backRightRotor2World, ccwBlade3Local);
    Matrix ccwBlade8World = MathUtils :: ParentChild(backRightRotor2World, ccwBlade4Local);

    Matrix cwBlade1World = MathUtils ::ParentChild(frontRightRotor2World, cwBlade1Local);
    Matrix cwBlade2World = MathUtils ::ParentChild(frontRightRotor2World, cwBlade2Local);
    Matrix cwBlade3World = MathUtils ::ParentChild(frontRightRotor2World, cwBlade3Local);
    Matrix cwBlade4World = MathUtils ::ParentChild(frontRightRotor2World, cwBlade4Local);

    Matrix cwBlade5World = MathUtils ::ParentChild(backLeftRotor2World, cwBlade1Local);
    Matrix cwBlade6World = MathUtils ::ParentChild(backLeftRotor2World, cwBlade2Local);
    Matrix cwBlade7World = MathUtils ::ParentChild(backLeftRotor2World, cwBlade3Local);
    Matrix cwBlade8World = MathUtils ::ParentChild(backLeftRotor2World, cwBlade4Local);


    rotorBlade.transform = ccwBlade1World;
    DrawModel(rotorBlade, {0.0f, 1.0f, 0.0f}, 1.0f, RED);
    DrawModelWires(rotorBlade, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);
    rotorBlade.transform = ccwBlade2World;
    DrawModel(rotorBlade, {0.0f, 1.0f, 0.0f}, 1.0f, RED);
    DrawModelWires(rotorBlade, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);
    rotorBlade.transform = ccwBlade3World;
    DrawModel(rotorBlade, {0.0f, 1.0f, 0.0f}, 1.0f, RED);
    DrawModelWires(rotorBlade, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);
    rotorBlade.transform = ccwBlade4World;
    DrawModel(rotorBlade, {0.0f, 1.0f, 0.0f}, 1.0f, RED);
    DrawModelWires(rotorBlade, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);

    rotorBlade.transform = ccwBlade5World;
    DrawModel(rotorBlade, {0.0f, 1.0f, 0.0f}, 1.0f, RED);
    DrawModelWires(rotorBlade, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);
    rotorBlade.transform = ccwBlade6World;
    DrawModel(rotorBlade, {0.0f, 1.0f, 0.0f}, 1.0f, RED);
    DrawModelWires(rotorBlade, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);
    rotorBlade.transform = ccwBlade7World;
    DrawModel(rotorBlade, {0.0f, 1.0f, 0.0f}, 1.0f, RED);
    DrawModelWires(rotorBlade, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);
    rotorBlade.transform = ccwBlade8World;
    DrawModel(rotorBlade, {0.0f, 1.0f, 0.0f}, 1.0f, RED);
    DrawModelWires(rotorBlade, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);
    
    rotorBlade.transform = cwBlade1World;
    DrawModel(rotorBlade, {0.0f, 1.0f, 0.0f}, 1.0f, RED);
    DrawModelWires(rotorBlade, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);
    rotorBlade.transform = cwBlade2World;
    DrawModel(rotorBlade, {0.0f, 1.0f, 0.0f}, 1.0f, RED);
    DrawModelWires(rotorBlade, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);
    rotorBlade.transform = cwBlade3World;
    DrawModel(rotorBlade, {0.0f, 1.0f, 0.0f}, 1.0f, RED);
    DrawModelWires(rotorBlade, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);
    rotorBlade.transform = cwBlade4World;
    DrawModel(rotorBlade, {0.0f, 1.0f, 0.0f}, 1.0f, RED);
    DrawModelWires(rotorBlade, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);

    rotorBlade.transform = cwBlade5World;
    DrawModel(rotorBlade, {0.0f, 1.0f, 0.0f}, 1.0f, RED);
    DrawModelWires(rotorBlade, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);
    rotorBlade.transform = cwBlade6World;
    DrawModel(rotorBlade, {0.0f, 1.0f, 0.0f}, 1.0f, RED);
    DrawModelWires(rotorBlade, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);
    rotorBlade.transform = cwBlade7World;
    DrawModel(rotorBlade, {0.0f, 1.0f, 0.0f}, 1.0f, RED);
    DrawModelWires(rotorBlade, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);
    rotorBlade.transform = cwBlade8World;
    DrawModel(rotorBlade, {0.0f, 1.0f, 0.0f}, 1.0f, RED);
    DrawModelWires(rotorBlade, {0.0f, 1.0f, 0.0f}, 1.0f, BLACK);


}

