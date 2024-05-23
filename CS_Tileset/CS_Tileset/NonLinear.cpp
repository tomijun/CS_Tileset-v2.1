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
#define PI 3.14159265

extern double Matrix_A[3][3];
extern double Inverse_A[3][3];
extern int THG2Opt, UNITOpt, MRGNOpt;
extern double CntrX, CntrY;
extern int SizeX, SizeY;
double MapSizeX, MapSizeY;
namespace WaveConst
{
    static double k1 = 0, k2 = 0, t1 = 0, t2 = 0, es = 1.e-2, _X = 0, _Y = 0;
    static int imax = 100;
}

namespace LensConst
{
    static double kx = 0, ky = 0, tx = 0, ty = 0, es = 1.e-2, _X = 0, _Y = 0;
    static int imax = 100;
}

namespace GravityConst
{
    static double kx = 0, ky = 0, tx = 0, ty = 0, es = 1.e-2, _X = 0, _Y = 0;
    static int imax = 100;
}

namespace WindowConst
{
    static double tx = 0, ty = 0, kx = 0, ky = 0, es = 1.e-2, _X = 0, _Y = 0;
    static int imax = 100;
}

namespace PinchConst
{
    static double tx = 0, ty = 0, kx = 0, ky = 0, sx = 0, sy = 0, es = 1.e-2, _X = 0, _Y = 0;
    static int imax = 100;
}

namespace SquishConst
{
    static double tx = 0, ty = 0, kx = 0, ky = 0, nx = 0, ny = 0, es = 1.e-2, _X = 0, _Y = 0;
    static int imax = 100;
}

namespace SwirlConst
{
    static double kx = 0, ky = 0, tx = 0, ty = 0;
}

namespace SphereConst
{
    static double rx = 0, ry = 0, Angle = 0, es = 1.e-2, _X = 0, _Y = 0;
    static int imax = 100;
}

namespace SaddleConst
{
    static double rx = 0, ry = 0, Angle = 0, es = 1.e-2, _X = 0, _Y = 0;
    static int imax = 100;
}

namespace CylinderConst
{
    static double r = 0, k = 0, es = 1.e-2, _X = 0, _Y = 0;
    static int dir = 0, imax = 100;
}

int CheckRange(double X, double Y, double rx, double ry)
{
    return (pow(X - CntrX, 2)/rx/rx + pow(X - CntrY, 2)/ry/ry > 1) ? 1 : 0;
}

void CalcData(int newMapX, int newMapY, double *op, int Type, double *x, double *y)
{
    double X = *x, Y = *y;
    int ret;
    if (Type == 0)
        NonLinear_Calc(&X, &Y, (int)op[0]);
    else if (Type == 1)
    {
        switch ((int)op[0])
        {
        case Gravity:
            GravityConst::_X = X;
            GravityConst::_Y = Y;
            break;
        case Window:
            WindowConst::_X = X;
            WindowConst::_Y = Y;
            break;
        case iPinch:
            PinchConst::_X = X;
            PinchConst::_Y = Y;
            break;
        case iSquish:
            SquishConst::_X = X;
            SquishConst::_Y = Y;
            break;
        }
        ret = Newton2((int)op[0], X, Y, &X, &Y);
        if (ret == -1) ret = Newton2((int)op[0], newMapX / 2., newMapY / 2., &X, &Y);
        if (ret == -1) ret = Newton2((int)op[0], 0, 0, &X, &Y);
        if (ret == -1) ret = Newton2((int)op[0], newMapX, 0, &X, &Y);
        if (ret == -1) ret = Newton2((int)op[0], 0, newMapY, &X, &Y);
        if (ret == -1) ret = Newton2((int)op[0], newMapX, newMapY, &X, &Y);
    }
    else if (Type == 2)
    {
        if (NonLinear_Func((int)op[0], Distance, X, Y))
        {
            X = X;
            Y = Y;
        }
        else
        {
            SaddleConst::_X = X;
            SaddleConst::_Y = Y;
            using namespace SaddleConst;
            double RX = rx / sqrt(2), RY = ry / sqrt(2);
            ret = Newton2((int)op[0], _X, _Y, &X, &Y); // x0 = _X, y0 = _Y
            for (int k = 0; k < 360; k += 45)
            {
                double A = (k - Angle) * PI / 180;
                if (ret == -1 || CheckRange(X,Y,rx,ry)) ret = Newton2((int)op[0], _X + RX * cos(A), _Y + RY * sin(A), &X, &Y);
            }
        }
    }
    *x = X;
    *y = Y;
}

double NonLinear_Wave(list num, double X, double Y) // Wave Func Package
{
    using namespace WaveConst;
    switch (num)
    {
    case fn:
        return X + k1 * sin(2 * PI * (Y - CntrX) / t1) - _X;
    case gn:
        return Y + k2 * cos(2 * PI * (X - CntrY) / t2) - _Y;
    case fx:
        return 1;
    case fy:
        return k1 * (2 * PI / t1) * cos(2 * PI * (Y - CntrX) / t1);
    case gx:
        return k2 * (-2 * PI / t2) * sin(2 * PI * (X - CntrY) / t2);
    case gy:
        return 1;
    case Ffn:
        return X + k1 * sin(2 * PI * (Y - CntrX) / t1);
    case Fgn:
        return Y + k2 * cos(2 * PI * (X - CntrY) / t2);
    }
}

