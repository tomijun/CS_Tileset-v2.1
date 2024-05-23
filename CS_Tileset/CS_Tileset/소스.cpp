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
#include "MTXM.h"
#include "Parser.h"
#include "NonLinear.h"

typedef unsigned long long QWORD;
MPQHANDLE* Save1[1024];
FILE* Save2[1024];
int Save1ptr = 0;
int Save2ptr = 0;
#define PI 3.14159265

void f_ScloseAll(MPQHANDLE hMPQ)
{
    for (int i = 0; i < Save2ptr; i++)
        fclose(Save2[i]);
    for (int i = 0; i < Save1ptr; i++)
        SFileCloseFile(*Save1[i]);
    SFileCloseArchive(hMPQ); // MPQ 닫기
    Save1ptr = 0;
    Save2ptr = 0;
    return;
}

int f_Sopen(MPQHANDLE hMPQ, LPCSTR lpFileName, MPQHANDLE* hFile)
{
    // SFileOpenFileEx([MPQ핸들], [파일명], 0, &[리턴받을 파일핸들]);
    if (!SFileOpenFileEx(hMPQ, lpFileName, 0, hFile)) {
        printf("%s이(가) 존재하지 않습니다. [%d]\n", lpFileName, GetLastError());
        return -1;
    }
    Save1[Save1ptr++] = hFile;
}

int f_Scopy(MPQHANDLE hMPQ, MPQHANDLE* hFile, LPCSTR foutName, FILE** fout)
{
    fopen_s(fout, foutName, "w+b");
    if (*fout == NULL) {
        f_ScloseAll(hMPQ);
        printf("%s을 열 수 없습니다.\n", foutName);
        return -1;
    }
    Save2[Save2ptr++] = *fout;
    DWORD fsize = SFileGetFileSize(*hFile, NULL);
    char buffer[4096] = { 0 };
    while (fsize > 0) {
        DWORD transfersize = min(4096, fsize);
        DWORD readbyte;
        SFileReadFile(*hFile, buffer, transfersize, &readbyte, NULL); //fread에 해당
        if (readbyte != transfersize) {
            printf("SFileReadFile read %d bytes / %d bytes expected.\n", readbyte, transfersize);
            f_ScloseAll(hMPQ);
            return -1;
        }
        fwrite(buffer, 1, readbyte, *fout);
        fsize -= transfersize;
    }
}

int f_Fcopy(FILE** fin, FILE** fout, DWORD fsize)
{
    FILE* Fin = *fin;
    FILE* Fout = *fout;

    char buffer[4096] = { 0 };

    while (fsize > 0) {
        DWORD transfersize = min(4096, fsize);
        DWORD readbyte;
        readbyte = fread(buffer, transfersize, 1, Fin);
        if (readbyte != 1) {
            printf("ReadFile Failed.\n");
            return -1;
        }
        fwrite(buffer, transfersize, 1, Fout);
        fsize -= transfersize;
    }
}

void f_Swrite(MPQHANDLE hMPQ, LPCSTR finName, LPCSTR MPQName)
{
    MpqAddFileToArchive(hMPQ, finName, MPQName, MAFA_COMPRESS);
}

void f_SwriteWav(MPQHANDLE hMPQ, LPCSTR finName, LPCSTR MPQName)
{
    MpqAddWaveToArchive(hMPQ, finName, MPQName, MAFA_COMPRESS, MAWA_QUALITY_LOW);
}

int getTotalLine(FILE* fp) {
    int line = 0;
    char c;
    while ((c = fgetc(fp)) != EOF)
        if (c == '\n') line++;
    return(line);
}

int GetChkSection(FILE* fp, const char* Name, DWORD *Ret1, DWORD *Ret2)
{
    DWORD Key = Name[0] + Name[1] * 256 + Name[2] * 65536 + Name[3] * 16777216;
    int ret = 0, size;
    fseek(fp, 0, 2);
    size = ftell(fp);
    fseek(fp, 0, 0);

    DWORD Section[2];
    DWORD Check = 0;
    for (int i = 0; i < size;)
    {
        fseek(fp, i, 0);
        fread(Section, 4, 2, fp);
        if (Section[0] == Key)
        {
            *Ret1 = i;
            *Ret2 = i + (Section[1] + 8);
            Check = 1;
            break;
        }
        else
            i += (Section[1] + 8);
    }
    if (Check == 0)
        printf("맵의 %s단락을 찾을 수 없습니다.", Name);
    return(*Ret2) - (*Ret1);
}

extern double Matrix_A[3][3];
extern double Inverse_A[3][3];
extern int THG2Opt = 0, UNITOpt = 0, MRGNOpt = 0, RangeOpt = 0;
extern double CntrX = 0, CntrY = 0;
extern int SizeX = 0, SizeY = 0;

