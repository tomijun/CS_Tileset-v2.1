#pragma once
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <Windows.h>
#include "SFmpqapi.h"
#include "SFmpq_static.h"
#include "SFmpqapi_no-lib.h"
#include "Matrix.h"
#define PI 3.14159265

extern double Matrix_A[3][3];
extern double Inverse_A[3][3];
extern int RangeOpt;

int Stack_Move(double X, double Y, DWORD* nMapX, DWORD* nMapY, DWORD MapX, DWORD MapY)
{
    double temp[3][3], newA[3][3] = { {1,0,0},{0,1,0},{0,0,1} };
    Matrix_Copy(Matrix_A, temp);

    newA[0][2] = X;
    newA[1][2] = Y;
    Matrix_Mul(newA, Matrix_A, Matrix_A);
    GetMapSize(nMapX, nMapY, MapX, MapY);

    int ret = Matrix_Inverse(Matrix_A, Inverse_A);

    if (ret == -1)
        Matrix_Copy(temp, Matrix_A);
    return ret;
}

int Stack_Scale(double X, double Y, DWORD* nMapX, DWORD* nMapY, DWORD MapX, DWORD MapY)
{
    if (abs(X) > 256 || abs(X) < 1 / 256 || abs(Y) > 256 || abs(Y) < 1 / 256) return -1;
    double temp[3][3], newA[3][3] = { {1,0,0},{0,1,0},{0,0,1} };
    Matrix_Copy(Matrix_A, temp);

    newA[0][0] = X;
    newA[1][1] = Y;
    Matrix_Mul(newA, Matrix_A, Matrix_A);
    GetMapSize(nMapX, nMapY, MapX, MapY);

    int ret = Matrix_Inverse(Matrix_A, Inverse_A);

    if (ret == -1)
        Matrix_Copy(temp, Matrix_A);
    return ret;
}

int Stack_setScale(int X, int Y, DWORD* nMapX, DWORD* nMapY, DWORD MapX, DWORD MapY)
{
    if (abs(X) > 65536 || abs(X) < 1 || abs(Y) > 65536 || abs(Y) < 1) return -1;
    double temp[3][3], newA[3][3] = { {1,0,0},{0,1,0},{0,0,1} };
    Matrix_Copy(Matrix_A, temp);
retry:
    newA[0][0] = (double)X / (*nMapX);
    newA[1][1] = (double)Y / (*nMapY);
    Matrix_Mul(newA, Matrix_A, Matrix_A);
    GetMapSize(nMapX, nMapY, MapX, MapY);
    if ((*nMapX) != X || (*nMapY) != Y)
        goto retry;

    int ret = Matrix_Inverse(Matrix_A, Inverse_A);

    if (ret == -1)
        Matrix_Copy(temp, Matrix_A);
    return ret;
}

int Stack_Reflect(double X, double Y, DWORD* nMapX, DWORD* nMapY, DWORD MapX, DWORD MapY)
{
    double temp[3][3], newA[3][3] = { {1,0,0},{0,1,0},{0,0,1} };
    Matrix_Copy(Matrix_A, temp);

    (abs(X) == 0) ? newA[0][0] = 1 : newA[0][0] = -1;
    (abs(Y) == 0) ? newA[1][1] = 1 : newA[1][1] = -1;
    Matrix_Mul(newA, Matrix_A, Matrix_A);
    GetMapSize(nMapX, nMapY, MapX, MapY);

    int ret = Matrix_Inverse(Matrix_A, Inverse_A);

    if (ret == -1)
        Matrix_Copy(temp, Matrix_A);
    return ret;
}

int Stack_Rotate(double Angle, DWORD* nMapX, DWORD* nMapY, DWORD MapX, DWORD MapY)
{
    double temp[3][3], newA[3][3] = { {1,0,0},{0,1,0},{0,0,1} };
    double newT[3][3] = { {1,0,0},{0,1,0},{0,0,1} };
    double InvT[3][3] = { {1,0,0},{0,1,0},{0,0,1} };

    if (RangeOpt == 0)
    {
        newT[0][2] = ((double)(*nMapX - 1)) / -2;
        newT[1][2] = ((double)(*nMapY - 1)) / -2;
        InvT[0][2] = ((double)(*nMapX - 1)) / 2;
        InvT[1][2] = ((double)(*nMapY - 1)) / 2;
    }
    else
    {
        newT[0][2] = ((double)(*nMapX)) / -2;
        newT[1][2] = ((double)(*nMapY)) / -2;
        InvT[0][2] = ((double)(*nMapX)) / 2;
        InvT[1][2] = ((double)(*nMapY)) / 2;
    }
    Matrix_Copy(Matrix_A, temp);

    Angle *= PI / 180;
    newA[0][0] = cos(Angle); newA[0][1] = -sin(Angle);
    newA[1][0] = sin(Angle); newA[1][1] = cos(Angle);
    Matrix_Mul(newT, Matrix_A, Matrix_A);
    Matrix_Mul(newA, Matrix_A, Matrix_A);
    Matrix_Mul(InvT, Matrix_A, Matrix_A);
    GetMapSize(nMapX, nMapY, MapX, MapY);

    int ret = Matrix_Inverse(Matrix_A, Inverse_A);

    if (ret == -1)
        Matrix_Copy(temp, Matrix_A);
    return ret;
}

