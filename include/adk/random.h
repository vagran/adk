/* This file is a part of 'ADK' project.
 * Copyright (c) 2012-2015, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file random.h */

#ifndef ADK_RANDOM_H_
#define ADK_RANDOM_H_

namespace adk {

/** Predictable, portable and fast random number generator based on CMWC4096
 * algorithm. Can be used to guarantee the same sequence on all platforms for
 * the same seed.
 */
class Random {
public:
    /** Initialized with random seed. */
    Random();

    /** Initialized with the specified seed. */
    Random(int seed);

    /** Get random 32-bits integer. */
    i32
    GetInt32();

    /** Get random 64-bits integer. */
    i64
    GetInt64();

    /** Get random float in range 0..1. */
    float
    GetFloat();

    /** Get random double in range 0..1. */
    double
    GetDouble();

    /** Get random number from Gaussian distribution. Median is zero.
     *
     * @param variance Desired variance value of the distribution.
     */
    double
    GetGaussian(double variance);

    /** Get random number from Gaussian distribution.
    *
    * @param variance Desired variance value of the distribution.
    * @param median Desired median value of the distribution.
    */
    double
    GetGaussian(double median, double deviation);

private:
    int Q[4096];
    int c = 362436;
    int idx = 4095;
    double gauss1, gauss2;
    bool gaussValid = false;
};

} /* namespace adk */

#endif /* ADK_RANDOM_H_ */
