#pragma once
#include <cmath>
#include <cstdint>

#ifndef  M_PI
#define M_PI		3.14159265358979323846
#endif

#define _1_FACT_2 0.5
#define _1_FACT_3 0.1666666667
#define _1_FACT_4 0.04166666667
#define _1_FACT_5 0.008333333333
#define _2M_PI 2.0 * M_PI

class OnePoleLPFilter {
public:
    // OnePoleLPFilter(float cutoffFreq = 22049.0, float initSampleRate = 32000.0);
    // OnePoleLPFilter(float cutoffFreq = 14080.0, float initSampleRate = 32000.0);
    OnePoleLPFilter(float cutoffFreq = -1.f, float initSampleRate = 32000.0f) {
        setSampleRate(initSampleRate);
        setCutoffFreq(cutoffFreq);
    }

    #pragma GCC push_options
    #pragma GCC optimize ("Ofast")

    inline float process() {
        _z =  _a * input + _z * _b;
        output = _z;
        return output;
    }

    #pragma GCC pop_options

    void clear() {
        input = 0.0f;
        _z = 0.0f;
        output = 0.0f;
    }

    void setSampleRate(float sampleRate) {
        _sampleRate = sampleRate;
        _1_sampleRate_pi = (1.0f / sampleRate) * -_2M_PI;
        _maxCutoffFreq = sampleRate / 2.0f - 1.0f;
        setCutoffFreq(_cutoffFreq);
    }

    #pragma GCC push_options
    #pragma GCC optimize ("Ofast")

    inline void setCutoffFreq(const float &cutoffFreq) {
        // if (cutoffFreq == _cutoffFreq) {
        //     return;
        // }

        _cutoffFreq = cutoffFreq;
        // _b = expf(-_2M_PI * _cutoffFreq * _1_sampleRate);
        // Using a linear equation instead of exponential because
        // it uses less operations. Not sure if it will make any
        // difference. This holds for the expected {-1 < x < 0}
        // _b = (0.361f * _cutoffFreq) + 1.f;
        _b = _cutoffFreq + 1.f;
        // _b = (0.361 * -_cutoffFreq * _1_sampleRate_pi) + 1;
        _a = 1.0f - _b;
    }

    #pragma GCC pop_options

    float input = 0.0f;
    float output = 0.0f;
private:
    float _sampleRate = 32000.0f;
    float _1_sampleRate_pi = (1.0f / _sampleRate) * -_2M_PI;
    float _cutoffFreq = 0.0f;
    float _maxCutoffFreq = _sampleRate / 2.0f;
    float _a = 0.0f;
    float _b = 0.0f;
    float _z = 0.0f;
};

//////////////////////////////////////////////////////////////////////////////////////////////////

class OnePoleHPFilter {
public:
    // OnePoleHPFilter(float initCutoffFreq = 10.0, float initSampleRate = 32000.0);
    OnePoleHPFilter(float initCutoffFreq = 0.0f, float initSampleRate = 32000.0f) {
        setSampleRate(initSampleRate);
        setCutoffFreq(initCutoffFreq);
        clear();
    }

    #pragma GCC push_options
    #pragma GCC optimize ("Ofast")

    inline float process() {
        _x0 = input;
        _y0 = _a0 * _x0 + _a1 * _x1 + _b1 * _y1;
        _y1 = _y0;
        _x1 = _x0;
        output = _y0;
        return _y0;
    }

    #pragma GCC pop_options

    void clear() {
        input = 0.0f;
        output = 0.0f;
        _x0 = 0.0f;
        _x1 = 0.0f;
        _y0 = 0.0f;
        _y1 = 0.0f;
    }

    #pragma GCC push_options
    #pragma GCC optimize ("Ofast")

    inline void setCutoffFreq(const float &cutoffFreq) {
        // if (cutoffFreq == _cutoffFreq) {
        //     return;
        // }
        
        _cutoffFreq = cutoffFreq;
        // See above.
        _b1 = _cutoffFreq + 1.f;
        // _b1 = (0.361f * _cutoffFreq) + 1.f;
        // _b1 = (0.361 * _cutoffFreq * _1_sampleRate_pi) + 1;
        // _b1 = expf(-_2M_PI * _cutoffFreq * _1_sampleRate);
        // _a0 = (1.0f + _b1) / 2.0f;
        _a0 = (1.0f + _b1) * 0.5f;
        _a1 = -_a0;
    }

    #pragma GCC pop_options

    void setSampleRate(float sampleRate) {
        _sampleRate = sampleRate;
        _1_sampleRate_pi = (1.0f / sampleRate) * -_2M_PI;
        _maxCutoffFreq = sampleRate / 2.0f - 1.0f;
        setCutoffFreq(_cutoffFreq);
        clear();
    }

    float input = 0.0f;
    float output = 0.0f;
private:
    float _sampleRate = 32000.0f;
    float _1_sampleRate_pi = (1.0f / _sampleRate) * -_2M_PI;
    float _cutoffFreq = 0.0f;
    float _maxCutoffFreq = _sampleRate / 2.0f - 1.0f;
    float _y0 = 0.0f;
    float _y1 = 0.0f;
    float _x0 = 0.0f;
    float _x1 = 0.0f;
    float _a0 = 0.0f;
    float _a1 = 0.0f;
    float _b1 = 0.0f;
};