double NonLinear_Lens(list num, double X, double Y) // Wave Func Package
{
    using namespace LensConst;
    switch (num)
    {
    case fn:
        return X + kx * (X - CntrX) * exp(-1. / tx * (pow(X - CntrX, 2) + pow(Y - CntrY, 2))) - _X;
    case gn:
        return Y + ky * (Y - CntrY) * exp(-1. / ty * (pow(X - CntrX, 2) + pow(Y - CntrY, 2))) - _Y;
    case fx:
        return 1 + kx * (1 + (-2. / tx) * pow(X - CntrX, 2)) * exp(-1. / tx * (pow(X - CntrX, 2) + pow(Y - CntrY, 2)));
    case fy:
        return kx * ((-2. / tx) * (X - CntrX) * (Y - CntrY)) * exp(-1. / tx * (pow(X - CntrX, 2) + pow(Y - CntrY, 2)));
    case gx:
        return ky * ((-2. / ty) * (X - CntrX) * (Y - CntrY)) * exp(-1. / ty * (pow(X - CntrX, 2) + pow(Y - CntrY, 2)));
    case gy:
        return 1 + ky * (1 + (-2. / ty) * pow(Y - CntrY, 2)) * exp(-1. / ty * (pow(X - CntrX, 2) + pow(Y - CntrY, 2)));
    case Ffn:
        return X + kx * (X - CntrX) * exp(-1. / tx * (pow(X - CntrX, 2) + pow(Y - CntrY, 2)));
    case Fgn:
        return Y + ky * (Y - CntrY) * exp(-1. / ty * (pow(X - CntrX, 2) + pow(Y - CntrY, 2)));
    }
}

double NonLinear_Gravity(list num, double X, double Y) // Wave Func Package
{
    using namespace GravityConst;
    switch (num)
    {
    case fn:
        return X + kx * (X - CntrX) * exp(-1. / tx * (pow(X - CntrX, 2) + pow(Y - CntrY, 2))) - _X;
    case gn:
        return Y + ky * (Y - CntrY) * exp(-1. / ty * (pow(X - CntrX, 2) + pow(Y - CntrY, 2))) - _Y;
    case fx:
        return 1 + kx * (1 + (-2. / tx) * pow(X - CntrX, 2)) * exp(-1. / tx * (pow(X - CntrX, 2) + pow(Y - CntrY, 2)));
    case fy:
        return kx * ((-2. / tx) * (X - CntrX) * (Y - CntrY)) * exp(-1. / tx * (pow(X - CntrX, 2) + pow(Y - CntrY, 2)));
    case gx:
        return ky * ((-2. / ty) * (X - CntrX) * (Y - CntrY)) * exp(-1. / ty * (pow(X - CntrX, 2) + pow(Y - CntrY, 2)));
    case gy:
        return 1 + ky * (1 + (-2. / ty) * pow(Y - CntrY, 2)) * exp(-1. / ty * (pow(X - CntrX, 2) + pow(Y - CntrY, 2)));
    case Ffn:
        return X + kx * (X - CntrX) * exp(-1. / tx * (pow(X - CntrX, 2) + pow(Y - CntrY, 2)));
    case Fgn:
        return Y + ky * (Y - CntrY) * exp(-1. / ty * (pow(X - CntrX, 2) + pow(Y - CntrY, 2)));
    }
}

double NonLinear_Window(list num, double X, double Y) // Wave Func Package
{
    using namespace WindowConst;
    switch (num)
    {
    case fn:
        return kx * tan(PI / tx * (X - CntrX)) + X - _X;
    case gn:
        return ky * tan(PI / ty * (Y - CntrY)) + Y - _Y;
    case fx:
        return kx * PI / tx * tan(PI / tx * (X - CntrX)) + 1;
    case fy:
        return 0;
    case gx:
        return 0;
    case gy:
        return ky * PI / ty * tan(PI / ty * (Y - CntrY)) + 1;
    case Ffn:
        return kx * tan(PI / tx * (X - CntrX)) + X;
    case Fgn:
        return ky * tan(PI / ty * (Y - CntrY)) + Y;
    }
}

