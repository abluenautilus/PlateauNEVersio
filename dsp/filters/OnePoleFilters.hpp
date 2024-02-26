#pragma once
#include <cmath>
#include <cstdint>

#ifndef  M_PI
#define  M_PI  3.1415926535897932384626433
#endif

#define _1_FACT_2 0.5
#define _1_FACT_3 0.1666666667
#define _1_FACT_4 0.04166666667
#define _1_FACT_5 0.008333333333
#define _2M_PI 2.0 * M_PI

class OnePoleLPFilter {
public:
    OnePoleLPFilter(float cutoffFreq = 22049.0, float initSampleRate = 48000.0);
    float process();
    void clear();
    void setCutoffFreq(float cutoffFreq);
    void setSampleRate(float sampleRate);
    float getMaxCutoffFreq() const;
    float input = 0.0;
    float output = 0.0;
private:
    float _sampleRate = 48000.0;
    float _1_sampleRate = 1.0 / _sampleRate;
    float _cutoffFreq = 0.0;
    float _maxCutoffFreq = _sampleRate / 2.0;
    float _a = 0.0;
    float _b = 0.0;
    float _z = 0.0;
};

//////////////////////////////////////////////////////////////////////////////////////////////////

class OnePoleHPFilter {
public:
    OnePoleHPFilter(float initCutoffFreq = 10.0, float initSampleRate = 48000.0);
    float process();
    void clear();
    void setCutoffFreq(float cutoffFreq);
    void setSampleRate(float sampleRate);
    float input = 0.0;
    float output = 0.0;
private:
    float _sampleRate = 0.0;
    float _1_sampleRate = 0.0;
    float _cutoffFreq = 0.0;
    float _maxCutoffFreq = _sampleRate / 2.0 - 1.0;
    float _y0 = 0.0;
    float _y1 = 0.0;
    float _x0 = 0.0;
    float _x1 = 0.0;
    float _a0 = 0.0;
    float _a1 = 0.0;
    float _b1 = 0.0;
};

class DCBlocker {
public:
    DCBlocker();
    DCBlocker(float cutoffFreq);
    float process(float input);
    void clear();
    void setCutoffFreq(float cutoffFreq);
    void setSampleRate(float sampleRate);
    float getMaxCutoffFreq() const;
    float output;
private:
    float _sampleRate;
    float _cutoffFreq;
    float _maxCutoffFreq;
    float _b;
    float _z;
};
