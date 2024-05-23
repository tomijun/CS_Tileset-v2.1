#pragma once
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <Windows.h>
#include "SFmpqapi.h"
#include "SFmpq_static.h"
#include "SFmpqapi_no-lib.h"
#include "Parser.h"

char* Find_Next(char* ptr)
{
    char* temp = ptr;
    while (1)
    {
        if (*temp == 0)
            return NULL;
        else if (*temp == ' ')
            break;
        temp++;
    }

    while (1)
    {
        if (*temp == 0)
            return NULL;
        else if ((*temp >= '0' && *temp <= '9') || *temp == '.' || *temp == '-' || *temp == '+')
        {
            ptr = temp;
            break;
        }
        temp++;
    }
    return ptr;
}

void ParseOpt(char* Input, int* opt)
{
    OptFunc flist;
    char* front = Input, * rear, tempstr[16]; // front ~ rear : 

    strncpy_s(tempstr, 16, Input, 4);
    if (!_stricmp(tempstr, "MRGN"))
        flist = MRGN;
    else if (!_stricmp(tempstr, "UNIT"))
        flist = UNIT;
    else if (!_stricmp(tempstr, "THG2"))
        flist = THG2;
    else if (!_stricmp(tempstr, "Help"))
        flist = HELP;
    else if (!_stricmp(tempstr, "List"))
        flist = LIST;
    else if (!_stricmp(tempstr, "All "))
        flist = ALL;
    else
        flist = ERR;
    opt[0] = flist;
    opt[9] = 0;
    if (flist == ERR) return;

    front = Find_Next(Input);
    if (front == NULL) return;
    int check = 0;
    for (int i = 1; i < 9; i++)
    {
        rear = Find_Next(front);
        if (rear == NULL)
        {
            check = 1;
            if (front == NULL) break;
        }

        opt[i] = atoi(front);
        opt[9]++;
        front = rear;

        if (check == 1) break;
    }
    return;
}


void ParseMsg(char* Input, double* op)
{
    TransFunc flist;
    char* front = Input, * rear; // front ~ rear : 
    if (!strncmp(Input, "Movemap", 7) || !strncmp(Input, "movemap", 7))
        flist = Movemap;
    else if (!strncmp(Input, "Move", 4) || !strncmp(Input, "move", 4))
        flist = Move;
    else if (!strncmp(Input, "Scale", 5) || !strncmp(Input, "scale", 5))
        flist = Scale;
    else if (!strncmp(Input, "Reflect", 7) || !strncmp(Input, "reflect", 7))
        flist = Reflect;
    else if (!strncmp(Input, "Rotate", 6) || !strncmp(Input, "rotate", 6))
        flist = Rotate;
    else if (!strncmp(Input, "Shear", 5) || !strncmp(Input, "shear", 5))
        flist = Shear;
    else if (!strncmp(Input, "Project", 6) || !strncmp(Input, "project", 6))
        flist = Affine;
    else if (!strncmp(Input, "setScale", 8) || !strncmp(Input, "setscale", 8))
        flist = setScale;
    else if (!strncmp(Input, "Apply", 5) || !strncmp(Input, "apply", 5))
        flist = Apply;
    else if (!strncmp(Input, "Range", 5) || !strncmp(Input, "range", 5))
        flist = Range;
    else if (!strncmp(Input, "Trim", 4) || !strncmp(Input, "trim", 4))
        flist = Trim;
    else if (!strncmp(Input, "setSize", 7) || !strncmp(Input, "setsize", 7))
        flist = setSize;
    else if (!strncmp(Input, "Help", 4) || !strncmp(Input, "help", 4))
        flist = Help;
    else if (!strncmp(Input, "Cancel", 6) || !strncmp(Input, "cancel", 6))
        flist = Cancel;
    else if (!strncmp(Input, "Size", 4) || !strncmp(Input, "size", 4))
        flist = Size;
    else if (!strncmp(Input, "Cntr", 4) || !strncmp(Input, "cntr", 4))
        flist = Cntr;
    else if (!strncmp(Input, "Wave", 4) || !strncmp(Input, "wave", 4))
        flist = Wave;
    else if (!strncmp(Input, "Lens", 4) || !strncmp(Input, "lens", 4))
        flist = Lens;
    else if (!strncmp(Input, "Gravity", 7) || !strncmp(Input, "gravity", 7))
        flist = Gravity;
    else if (!strncmp(Input, "Window", 6) || !strncmp(Input, "window", 6))
        flist = Window;
    else if (!strncmp(Input, "Pinch", 5) || !strncmp(Input, "pinch", 5))
        flist = Pinch;
    else if (!strncmp(Input, "Squish", 6) || !strncmp(Input, "squish", 6))
        flist = Squish;
    else if (!strncmp(Input, "Swirl", 5) || !strncmp(Input, "swirl", 5))
        flist = Swirl;
    else if (!strncmp(Input, "Sphere", 6) || !strncmp(Input, "sphere", 6))
        flist = Sphere;
    else if (!strncmp(Input, "Saddle", 6) || !strncmp(Input, "saddle", 6))
        flist = Saddle;
    else if (!strncmp(Input, "Cylinder", 8) || !strncmp(Input, "cylinder", 8))
        flist = Cylinder;
    else
        flist = Err;
    op[0] = flist;
    op[9] = 0;
    if (flist == Err) return;

    front = Find_Next(Input);
    if (front == NULL) return;
    int check = 0;
    for (int i = 1; i < 9; i++)
    {
        rear = Find_Next(front);
        if (rear == NULL)
        {
            check = 1;
            if (front == NULL) break;
        }

        op[i] = atof(front);
        op[9]++;
        front = rear;

        if (check == 1) break;
    }
    return;
}