/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** \file
 * Provides some support for working with PCM audio.
 */
#ifndef __WVPCMUTILS_H
#define __WVPCMUTILS_H

#include "wvencoder.h"
#include "wvfunctorencoder.h"

struct WvPCMSigned16ToNormFloatFunctor
{
    inline float operator()(signed short int pcm) const
    {
        return float(pcm) / 32768;
    }
};
struct WvPCMNormFloatToSigned16Functor
{
    inline signed short int operator()(float pcm) const
    {
        return (pcm < -1.0f) ? -32768 : (pcm >= 1.0f) ? 32767 :
            (signed short int)(pcm * 32768);
    }
};
struct WvPCMSigned16ToUnnormFloatFunctor
{
    inline float operator()(signed short int pcm) const
    {
        return float(pcm);
    }
};
struct WvPCMUnnormFloatToSigned16Functor
{
    inline signed short int operator()(float pcm) const
    {
        return (pcm < -32768.0f) ? -32768 : (pcm >= 32767.0f) ? 32767 :
            (signed short int)(pcm);
    }
};
struct WvPCMSigned16ToNormDoubleFunctor
{
    inline double operator()(signed short int pcm) const
    {
        return double(pcm) / 32768;
    }
};
struct WvPCMNormDoubleToSigned16Functor
{
    inline signed short int operator()(double pcm) const
    {
        return (pcm < -1.0) ? -32768 : (pcm >= 1.0f) ? 32767 :
            (signed short int)(pcm * 32768);
    }
};


/**
 * An encoder that converts PCM audio from <em>normalized</em>
 * floats to 16 bit signed short ints.
 */
class WvPCMNormFloatToSigned16Encoder : public WvFunctorEncoder
    <float, signed short int, WvPCMNormFloatToSigned16Functor>
{
public:
    WvPCMNormFloatToSigned16Encoder() :
        WvFunctorEncoder<IType, OType, FType>(FType()) { }
};


/**
 * An encoder that converts PCM audio from 16 bit signed short ints
 * to <em>normalized</em> floats.
 */
class WvPCMSigned16ToNormFloatEncoder : public WvFunctorEncoder
    <signed short int, float, WvPCMSigned16ToNormFloatFunctor>
{
public:
    WvPCMSigned16ToNormFloatEncoder() :
        WvFunctorEncoder<IType, OType, FType>(FType()) { }
};

/**
 * An encoder that converts PCM audio from <em>unnormalized</em>
 * floats to 16 bit signed short ints.
 */
class WvPCMUnnormFloatToSigned16Encoder : public WvFunctorEncoder
    <float, signed short int, WvPCMUnnormFloatToSigned16Functor>
{
public:
    WvPCMUnnormFloatToSigned16Encoder() :
        WvFunctorEncoder<IType, OType, FType>(FType()) { }
};


/**
 * An encoder that converts PCM audio from 16 bit signed short ints
 * to <em>unnormalized</em> floats.
 */
class WvPCMSigned16ToUnnormFloatEncoder : public WvFunctorEncoder
    <signed short int, float, WvPCMSigned16ToUnnormFloatFunctor>
{
public:
    WvPCMSigned16ToUnnormFloatEncoder() :
        WvFunctorEncoder<IType, OType, FType>(FType()) { }
};

/**
 * An encoder that converts PCM audio from <em>normalized</em>
 * doubles to 16 bit signed short ints.
 */
class WvPCMNormDoubleToSigned16Encoder : public WvFunctorEncoder
    <double, signed short int, WvPCMNormDoubleToSigned16Functor>
{
public:
    WvPCMNormDoubleToSigned16Encoder() :
        WvFunctorEncoder<IType, OType, FType>(FType()) { }
};


/**
 * An encoder that converts PCM audio from 16 bit signed short ints
 * to <em>normalized</em> doubles.
 */
class WvPCMSigned16ToNormDoubleEncoder : public WvFunctorEncoder
    <signed short int, double, WvPCMSigned16ToNormDoubleFunctor>
{
public:
    WvPCMSigned16ToNormDoubleEncoder() :
        WvFunctorEncoder<IType, OType, FType>(FType()) { }
};

#endif // __WVPCMUTILS_H
