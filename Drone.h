#pragma once
#include "raylib.h"

class Drone {
private:
    //Main Body variables
    Model body;
    Model head;
    Vector3 position;
    Vector3 axis;

    float xRot; //pitch angle
    float yRot; //yaw angle
    float zRot; //roll angle

    float speed;
    float liftSpeed;

    float turnSpeed;
    float rotSpeed;
    float retSpeed;

    //Children models
    Model legModel;
    Model branchModel;
    Model rotorModel;
    Model rotorModel2;
    Model cwRotor;
    Model ccwRotor;
    Model rotorHousing;
    Model housingBranch;
    Model rotorBlade;

    //child variables

    void DrawProp(const Matrix& parentWorld, const Matrix& localOffset);


public:
    Drone(Vector3 startPos);

    void Update();
    void Draw();

    Vector3 GetPosition() const {return position; }
    float GetSpeed() const {return speed; }
    float GetLift() const {return liftSpeed; }
    float GetY() const { return yRot; }
};