double NonLinear_Pinch(list num, double X, double Y) // Wave Func Package
{
    using namespace PinchConst;
    switch (num)
    {
    case fn:
        return (X - sx) * (1 + kx * exp(-1. / tx * (pow(X - CntrX, 2) + pow(Y - CntrY, 2)))) + sx - _X;
    case gn:
        return (Y - sy) * (1 + ky * exp(-1. / ty * (pow(X - CntrX, 2) + pow(Y - CntrY, 2)))) + sy - _Y;
    case fx:
        return 1 + kx * (1 + (-2. / tx) * (X - CntrX) * (X - sx)) * exp(-1. / tx * (pow(X - CntrX, 2) + pow(Y - CntrY, 2)));
    case fy:
        return kx * ((-2. / tx) * (X - sx) * (Y - CntrY)) * exp(-1. / tx * (pow(X - CntrX, 2) + pow(Y - CntrY, 2)));
    case gx:
        return ky * ((-2. / ty) * (X - CntrX) * (Y - sy)) * exp(-1. / ty * (pow(X - CntrX, 2) + pow(Y - CntrY, 2)));
    case gy:
        return 1 + ky * (1 + (-2. / ty) * (Y - CntrY) * (Y - sy)) * exp(-1. / ty * (pow(X - CntrX, 2) + pow(Y - CntrY, 2)));
    case Ffn:
        return (X - sx) * (1 + kx * exp(-1. / tx * (pow(X - CntrX, 2) + pow(Y - CntrY, 2)))) + sx;
    case Fgn:
        return (Y - sy) * (1 + ky * exp(-1. / ty * (pow(X - CntrX, 2) + pow(Y - CntrY, 2)))) + sy;
    }
}

double NonLinear_Squish(list num, double X, double Y) // Wave Func Package
{
    using namespace SquishConst;
    double temp;
    switch (num)
    {
    case fn:
        return CntrX + (X - CntrX) * (1 + kx * exp(-tx * pow(PI * nx / 180. - atan2(Y - CntrY, X - CntrX), 2))) - _X;
    case gn:
        return CntrY + (Y - CntrY) * (1 + ky * exp(-ty * pow(PI * ny / 180. - atan2(Y - CntrY, X - CntrX), 2))) - _Y;
    case fx:
        temp = PI * nx / 180. - atan2(Y - CntrY, X - CntrX);
        return 1 + (1 + (X - CntrX) * -2. * tx * temp * (-1 / (1 + pow((Y - CntrY) / (X - CntrX), 2))) * (Y - CntrY) / -pow(X - CntrX, 2)) * (kx * exp(-tx * pow(temp, 2)));
    case fy:
        temp = PI * nx / 180. - atan2(Y - CntrY, X - CntrX);
        return ((X - CntrX) * -2. * tx * temp * (-1 / (1 + pow((Y - CntrY) / (X - CntrX), 2))) / (X - CntrX)) * (kx * exp(-tx * pow(temp, 2)));
    case gx:
        temp = PI * ny / 180. - atan2(Y - CntrY, X - CntrX);
        return ((Y - CntrY) * -2. * ty * temp * (-1 / (1 + pow((Y - CntrY) / (X - CntrX), 2))) * (Y - CntrY) / -pow(X - CntrX, 2)) * (ky * exp(-ty * pow(temp, 2)));
    case gy:
        temp = PI * ny / 180. - atan2(Y - CntrY, X - CntrX);
        return 1 + (1 + (Y - CntrY) * -2. * ty * temp * (-1 / (1 + pow((Y - CntrY) / (X - CntrX), 2))) / (X - CntrX)) * (ky * exp(-ty * pow(temp, 2)));
    case Ffn:
        return CntrX + (X - CntrX) * (1 + kx * exp(-tx * pow(PI * nx / 180. - atan2(Y - CntrY, X - CntrX), 2)));
    case Fgn:
        return CntrY + (Y - CntrY) * (1 + ky * exp(-ty * pow(PI * ny / 180. - atan2(Y - CntrY, X - CntrX), 2)));
    }
}

double NonLinear_Swirl(list num, double X, double Y) 
{
    using namespace SwirlConst;
    double R, A;
    switch (num)
    {
    case fn: // forward X, Y -> _X
        R = NonLinear_Swirl(XYtoR, X, Y), A = NonLinear_Swirl(XYtoA, X, Y);
        A += kx * exp(-R / tx);
        return NonLinear_Swirl(RAtoX, R, A);
    case gn: // forward X, Y -> _Y
        R = NonLinear_Swirl(XYtoR, X, Y), A = NonLinear_Swirl(XYtoA, X, Y);
        A += ky * exp(-R / ty);
        return NonLinear_Swirl(RAtoY, R, A);
    case Ffn: // inverse _X, _Y -> X
        R = NonLinear_Swirl(XYtoR, X, Y), A = NonLinear_Swirl(XYtoA, X, Y);
        A -= kx * exp(-R / tx);
        return NonLinear_Swirl(RAtoX, R, A);
    case Fgn: // inverse _X, _Y -> Y
        R = NonLinear_Swirl(XYtoR, X, Y), A = NonLinear_Swirl(XYtoA, X, Y);
        A -= ky * exp(-R / ty);
        return NonLinear_Swirl(RAtoY, R, A);
    case XYtoR:
        return sqrt(pow(X - CntrX, 2) + pow(Y - CntrY, 2));
    case XYtoA:
        return atan2(Y - CntrY, X - CntrX);
    case RAtoX:
        return X * cos(Y) + CntrX;
    case RAtoY:
        return X * sin(Y) + CntrY;
    }
}

