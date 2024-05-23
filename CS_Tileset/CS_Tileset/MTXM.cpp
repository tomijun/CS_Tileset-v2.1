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
#include "MTXM.h"
#define PI 3.14159265

extern double Matrix_A[3][3];
extern double Inverse_A[3][3];
extern int THG2Opt, UNITOpt, MRGNOpt, RangeOpt;


void ClearMTXM(MTXMData*** MTXMptr, int sizeY)
{
    MTXMData** ptr = *MTXMptr;
    for (int i = 0; i < sizeY; i++)
        free(ptr[i]);
    free(ptr);
    MTXMptr = NULL;
}

void ClearTHG2(THG2Data*** THG2ptr, int sizeY)
{
    THG2Data** ptr = *THG2ptr;
    for (int i = 0; i < sizeY; i++)
        free(ptr[i]);
    free(ptr);
    THG2ptr = NULL;
}

void ClearUNIT(UNITData*** UNITptr, int sizeY)
{
    UNITData** ptr = *UNITptr;
    for (int i = 0; i < sizeY; i++)
        free(ptr[i]);
    free(ptr);
    UNITptr = NULL;
}

int MTXM_GetData(MTXMData** MTXM, double j, double i, DWORD MapX, DWORD MapY, unsigned short* Value, unsigned* Index, BYTE* Type)
{
    Matrix_Trans(Inverse_A, &j, &i);

    int I = (int)(round(i * 1000) / 1000), J = (int)(round(j * 1000) / 1000);
    if (I >= 0 && I < MapY && J >= 0 && J < MapX)
    {
        *Value = MTXM[I][J].Value;
        *Index = MTXM[I][J].Index;
        *Type = MTXM[I][J].Type;
        return 0;
    }
    else
        return -1;
}

void MTXM_Apply(MTXMData*** CurMTXM, DWORD* CurMapX, DWORD* CurMapY, THG2Data* CurTHG2, int THG2Num, UNITData* CurUNIT, int UNITNum, MRGNData* CurMRGN, int MRGNNum)
{
    MTXMData** newMTXM;
    DWORD newMapX, newMapY, MapX = *CurMapX, MapY = *CurMapY;

    int xmin, xmax, ymin, ymax;
    double retX[4], retY[4];
    double X, Y, Xmin, Xmax, Ymin, Ymax, Xstart, Ystart;
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
        newMapX = xmax - xmin + 1;
        newMapY = ymax - ymin + 1;

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

        Xmin = 16777216, Ymin = 16777216;
        for (int i = 0; i < 4; i++)
        {
            if (retX[i] < Xmin)
                Xmin = retX[i];
            if (retY[i] < Ymin)
                Ymin = retY[i];
        }

        Xmin = round(Xmin * 1000) / 1000;
        Ymin = round(Ymin * 1000) / 1000;
        Xstart = (int)(Xmin),  Ystart = (int)(Ymin);
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
        newMapX = xmax - xmin;
        newMapY = ymax - ymin;

        Xstart = xmin; Ystart = ymin;
    }

    newMTXM = (MTXMData**)malloc(newMapY * sizeof(MTXMData*));
    for (int i = 0; i < newMapY; i++)
        newMTXM[i] = (MTXMData*)malloc(newMapX * sizeof(MTXMData));

    for (int i = 0; i < newMapY; i++)
    {
        for (int j = 0; j < newMapX; j++)
        {
            unsigned Index; unsigned short Value; BYTE Type;
            int ret = MTXM_GetData(*CurMTXM, j + xmin, i + ymin, MapX, MapY, &Value, &Index, &Type);
            if (ret == -1) // BLANK
            {
                newMTXM[i][j].Value = 0x001D;
                newMTXM[i][j].Index = -1;
                newMTXM[i][j].Type = 1;
            }
            else // Vaild Tile
            {
                newMTXM[i][j].Value = Value;
                newMTXM[i][j].Index = Index;
                newMTXM[i][j].Type = Type;
            }
        }
    }
    if (THG2Opt == 0)
        for (int i = 0; i < THG2Num; i++)
        {
            double X = CurTHG2[i].X;
            double Y = CurTHG2[i].Y;
            Matrix_Trans(Matrix_A, &X, &Y);
            CurTHG2[i].X = X - Xstart;
            CurTHG2[i].Y = Y - Ystart;
        }
    if (TRUE)
        for (int i = 0; i < UNITNum; i++)
        {
            double X = CurUNIT[i].X;
            double Y = CurUNIT[i].Y;
            Matrix_Trans(Matrix_A, &X, &Y);
            CurUNIT[i].X = X - Xstart;
            CurUNIT[i].Y = Y - Ystart;
        }

    if (MRGNOpt == 1)
    {
        for (int i = 0; i < MRGNNum; i++)
        {
            double X = CurMRGN[i].CX/32.;
            double Y = CurMRGN[i].CY/32.;
            Matrix_Trans(Matrix_A, &X, &Y);
            CurMRGN[i].CX = (X - Xstart)*32.;
            CurMRGN[i].CY = (Y - Ystart)*32.;
            for (int j = 0; j < 4; j++)
            {
                if (j % 2 == 0) // LR
                {
                    X = CurMRGN[i].Value[j]/32.;
                    Y = CurMRGN[i].Loc[j]/32.;
                    Matrix_Trans(Matrix_A, &X, &Y);
                    CurMRGN[i].Value[j] = (X - Xstart)*32.;
                    CurMRGN[i].Loc[j] = CurMRGN[i].CY;
                }
                else // UD
                {
                    X = CurMRGN[i].Loc[j]/32.;
                    Y = CurMRGN[i].Value[j]/32.;
                    Matrix_Trans(Matrix_A, &X, &Y);
                    CurMRGN[i].Value[j] = (Y - Ystart)*32.;
                    CurMRGN[i].Loc[j] = CurMRGN[i].CX;
                }
            }
        }
    }
    else
    {
        for (int i = 0; i < MRGNNum; i++)
        {
            double X = CurMRGN[i].CX/32.;
            double Y = CurMRGN[i].CY/32.;
            Matrix_Trans(Matrix_A, &X, &Y);
            CurMRGN[i].CX = (X - Xstart)*32.;
            CurMRGN[i].CY = (Y - Ystart)*32.;
        }
    }

    MTXMData** tempMTXM = *CurMTXM;
    *CurMTXM = newMTXM;
    *CurMapX = newMapX;
    *CurMapY = newMapY;
    ClearMTXM(&tempMTXM, MapY);
    return;
}

