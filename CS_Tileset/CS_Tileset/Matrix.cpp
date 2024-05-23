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

extern double Matrix_A[3][3] = { {1,0,0},{0,1,0},{0,0,1} };
extern double Inverse_A[3][3] = { {1,0,0},{0,1,0},{0,0,1} };
extern int RangeOpt;

void GetMapSize(DWORD* nMapX, DWORD* nMapY, DWORD MapX, DWORD MapY)
{
    double retX[4], retY[4];
    int xmin, xmax, ymin, ymax;
    double X, Y, Xmin, Xmax, Ymin, Ymax;

    if (RangeOpt == 0)
    {
        X = 0; Y = 0;
        Matrix_Trans(Matrix_A, &X, &Y);
        retX[0] = X, retY[0] = Y;
        X = MapX - 1; Y = 0;
        Matrix_Trans(Matrix_A, &X, &Y);
        retX[1] = X, retY[1] = Y;
        X = 0; Y = MapY - 1;
        Matrix_Trans(Matrix_A, &X, &Y);
        retX[2] = X, retY[2] = Y;
        X = MapX - 1; Y = MapY - 1;
        Matrix_Trans(Matrix_A, &X, &Y);
        retX[3] = X, retY[3] = Y;

        double Xmin = 16777216, Xmax = -16777216, Ymin = 16777216, Ymax = -16777216;
        for (int i = 0; i < 4; i++)
        {
            if (retX[i] < Xmin)
                Xmin = retX[i];
            if (retX[i] > Xmax)
                Xmax = retX[i];
            if (retY[i] < Ymin)
                Ymin = retY[i];
            if (retY[i] > Ymax)
                Ymax = retY[i];
        }

        Xmax = round(Xmax * 1000) / 1000;
        Xmin = round(Xmin * 1000) / 1000;
        Ymax = round(Ymax * 1000) / 1000;
        Ymin = round(Ymin * 1000) / 1000;
        int xmax = (int)(Xmax), xmin = (int)(Xmin), ymax = (int)(Ymax), ymin = (int)(Ymin);
        *nMapX = xmax - xmin + 1;
        *nMapY = ymax - ymin + 1;
    }
    else
    {
        X = 0; Y = 0;
        Matrix_Trans(Matrix_A, &X, &Y);
        retX[0] = X, retY[0] = Y;
        X = MapX; Y = 0;
        Matrix_Trans(Matrix_A, &X, &Y);
        retX[1] = X, retY[1] = Y;
        X = 0; Y = MapY;
        Matrix_Trans(Matrix_A, &X, &Y);
        retX[2] = X, retY[2] = Y;
        X = MapX; Y = MapY;
        Matrix_Trans(Matrix_A, &X, &Y);
        retX[3] = X, retY[3] = Y;

        Xmin = 16777216, Xmax = -16777216, Ymin = 16777216, Ymax = -16777216;
        for (int i = 0; i < 4; i++)
        {
            if (retX[i] < Xmin)
                Xmin = retX[i];
            if (retX[i] > Xmax)
                Xmax = retX[i];
            if (retY[i] < Ymin)
                Ymin = retY[i];
            if (retY[i] > Ymax)
                Ymax = retY[i];
        }

        Xmax = round(Xmax * 1000) / 1000;
        Xmin = round(Xmin * 1000) / 1000;
        Ymax = round(Ymax * 1000) / 1000;
        Ymin = round(Ymin * 1000) / 1000;
        xmax = (int)(Xmax), xmin = (int)(Xmin), ymax = (int)(Ymax), ymin = (int)(Ymin);
        *nMapX = xmax - xmin;
        *nMapY = ymax - ymin;
    }
    /*
    *nMapX = (DWORD)ceil(round((Xmax - Xmin) * 1000) / 1000) + 1;
    if (round(Xmin * 1000) == 0 && round(Xmax * 1000) == 0) (*nMapX)++;
    *nMapY = (DWORD)ceil(round((Ymax - Ymin) * 1000) / 1000) + 1;
    if (round(Ymin * 1000) == 0 && round(Ymax * 1000) == 0) (*nMapY)++;
    */
    return;
}

void Matrix_Mul(double(*A)[3], double(*B)[3], double(*C)[3]) // C = A * B
{
    double temp[3][3];
    double sum;
    for (int i = 0; i < 3; i++) // row
    {
        for (int j = 0; j < 3; j++) // col
        {
            sum = 0;
            for (int k = 0; k < 3; k++)
                sum += A[i][k] * B[k][j];
            temp[i][j] = sum;
        }
    }
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            C[i][j] = temp[i][j];
}

void Matrix_Trans(double(*A)[3], double* X, double* Y) // b = A*x
{
    double temp[3];
    double x[3] = { *X, *Y, 1 };
    double sum;
    for (int i = 0; i < 3; i++)
    {
        sum = 0;
        for (int j = 0; j < 3; j++)
            sum += A[i][j] * x[j];
        temp[i] = sum;
    }
    *X = temp[0] / temp[2];
    *Y = temp[1] / temp[2];
}

double Matrix_GetDet3(double(*A)[3])
{
    return A[0][0] * (A[1][1] * A[2][2] - A[1][2] * A[2][1])
        - A[0][1] * (A[1][0] * A[2][2] - A[1][2] * A[2][0])
        + A[0][2] * (A[1][0] * A[2][1] - A[1][1] * A[2][0]);
}

double Matrix_GetDet2(double(*A)[2])
{
    return A[0][0] * A[1][1] - A[0][1] * A[1][0]; // ad-bc
}

void Matrix_Transpose(double(*A)[3], double(*B)[3]) // B = A^T
{
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            B[j][i] = A[i][j];
}

void Matrix_Copy(double(*A)[3], double(*B)[3]) // B = A
{
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            B[i][j] = A[i][j];
}

int Matrix_Inverse(double(*A)[3], double(*B)[3]) // B = A^-1
{
    double detA = Matrix_GetDet3(A);
    if (detA == 0) return -1;

    double CT[3][3];
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            double M[2][2];
            int row = 0, col = 0;
            for (int u = 0; u < 3; u++)
            {
                if (u == i)
                    continue;
                else
                {
                    for (int v = 0; v < 3; v++)
                        if (v == j)
                            continue;
                        else
                        {
                            if (row > 1) goto Coef;
                            M[row][col] = A[u][v];
                            col++;
                            if (col > 1) col = 0;
                        }
                    row++;
                }
            }
        Coef:
            double temp = Matrix_GetDet2(M);
            if ((i + j) % 2 == 1)
                temp = -temp;
            CT[j][i] = temp / detA;
        }
    }

    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            B[i][j] = CT[i][j];

    return 0;
}