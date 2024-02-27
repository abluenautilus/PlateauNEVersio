#pragma once
#include "InterpDelay.hpp"

class AllpassFilter {
public:
    AllpassFilter() {
        clear();
        gain = 0.;
    }

    AllpassFilter(int maxDelay, int initDelay = 0, float gain = 0.) {
        clear();
        delay = InterpDelay(maxDelay, initDelay);
        this->gain = gain;
    }

    float inline process() {
        _inSum = input + delay.output * gain;
        output = delay.output + _inSum * gain * -1;
        delay.input = _inSum;
        delay.process();
        return output;
    }

    void clear() {
        input = 0;
        output = 0;
        _inSum = 0;
        _outSum = 0;
        delay.clear();
    }

    void setGain(const float newGain) {
        gain = newGain;
    }

    float input;
    float output;
    InterpDelay delay;

private:
    float gain;
    float _inSum;
    float _outSum;
};