double NonLinear_Sphere(list num, double X, double Y)
{
    using namespace SphereConst;
    double R, A, P, x2 = X-CntrX, y2 = Y-CntrY;
    switch (num)
    {
    case fn: // forward X, Y -> _X
        R = NonLinear_Sphere(XYtoR, X, Y), A = NonLinear_Sphere(XYtoA, X, Y) + Angle;
        P = PI / 2 * (1 - R / rx);
        return rx * cos(P) * cos(A) + CntrX - _X;
    case gn: // forward X, Y -> _Y
        R = NonLinear_Sphere(XYtoR, X, Y), A = NonLinear_Sphere(XYtoA, X, Y) + Angle;
        P = PI / 2 * (1 - R / ry);
        return ry * cos(P) * sin(A) + CntrY - _Y;
    case fx:
        R = NonLinear_Sphere(XYtoR, X, Y), A = NonLinear_Sphere(XYtoA, X, Y) + Angle;
        P = PI / 2 * (1 - R / rx);
        return rx * cos(P) * sin(A) / (1 + pow(y2 / x2, 2)) * (y2 / x2 / x2) + rx * sin(P) * cos(A) * (PI / 2 / rx) * pow(x2 * x2 + y2 * y2, -0.5) * x2;
    case fy:
        R = NonLinear_Sphere(XYtoR, X, Y), A = NonLinear_Sphere(XYtoA, X, Y) + Angle;
        P = PI / 2 * (1 - R / rx);
        return rx * cos(P) * sin(A) / (1 + pow(y2 / x2, 2)) * (-1 / x2) + rx * sin(P) * cos(A) * (PI / 2 / rx) * pow(x2 * x2 + y2 * y2, -0.5) * y2;
    case gx:
        R = NonLinear_Sphere(XYtoR, X, Y), A = NonLinear_Sphere(XYtoA, X, Y) + Angle;
        P = PI / 2 * (1 - R / ry);
        return ry * cos(P) * cos(A) / (1 + pow(y2 / x2, 2)) * (-y2 / x2 / x2) + ry * sin(P) * sin(A) * (PI / 2 / ry) * pow(x2 * x2 + y2 * y2, -0.5) * x2;
    case gy:
        R = NonLinear_Sphere(XYtoR, X, Y), A = NonLinear_Sphere(XYtoA, X, Y) + Angle;
        P = PI / 2 * (1 - R / ry);
        return ry * cos(P) * cos(A) / (1 + pow(y2 / x2, 2)) * (1 / x2) + ry * sin(P) * sin(A) * (PI / 2 / ry) * pow(x2 * x2 + y2 * y2, -0.5) * y2;
    case Ffn: // inverse _X, _Y -> X
        if (NonLinear_Sphere(Distance, X, Y))
            return X;
        else
        {
            R = NonLinear_Sphere(XYtoR, X, Y), A = NonLinear_Sphere(XYtoA, X, Y) + Angle;
            P = PI / 2 * (1 - R / rx);
            return rx * cos(P) * cos(A) + CntrX;
        }
    case Fgn: // inverse _X, _Y -> Y
        if (NonLinear_Sphere(Distance, X, Y))
            return Y;
        else
        {
            R = NonLinear_Sphere(XYtoR, X, Y), A = NonLinear_Sphere(XYtoA, X, Y) + Angle;
            P = PI / 2 * (1 - R / ry);
            return ry * cos(P) * sin(A) + CntrY;
        }
    case XYtoR:
        return sqrt(pow(X - CntrX, 2) + pow(Y - CntrY, 2));
    case XYtoA:
        return atan2(Y - CntrY, X - CntrX);
    case RAtoX:
        return X * cos(Y) + CntrX;
    case RAtoY:
        return X * sin(Y) + CntrY;
    case Distance: 
        return ((pow(X - CntrX, 2) / rx / rx + pow(Y - CntrY, 2) / ry / ry) > 1) ? 1 : 0;
    }
}

