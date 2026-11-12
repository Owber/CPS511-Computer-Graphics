#pragma once
#include "raylib.h"
#include "raymath.h"

namespace MathUtils {
    inline Matrix TRS ( const Vector3& t, const Vector3& axis, float pitch, float yaw, float roll, Vector3 s){
        Matrix T = MatrixTranslate( t.x, t.y, t.z);

        Matrix Rx = MatrixRotateX(pitch * DEG2RAD);
        Matrix Ry = MatrixRotateY(yaw * DEG2RAD);
        Matrix Rz = MatrixRotateZ(roll * DEG2RAD);
        Matrix R = MatrixMultiply(MatrixMultiply(Rz, Rx), Ry); //to make sure that pitch and roll happen before yaw
        Matrix S = MatrixScale(s.x,s.y,s.z);
        return MatrixMultiply(MatrixMultiply(S,R), T);
    }

    inline Matrix ParentChild (const Matrix& parentWorld, const Matrix& childLocal){
        return MatrixMultiply(childLocal, parentWorld);
    }
}
