#pragma once
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <Windows.h>
#include "SFmpqapi.h"
#include "SFmpq_static.h"
#include "SFmpqapi_no-lib.h"

enum TransFunc { Err, Move, Scale, Reflect, Rotate, Shear, Affine, Distort, Apply, Range, Help, Cancel, Trim, setSize, setScale, Cntr, Size, Wave, Lens, Gravity, Window, Pinch, iPinch, Squish, iSquish, Swirl, Sphere, Saddle, Cylinder, Movemap};
enum OptFunc { ERR, MRGN, UNIT, THG2, HELP, LIST, ALL };
char* Find_Next(char* ptr);
void ParseOpt(char* Input, int* opt);
void ParseMsg(char* Input, double* op);