double NonLinear_Saddle(list num, double X, double Y)
{
    using namespace SaddleConst;
    double R, A, P, x2 = X - CntrX, y2 = Y - CntrY;
    switch (num)
    {
    case fn: // forward X, Y -> _X
        R = NonLinear_Saddle(XYtoR, X, Y), A = NonLinear_Saddle(XYtoA, X, Y) + Angle;
        P = PI / 2 * (1 - R / rx);
        return rx * cos(P) * cos(A) + CntrX - _X;
    case gn: // forward X, Y -> _Y
        R = NonLinear_Saddle(XYtoR, X, Y), A = NonLinear_Saddle(XYtoA, X, Y) + Angle;
        P = PI / 2 * (1 - R / ry);
        return ry * cos(P) * sin(A) + CntrY - _Y;
    case fx:
        R = NonLinear_Saddle(XYtoR, X, Y), A = NonLinear_Saddle(XYtoA, X, Y) + Angle;
        P = PI / 2 * (1 - R / rx);
        return rx * cos(P) * sin(A) / (1 + pow(y2 / x2, 2)) * (y2 / x2 / x2) + rx * sin(P) * cos(A) * (PI / 2 / rx) * pow(x2 * x2 + y2 * y2, -0.5) * x2;
    case fy:
        R = NonLinear_Saddle(XYtoR, X, Y), A = NonLinear_Saddle(XYtoA, X, Y) + Angle;
        P = PI / 2 * (1 - R / rx);
        return rx * cos(P) * sin(A) / (1 + pow(y2 / x2, 2)) * (-1 / x2) + rx * sin(P) * cos(A) * (PI / 2 / rx) * pow(x2 * x2 + y2 * y2, -0.5) * y2;
    case gx:
        R = NonLinear_Saddle(XYtoR, X, Y), A = NonLinear_Saddle(XYtoA, X, Y) + Angle;
        P = PI / 2 * (1 - R / ry);
        return ry * cos(P) * cos(A) / (1 + pow(y2 / x2, 2)) * (-y2 / x2 / x2) + ry * sin(P) * sin(A) * (PI / 2 / ry) * pow(x2 * x2 + y2 * y2, -0.5) * x2;
    case gy:
        R = NonLinear_Saddle(XYtoR, X, Y), A = NonLinear_Saddle(XYtoA, X, Y) + Angle;
        P = PI / 2 * (1 - R / ry);
        return ry * cos(P) * cos(A) / (1 + pow(y2 / x2, 2)) * (1 / x2) + ry * sin(P) * sin(A) * (PI / 2 / ry) * pow(x2 * x2 + y2 * y2, -0.5) * y2;
    case Ffn: // inverse _X, _Y -> X
        if (NonLinear_Saddle(Distance, X, Y))
            return X;
        else
        {
            R = NonLinear_Saddle(XYtoR, X, Y), A = NonLinear_Saddle(XYtoA, X, Y) + Angle;
            P = PI / 2 * (1 - R / rx);
            return rx * cos(P) * cos(A) + CntrX;
        }
    case Fgn: // inverse _X, _Y -> Y
        if (NonLinear_Saddle(Distance, X, Y))
            return Y;
        else
        {
            R = NonLinear_Saddle(XYtoR, X, Y), A = NonLinear_Saddle(XYtoA, X, Y) + Angle;
            P = PI / 2 * (1 - R / ry);
            return ry * cos(P) * sin(A) + CntrY;
        }
    case XYtoR:
        return sqrt(pow(X - CntrX, 2) + pow(Y - CntrY, 2));
    case XYtoA:
        return atan2(Y - CntrY, X - CntrX);
    case RAtoX:
        return X * cos(Y) + CntrX;
    case RAtoY:
        return X * sin(Y) + CntrY;
    case Distance:
        return ((pow(X - CntrX, 2) / rx / rx + pow(Y - CntrY, 2) / ry / ry) > 1) ? 1 : 0;
    }
}

double NonLinear_Cylinder(list num, double X, double Y)
{
    using namespace CylinderConst;
    double P, x2 = X - CntrX, y2 = Y - CntrY;
    switch (num)
    {
    case fn: // forward X, Y -> _X
        if (dir == 0) // 技肺
        {
            P = PI / 2 * (1 - x2 / r);
            return r * cos(P) + CntrX - _X;
        }
        else
            return x2 * (1 + pow(x2 * x2 + y2 * y2, 0.5) * k) + CntrX - _X;
    case gn: // forward X, Y -> _Y
        if (dir == 0) // 技肺
            return y2 * (1 + pow(x2 * x2 + y2 * y2, 0.5) * k) + CntrY - _Y;
        else
        {
            P = PI / 2 * (1 - y2 / r);
            return r * cos(P) + CntrY - _Y;
        }
    case fx:
        if (dir == 0) // 技肺
        {
            P = PI / 2 * (1 - x2 / r);
            return sin(P) * (PI / 2);
        }
        else
            return 1 + pow(x2 * x2 + y2 * y2, 0.5) * k + x2 * x2 * k * pow(x2 * x2 + y2 * y2, -0.5);
    case fy:
        if (dir == 0) // 技肺
            return 0;
        else
            return x2 * y2 * k * pow(x2 * x2 + y2 * y2, -0.5);
    case gx:
        if (dir == 0) // 技肺
            return x2 * y2 * k * pow(x2 * x2 + y2 * y2, -0.5);
        else
            return 0;
    case gy:
        if (dir == 0) // 技肺
            return 1 + pow(x2 * x2 + y2 * y2, 0.5) * k + y2 * y2 * k * pow(x2 * x2 + y2 * y2, -0.5);
        else
        {
            P = PI / 2 * (1 - y2 / r);
            return sin(P) * (PI / 2);
        }
    case Ffn: // inverse _X, _Y -> X
        if (dir == 0) // 技肺
        {
            if (NonLinear_Cylinder(Distance, X, Y))
                return X;
            else
            {
                P = PI / 2 * (1 - x2 / r);
                return r * cos(P) + CntrX;
            }
        }
        else
        {
            if (NonLinear_Cylinder(Distance, X, Y))
                return X;
            else
            {
                return x2 * (1 + pow(x2 * x2 + y2 * y2, 0.5) * k) + CntrX;
            }
        }
    case Fgn: // inverse _X, _Y -> Y
        if (dir == 0) // 技肺
        {
            if (NonLinear_Cylinder(Distance, X, Y))
                return Y;
            else
            {
                return y2 * (1 + pow(x2 * x2 + y2 * y2, 0.5) * k) + CntrY;
            }
        }
        else
        {
            if (NonLinear_Cylinder(Distance, X, Y))
                return Y;
            else
            {
                P = PI / 2 * (1 - y2 / r);
                return r * cos(P) + CntrY;
            }
        }
    case Distance:
        if (dir == 0)
            return (abs(x2) > r) ? 1 : 0;
        else
            return (abs(y2) > r) ? 1 : 0;
    }
}

