/**
 * @file InterpDelay2.hpp
 * @author Dale Johnson, Valley Audio Soft
 * @brief A more optimised version of the linear interpolating delay.
 */

#pragma once
#include <vector>
#include <cstdint>
#include "../../utilities/Utilities.hpp"

// float linterp(float a, float b, float f) {
//     return a + f * (b - a);
// }

extern float DSY_SDRAM_BSS sdramData[32][200000];
extern int count;

class InterpDelay {
public:
    float input = 0.;
    float output = 0.;
    int bufferNumber;

    InterpDelay(uint32_t maxLength = 512, uint32_t initDelayTime = 0) {
        l = maxLength;

        bufferNumber = count++;

        setDelayTime(initDelayTime);
    }

    void process() {
        sdramData[bufferNumber][static_cast<size_t>(w)] = input;
        int32_t r = w - t;
        
        if (r < 0) {
            r += l;
        }

        ++w;
        if (w == l) {
            w = 0;
        }

        int32_t upperR = r - 1;
        if (upperR < 0) {
            upperR += l;
        }

        output = sdramData[bufferNumber][static_cast<size_t>(r)] + f * (sdramData[bufferNumber][static_cast<size_t>(upperR)] - sdramData[bufferNumber][static_cast<size_t>(r)]);
        // output = linterp(sdramData[bufferNumber][static_cast<size_t>(r)], sdramData[bufferNumber][static_cast<size_t>(upperR)], f);
    }

    float tap(int32_t i) const {
        int32_t j = w - i;
        if (j < 0) {
            j += l;
        }
        return sdramData[bufferNumber][static_cast<size_t>(j)];
    }

    void setDelayTime(float newDelayTime) {
        if (newDelayTime >= l) {
            newDelayTime = l - 1;
        }
        if (newDelayTime < 0) {
            newDelayTime = 0;
        }
        t = static_cast<int32_t>(newDelayTime);
        f = newDelayTime - static_cast<float>(t);
    }

    void clear() {
        input = 0.;
        output = 0.;
    }

private:
    int32_t w = 0;
    int32_t t = 0;
    float f = 0.;
    int32_t l = 512;
};
