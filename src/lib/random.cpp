/* This file is a part of 'ADK' project.
 * Copyright (c) 2012-2015, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file random.cpp */

#include <adk.h>

using namespace adk;

Random::Random():
    Random(rand())
{}

Random::Random(int seed)
{
    const int PHI = 0x9e3779b9;
    Q[0] = seed + PHI;
    Q[1] = seed + PHI + PHI;
    Q[2] = seed + PHI + PHI + PHI;
    for (int i = 3; i < 4096; i++) {
        Q[i] = Q[i - 3] + Q[i - 2] + PHI + i;
    }
}

int
Random::GetInt32()
{
    idx = (idx + 1) & 4095;
    long t = (18705L * Q[idx]) + c;
    c = static_cast<int>(t >> 32);
    Q[idx] = static_cast<int>(0xfffffffeL - t);
    return Q[idx];
}

long
Random::GetInt64()
{
    return (static_cast<long>(GetInt32()) << 32) + GetInt32();
}

float
Random::GetFloat()
{
    u32 i = static_cast<u32>(GetInt32());
    return static_cast<float>(i) / MAX_U32;
}

double
Random::GetDouble()
{
    u64 i = static_cast<u64>(GetInt64());
    return static_cast<double>(i) / static_cast<double>(MAX_U64);
}

double
Random::GetGaussian(double variance)
{
    if (gaussValid) {
        gaussValid = false;
        return sqrt(variance * gauss1) * sin(gauss2);
    }
    gaussValid = true;
    gauss1 = GetDouble();
    if (gauss1 < 1e-100) {
        gauss1 = 1e-100;
    }
    gauss1 = -2.0 * log(gauss1);
    gauss2 = GetDouble() * M_PI * 2.0;
    return sqrt(variance * gauss1) * cos(gauss2);
}

double
Random::GetGaussian(double median, double deviation)
{
    return median + GetGaussian(deviation * deviation);
}