double NonLinear_Func(int fnum, list num, double X, double Y)
{
    switch (fnum)
    {
    case Wave:
        return NonLinear_Wave(num, X, Y);
    case Lens:
        return NonLinear_Lens(num, X, Y);
    case Gravity:
        return NonLinear_Gravity(num, X, Y);
    case Window:
        return NonLinear_Window(num, X, Y);
    case Pinch:
    case iPinch:
        return NonLinear_Pinch(num, X, Y);
    case Squish:
    case iSquish:
        return NonLinear_Squish(num, X, Y);
    case Swirl:
        return NonLinear_Swirl(num, X, Y);
    case Sphere:
        return NonLinear_Sphere(num, X, Y);
    case Saddle:
        return NonLinear_Saddle(num, X, Y);
    case Cylinder:
        return NonLinear_Cylinder(num, X, Y);
    }
}

int Newton2(int fnum, double x0, double y0, double *X, double *Y)
{
    int imax; double es;
    switch (fnum)
    {
    case Wave:
        imax = WaveConst::imax;
        es = WaveConst::es;
        break;
    case Lens:
        imax = LensConst::imax;
        es = LensConst::es;
        break;
    case Gravity:
        imax = GravityConst::imax;
        es = GravityConst::es;
        break;
    case Window:
        imax = WindowConst::imax;
        es = WindowConst::es;
        break;
    case Pinch:
    case iPinch:
        imax = PinchConst::imax;
        es = PinchConst::es;
        break;
    case Squish:
    case iSquish:
        imax = SquishConst::imax;
        es = SquishConst::es;
        break;
    case Swirl:
        break;
    case Sphere:
        imax = SphereConst::imax;
        es = SphereConst::es;
        break;
    case Saddle:
        imax = SaddleConst::imax;
        es = SaddleConst::es;
        break;
    case Cylinder:
        imax = CylinderConst::imax;
        es = CylinderConst::es;
        break;
    }
    double ur, vr, yr = y0, xr = x0, xrold, yrold, eax = INT_MAX, eay = INT_MAX, temp;
    int iter = 0;
    ur = NonLinear_Func(fnum, fn, xr, yr); vr = NonLinear_Func(fnum, gn, xr, yr);
    do
    {
        xrold = xr;
        yrold = yr;
        temp = (NonLinear_Func(fnum, fx, xrold, yrold) * NonLinear_Func(fnum, gy, xrold, yrold) - NonLinear_Func(fnum, fy, xrold, yrold) * NonLinear_Func(fnum, gx, xrold, yrold));
        xr = xrold - (ur * NonLinear_Func(fnum, gy, xrold, yrold) - vr * NonLinear_Func(fnum, fy, xrold, yrold)) / temp;
        yr = yrold - (vr * NonLinear_Func(fnum, fx, xrold, yrold) - ur * NonLinear_Func(fnum, gx, xrold, yrold)) / temp;

        ur = NonLinear_Func(fnum, fn, xr, yr); vr = NonLinear_Func(fnum, gn, xr, yr);
        iter++;
        if (xr != 0.) eax = fabs((xr - xrold) / xr) * 100.;
        if (yr != 0.) eay = fabs((yr - yrold) / yr) * 100.;
    } while (!(eax < es && eay < es || iter >= imax));
    //if (iter >= imax || xr < 0 || yr < 0 || xr >= MapSizeX || yr >= MapSizeY)
    if (iter >= imax)
        return -1;
    else
    {
        *X = xr;
        *Y = yr;
    }
}

