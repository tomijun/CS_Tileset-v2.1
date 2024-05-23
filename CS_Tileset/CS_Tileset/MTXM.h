#pragma once
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <Windows.h>
#include "SFmpqapi.h"
#include "SFmpq_static.h"
#include "SFmpqapi_no-lib.h"
#include "Stack.h"
#include "Matrix.h"

typedef struct MTXM
{
    unsigned int Index;
    unsigned short Value;
    BYTE Type; // 0 : blank, 1 : null, 2 : normal
    BYTE BLANK;

}MTXMData;

typedef struct THG2
{
    unsigned short Value[5]; // 1 : X, 2 : Y
    BYTE dx;
    BYTE dy;
    double X;
    double Y;
}THG2Data;

typedef struct UNIT
{
    unsigned short Value[18]; // 2 : X, 3 : Y
    BYTE dx;
    BYTE dy;
    double X;
    double Y;
}UNITData;

typedef struct _THG2
{
    THG2Data data;
    _THG2* link;

}THG2Node;

typedef struct _UNIT
{
    UNITData data;
    _UNIT* link;

}UNITNode;

typedef struct MRGN
{
    int Value[5];
    double Loc[4];
    double CX, CY;
    BYTE Xdir, Ydir;
}MRGNData;

void MTXM_setSize(MTXMData*** CurMTXM, DWORD* CurMapX, DWORD* CurMapY, int X, int Y, THG2Data* CurTHG2, int THG2Num, UNITData* CurUNIT, int UNITNum, MRGNData* CurMRGN, int MRGNNum);
void MTXM_Trim(MTXMData*** CurMTXM, DWORD* CurMapX, DWORD* CurMapY, THG2Data* CurTHG2, int THG2Num, UNITData* CurUNIT, int UNITNum, MRGNData* CurMRGN, int MRGNNum);
void MTXM_Movemap(MTXMData*** CurMTXM, DWORD* CurMapX, DWORD* CurMapY, int X, int Y, THG2Data* CurTHG2, int THG2Num, UNITData* CurUNIT, int UNITNum, MRGNData* CurMRGN, int MRGNNum);
void MTXM_Apply(MTXMData*** CurMTXM, DWORD* CurMapX, DWORD* CurMapY, THG2Data* CurTHG2, int THG2Num, UNITData* CurUNIT, int UNITNum, MRGNData* CurMRGN, int MRGNNum);
int MTXM_GetData(MTXMData** MTXM, double j, double i, DWORD MapX, DWORD MapY, unsigned short* Value, unsigned* Index, BYTE* Type);
void ClearUNIT(UNITData*** UNITptr, int sizeY);
void ClearTHG2(THG2Data*** THG2ptr, int sizeY);
void ClearMTXM(MTXMData*** MTXMptr, int sizeY);