int main(int argc, char* argv[])
{
    FILE* fout, * lout;
    FILE* wout[1024];
    MPQHANDLE hMPQ;
    MPQHANDLE hFile;
    MPQHANDLE hList;
    MPQHANDLE hWav[1024];
    char* WavName[1024];
    int Wavptr = 0;

    // Open MPQ

    printf("--------------------------------------\n     。`+˚CS_Tileset v2.1 。+.˚\n--------------------------------------\n\t\t\tMade By Ninfia\n");

    char* input = argv[1];
    //Test
    //char input2[] = "1.scx";
    //if (argc == 1)
    //   input = input2;
    //Test

    
    if (argc == 1) // Selected No file
    {
        printf("선택된 파일이 없습니다.\n");
        system("pause");
        return 0;
    }
    

    char iname[512];
    strcpy_s(iname, 512, input);
    int ilength = strlen(iname) - 4;
    if (ilength < 0) return 0;
    strcpy_s(iname + ilength, 512 - ilength, "_out.scx");


    if (!SFileOpenArchive(input, 0, 0, &hMPQ)) {
        // SFileOpenArchive([파일명], 0, 0, &[리턴받을 MPQ핸들]);
        printf("SFileOpenArchive failed. [%d]\n", GetLastError());
        return -1;
    }
    printf("%s 의 MPQ 로드 완료\n", input);
    // Open Files
    f_Sopen(hMPQ, "(listfile)", &hList);
    f_Scopy(hMPQ, &hList, "(listfile).txt", &lout);
    printf("(listfile)을 불러와 맵 내부의 파일 목록을 읽는중\n");
    fseek(lout, 0, 0);
    char strTemp[512] = { 0 };
    int strLength, listline, line;
    listline = getTotalLine(lout);
    line = 0;
    fseek(lout, 0, 0);
    int chksize = 0;
    while (line < listline)
    {
        line++;
        fgets(strTemp, 512, lout);
        strLength = strlen(strTemp);
        strTemp[strLength - 2] = 0;
        if (!strcmp(strTemp, "staredit\\scenario.chk"))
        {
            f_Sopen(hMPQ, "staredit\\scenario.chk", &hFile);
            f_Scopy(hMPQ, &hFile, "scenario.chk", &fout);
            fseek(fout, 0, 2);
            chksize = ftell(fout);
            fseek(fout, 0, 0);
            fclose(fout);
            SFileCloseFile(hFile);
        }
        else
        {
            char* tmpBuffer = (char*)malloc(512);
            tmpnam_s(tmpBuffer, 512);
            WavName[Wavptr] = tmpBuffer;
            f_Sopen(hMPQ, strTemp, &hWav[Wavptr]);
            f_Scopy(hMPQ, &hWav[Wavptr], WavName[Wavptr], &wout[Wavptr]);
            fclose(wout[Wavptr]);
            SFileCloseFile(hWav[Wavptr]);
            Wavptr++;
        }

    }
    fclose(lout);
    SFileCloseFile(hList);
    SFileCloseArchive(hMPQ); // MPQ 닫기

    printf("scenario.chk %d bytes, 사운드 %d개 추출됨\n\n", chksize, Wavptr);


    DWORD JumpSize[10] = { 0 };
    fopen_s(&fout, "scenario.chk", "rb");
    FILE* fnew, * fMASK, * fUNIT, * fMRGN, * fDIM, * fTHG2, * fMTXM;
    fopen_s(&fnew, "scenario_new.chk", "wb");

    DWORD UNIToff1, UNIToff2, UNITsize = GetChkSection(fout, "UNIT", &UNIToff1, &UNIToff2);
    DWORD MRGNoff1, MRGNoff2, MRGNsize = GetChkSection(fout, "MRGN", &MRGNoff1, &MRGNoff2);
    DWORD DIMoff1, DIMoff2, DIMsize = GetChkSection(fout, "DIM ", &DIMoff1, &DIMoff2);
    DWORD THG2off1, THG2off2, THG2size = GetChkSection(fout, "THG2", &THG2off1, &THG2off2);
    DWORD MTXMoff1, MTXMoff2, MTXMsize = GetChkSection(fout, "MTXM", &MTXMoff1, &MTXMoff2);

    fopen_s(&fDIM, "DIM.chk", "w+b");
    fseek(fout, DIMoff1, 0);
    f_Fcopy(&fout, &fDIM, DIMsize);
    DWORD MapX = 0, MapY = 0, temp;
    fseek(fDIM, 8, 0);
    fread(&MapX, 2, 1, fDIM);
    fread(&MapY, 2, 1, fDIM);


    fopen_s(&fMTXM, "MTXM.chk", "w+b");
    fseek(fout, MTXMoff1, 0);
    f_Fcopy(&fout, &fMTXM, MTXMsize);
    int MTXMindex = 0;
    MTXMData** CurMTXM = (MTXMData**)malloc(MapY * sizeof(MTXMData*));
    for (int i = 0; i < MapY; i++)
        CurMTXM[i] = (MTXMData*)malloc(MapX * sizeof(MTXMData));

    fseek(fMTXM, 8, 0);
    for (int i = 0; i < MapY; i++)
    {
        for (int j = 0; j < MapX; j++)
        {
            unsigned short stemp;
            fread(&stemp, 2, 1, fMTXM);
            CurMTXM[i][j].Value = stemp;
            CurMTXM[i][j].Index = MTXMindex++;
            CurMTXM[i][j].Type = 2;
        }
    }

    fopen_s(&fTHG2, "THG2.chk", "w+b");
    fseek(fout, THG2off1, 0);
    f_Fcopy(&fout, &fTHG2, THG2size);
    fopen_s(&fUNIT, "UNIT.chk", "w+b");
    fseek(fout, UNIToff1, 0);
    f_Fcopy(&fout, &fUNIT, UNITsize);

    fopen_s(&fMRGN, "MRGN.chk", "w+b");
    fseek(fout, MRGNoff1, 0);
    f_Fcopy(&fout, &fMRGN, MRGNsize);


    // Patch MTXM
    printf("%d x %d 크기의 지형 로드됨\n", MapX, MapY);
    char Input[512] = { 0 };
    double op[10];
    DWORD nMapX = MapX, nMapY = MapY, CurMapX = MapX, CurMapY = MapY;
    SizeX = CurMapX, SizeY = CurMapY, CntrX = (CurMapX-1) / 2., CntrY = (CurMapY-1) / 2.;
    // Input Options
    int opt[10];
    do {
        printf("지형을 변형할때 설정할 옵션을 입력 (Help:도움말, 0:종료)\n");
        scanf_s("%[^\n]", Input, 512);  getchar();
        if (Input[0] == '0') break;
        ParseOpt(Input, opt);
        OptFunc list = (OptFunc)(opt[0]);
        int count = opt[9];
        switch (opt[0])
        {
        case MRGN:
            if (opt[9] < 1)
            {
                printf("입력된 함수의 인자 갯수가 부족합니다. 다시 입력하세요.\n\n");
                break;
            }
            else // Run Move func
            {
                if (opt[1] == 0)
                {
                    MRGNOpt = 0;
                    printf("→MRGN = 0 설정됨 (로케이션의 위치만 변경함)\n\n");
                }
                else
                {
                    MRGNOpt = 1;
                    printf("→MRGN = 1 설정됨 (로케이션의 위치와 크기 모두 변경함)\n\n");
                }
            }
            break;
        case UNIT:
            if (opt[9] < 1)
            {
                printf("입력된 함수의 인자 갯수가 부족합니다. 다시 입력하세요.\n\n");
                break;
            }
            else // Run Move func
            {
                if (opt[1] == 0)
                {
                    UNITOpt = 0;
                    printf("→UNIT = 0 설정됨 (배치된유닛의 위치만 변경함)\n\n");
                }
                else
                {
                    UNITOpt = 1;
                    printf("→UNIT = 1 설정됨 (배치된유닛의 위치와 갯수 모두 변경함)\n\n");
                }
            }
            break;
        case THG2:
            if (opt[9] < 1)
            {
                printf("입력된 함수의 인자 갯수가 부족합니다. 다시 입력하세요.\n\n");
                break;
            }
            else // Run Move func
            {
                if (opt[1] == 0)
                {
                    THG2Opt = 0;
                    printf("→THG2 = 0 설정됨 (스프라이트의 위치만 변경함)\n\n");
                }
                else
                {
                    THG2Opt = 1;
                    printf("→THG2 = 1 설정됨 (스프라이트의 위치와 갯수 모두 변경함)\n\n");
                }
            }
            break;
        case ALL:
            if (opt[9] < 1)
            {
                printf("입력된 함수의 인자 갯수가 부족합니다. 다시 입력하세요.\n\n");
                break;
            }
            else // Run Move func
            {
                if (opt[1] == 0)
                {
                    MRGNOpt = 0;
                    printf("→MRGN = 0 설정됨 (로케이션의 위치만 변경함)\n");
                    UNITOpt = 0;
                    printf("→UNIT = 0 설정됨 (배치된유닛의 위치만 변경함)\n");
                    THG2Opt = 0;
                    printf("→THG2 = 0 설정됨 (스프라이트의 위치만 변경함)\n\n");
                }
                else
                {
                    MRGNOpt = 1;
                    printf("→MRGN = 1 설정됨 (로케이션의 위치와 크기 모두 변경함)\n");
                    UNITOpt = 1;
                    printf("→UNIT = 1 설정됨 (배치된유닛의 위치와 갯수 모두 변경함)\n");
                    THG2Opt = 1;
                    printf("→THG2 = 1 설정됨 (스프라이트의 위치와 갯수 모두 변경함)\n\n");
                }
            }
            break;
        case HELP:
            printf("\n-------------------- CS_Tileset Option List ----------------------------------------------------------\n\n\t");
            printf("MRGN X  : 로케이션 출력모드를 설정함 (0:위치만 변경, 1:위치와 크기 모두 변경)\n\t");
            printf("UNIT X  : 배치된유닛 출력모드를 설정함 (0:위치만 변경, 1:타일갯수에 비례)\n\t");
            printf("THG2 X  : 스프라이트 출력모드를 설정함 (0:위치만 변경, 1:타일갯수에 비례)\n\t");
            printf("All X   : 모든 옵션을 한번에 입력값으로 설정함 (0:모두 0으로 설정, 1:모두 1로 설정)\n\t");
            printf("List    : 현재 설정된 옵션을 출력함\n\t");
            printf("Help    : 도움말을 출력함\n\t");
            printf("0       : [입력종료] 입력을 종료하고 설정된 옵션으로 지형편집을 시작함\n");
            printf("\n-------------------------------------------------------------------------------------------------------\n\n");
            break;
        case LIST:
            printf("\n-------------------- Current Option Setting ----------------------------------------------------------\n");
            (MRGNOpt == 0) ? (printf("\tMRGN = 0 (로케이션의 위치만 변경함)\n")) : (printf("\tMRGN = 1 (로케이션의 위치와 크기 모두 변경함)\n"));
            (UNITOpt == 0) ? (printf("\tUNIT = 0 (배치된유닛의 위치만 변경함)\n")) : (printf("\tUNIT = 1 (배치된유닛의 위치와 갯수 모두 변경함)\n"));
            (THG2Opt == 0) ? (printf("\tTHG2 = 0 (스프라이트의 위치만 변경함)\n")) : (printf("\tTHG2 = 1 (스프라이트의 위치와 갯수 모두 변경함)\n"));
            printf("-------------------------------------------------------------------------------------------------------\n\n");
            break;
        case Err: printf("잘못된 명령어입니다. 다시 입력하세요.\n");
            break;
        }
        list = ERR;
    } while (1);


    THG2Data* CurTHG2 = NULL;
    THG2Node*** THG2Map = NULL;
    int THG2Num = (THG2size - 8) / 10;

    if (THG2Opt == 1)
    {
        THG2Map = (THG2Node***)malloc(MapY * sizeof(THG2Node**));
        for (int i = 0; i < MapY; i++)
            THG2Map[i] = (THG2Node**)malloc(MapX * sizeof(THG2Node*));

        for (int i = 0; i < MapY; i++)
        {
            for (int j = 0; j < MapX; j++)
            {
                THG2Map[i][j] = NULL;
            }
        }

        fseek(fTHG2, 8, 0);
        int X, Y;
        unsigned short stemp[5];
        for (int i = 8; i < THG2size; i += 10)
        {
            fread(&stemp, 2, 5, fTHG2);
            X = stemp[1] / 32; Y = stemp[2] / 32;
            if (X >= 0 && X < MapX && Y >= 0 && Y < MapY) // Push
            {
                THG2Node* temp = (THG2Node*)malloc(sizeof(THG2Node));
                for (int j = 0; j < 5; j++)
                    temp->data.Value[j] = stemp[j];
                temp->data.dx = stemp[1] % 32;
                temp->data.dy = stemp[2] % 32;
                temp->link = THG2Map[Y][X];
                THG2Map[Y][X] = temp;
            }
        }
    }
    else
    {
        CurTHG2 = (THG2Data*)malloc(THG2Num * sizeof(THG2Data));
        if (CurTHG2 == NULL) printf("THG2 할당오류\a\n");
        fseek(fTHG2, 8, 0);
        int count = 0;
        unsigned short stemp[5];
        for (int i = 8; i < THG2size; i += 10)
        {
            fread(&stemp, 2, 5, fTHG2);
            for (int j = 0; j < 5; j++)
                CurTHG2[count].Value[j] = stemp[j];
            CurTHG2[count].X = (double)stemp[1] / 32;
            CurTHG2[count].Y = (double)stemp[2] / 32;
            count++;
        }
    }

    UNITData* CurUNIT = NULL;
    UNITNode*** UNITMap = NULL;
    int UNITNum = (UNITsize - 8) / 36;

    if (UNITOpt == 1)
    {
        UNITMap = (UNITNode***)malloc(MapY * sizeof(UNITNode**));
        for (int i = 0; i < MapY; i++)
            UNITMap[i] = (UNITNode**)malloc(MapX * sizeof(UNITNode*));

        for (int i = 0; i < MapY; i++)
        {
            for (int j = 0; j < MapX; j++)
            {
                UNITMap[i][j] = NULL;
            }
        }

        fseek(fUNIT, 8, 0);
        int X, Y;
        unsigned short stemp[18];
        for (int i = 8; i < UNITsize; i += 36)
        {
            fread(&stemp, 2, 18, fUNIT);
            X = stemp[2] / 32; Y = stemp[3] / 32;
            if (X >= 0 && X < MapX && Y >= 0 && Y < MapY) // Push
            {
                UNITNode* temp = (UNITNode*)malloc(sizeof(UNITNode));
                for (int j = 0; j < 18; j++)
                    temp->data.Value[j] = stemp[j];
                temp->data.dx = stemp[2] % 32;
                temp->data.dy = stemp[3] % 32;
                temp->link = UNITMap[Y][X];
                UNITMap[Y][X] = temp;
            }
        }
    }
    if (TRUE)
    {
        CurUNIT = (UNITData*)malloc(UNITNum * sizeof(UNITData));
        if (CurUNIT == NULL) printf("UNIT 할당오류\a\n");
        fseek(fUNIT, 8, 0);
        int count = 0;
        unsigned short stemp[18];
        for (int i = 8; i < UNITsize; i += 36)
        {
            fread(&stemp, 2, 18, fUNIT);
            for (int j = 0; j < 18; j++)
                CurUNIT[count].Value[j] = stemp[j];
            CurUNIT[count].X = (double)stemp[2] / 32;
            CurUNIT[count].Y = (double)stemp[3] / 32;
            count++;
        }
    }


    FILE* fMRGN2 = NULL;
    MRGNData* CurMRGN = NULL;
    int MRGNNum = (MRGNsize - 8) / 20;
    CurMRGN = (MRGNData*)malloc(MRGNNum * sizeof(MRGNData));
    if (CurMRGN == NULL) printf("MRGN 할당오류\a\n");
    fseek(fMRGN, 8, 0);
    if (MRGNOpt == 1)
    {
        int count = 0;
        int stemp[5];
        for (int i = 8; i < MRGNsize; i += 20)
        {
            fread(&stemp, 4, 5, fMRGN);
            for (int j = 0; j < 5; j++)
                CurMRGN[count].Value[j] = stemp[j];
            int CX = (stemp[0] + stemp[2]) / 2;
            int CY = (stemp[1] + stemp[3]) / 2;
            CurMRGN[count].Loc[0] = CY;
            CurMRGN[count].Loc[1] = CX;
            CurMRGN[count].Loc[2] = CY;
            CurMRGN[count].Loc[3] = CX;
            CurMRGN[count].CX = CX;
            CurMRGN[count].CY = CY;
            if (stemp[0] <= stemp[2])
                CurMRGN[count].Xdir = 0;
            else
                CurMRGN[count].Xdir = 1;
            if (stemp[1] <= stemp[3])
                CurMRGN[count].Ydir = 0;
            else
                CurMRGN[count].Ydir = 1;
            count++;
        }
    }
    else
    {
        int count = 0;
        int stemp[5];
        for (int i = 8; i < MRGNsize; i += 20)
        {
            fread(&stemp, 4, 5, fMRGN);
            for (int j = 0; j < 5; j++)
                CurMRGN[count].Value[j] = stemp[j];
            int CX = (stemp[0] + stemp[2]) / 2;
            int CY = (stemp[1] + stemp[3]) / 2;
            CurMRGN[count].Loc[0] = stemp[0] - CX;
            CurMRGN[count].Loc[1] = stemp[1] - CY;
            CurMRGN[count].Loc[2] = stemp[2] - CX;
            CurMRGN[count].Loc[3] = stemp[3] - CY;
            CurMRGN[count].CX = CX;
            CurMRGN[count].CY = CY; 
            if (stemp[0] <= stemp[2])
                CurMRGN[count].Xdir = 0;
            else
                CurMRGN[count].Xdir = 1;
            if (stemp[1] <= stemp[3])
                CurMRGN[count].Ydir = 0;
            else
                CurMRGN[count].Ydir = 1;
            count++;
        }
    }

    int lock = 0;
    do
    {
        printf("지형에 적용할 효과를 입력 (Help:도움말, 0:종료)\n");
        scanf_s("%[^\n]", Input, 512);  getchar();
        if (Input[0] == '0') break;
        ParseMsg(Input, op);
        TransFunc list = (TransFunc)((int)op[0]);
        int count = (int)op[9];
        switch (list)
        {
        case Err: printf("잘못된 명령어입니다. 다시 입력하세요.\n");
            break;
        case Help: printf("-------------------- Linear Transform List (변형적용시 반드시 Apply를 사용해야함) --------------------\n\n\t");
            printf("Move X Y      : 지형을 평행이동시킴 (X:가로이동, Y:세로이동)\n\t");
            printf("Scale X Y     : 지형의 크기를 변경함 (X:가로배율, Y:세로배율)\n\t");
            printf("setScale X Y  : 지형의 크기를 입력값으로 변경함 (X:가로크기, Y:세로크기)\n\t");
            printf("Reflect X Y   : 지형을 축방향으로 뒤집음 (X:1입력시 X축으로 뒤집음, Y:1입력시 Y축으로 뒤집음 | 미사용시 0입력)\n\t");
            printf("Rotate Θ X Y : 지형을 반시계방향으로 돌림 (Θ:회전각도 | (선택) X:회전중심 X좌표, Y:회전중심 Y좌표)\n\t");
            printf("Shear X Y     : 지형을 축방향으로 기울임 (X:X축으로 기울일정도, Y:Y축으로 기울일정도)\n\t");
            printf("Project LUX LUY LDX LDY RUX RUY RDX RDY : 지형을 선형으로 왜곡함 (각각 4꼭짓점(↖↙↗↘)의 좌표입력)\n");
            printf("\n-------------------- NonLinear Transform List (사용시 즉시 변형적용됨) -------------------------------\n\n\t");
            printf("Wave k1 k2 t1 t2          :X' = X+k1*Sin(2π(Y-Cx)/t1), Y' = Y+k2*Cos(2π(X-Cy)/t2)\n\t\t\t\t   t1,t2:삼각함수의 주기, k1,k2:삼각함수의 진폭 (2πk<t를 권장함)\n\t");
            printf("Lens kx ky tx ty          :Z' = Z+k*(Z-C)*exp(-1/t*(abs(Z-C)^2)\n\t\t\t\t   k:렌즈의 세기(-:오목, +:볼록), t:렌즈의 크기 (t≥0, 32이상 권장)\n\t");
            printf("Gravity kx ky tx ty       :Z = Z'+k*(Z'-C)*exp(-1/t*(abs(Z'-C)^2)\n\t\t\t\t   k:중력의 세기(-:반중력, +:중력), t:중력범위의 크기 (t≥0, 32이상 권장)\n\t");
            printf("Window kx ky tx ty        :X = kx*tan(π*(X'-Cx)/tx), Y = ky*tan(π*(Y'-Cy)/ty)\n\t\t\t\t   kx:x축배율, ky:y축배율, tx:x축블럭크기, ty:y축블럭크기 (배율이 음수면 반사효과)\n\t");
            printf("Pinch i sx sy kx ky tx ty :Z(') = Z(')*(1+k*exp(-1/t*(abs(Z(')-C)^2))\n\t\t   i:(0:오목, 1:볼록), s:왜곡방향원점, k:왜곡의 세기(k≥0 권장), t:왜곡범위의 크기 (t≥0, 32이상 권장)\n\t");
            printf("Squish i nx ny kx ky tx ty:입력한 각도 방향으로 왜곡을 적용함 i:(0:볼록, 1:오목)\n\t\t   n:왜곡방향(-90~90), k:왜곡의 세기(k≥0 권장), t:왜곡범위의 크기 (t≥0, 크기가 클수록 작아짐)\n\t");
            printf("Swirl kx ky tx ty         :R' = R, Θ' = Θ+k*exp(-R/t)\n\t\t\t\t   k:소용돌이의 세기(-:반시계, +:시계), t:소용돌이 범위 (t≥0, 8이상 권장)\n\t");
            printf("Sphere Θ rx ry           :평면에 구면왜곡 효과를 적용함 (반지름 바깥은 효과적용X)\n\t\t\t\t   Θ:내부영역 회전각도, rx:x축 반지름, ry:y축 반지름 (r≥0필수)\n\t");
            printf("Saddle Θ rx ry           :평면에 쌍곡면왜곡 효과를 적용함 (반지름 바깥은 효과적용X)\n\t\t\t\t   Θ:내부영역 회전각도, rx:x축 반지름, ry:y축 반지름 (r≥0필수)\n\t");
            printf("Cylinder i k r            :평면에 원통왜곡 효과를 적용함 (반지름 바깥은 효과적용X)\n\t\t\t\t   i:원통의 방향(0:세로, 1:가로), k:왜곡의 세기 r:반지름 (r≥0필수)\n\t");
            printf("\n-------------------- Map Edit Tool List --------------------------------------------------------------\n\n\t");
            printf("Range         : [Linear Transform 전용] 변형 적용시 경계선 설정방식 설정 (0:0~≤Size-1, 1:0~＜Size)\n\t");
            printf("Apply         : [Linear Transform 전용] 입력한 변형을 적용함\n\t");
            printf("Cancel        : [Linear Transform 전용] 입력한 변형을 취소함 (Apply 적용전까지 취소됨)\n\t");
            printf("Cntr Cx Cy    : [NonLinear Transform 전용] 비선형변환시 변형의 중심좌표를 설정함 (Cx:중심 X좌표, Cy:중심 Y좌표)\n\t");
            printf("Size X Y      : [NonLinear Transform 전용] 비선형변환시 변형 후의 지형크기를 설정함 (X:가로크기, Y:세로크기)\n\t");
            printf("setSize X Y   : 배경지형의 크기를 설정함 (X:가로크기, Y:세로크기 | 현재 지형크기보다 작게 설정 불가)\n\t");
            printf("Trim          : 상하좌우 공백지형을 삭제함\n\t");
            printf("Help          : 도움말을 출력함\n\t");
            printf("Movemap X Y   : 맵 지형을 평행이동시킴 (X:가로이동, Y:세로이동) ※맵 바깥으로 나간 지형은 잘려나감\n\t");
            printf("0             : [편집종료] 입력을 종료하고 변형된 지형으로 맵파일을 생성함\n");
            printf("\n-------------------------------------------------------------------------------------------------------\n\n");
            break;
        case Cancel:
            for (int i = 0; i < 3; i++)
                for (int j = 0; j < 3; j++)
                    if (i == j)
                    {
                        Matrix_A[i][j] = 1;
                        Inverse_A[i][j] = 1;
                    }
                    else
                    {
                        Matrix_A[i][j] = 0;
                        Inverse_A[i][j] = 0;
                    }
            nMapX = CurMapX; nMapY = CurMapY;
            printf("입력된 변환수식을 초기화했습니다. (현재 맵크기 %dx%d)\n\n", nMapX, nMapY);
            lock = 0;
            break;
        case Move:
            if (op[9] < 2)
            {
                printf("입력된 함수의 인자 갯수가 부족합니다. 다시 입력하세요.\n\n");
                break;
            }
            else // Run Move func
            {
                int ret = Stack_Move(op[1], op[2], &nMapX, &nMapY, CurMapX, CurMapY);
                if (ret == -1)
                    printf("입력된 변환 작업을 수행할수 없습니다. (역함수 존재X)\n\n");
                else
                {
                    lock = 1;
                    printf("→Move X:%.3lf Y:%.3lf (적용후 맵크기 %dx%d)\n\n", op[1], op[2], nMapX, nMapY);
                }
            }
            break;
        case Scale:
            if (op[9] < 2)
            {
                printf("입력된 함수의 인자 갯수가 부족합니다. 다시 입력하세요.\n\n");
                break;
            }
            else // Run Scale func
            {
                int ret = Stack_Scale(op[1], op[2], &nMapX, &nMapY, CurMapX, CurMapY);
                if (ret == -1)
                    printf("입력된 변환 작업을 수행할수 없습니다. (역함수 존재X)\n\n");
                else
                {
                    lock = 1;
                    printf("→Scale X:x%.3lf Y:x%.3lf (적용후 맵크기 %dx%d)\n\n", op[1], op[2], nMapX, nMapY);
                }
            }
            break;
        case setScale:
            if (op[9] < 2)
            {
                printf("입력된 함수의 인자 갯수가 부족합니다. 다시 입력하세요.\n\n");
                break;
            }
            else // Run Scale func
            {
                int ret = Stack_setScale((int)op[1], (int)op[2], &nMapX, &nMapY, CurMapX, CurMapY);
                if (ret == -1)
                    printf("입력된 변환 작업을 수행할수 없습니다. (역함수 존재X)\n\n");
                else
                {
                    lock = 1;
                    printf("→setScale X:%d Y:%d (적용후 맵크기 %dx%d)\n\n", (int)op[1], (int)op[2], nMapX, nMapY);
                }
            }
            break;
        case Reflect:
            if (op[9] < 2)
            {
                printf("입력된 함수의 인자 갯수가 부족합니다. 다시 입력하세요.\n\n");
                break;
            }
            else // Run Scale func
            {
                int ret = Stack_Reflect(op[1], op[2], &nMapX, &nMapY, CurMapX, CurMapY);
                if (ret == -1)
                    printf("입력된 변환 작업을 수행할수 없습니다. (역함수 존재X)\n\n");
                else
                {
                    const char* op1, * op2;
                    if (abs(op[1]) != 0) op1 = "TRUE";
                    else op1 = "FALSE";
                    if (abs(op[2]) != 0) op2 = "TRUE";
                    else op2 = "FALSE";
                    printf("→Reflect X:%s Y:%s (적용후 맵크기 %dx%d)\n\n", op1, op2, nMapX, nMapY);
                    lock = 1;
                }
            }
            break;
        case Rotate:
            if (op[9] < 1 || op[9] == 2)
            {
                printf("입력된 함수의 인자 갯수가 부족합니다. 다시 입력하세요.\n\n");
                break;
            }
            else if (op[9] == 1)// Run Scale func
            {
                int ret = Stack_Rotate(op[1], &nMapX, &nMapY, CurMapX, CurMapY);
                if (ret == -1)
                    printf("입력된 변환 작업을 수행할수 없습니다. (역함수 존재X)\n\n");
                else
                {
                    lock = 1;
                    printf("→Rotate Angle:%.3lf˚(Center) (적용후 맵크기 %dx%d)\n\n", op[1], nMapX, nMapY);
                }
            }
            else
            {
                int ret = Stack_RotateAt(op[1], op[2], op[3], &nMapX, &nMapY, CurMapX, CurMapY);
                if (ret == -1)
                    printf("입력된 변환 작업을 수행할수 없습니다. (역함수 존재X)\n\n");
                else
                {
                    lock = 1;
                    printf("→Rotate Angle:%.3lf˚(%.2lf,%.2lf) (적용후 맵크기 %dx%d)\n\n", op[1], op[2], op[3], nMapX, nMapY);
                }
            }
            break;
        case Shear:
            if (op[9] < 2)
            {
                printf("입력된 함수의 인자 갯수가 부족합니다. 다시 입력하세요.\n\n");
                break;
            }
            else // Run Scale func
            {
                int ret = Stack_Shear(op[1], op[2], &nMapX, &nMapY, CurMapX, CurMapY);
                if (ret == -1)
                    printf("입력된 변환 작업을 수행할수 없습니다. (역함수 존재X)\n\n");
                else
                {
                    lock = 1;
                    printf("→Shear X:→x%.3lf Y:↑x%.3lf (적용후 맵크기 %dx%d)\n\n", op[1], op[2], nMapX, nMapY);
                }
            }
            break;
        case Affine:
            if (op[9] < 8)
            {
                printf("입력된 함수의 인자 갯수가 부족합니다. 다시 입력하세요.\n\n");
                break;
            }
            else // Run Scale func
            {
                int ret = Stack_Affine(op[1], op[2], op[3], op[4], op[5], op[6], op[7], op[8], &nMapX, &nMapY, CurMapX, CurMapY);
                if (ret == -1)
                    printf("입력된 변환 작업을 수행할수 없습니다. (역함수 존재X)\n\n");
                else
                {
                    lock = 1;
                    printf("→Project LU:(%.2lf,%.2lf), LD:(%.2lf,%.2lf), RU:(%.2lf,%.2lf), RD:(%.2lf,%.2lf) (적용후 맵크기 %dx%d)\n\n", op[1], op[2], op[3], op[4], op[5], op[6], op[7], op[8], nMapX, nMapY);
                }
            }
            break;
        case Range:
            if (op[9] < 1)
            {
                printf("입력된 함수의 인자 갯수가 부족합니다. 다시 입력하세요.\n\n");
                break;
            }
            else if (lock == 1)
            {
                printf("선형변환 적용 전까지 Range 옵션을 바꿀 수 없습니다.\n\n");
                break;
            }
            else 
            {
                if ((int)op[1] != 0)
                    RangeOpt = 1;
                else
                    RangeOpt = 0;
                if (RangeOpt == 0)
                    printf("→Range %d : 변형 적용시 꼭짓점을 경계선으로 설정합니다. (0~≤Size-1)\n\n", RangeOpt);
                else
                    printf("→Range %d : 변형 적용시 맵크기를 경계선으로 설정합니다. (0~＜Size)\n\n", RangeOpt);
            }
            break;
        case Apply:
            MTXM_Apply(& CurMTXM, &CurMapX, &CurMapY, CurTHG2, THG2Num, CurUNIT, UNITNum, CurMRGN, MRGNNum);
            for (int i = 0; i < 3; i++)
                for (int j = 0; j < 3; j++)
                    if (i == j)
                    {
                        Matrix_A[i][j] = 1;
                        Inverse_A[i][j] = 1;
                    }
                    else
                    {
                        Matrix_A[i][j] = 0;
                        Inverse_A[i][j] = 0;
                    }
            nMapX = CurMapX; nMapY = CurMapY;
            SizeX = CurMapX, SizeY = CurMapY, CntrX = (CurMapX-1) / 2., CntrY = (CurMapY-1) / 2.;
            printf("→Apply Linear Transform [%d] : 입력된 수식으로 지형이 변환되었습니다. (적용후 맵크기 %dx%d)\n\n", RangeOpt, nMapX, nMapY);
            lock = 0;
            break;
        case Trim:
            MTXM_Trim(&CurMTXM, &CurMapX, &CurMapY, CurTHG2, THG2Num, CurUNIT, UNITNum, CurMRGN, MRGNNum);
            for (int i = 0; i < 3; i++)
                for (int j = 0; j < 3; j++)
                    if (i == j)
                    {
                        Matrix_A[i][j] = 1;
                        Inverse_A[i][j] = 1;
                    }
                    else
                    {
                        Matrix_A[i][j] = 0;
                        Inverse_A[i][j] = 0;
                    }
            nMapX = CurMapX; nMapY = CurMapY;
            SizeX = CurMapX, SizeY = CurMapY, CntrX = (CurMapX-1) / 2., CntrY = (CurMapY-1) / 2.;
            printf("→Trim : 지형의 여백이 모두 제거되었습니다. (적용후 맵크기 %dx%d)\n\n", nMapX, nMapY);
            lock = 0;
            break;
        case setSize:
            if (op[9] < 2)
            {
                printf("입력된 함수의 인자 갯수가 부족합니다. 다시 입력하세요.\n\n");
                break;
            }
            else // Run Scale func
            {
                int param1 = (int)op[1], param2 = (int)op[2];
                if (op[1] < CurMapX || op[2] < CurMapY)
                    printf("입력된 변환 작업을 수행할수 없습니다. (배경크기가 맵크기보다 작을 수 없음)\n\n");
                else
                {
                    MTXM_setSize(&CurMTXM, &CurMapX, &CurMapY, param1, param2, CurTHG2, THG2Num, CurUNIT, UNITNum, CurMRGN, MRGNNum);
                    for (int i = 0; i < 3; i++)
                        for (int j = 0; j < 3; j++)
                            if (i == j)
                            {
                                Matrix_A[i][j] = 1;
                                Inverse_A[i][j] = 1;
                            }
                            else
                            {
                                Matrix_A[i][j] = 0;
                                Inverse_A[i][j] = 0;
                            }
                    nMapX = CurMapX; nMapY = CurMapY;
                    SizeX = CurMapX, SizeY = CurMapY, CntrX = (CurMapX-1) / 2., CntrY = (CurMapY-1) / 2.;
                    printf("→setSize X:%d Y:%d : 배경지형의 크기가 변경되었습니다. (적용후 맵크기 %dx%d)\n\n", param1, param2, nMapX, nMapY);
                    lock = 0;
                }
            }
            break;
        case Movemap:
            if (op[9] < 2)
            {
                printf("입력된 함수의 인자 갯수가 부족합니다. 다시 입력하세요.\n\n");
                break;
            }
            else // Run Move func
            {
                int param1 = (int)op[1], param2 = (int)op[2];
                MTXM_Movemap(&CurMTXM, &CurMapX, &CurMapY, param1, param2, CurTHG2, THG2Num, CurUNIT, UNITNum, CurMRGN, MRGNNum);
                for (int i = 0; i < 3; i++)
                    for (int j = 0; j < 3; j++)
                        if (i == j)
                        {
                            Matrix_A[i][j] = 1;
                            Inverse_A[i][j] = 1;
                        }
                        else
                        {
                            Matrix_A[i][j] = 0;
                            Inverse_A[i][j] = 0;
                        }
                nMapX = CurMapX; nMapY = CurMapY;
                SizeX = CurMapX, SizeY = CurMapY, CntrX = (CurMapX - 1) / 2., CntrY = (CurMapY - 1) / 2.;
                printf("→Movemap X:%d Y:%d : 배경지형의 위치가 변경되었습니다. (적용후 맵크기 %dx%d)\n\n", param1, param2, nMapX, nMapY);
                lock = 0;
            } 
            break;
        case Cntr:
            if (op[9] < 2)
            {
                printf("입력된 함수의 인자 갯수가 부족합니다. 다시 입력하세요.\n\n");
                break;
            }
            else // Run Scale func
            {
                printf("→Cntr X:%.3lf Y:%.3lf : 비선형변환의 중심좌표가 설정되었습니다.\n\n", op[1], op[2]);
                CntrX = op[1]; CntrY = op[2];
            }
            break;
        case Size:
            if (op[9] < 2)
            {
                printf("입력된 함수의 인자 갯수가 부족합니다. 다시 입력하세요.\n\n");
                break;
            }
            else // Run Scale func
            {
                printf("→Size X:%d Y:%d : 비선형변환 적용후 맵크기가 설정되었습니다.\n\n", (int)op[1], (int)op[2]);
                SizeX = (int)op[1]; SizeY = (int)op[2];
            }
            break;
        case Wave:
            if (op[9] < 4)
            {
                printf("입력된 함수의 인자 갯수가 부족합니다. 다시 입력하세요.\n\n");
                break;
            }
            else // Run Scale func
            {
                NonLinear(op, &CurMTXM, &CurMapX, &CurMapY, CurTHG2, THG2Num, CurUNIT, UNITNum, CurMRGN, MRGNNum);
                for (int i = 0; i < 3; i++)
                    for (int j = 0; j < 3; j++)
                        if (i == j)
                        {
                            Matrix_A[i][j] = 1;
                            Inverse_A[i][j] = 1;
                        }
                        else
                        {
                            Matrix_A[i][j] = 0;
                            Inverse_A[i][j] = 0;
                        }
                nMapX = CurMapX; nMapY = CurMapY;
                printf("→Wave : x:%.2lf y:%.2lf k1:%.2lf k2:%.2lf t1:%.2lf t2:%.2lf (적용후 맵크기 %dx%d)\n\n", CntrX, CntrY, op[1], op[2], op[3], op[4], nMapX, nMapY);
                SizeX = CurMapX, SizeY = CurMapY, CntrX = (CurMapX-1) / 2., CntrY = (CurMapY-1) / 2.;
                lock = 0;
                break;
            }
        case Lens:
            if (op[9] < 4)
            {
                printf("입력된 함수의 인자 갯수가 부족합니다. 다시 입력하세요.\n\n");
                break;
            }
            else // Run Scale func
            {
                NonLinear(op, &CurMTXM, &CurMapX, &CurMapY, CurTHG2, THG2Num, CurUNIT, UNITNum, CurMRGN, MRGNNum);
                for (int i = 0; i < 3; i++)
                    for (int j = 0; j < 3; j++)
                        if (i == j)
                        {
                            Matrix_A[i][j] = 1;
                            Inverse_A[i][j] = 1;
                        }
                        else
                        {
                            Matrix_A[i][j] = 0;
                            Inverse_A[i][j] = 0;
                        }
                nMapX = CurMapX; nMapY = CurMapY;
                char sign = (op[1] == 0) ? ('-') : ('+');
                printf("→Lens : x:%.2lf y:%.2lf kx:%.2lf ky:%.2lf tx:%.2lf ty:%.2lf (적용후 맵크기 %dx%d)\n\n", CntrX, CntrY, op[1], op[2],op[3],op[4], nMapX, nMapY);
                SizeX = CurMapX, SizeY = CurMapY, CntrX = (CurMapX-1) / 2., CntrY = (CurMapY-1) / 2.;
                lock = 0;
                break;
            }
        case Gravity:
            if (op[9] < 4)
            {
                printf("입력된 함수의 인자 갯수가 부족합니다. 다시 입력하세요.\n\n");
                break;
            }
            else // Run Scale func
            {
                NonLinear(op, &CurMTXM, &CurMapX, &CurMapY, CurTHG2, THG2Num, CurUNIT, UNITNum, CurMRGN, MRGNNum);
                for (int i = 0; i < 3; i++)
                    for (int j = 0; j < 3; j++)
                        if (i == j)
                        {
                            Matrix_A[i][j] = 1;
                            Inverse_A[i][j] = 1;
                        }
                        else
                        {
                            Matrix_A[i][j] = 0;
                            Inverse_A[i][j] = 0;
                        }
                nMapX = CurMapX; nMapY = CurMapY;
                char sign = (op[1] == 0) ? ('-') : ('+');
                printf("→Gravity : x:%.2lf y:%.2lf kx:%.2lf ky:%.2lf tx:%.2lf ty:%.2lf (적용후 맵크기 %dx%d)\n\n", CntrX, CntrY, op[1], op[2], op[3], op[4], nMapX, nMapY);
                SizeX = CurMapX, SizeY = CurMapY, CntrX = (CurMapX-1) / 2., CntrY = (CurMapY-1) / 2.;
                lock = 0;
                break;
            }
        case Window:
            if (op[9] < 4)
            {
                printf("입력된 함수의 인자 갯수가 부족합니다. 다시 입력하세요.\n\n");
                break;
            }
            else // Run Scale func
            {
                NonLinear(op, &CurMTXM, &CurMapX, &CurMapY, CurTHG2, THG2Num, CurUNIT, UNITNum, CurMRGN, MRGNNum);
                for (int i = 0; i < 3; i++)
                    for (int j = 0; j < 3; j++)
                        if (i == j)
                        {
                            Matrix_A[i][j] = 1;
                            Inverse_A[i][j] = 1;
                        }
                        else
                        {
                            Matrix_A[i][j] = 0;
                            Inverse_A[i][j] = 0;
                        }
                nMapX = CurMapX; nMapY = CurMapY;
                printf("→Window : x:%.2lf y:%.2lf kx:%.2lf ky:%.2lf tx:%.2lf ty:%.2lf (적용후 맵크기 %dx%d)\n\n", CntrX, CntrY, op[1], op[2], op[3], op[4], nMapX, nMapY);
                SizeX = CurMapX, SizeY = CurMapY, CntrX = (CurMapX-1) / 2., CntrY = (CurMapY-1) / 2.;
                lock = 0;
                break;
            }
        case Pinch:
            if (op[9] < 7)
            {
                printf("입력된 함수의 인자 갯수가 부족합니다. 다시 입력하세요.\n\n");
                break;
            }
            else // Run Scale func
            {
                if ((int)op[1] != 0)
                    op[0] = iPinch;
                NonLinear(op, &CurMTXM, &CurMapX, &CurMapY, CurTHG2, THG2Num, CurUNIT, UNITNum, CurMRGN, MRGNNum);
                for (int i = 0; i < 3; i++)
                    for (int j = 0; j < 3; j++)
                        if (i == j)
                        {
                            Matrix_A[i][j] = 1;
                            Inverse_A[i][j] = 1;
                        }
                        else
                        {
                            Matrix_A[i][j] = 0;
                            Inverse_A[i][j] = 0;
                        }
                nMapX = CurMapX; nMapY = CurMapY;
                printf("→Pinch : x:%.2lf y:%.2lf i:%d sx:%.2lf sy:%.2lf kx:%.2lf ky:%.2lf tx:%.2lf ty:%.2lf (적용후 맵크기 %dx%d)\n\n", CntrX, CntrY, (int)op[1], op[2], op[3], op[4], op[5], op[6], op[7], nMapX, nMapY);
                SizeX = CurMapX, SizeY = CurMapY, CntrX = (CurMapX-1) / 2., CntrY = (CurMapY-1) / 2.;
                lock = 0;
                break;
            }
        case Squish:
            if (op[9] < 7)
            {
                printf("입력된 함수의 인자 갯수가 부족합니다. 다시 입력하세요.\n\n");
                break;
            }
            else // Run Scale func
            {
                if ((int)op[1] != 0)
                    op[0] = iSquish;
                NonLinear(op, &CurMTXM, &CurMapX, &CurMapY, CurTHG2, THG2Num, CurUNIT, UNITNum, CurMRGN, MRGNNum);
                for (int i = 0; i < 3; i++)
                    for (int j = 0; j < 3; j++)
                        if (i == j)
                        {
                            Matrix_A[i][j] = 1;
                            Inverse_A[i][j] = 1;
                        }
                        else
                        {
                            Matrix_A[i][j] = 0;
                            Inverse_A[i][j] = 0;
                        }
                nMapX = CurMapX; nMapY = CurMapY;
                printf("→Squish : x:%.2lf y:%.2lf i:%d nx:%.2lf ny:%.2lf kx:%.2lf ky:%.2lf tx:%.2lf ty:%.2lf (적용후 맵크기 %dx%d)\n\n", CntrX, CntrY, (int)op[1], op[2], op[3], op[4], op[5], op[6], op[7], nMapX, nMapY);
                SizeX = CurMapX, SizeY = CurMapY, CntrX = (CurMapX-1) / 2., CntrY = (CurMapY-1) / 2.;
                lock = 0;
                break;
            }
        case Swirl:
            if (op[9] < 4)
            {
                printf("입력된 함수의 인자 갯수가 부족합니다. 다시 입력하세요.\n\n");
                break;
            }
            else // Run Scale func
            {
                NonLinear(op, &CurMTXM, &CurMapX, &CurMapY, CurTHG2, THG2Num, CurUNIT, UNITNum, CurMRGN, MRGNNum);
                for (int i = 0; i < 3; i++)
                    for (int j = 0; j < 3; j++)
                        if (i == j)
                        {
                            Matrix_A[i][j] = 1;
                            Inverse_A[i][j] = 1;
                        }
                        else
                        {
                            Matrix_A[i][j] = 0;
                            Inverse_A[i][j] = 0;
                        }
                nMapX = CurMapX; nMapY = CurMapY;
                printf("→Swirl : x:%.2lf y:%.2lf kx:%.2lf ky:%.2lf tx:%.2lf ty:%.2lf (적용후 맵크기 %dx%d)\n\n", CntrX, CntrY, op[1], op[2], op[3], op[4], nMapX, nMapY);
                SizeX = CurMapX, SizeY = CurMapY, CntrX = (CurMapX-1) / 2., CntrY = (CurMapY-1) / 2.;
                lock = 0;
                break;
            }
        case Sphere:
            if (op[9] < 3)
            {
                printf("입력된 함수의 인자 갯수가 부족합니다. 다시 입력하세요.\n\n");
                break;
            }
            else // Run Scale func
            {
                NonLinear(op, &CurMTXM, &CurMapX, &CurMapY, CurTHG2, THG2Num, CurUNIT, UNITNum, CurMRGN, MRGNNum);
                for (int i = 0; i < 3; i++)
                    for (int j = 0; j < 3; j++)
                        if (i == j)
                        {
                            Matrix_A[i][j] = 1;
                            Inverse_A[i][j] = 1;
                        }
                        else
                        {
                            Matrix_A[i][j] = 0;
                            Inverse_A[i][j] = 0;
                        }
                nMapX = CurMapX; nMapY = CurMapY;
                printf("→Sphere : x:%.2lf y:%.2lf Θ:%.2lf˚ rx:%.2lf ry:%.2lf (적용후 맵크기 %dx%d)\n\n", CntrX, CntrY, op[1], op[2], op[3], nMapX, nMapY);
                SizeX = CurMapX, SizeY = CurMapY, CntrX = (CurMapX-1) / 2., CntrY = (CurMapY-1) / 2.;
                lock = 0;
                break;
            }
        case Saddle:
            if (op[9] < 3)
            {
                printf("입력된 함수의 인자 갯수가 부족합니다. 다시 입력하세요.\n\n");
                break;
            }
            else // Run Scale func
            {
                NonLinear(op, &CurMTXM, &CurMapX, &CurMapY, CurTHG2, THG2Num, CurUNIT, UNITNum, CurMRGN, MRGNNum);
                for (int i = 0; i < 3; i++)
                    for (int j = 0; j < 3; j++)
                        if (i == j)
                        {
                            Matrix_A[i][j] = 1;
                            Inverse_A[i][j] = 1;
                        }
                        else
                        {
                            Matrix_A[i][j] = 0;
                            Inverse_A[i][j] = 0;
                        }
                nMapX = CurMapX; nMapY = CurMapY;
                printf("→Saddle : x:%.2lf y:%.2lf Θ:%.2lf˚ rx:%.2lf ry:%.2lf (적용후 맵크기 %dx%d)\n\n", CntrX, CntrY, op[1], op[2], op[3], nMapX, nMapY);
                SizeX = CurMapX, SizeY = CurMapY, CntrX = (CurMapX-1) / 2., CntrY = (CurMapY-1) / 2.;
                lock = 0;
                break;
            }
        case Cylinder:
            if (op[9] < 3)
            {
                printf("입력된 함수의 인자 갯수가 부족합니다. 다시 입력하세요.\n\n");
                break;
            }
            else // Run Scale func
            {
                if ((int)op[1] != 0)
                    op[1] = 1;
                NonLinear(op, &CurMTXM, &CurMapX, &CurMapY, CurTHG2, THG2Num, CurUNIT, UNITNum, CurMRGN, MRGNNum);
                for (int i = 0; i < 3; i++)
                    for (int j = 0; j < 3; j++)
                        if (i == j)
                        {
                            Matrix_A[i][j] = 1;
                            Inverse_A[i][j] = 1;
                        }
                        else
                        {
                            Matrix_A[i][j] = 0;
                            Inverse_A[i][j] = 0;
                        }
                nMapX = CurMapX; nMapY = CurMapY;
                printf("→Cylinder : x:%.2lf y:%.2lf i:%d k:%.2lf r:%.2lf (적용후 맵크기 %dx%d)\n\n", CntrX, CntrY, (int)op[1], op[2], op[3], nMapX, nMapY);
                SizeX = CurMapX, SizeY = CurMapY, CntrX = (CurMapX-1) / 2., CntrY = (CurMapY-1) / 2.;
                lock = 0;
                break;
            }
        }
        list = Err;
    } while (1);

    // Dim
    fseek(fDIM, 8, 0);
    fwrite(&CurMapX, 2, 1, fDIM);
    fwrite(&CurMapY, 2, 1, fDIM);

    // MTXM
    FILE* fMTXM2;
    DWORD MTXMsize2 = 8 + CurMapX * CurMapY * 2;
    char BGMap[512] = { 0 };
    printf("배경으로 사용할 맵파일을 입력 (0입력시 배경타일은 Null타일(0001,13)로 채워짐)\n");
    scanf_s("%s", BGMap, 512);
    fopen_s(&fMTXM2, "newMTXM.chk", "w+b");

pass:
    if (strcmp(BGMap, "0") == 0)
    {
        temp = (int)'M' + (int)'T' * 256 + (int)'X' * 65536 + (int)'M' * 16777216;
        fwrite(&temp, 4, 1, fMTXM2);
        temp = MTXMsize2 - 8;
        fwrite(&temp, 4, 1, fMTXM2);

        for (int i = 0; i < CurMapY; i++)
            for (int j = 0; j < CurMapX; j++)
            {
                WORD stemp = CurMTXM[i][j].Value;
                fwrite(&stemp, 2, 1, fMTXM2);
            }
    }
    else
    {
        MPQHANDLE BGMPQ;
        MPQHANDLE BGChk;
        FILE* BGout;

        if (FALSE)
        {
        retry:
            scanf_s("%s", BGMap, 512);
            if (strcmp(BGMap, "0") == 0)
                goto pass;
        }
        if (!SFileOpenArchive(BGMap, 0, 0, &BGMPQ)) {
            // SFileOpenArchive([파일명], 0, 0, &[리턴받을 MPQ핸들]);
            printf("잘못된 맵파일 입니다. 다시 입력하세요.\n");
            goto retry;
        }
        f_Sopen(BGMPQ, "staredit\\scenario.chk", &BGChk);
        f_Scopy(BGMPQ, &BGChk, "BGM.chk", &BGout);
        SFileCloseFile(BGChk);
        SFileCloseArchive(BGMPQ); // MPQ 닫기


        DWORD BGDIMoff1, BGDIMoff2, BGDIMsize = GetChkSection(BGout, "DIM ", &BGDIMoff1, &BGDIMoff2);
        fseek(BGout, BGDIMoff1 + 8, 0);
        DWORD BGMapX = 0, BGMapY = 0;
        fread(&BGMapX, 2, 1, BGout);
        fread(&BGMapY, 2, 1, BGout);

        if (BGMapX != CurMapX || BGMapY != CurMapY)
        {
            printf("배경맵파일의 지형크기(%dx%d)가 출력크기(%dx%d)와 일치하지 않습니다. 다시 입력하세요.\n", BGMapX, BGMapY, CurMapX, CurMapY);
            fclose(BGout);
            goto retry;
        }

        printf("%s 의 지형이 배경으로 설정되었습니다. (맵크기 %dx%d)\n", BGMap, BGMapX, BGMapY);
        DWORD BGMTXMoff1, BGMTXMoff2, BGMTXMsize = GetChkSection(BGout, "MTXM", &BGMTXMoff1, &BGMTXMoff2);
        fseek(BGout, BGMTXMoff1, 0);
        f_Fcopy(&BGout, &fMTXM2, BGMTXMsize);

        for (int i = 0; i < CurMapY; i++)
            for (int j = 0; j < CurMapX; j++)
                if (CurMTXM[i][j].Type == 2)
                {
                    WORD stemp = CurMTXM[i][j].Value;
                    fseek(fMTXM2, (i * CurMapX + j) * 2 + 8, 0);
                    fwrite(&stemp, 2, 1, fMTXM2);
                }
        fclose(BGout);
        DeleteFileA("BGM.chk");
    }


    fseek(fout, 0, 2);
    int fsize = ftell(fout);
    fseek(fout, 0, 0);
    const char* Pass[] = { "DIM ", "DD2 ", "TILE", "ISOM", "MTXM", "MASK", "UNIT", "THG2" ,"MRGN" };
    DWORD Section[2];
    for (int i = 0; i < fsize;)
    {
        fseek(fout, i, 0);
        fread(Section, 4, 2, fout);

        DWORD Offset, check = 0;
        for (int j = 0; j < 9; j++)
        {
            DWORD Key = Pass[j][0] + (Pass[j][1] << 8) + (Pass[j][2] << 16) + (Pass[j][3] << 24);
            if (Section[0] == Key) check = 1;
        }

        if (check == 0) // copy section
        {
            Offset = i;
            fseek(fout, Offset, 0);
            f_Fcopy(&fout, &fnew, Section[1] + 8); // Copy UNIT
        }
        i += (Section[1] + 8);
    }

    // make MASK
    fopen_s(&fMASK, "MASK.chk", "w+b");
    DWORD MASKsize = CurMapX * CurMapY + 8;
    BYTE mtemp = 0xFF;
    temp = (int)'M' + (int)'A' * 256 + (int)'S' * 65536 + (int)'K' * 16777216;
    fwrite(&temp, 4, 1, fMASK);
    temp = MASKsize - 8;
    fwrite(&temp, 4, 1, fMASK);
    for (int i = 0; i < MASKsize - 8; i++)
        fwrite(&mtemp, 1, 1, fMASK);
    // fill empty DD2
    fseek(fnew, 0, 2);
    temp = (int)'D' + (int)'D' * 256 + (int)'2' * 65536 + (int)' ' * 16777216;
    fwrite(&temp, 4, 1, fnew);
    temp = 0;
    fwrite(&temp, 4, 1, fnew);

    // write MRGN
    fopen_s(&fMRGN2, "MRGN2.chk", "w+b");
    temp = (int)'M' + (int)'R' * 256 + (int)'G' * 65536 + (int)'N' * 16777216;
    fwrite(&temp, 4, 1, fMRGN2);
    temp = MRGNsize - 8;
    fwrite(&temp, 4, 1, fMRGN2);
    int t1, t2;
    if (MRGNOpt == 1)
    {
        for (int i = 0; i < MRGNNum; i++)
        {
            if (i != 63) 
            {
                if (CurMRGN[i].Xdir == 0)
                {
                    t1 = min(CurMRGN[i].Value[0], CurMRGN[i].Value[2]);
                    t2 = max(CurMRGN[i].Value[0], CurMRGN[i].Value[2]);
                }
                else
                {
                    t1 = max(CurMRGN[i].Value[0], CurMRGN[i].Value[2]);
                    t2 = min(CurMRGN[i].Value[0], CurMRGN[i].Value[2]);
                }
                CurMRGN[i].Value[0] = t1;
                CurMRGN[i].Value[2] = t2;

                if (CurMRGN[i].Ydir == 0)
                {
                    t1 = min(CurMRGN[i].Value[1], CurMRGN[i].Value[3]);
                    t2 = max(CurMRGN[i].Value[1], CurMRGN[i].Value[3]);
                }
                else
                {
                    t1 = max(CurMRGN[i].Value[1], CurMRGN[i].Value[3]);
                    t2 = min(CurMRGN[i].Value[1], CurMRGN[i].Value[3]);
                }
                CurMRGN[i].Value[1] = t1;
                CurMRGN[i].Value[3] = t2;
            }
            else
            {
                CurMRGN[i].Value[0] = 0;
                CurMRGN[i].Value[1] = 0;
                CurMRGN[i].Value[2] = CurMapX*32;
                CurMRGN[i].Value[3] = CurMapY*32;
            }
            fwrite(CurMRGN[i].Value, 4, 5, fMRGN2);
        }
    }
    else
    {
        for (int i = 0; i < MRGNNum; i++)
        {
            if (i != 63)
            {
                double X = CurMRGN[i].CX, Y = CurMRGN[i].CY;
                if (CurMRGN[i].Xdir == 0)
                {
                    CurMRGN[i].Value[0] = X + min(CurMRGN[i].Loc[0], CurMRGN[i].Loc[2]);
                    CurMRGN[i].Value[2] = X + max(CurMRGN[i].Loc[0], CurMRGN[i].Loc[2]);
                }
                else
                {
                    CurMRGN[i].Value[0] = X + max(CurMRGN[i].Loc[0], CurMRGN[i].Loc[2]);
                    CurMRGN[i].Value[2] = X + min(CurMRGN[i].Loc[0], CurMRGN[i].Loc[2]);
                }

                if (CurMRGN[i].Ydir == 0)
                {
                    CurMRGN[i].Value[1] = Y + min(CurMRGN[i].Loc[1], CurMRGN[i].Loc[3]);
                    CurMRGN[i].Value[3] = Y + max(CurMRGN[i].Loc[1], CurMRGN[i].Loc[3]);
                }
                else
                {
                    CurMRGN[i].Value[1] = Y + max(CurMRGN[i].Loc[1], CurMRGN[i].Loc[3]);
                    CurMRGN[i].Value[3] = Y + min(CurMRGN[i].Loc[1], CurMRGN[i].Loc[3]);
                }
            }
            else
            {
                CurMRGN[i].Value[0] = 0;
                CurMRGN[i].Value[1] = 0;
                CurMRGN[i].Value[2] = CurMapX * 32;
                CurMRGN[i].Value[3] = CurMapY * 32;
            }
            fwrite(CurMRGN[i].Value, 4, 5, fMRGN2);
        }
    }


    // write THG2
    FILE* fTHG22;
    fopen_s(&fTHG22, "THG22.chk", "w+b");
    temp = (int)'T' + (int)'H' * 256 + (int)'G' * 65536 + (int)'2' * 16777216;
    fwrite(&temp, 4, 1, fTHG22);
    temp = 0;
    fwrite(&temp, 4, 1, fTHG22);
    int THG2SIZE = 0;
    if (THG2Opt == 1)
    {
        for (int i = 0; i < CurMapY; i++)
            for (int j = 0; j < CurMapX; j++)
            {
                if (CurMTXM[i][j].Type == 2) 
                {
                    int index = CurMTXM[i][j].Index;
                    int X = index % MapX;
                    int Y = index / MapX;
                    THG2Node* temp = THG2Map[Y][X];
                    for (; temp; temp = temp->link)
                    {
                        temp->data.Value[1] = temp->data.dx + j * 32;
                        temp->data.Value[2] = temp->data.dy + i * 32;
                        fwrite(temp->data.Value, 2, 5, fTHG22);
                        THG2SIZE += 10;
                    }
                }
            }
    }
    else
    {
        for (int i = 0; i < THG2Num; i++)
        {
            double X = CurTHG2[i].X, Y = CurTHG2[i].Y;
            if (X >= 0 && X < CurMapX && Y >= 0 && Y < CurMapY)
            {
                CurTHG2[i].Value[1] = (unsigned short)(32 * X);
                CurTHG2[i].Value[2] = (unsigned short)(32 * Y);
                fwrite(CurTHG2[i].Value, 2, 5, fTHG22);
                THG2SIZE += 10;
            }
        }
    }
    fseek(fTHG22, 4, 0);
    fwrite(&THG2SIZE, 4, 1, fTHG22);
    THG2SIZE += 8;

    // write UNIT
    FILE* fUNIT2;
    fopen_s(&fUNIT2, "UNIT2.chk", "w+b");
    temp = (int)'U' + (int)'N' * 256 + (int)'I' * 65536 + (int)'T' * 16777216;
    fwrite(&temp, 4, 1, fUNIT2);
    temp = 0;
    fwrite(&temp, 4, 1, fUNIT2);
    int UNITSIZE = 0;
    if (UNITOpt == 1)
    {
        for (int i = 0; i < CurMapY; i++)
            for (int j = 0; j < CurMapX; j++)
            {
                if (CurMTXM[i][j].Type == 2)
                {
                    int index = CurMTXM[i][j].Index;
                    int X = index % MapX;
                    int Y = index / MapX;
                    UNITNode* temp = UNITMap[Y][X];
                    for (; temp; temp = temp->link)
                    {
                        if (temp->data.Value[4] != 0xD6)
                        {
                            temp->data.Value[2] = temp->data.dx + j * 32;
                            temp->data.Value[3] = temp->data.dy + i * 32;
                            fwrite(temp->data.Value, 2, 18, fUNIT2);
                            UNITSIZE += 36;
                        }
                    }
                }
            }

        for (int i = 0; i < UNITNum; i++)
        {
            if (CurUNIT[i].Value[4] == 0xD6)
            {
                double X = CurUNIT[i].X, Y = CurUNIT[i].Y;
                if (X >= 0 && X < CurMapX && Y >= 0 && Y < CurMapY)
                {
                    CurUNIT[i].Value[2] = (unsigned short)(32 * X);
                    CurUNIT[i].Value[3] = (unsigned short)(32 * Y);
                    fwrite(CurUNIT[i].Value, 2, 18, fUNIT2);
                    UNITSIZE += 36;
                }
            }
        }
    }
    else
    {
        for (int i = 0; i < UNITNum; i++)
        {
            double X = CurUNIT[i].X, Y = CurUNIT[i].Y;
            if (X >= 0 && X < CurMapX && Y >= 0 && Y < CurMapY)
            {
                CurUNIT[i].Value[2] = (unsigned short)(32 * X);
                CurUNIT[i].Value[3] = (unsigned short)(32 * Y);
                fwrite(CurUNIT[i].Value, 2, 18, fUNIT2);
                UNITSIZE += 36;
            }
        }
    }
    fseek(fUNIT2, 4, 0);
    fwrite(&UNITSIZE, 4, 1, fUNIT2);
    UNITSIZE += 8;

    // Add Sections
    fseek(fMASK, 0, 0);
    fseek(fDIM, 0, 0);
    fseek(fMTXM2, 0, 0);
    f_Fcopy(&fMASK, &fnew, MASKsize); // Copy MASK
    f_Fcopy(&fDIM, &fnew, DIMsize); // Copy DIM
    f_Fcopy(&fMTXM2, &fnew, MTXMsize2); // Copy MTXM
    int TILEoff = ftell(fnew);
    fseek(fMTXM2, 0, 0);
    f_Fcopy(&fMTXM2, &fnew, MTXMsize2); // Copy MTXM
    fseek(fnew, TILEoff, 0);
    temp = (int)'T' + (int)'I' * 256 + (int)'L' * 65536 + (int)'E' * 16777216;
    fwrite(&temp, 4, 1, fnew);
    fseek(fnew, 0, 2);

    
    
    fseek(fTHG22, 0, 0);
    f_Fcopy(&fTHG22, &fnew, THG2SIZE); // Copy THG2
    fseek(fUNIT2, 0, 0);
    f_Fcopy(&fUNIT2, &fnew, UNITSIZE); // Copy UNIT

    fseek(fMRGN2, 0, 0);
    f_Fcopy(&fMRGN2, &fnew, MRGNsize); // Copy MRGN

   

    chksize = ftell(fnew);
    fclose(fout);
    fclose(fnew);
    fclose(fMASK);
    fclose(fMTXM);
    fclose(fMTXM2);
    fclose(fDIM);
    fclose(fTHG2);
    fclose(fTHG22);
    fclose(fUNIT);
    fclose(fUNIT2);
    fclose(fMRGN);
    fclose(fMRGN2);

    DeleteFileA("MASK.chk");
    DeleteFileA("UNIT.chk");
    DeleteFileA("UNIT2.chk");
    DeleteFileA("MRGN.chk");
    DeleteFileA("MRGN2.chk");
    DeleteFileA("MTXM.chk");
    DeleteFileA("MTXM2.chk");
    DeleteFileA("DIM.chk");
    DeleteFileA("THG2.chk");
    DeleteFileA("THG22.chk");
    DeleteFileA("newMTXM.chk");
    // Write MPQ
    char* out = iname;
    hMPQ = MpqOpenArchiveForUpdate(out, MOAU_CREATE_ALWAYS, 1024);
    if (hMPQ == INVALID_HANDLE_VALUE) { DeleteFileA(out);  return false; }

    // Write Files & Delete Temp
    f_Swrite(hMPQ, "(listfile).txt", "(listfile)");
    fopen_s(&lout, "(listfile).txt", "rb");

    Wavptr = 0;
    line = 0;
    while (line < listline)
    {
        line++;
        fgets(strTemp, 512, lout);
        strLength = strlen(strTemp);
        strTemp[strLength - 2] = 0;
        if (!strcmp(strTemp, "staredit\\scenario.chk"))
        {
            f_Swrite(hMPQ, "scenario_new.chk", "staredit\\scenario.chk");
            DeleteFileA("scenario.chk");
            DeleteFileA("scenario_new.chk");
        }
        else
        {
            f_SwriteWav(hMPQ, WavName[Wavptr], strTemp);
            DeleteFileA(WavName[Wavptr]);
            free(WavName[Wavptr]);
            Wavptr++;
        }
    }
    fclose(lout);
    DeleteFileA("(listfile).txt");
    MpqCloseUpdatedArchive(hMPQ, 0);

    printf("적용후 scenario.chk 의 크기 : %dbytes\n%s 로 저장됨 (맵크기 %dx%d)\a\n", chksize, iname, CurMapX, CurMapY);

    system("pause");
    return 0;
}