int Stack_RotateAt(double Angle, double X, double Y, DWORD* nMapX, DWORD* nMapY, DWORD MapX, DWORD MapY)
{
    double temp[3][3], newA[3][3] = { {1,0,0},{0,1,0},{0,0,1} };
    double newT[3][3] = { {1,0,-X},{0,1,-Y},{0,0,1} };
    double InvT[3][3] = { {1,0,X},{0,1,Y},{0,0,1} };
    Matrix_Copy(Matrix_A, temp);

    Angle *= PI / 180;
    newA[0][0] = cos(Angle); newA[0][1] = -sin(Angle);
    newA[1][0] = sin(Angle); newA[1][1] = cos(Angle);
    Matrix_Mul(newT, Matrix_A, Matrix_A);
    Matrix_Mul(newA, Matrix_A, Matrix_A);
    Matrix_Mul(InvT, Matrix_A, Matrix_A);
    GetMapSize(nMapX, nMapY, MapX, MapY);

    int ret = Matrix_Inverse(Matrix_A, Inverse_A);

    if (ret == -1)
        Matrix_Copy(temp, Matrix_A);
    return ret;
}


int Stack_Shear(double X, double Y, DWORD* nMapX, DWORD* nMapY, DWORD MapX, DWORD MapY)
{
    double temp[3][3], newA[3][3] = { {1,0,0},{0,1,0},{0,0,1} };
    Matrix_Copy(Matrix_A, temp);

    newA[0][1] = X;
    newA[1][0] = Y;
    Matrix_Mul(newA, Matrix_A, Matrix_A);
    GetMapSize(nMapX, nMapY, MapX, MapY);

    int ret = Matrix_Inverse(Matrix_A, Inverse_A);

    if (ret == -1)
        Matrix_Copy(temp, Matrix_A);
    return ret;
}

int Stack_Affine(double LUX, double LUY, double LDX, double LDY, double RUX, double RUY, double RDX, double RDY, DWORD* nMapX, DWORD* nMapY, DWORD MapX, DWORD MapY)
{
    double temp[3][3], newA[3][3] = { {1,0,0},{0,1,0},{0,0,1} };
    Matrix_Copy(Matrix_A, temp);

    double X, Y;
    if (RangeOpt == 0)
    {
        X = *nMapX - 1, Y = *nMapY - 1;
    }
    else
    {
        X = *nMapX, Y = *nMapY;
    }
    newA[0][2] = LUX; // c
    newA[1][2] = LUY; // f
    double GHA[2][2] = {
        {X * (RUX - RDX), Y * (LDX - RDX)},
        {X * (RUY - RDY), Y * (LDY - RDY)}
    };
    double GHb[2] = {
        RDX - RUX - LDX + LUX,
        RDY - RUY - LDY + LUY
    };
    double GHI[2][2], detGH = Matrix_GetDet2(GHA);
    if (GHI == 0) return -1;
    GHI[0][0] = GHA[1][1] / detGH;
    GHI[0][1] = -GHA[0][1] / detGH;
    GHI[1][0] = -GHA[1][0] / detGH;
    GHI[1][1] = GHA[0][0] / detGH;

    newA[2][0] = GHI[0][0] * GHb[0] + GHI[0][1] * GHb[1]; // g
    newA[2][1] = GHI[1][0] * GHb[0] + GHI[1][1] * GHb[1]; // h

    newA[0][0] = RUX * newA[2][0] + (RUX - LUX) / X; // a
    newA[1][0] = RUY * newA[2][0] + (RUY - LUY) / X; // d
    newA[0][1] = LDX * newA[2][1] + (LDX - LUX) / Y; // b
    newA[1][1] = LDY * newA[2][1] + (LDY - LUY) / Y; // e

    Matrix_Mul(newA, Matrix_A, Matrix_A);
    GetMapSize(nMapX, nMapY, MapX, MapY);

    int ret = Matrix_Inverse(Matrix_A, Inverse_A);

    if (ret == -1)
        Matrix_Copy(temp, Matrix_A);
    return ret;
}
