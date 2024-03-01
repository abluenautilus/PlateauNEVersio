#pragma once
#include <vector>
#include <cstdint>
#include "../../utilities/Utilities.hpp"

extern float DSY_SDRAM_BSS sdramData[50][200000];
extern unsigned int count;
extern bool hold;

class InterpDelay {
public:
    float input = 0.;
    float output = 0.;
    int bufferNumber = 0;
    int r = 0;
    int upperR = 0;
    int j = 0;
    float dataR = 0.f;
    float dataUpperR = 0.f;

    InterpDelay(unsigned int maxLength = 512, unsigned int initDelayTime = 0) {
        l = maxLength;

        bufferNumber = ++count;

        setDelayTime(initDelayTime);
    }

    #pragma GCC push_options
    #pragma GCC optimize ("Ofast")

    void process() {
        sdramData[bufferNumber][w] = input;
        r = w - t;
        
        if (r < 0) {
            r += l;
        }

        // if (r >= l) {
        //     r -= l;
        // }

        ++w;
        if (w >= l) {
            w = 0;
        }

        upperR = r - 1;
        if (upperR < 0) {
            upperR += l;
        }

        // if (upperR >= l) {
        //     upperR -= l;
        // }

        dataR = sdramData[bufferNumber][r];

        dataUpperR = sdramData[bufferNumber][upperR];

        output = hold * (dataR + f * (dataUpperR - dataR));
    }

    #pragma GCC pop_options
    #pragma GCC push_options
    #pragma GCC optimize ("Ofast")

    float tap(int i) {
        j = w - i;
        if (j < 0) {
            j += l;
        }
        return sdramData[bufferNumber][j];
    }

    #pragma GCC pop_options
    #pragma GCC push_options
    #pragma GCC optimize ("Ofast")

    void setDelayTime(float newDelayTime) {
        if (newDelayTime >= l) {
            newDelayTime = l - 1;
        }
        if (newDelayTime < 0) {
            newDelayTime = 0;
        }
        t = newDelayTime;
        f = newDelayTime - t;
    }

    #pragma GCC pop_options

    void clear() {
        input = 0.;
        output = 0.;
    }

private:
    int  w = 0;
    int t = 0;
    float f = 0.f;
    int l = 512;
};
