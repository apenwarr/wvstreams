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
        return (pcm < -1.0f) ? -32768 : (pcm >= 1.0f) ? 32767 :
            (signed short int)(pcm * 32768);
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
        return (pcm < -1.0) ? -32768 : (pcm >= 1.0f) ? 32767 :
            (signed short int)(pcm * 32768);
    }
};


/**
 * An encoder that converts PCM audio from normalized floats to 
 * 16 bit signed short ints.
 */
class WvPCMFloatToSigned16Encoder : public WvFunctorEncoder
    <float, signed short int, WvPCMFloatToSigned16Functor>
{
public:
    WvPCMFloatToSigned16Encoder() :
        WvFunctorEncoder<IType, OType, FType>(FType()) { }
};


/**
 * An encoder that converts PCM audio from 16 bit signed short ints
 * to normalized floats.
 */
class WvPCMSigned16ToFloatEncoder : public WvFunctorEncoder
    <signed short int, float, WvPCMSigned16ToFloatFunctor>
{
public:
    WvPCMSigned16ToFloatEncoder() :
        WvFunctorEncoder<IType, OType, FType>(FType()) { }
};

/**
 * An encoder that converts PCM audio from normalized doubles to
 * 16 bit signed short ints.
 */
class WvPCMDoubleToSigned16Encoder : public WvFunctorEncoder
    <double, signed short int, WvPCMDoubleToSigned16Functor>
{
public:
    WvPCMDoubleToSigned16Encoder() :
        WvFunctorEncoder<IType, OType, FType>(FType()) { }
};


/**
 * An encoder that converts PCM audio from 16 bit signed short ints
 * to normalized doubles.
 */
class WvPCMSigned16ToDoubleEncoder : public WvFunctorEncoder
    <signed short int, double, WvPCMSigned16ToDoubleFunctor>
{
public:
    WvPCMSigned16ToDoubleEncoder() :
        WvFunctorEncoder<IType, OType, FType>(FType()) { }
};

#endif // __WVPCMUTILS_H
