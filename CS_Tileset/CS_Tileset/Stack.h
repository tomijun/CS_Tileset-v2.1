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

int Stack_Move(double X, double Y, DWORD* nMapX, DWORD* nMapY, DWORD MapX, DWORD MapY);
int Stack_Scale(double X, double Y, DWORD* nMapX, DWORD* nMapY, DWORD MapX, DWORD MapY);
int Stack_setScale(int X, int Y, DWORD* nMapX, DWORD* nMapY, DWORD MapX, DWORD MapY);
int Stack_Reflect(double X, double Y, DWORD* nMapX, DWORD* nMapY, DWORD MapX, DWORD MapY);
int Stack_Rotate(double Angle, DWORD* nMapX, DWORD* nMapY, DWORD MapX, DWORD MapY);
int Stack_RotateAt(double Angle, double X, double Y, DWORD* nMapX, DWORD* nMapY, DWORD MapX, DWORD MapY);
int Stack_Shear(double X, double Y, DWORD* nMapX, DWORD* nMapY, DWORD MapX, DWORD MapY);
int Stack_Affine(double LUX, double LUY, double LDX, double LDY, double RUX, double RUY, double RDX, double RDY, DWORD* nMapX, DWORD* nMapY, DWORD MapX, DWORD MapY);