#pragma once
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <Windows.h>
#include "SFmpqapi.h"
#include "SFmpq_static.h"
#include "SFmpqapi_no-lib.h"
#define PI 3.14159265

enum list { fn, gn, fx, fy, gx, gy, Ffn, Fgn, XYtoR, XYtoA, RAtoX, RAtoY, Distance};

int CheckRange(double X, double Y, double R);
void CalcData(int newMapX, int newMapY, double* op, int Type, double* x, double* y);
double NonLinear_Wave(list num, double X, double Y);
double NonLinear_Lens(list num, double X, double Y);
double NonLinear_Gravity(list num, double X, double Y);
double NonLinear_Window(list num, double X, double Y);
double NonLinear_Pinch(list num, double X, double Y);
double NonLinear_Squish(list num, double X, double Y);
double NonLinear_Swirl(list num, double X, double Y);
double NonLinear_Sphere(list num, double X, double Y);
double NonLinear_Saddle(list num, double X, double Y);
double NonLinear_Cylinder(list num, double X, double Y);
double NonLinear_Func(int fnum, list num, double X, double Y);
int Newton2(int fnum, double x0, double y0, double* X, double* Y);
int NonLinear_GetData(MTXMData** MTXM, double _X, double _Y, int MapX, int MapY, MTXMData* TempData, int op);
void NonLinear_Calc(double* X, double* Y, int op);
void NonLinear(double* op, MTXMData*** CurMTXM, DWORD* CurMapX, DWORD* CurMapY, THG2Data* CurTHG2, int THG2Num, UNITData* CurUNIT, int UNITNum, MRGNData* CurMRGN, int MRGNNum);