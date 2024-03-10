#pragma once
#include <vector>
#include <cstdint>
#include "../../utilities/Utilities.hpp"

extern float DSY_SDRAM_BSS sdramData[50][144000];
extern unsigned int count;
extern float hold;

class InterpDelay {
public:
    float input = 0.f;
    float output = 0.f;
    int bufferNumber = 0;
    int r = 0;
    int upperR = 0;
    int j = 0;
    float dataR = 0.f;
    float dataUpperR = 0.f;

    //InterpDelay () {}

    InterpDelay(unsigned int maxLength = 512, float initDelayTime = 0.f) {
        l = maxLength;
        lFloat = static_cast<float>(maxLength);

        bufferNumber = ++count;

        setDelayTime(initDelayTime);
    }

    // inline void initializeDelay(const unsigned int &length = 512, const float &delayTime = 0.f) {
    //     l = length;
    //     lFloat = static_cast<float>(length);

    //     bufferNumber = ++count;

    //     setDelayTime(delayTime);
    // }

    #pragma GCC push_options
    #pragma GCC optimize ("Ofast")

    inline void process() {
        sdramData[bufferNumber][w] = input;
        r = w - t;
        
        if (r < 0) {
            r += l;
        }

        ++w;
        if (w >= l) {
            w = 0;
        }

        upperR = r - 1;
        if (upperR < 0) {
            upperR += l;
        }

        dataR = sdramData[bufferNumber][r];

        output = hold * (dataR + f * (sdramData[bufferNumber][upperR] - dataR));
    }

    #pragma GCC pop_options
    #pragma GCC push_options
    #pragma GCC optimize ("Ofast")

    inline float tap(const int &i) {
        j = w - i;
        if (j < 0) {
            j += l;
        }
        return sdramData[bufferNumber][j];
    }

    #pragma GCC pop_options
    #pragma GCC push_options
    #pragma GCC optimize ("Ofast")

    inline void setDelayTime(float newDelayTime) {
        if (newDelayTime >= lFloat) {
            newDelayTime = lFloat - 1.f;
        }
        if (newDelayTime < 0.f) {
            newDelayTime = 0.f;
        }
        t = static_cast<int>(newDelayTime);
        f = newDelayTime - static_cast<float>(t);
    }

    #pragma GCC pop_options

    void clear() {
        // for(int i = 0; i < l; ++i) {
        //     sdramData[bufferNumber][i] = 0;
        // }
        input = 0.f;
        output = 0.f;
    }

private:
    int  w = 0;
    int t = 0;
    float f = 0.f;
    int l = 512;
    float lFloat = 512.f;
};
