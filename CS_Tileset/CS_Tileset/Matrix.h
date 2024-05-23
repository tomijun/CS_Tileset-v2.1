#pragma once
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <Windows.h>
#include "SFmpqapi.h"
#include "SFmpq_static.h"
#include "SFmpqapi_no-lib.h"

void GetMapSize(DWORD* nMapX, DWORD* nMapY, DWORD MapX, DWORD MapY);
int Matrix_Inverse(double(*A)[3], double(*B)[3]); // B = A^-1
void Matrix_Copy(double(*A)[3], double(*B)[3]); // B = A
void Matrix_Transpose(double(*A)[3], double(*B)[3]); // B = A^T
double Matrix_GetDet2(double(*A)[2]);
double Matrix_GetDet3(double(*A)[3]);
void Matrix_Trans(double(*A)[3], double* X, double* Y); // b = A*x
void Matrix_Mul(double(*A)[3], double(*B)[3], double(*C)[3]); // C = A * B