void MTXM_Trim(MTXMData*** CurMTXM, DWORD* CurMapX, DWORD* CurMapY, THG2Data* CurTHG2, int THG2Num, UNITData* CurUNIT, int UNITNum, MRGNData* CurMRGN, int MRGNNum)
{
    MTXMData** newMTXM, ** MTXM = *CurMTXM;
    DWORD newMapX, newMapY, MapX = *CurMapX, MapY = *CurMapY;

    int MapL = 16777216, MapR = -16777216, MapU = 16777216, MapD = -16777216;
    for (int i = 0; i < *CurMapY; i++)
        for (int j = 0; j < *CurMapX; j++)
        {
            if (MTXM[i][j].Type == 2)
            {
                if (j < MapL) MapL = j;
                if (i < MapU) MapU = i;
                if (j > MapR) MapR = j;
                if (i > MapD) MapD = i;
            }
        }

    newMapX = MapR - MapL + 1;
    newMapY = MapD - MapU + 1;

    newMTXM = (MTXMData**)malloc(newMapY * sizeof(MTXMData*));
    for (int i = 0; i < newMapY; i++)
        newMTXM[i] = (MTXMData*)malloc(newMapX * sizeof(MTXMData));

    for (int i = 0; i < newMapY; i++)
    {
        for (int j = 0; j < newMapX; j++)
        {
            newMTXM[i][j] = (*CurMTXM)[i + MapU][j + MapL];
        }
    }

    if (THG2Opt == 0)
        for (int i = 0; i < THG2Num; i++)
        {
            CurTHG2[i].X -= MapL;
            CurTHG2[i].Y -= MapU;
        }
    if (TRUE)
        for (int i = 0; i < UNITNum; i++)
        {
            CurUNIT[i].X -= MapL;
            CurUNIT[i].Y -= MapU;
        }
    if (MRGNOpt == 1)
    {
        for (int i = 0; i < MRGNNum; i++)
        {
            CurMRGN[i].CX -= MapL*32;
            CurMRGN[i].CY -= MapU*32;
            for (int j = 0; j < 4; j++)
            {
                if (j % 2 == 0) // LR
                {
                    CurMRGN[i].Value[j] -= MapL*32;
                    CurMRGN[i].Loc[j] = CurMRGN[i].CY;
                }
                else // UD
                {
                    CurMRGN[i].Value[j] -= MapU*32;
                    CurMRGN[i].Loc[j] = CurMRGN[i].CX;
                }
            }
        }
    }
    else
    {
        for (int i = 0; i < MRGNNum; i++)
        {
            CurMRGN[i].CX -= MapL*32;
            CurMRGN[i].CY -= MapU*32;
        }
    }

    MTXMData** tempMTXM = *CurMTXM;
    *CurMTXM = newMTXM;
    *CurMapX = newMapX;
    *CurMapY = newMapY;
    ClearMTXM(&tempMTXM, MapY);
    return;
}

