/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Provides some support for working with PCM audio.
 */
#ifndef __WVPCMUTILS_H
#define __WVPCMUTILS_H

#include "wvencoder.h"
#include "wvfunctorencoder.h"

/**
 * Data type conversion and renormalization functors.
 */
struct WvPCMSigned16ToFloatFunctor
{
    inline float operator()(signed short int pcm) const
    {
        return float(pcm) / 32768;
    }
};
struct WvPCMFloatToSigned16Functor
{
    inline signed short int operator()(float pcm) const
    {
        return (signed short int)(pcm * 32768);
    }
};
struct WvPCMSigned16ToDoubleFunctor
{
    inline double operator()(signed short int pcm) const
    {
        return double(pcm) / 32768;
    }
};
struct WvPCMDoubleToSigned16Functor
{
    inline signed short int operator()(double pcm) const
    {
        return (signed short int)(pcm * 32768);
    }
};

/**
 * Data type conversion and renormalization encoders.
 */
class WvPCMFloatToSigned16Encoder : public WvFunctorEncoder
    <signed short int, float, WvPCMFloatToSigned16Functor>
{
public:
    WvPCMFloatToSigned16Encoder() :
        WvFunctorEncoder<OT, IT, FT>(FT()) { }
};


class WvPCMSigned16ToFloatEncoder : public WvFunctorEncoder
    <float, signed short int, WvPCMSigned16ToFloatFunctor>
{
public:
    WvPCMSigned16ToFloatEncoder() :
        WvFunctorEncoder<OT, IT, FT>(FT()) { }
};

class WvPCMDoubleToSigned16Encoder : public WvFunctorEncoder
    <signed short int, double, WvPCMDoubleToSigned16Functor>
{
public:
    WvPCMDoubleToSigned16Encoder() :
        WvFunctorEncoder<OT, IT, FT>(FT()) { }
};


class WvPCMSigned16ToDoubleEncoder : public WvFunctorEncoder
    <double, signed short int, WvPCMSigned16ToDoubleFunctor>
{
public:
    WvPCMSigned16ToDoubleEncoder() :
        WvFunctorEncoder<OT, IT, FT>(FT()) { }
};

#endif // __WVPCMUTILS_H