int NonLinear_GetData(MTXMData** MTXM, double _X, double _Y, int MapX, int MapY, MTXMData* TempData, int op) // Inverse 
{   // _X, _Y -> X, Y (f^-1)
    int Type = 0, ret;
    switch (op)
    {
    case Wave:
        WaveConst::_X = _X;
        WaveConst::_Y = _Y;
        break;
    case Lens:
        LensConst::_X = _X;
        LensConst::_Y = _Y;
        break;
    case Gravity:
        Type = 1;
        break;
    case Window:
        Type = 1;
        break;
    case Pinch:
        PinchConst::_X = _X;
        PinchConst::_Y = _Y;
        break;
    case iPinch:
        Type = 1;
        break;
    case Squish:
        SquishConst::_X = _X;
        SquishConst::_Y = _Y;
        break;
    case iSquish:
        Type = 1;
        break;
    case Swirl:
        Type = 1;
        break;
    case Sphere:
        Type = 2;
        SphereConst::_X = _X;
        SphereConst::_Y = _Y;
        break;
    case Saddle:
        Type = 1;
        break;
    case Cylinder:
        Type = 3;
        CylinderConst::_X = _X;
        CylinderConst::_Y = _Y;
        break;
    }

    double X = 0, Y = 0;
    if (Type == 0) {
        ret = Newton2(op, _X, _Y, &X, &Y); // x0 = _X, y0 = _Y
        if (ret == -1) ret = Newton2(op, MapX / 2., MapY / 2., &X, &Y); // retry : x0 = Map_Cx, y0 = Map_Cy
        if (ret == -1) ret = Newton2(op, 0, 0, &X, &Y);
        if (ret == -1) ret = Newton2(op, MapX, 0, &X, &Y);
        if (ret == -1) ret = Newton2(op, 0, MapY, &X, &Y);
        if (ret == -1) ret = Newton2(op, MapX, MapY, &X, &Y);
    }
    else if (Type == 1)// forward
    {
        X = NonLinear_Func(op, Ffn, _X, _Y);
        Y = NonLinear_Func(op, Fgn, _X, _Y);
    }
    else if (Type == 2)// 0 + 1
    {
        if (NonLinear_Func(op, Distance, _X, _Y))
        {
            X = _X;
            Y = _Y;
        }
        else
        {
            using namespace SphereConst;
            double RX = rx / sqrt(2), RY = ry / sqrt(2);
            ret = Newton2(op, _X, _Y, &X, &Y); // x0 = _X, y0 = _Y
            for (int k = 0; k < 360; k += 45)
            {
                double A = (k - Angle) * PI / 180;
                if (ret == -1 || X < 0 || X > MapSizeX || Y < 0 || Y > MapSizeY) ret = Newton2(op, _X + RX * cos(A), _Y + RY * sin(A), &X, &Y);
            }
        }
    }
    else if (Type == 3)// 0 + 1
    {
        if (NonLinear_Func(op, Distance, _X, _Y))
        {
            X = _X;
            Y = _Y;
        }
        else
        {
            ret = Newton2(op, _X, _Y, &X, &Y); // x0 = _X, y0 = _Y
            if (ret == -1) ret = Newton2(op, MapX / 2., MapY / 2., &X, &Y); // retry : x0 = Map_Cx, y0 = Map_Cy
            if (ret == -1) ret = Newton2(op, 0, 0, &X, &Y);
            if (ret == -1) ret = Newton2(op, MapX, 0, &X, &Y);
            if (ret == -1) ret = Newton2(op, 0, MapY, &X, &Y);
            if (ret == -1) ret = Newton2(op, MapX, MapY, &X, &Y);
        }
    }
    int I = (int)(round(Y * 1000) / 1000), J = (int)(round(X * 1000) / 1000);
    if (I >= 0 && I < MapY && J >= 0 && J < MapX)
    {
        (*TempData) = MTXM[I][J];
        return 0;
    }
    else
        return -1;
}

void NonLinear_Calc(double* X, double* Y, int op) // forward
{
    double _X, _Y;
    if (op == Swirl)
    {
        _X = NonLinear_Func(op, fn, *X, *Y);
        _Y = NonLinear_Func(op, gn, *X, *Y);
    }
    else
    {
        _X = NonLinear_Func(op, Ffn, *X, *Y);
        _Y = NonLinear_Func(op, Fgn, *X, *Y);
    }
    *X = _X;
    *Y = _Y;
    return;
}