void MTXM_setSize(MTXMData*** CurMTXM, DWORD* CurMapX, DWORD* CurMapY, int X, int Y, THG2Data* CurTHG2, int THG2Num, UNITData* CurUNIT, int UNITNum, MRGNData* CurMRGN, int MRGNNum)
{
    MTXMData** newMTXM, ** MTXM = *CurMTXM;
    DWORD newMapX, newMapY, MapX = *CurMapX, MapY = *CurMapY;

    newMapX = X;
    newMapY = Y;

    newMTXM = (MTXMData**)malloc(newMapY * sizeof(MTXMData*));
    for (int i = 0; i < newMapY; i++)
        newMTXM[i] = (MTXMData*)malloc(newMapX * sizeof(MTXMData));

    int CX = (X - *CurMapX) / 2, CY = (Y - *CurMapY) / 2;
    MTXMData BGM;
    BGM.Value = 0x001D;
    BGM.Index = -1;
    BGM.Type = 1;

    for (int i = 0; i < newMapY; i++)
    {
        for (int j = 0; j < newMapX; j++)
        {
            if (j >= CX && j < *CurMapX + CX && i >= CY && i < *CurMapY + CY)
                newMTXM[i][j] = (*CurMTXM)[i - CY][j - CX];
            else
                newMTXM[i][j] = BGM;
        }
    }

    if (THG2Opt == 0)
        for (int i = 0; i < THG2Num; i++)
        {
            CurTHG2[i].X += CX;
            CurTHG2[i].Y += CY;
        }
    if (TRUE)
        for (int i = 0; i < UNITNum; i++)
        {
            CurUNIT[i].X += CX;
            CurUNIT[i].Y += CY;
        }
    if (MRGNOpt == 1)
    {
        for (int i = 0; i < MRGNNum; i++)
        {
            CurMRGN[i].CX += CX*32;
            CurMRGN[i].CY += CY*32;
            for (int j = 0; j < 4; j++)
            {
                if (j % 2 == 0) // LR
                {
                    CurMRGN[i].Value[j] += CX*32;
                    CurMRGN[i].Loc[j] = CurMRGN[i].CY;
                }
                else // UD
                {
                    CurMRGN[i].Value[j] += CY*32;
                    CurMRGN[i].Loc[j] = CurMRGN[i].CX;
                }
            }
        }
    }
    else
    {
        for (int i = 0; i < MRGNNum; i++)
        {
            CurMRGN[i].CX += CX*32;
            CurMRGN[i].CY += CY*32;
        }
    }

    MTXMData** tempMTXM = *CurMTXM;
    *CurMTXM = newMTXM;
    *CurMapX = newMapX;
    *CurMapY = newMapY;
    ClearMTXM(&tempMTXM, MapY);
    return;
}

void MTXM_Movemap(MTXMData*** CurMTXM, DWORD* CurMapX, DWORD* CurMapY, int X, int Y, THG2Data* CurTHG2, int THG2Num, UNITData* CurUNIT, int UNITNum, MRGNData* CurMRGN, int MRGNNum)
{
    MTXMData** newMTXM, ** MTXM = *CurMTXM;
    DWORD MapX = *CurMapX, MapY = *CurMapY;

    newMTXM = (MTXMData**)malloc(MapY * sizeof(MTXMData*));
    for (int i = 0; i < MapY; i++)
        newMTXM[i] = (MTXMData*)malloc(MapX * sizeof(MTXMData));

    MTXMData BGM;
    BGM.Value = 0x001D;
    BGM.Index = -1;
    BGM.Type = 1;

    for (int i = 0; i < MapY; i++)
    {
        for (int j = 0; j < MapX; j++)
        {
            if (j-X >= 0 && j-X < *CurMapX && i-Y >= 0 && i-Y < *CurMapY)
                newMTXM[i][j] = (*CurMTXM)[i-Y][j-X];
            else
                newMTXM[i][j] = BGM;
        }
    }

    if (THG2Opt == 0)
        for (int i = 0; i < THG2Num; i++)
        {
            CurTHG2[i].X += X;
            CurTHG2[i].Y += Y;
        }
    if (TRUE)
        for (int i = 0; i < UNITNum; i++)
        {
            CurUNIT[i].X += X;
            CurUNIT[i].Y += Y;
        }
    if (MRGNOpt == 1)
    {
        for (int i = 0; i < MRGNNum; i++)
        {
            CurMRGN[i].CX += X * 32;
            CurMRGN[i].CY += Y * 32;
            for (int j = 0; j < 4; j++)
            {
                if (j % 2 == 0) // LR
                {
                    CurMRGN[i].Value[j] += X * 32;
                    CurMRGN[i].Loc[j] = CurMRGN[i].CY;
                }
                else // UD
                {
                    CurMRGN[i].Value[j] += Y * 32;
                    CurMRGN[i].Loc[j] = CurMRGN[i].CX;
                }
            }
        }
    }
    else
    {
        for (int i = 0; i < MRGNNum; i++)
        {
            CurMRGN[i].CX += X * 32;
            CurMRGN[i].CY += Y * 32;
        }
    }

    MTXMData** tempMTXM = *CurMTXM;
    *CurMTXM = newMTXM;
    ClearMTXM(&tempMTXM, MapY);
    return;
}