void NonLinear(double *op, MTXMData*** CurMTXM, DWORD* CurMapX, DWORD* CurMapY, THG2Data* CurTHG2, int THG2Num, UNITData* CurUNIT, int UNITNum, MRGNData* CurMRGN, int MRGNNum)
{
    MTXMData** newMTXM;
    DWORD newMapX, newMapY, MapX = *CurMapX, MapY = *CurMapY;
    int Type = 0, ret;
    switch ((int)op[0])
    {
    case Wave:
        WaveConst::k1 = op[1], WaveConst::k2 = op[2], WaveConst::t1 = op[3], WaveConst::t2 = op[4];
        break;
    case Lens:
        LensConst::kx = op[1], LensConst::ky = op[2], LensConst::tx = op[3], LensConst::ty = op[4];
        break;
    case Gravity:
        Type = 1;
        GravityConst::kx = op[1], GravityConst::ky = op[2], GravityConst::tx = op[3], GravityConst::ty = op[4];
        break;
    case Window:
        Type = 1;
        WindowConst::kx = op[1], WindowConst::ky = op[2], WindowConst::tx = op[3], WindowConst::ty = op[4];
        break;
    case iPinch:
        Type = 1;
    case Pinch:
        PinchConst::kx = op[4], PinchConst::ky = op[5], PinchConst::tx = op[6], PinchConst::ty = op[7], PinchConst::sx = op[3], PinchConst::sy = op[2];
        break;
    case iSquish:
        Type = 1;
    case Squish:
        SquishConst::kx = op[4], SquishConst::ky = op[5], SquishConst::tx = op[6], SquishConst::ty = op[7], SquishConst::nx = -op[3], SquishConst::ny = -op[2];
        break;
    case Swirl:
        SwirlConst::kx = op[1], SwirlConst::ky = op[2], SwirlConst::tx = op[3], SwirlConst::ty = op[4];
        break;
    case Sphere:
        SphereConst::Angle = op[1]*PI/180, SphereConst::rx = op[2], SphereConst::ry = op[3];
        break;
    case Saddle:
        Type = 2;
        SaddleConst::Angle = -op[1]*PI/180, SaddleConst::rx = op[2], SaddleConst::ry = op[3];
        break;
    case Cylinder:
        CylinderConst::dir = op[1], CylinderConst::k = op[2]/100, CylinderConst::r = op[3];
        break;
    }
    double X, Y, dX, dY;
    X = 0; Y = 0;

    newMapX = SizeX;
    newMapY = SizeY;
    dX = (newMapX - (*CurMapX)) / 2.;
    dY = (newMapY - (*CurMapY)) / 2.;
    MapSizeX = *CurMapX;
    MapSizeY = *CurMapY;

    newMTXM = (MTXMData**)malloc(newMapY * sizeof(MTXMData*));
    for (int i = 0; i < newMapY; i++)
        newMTXM[i] = (MTXMData*)malloc(newMapX * sizeof(MTXMData));

    for (int i = 0; i < newMapY; i++)
    {
        for (int j = 0; j < newMapX; j++)
        {
            MTXMData TempData;
            int ret = NonLinear_GetData(*CurMTXM, j - dX, i - dY, MapX, MapY, &TempData, (int)op[0]);

            if (ret == -1) // BLANK
            {
                newMTXM[i][j].Value = 0x001D;
                newMTXM[i][j].Index = -1;
                newMTXM[i][j].Type = 1;
            }
            else // Vaild Tile
            {
                newMTXM[i][j].Value = TempData.Value;
                newMTXM[i][j].Index = TempData.Index;
                newMTXM[i][j].Type = TempData.Type;
            }
        }
    }
    
    if (THG2Opt == 0)
        for (int i = 0; i < THG2Num; i++)
        {
            double X = CurTHG2[i].X;
            double Y = CurTHG2[i].Y;
            CalcData(newMapX, newMapY, op, Type, &X, &Y);
            CurTHG2[i].X = X + dX;
            CurTHG2[i].Y = Y + dY;
        }
    if (TRUE)
        for (int i = 0; i < UNITNum; i++)
        {
            double X = CurUNIT[i].X;
            double Y = CurUNIT[i].Y;
            CalcData(newMapX, newMapY, op, Type, &X, &Y);
            CurUNIT[i].X = X + dX;
            CurUNIT[i].Y = Y + dY;
        }
 
    if (MRGNOpt == 1)
    {
        for (int i = 0; i < MRGNNum; i++)
        {
            double X = CurMRGN[i].CX / 32;
            double Y = CurMRGN[i].CY / 32;
            CalcData(newMapX, newMapY, op, Type, &X, &Y);
            CurMRGN[i].CX = (X + dX) * 32;
            CurMRGN[i].CY = (Y + dY) * 32;
            for (int j = 0; j < 4; j++)
            {
                if (j % 2 == 0) // LR
                {
                    X = CurMRGN[i].Value[j] / 32;
                    Y = CurMRGN[i].Loc[j] / 32;
                    CalcData(newMapX, newMapY, op, Type, &X, &Y);
                    CurMRGN[i].Value[j] = (X + dX) * 32;
                    CurMRGN[i].Loc[j] = CurMRGN[i].CY;
                }
                else // UD
                {
                    X = CurMRGN[i].Loc[j] / 32;
                    Y = CurMRGN[i].Value[j] / 32;
                    CalcData(newMapX, newMapY, op, Type, &X, &Y);
                    CurMRGN[i].Value[j] = (Y + dY) * 32;
                    CurMRGN[i].Loc[j] = CurMRGN[i].CX;
                }
            }
        }
    }
    else
    {
        for (int i = 0; i < MRGNNum; i++)
        {
            double X = CurMRGN[i].CX / 32;
            double Y = CurMRGN[i].CY / 32;
            CalcData(newMapX, newMapY, op, Type, &X, &Y);
            CurMRGN[i].CX = (X + dX) * 32;
            CurMRGN[i].CY = (Y + dY) * 32;
        }
    }
    
    MTXMData** tempMTXM = *CurMTXM;
    *CurMTXM = newMTXM;
    *CurMapX = newMapX;
    *CurMapY = newMapY;
    ClearMTXM(&tempMTXM, MapY);
    return;
}

