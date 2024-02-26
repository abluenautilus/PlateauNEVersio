/**
 * @file AllpassFilter.hpp
 * @author Dale Johnson
 * @date ...uhhh
 */

#pragma once
#include "InterpDelay.hpp"

class AllpassFilter {
public:
    AllpassFilter() {
        clear();
        gain = 0.;
    }

    AllpassFilter(long maxDelay, long initDelay = 0, float gain = 0.) {
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

// template<class T>
// class NestedAllPassType1 {
// public:
//     NestedAllPassType1() {
//         gain1 = 0;
//         gain2 = 0;
//         decay1 = 0;
//         decay2 = 0;
//         clear();
//     }

//     NestedAllPassType1(long maxDelay, long delayTime1, long delayTime2) {
//         delay1 = InterpDelay(maxDelay, delayTime1);
//         delay2 = InterpDelay(maxDelay, delayTime2);
//         gain1 = 0;
//         gain2 = 0;
//         decay1 = 0;
//         decay2 = 0;
//         clear();
//     }

//     float inline process() {
//         _inSum1 = input + delay1.output * gain1;
//         _inSum2 = _inSum1 + delay2.output * gain2;
//         delay2.input = _inSum2;
//         delay1.input = delay2.output * decay2 + _inSum2 * -gain2;
//         output = delay1.output * decay1 + _inSum1 * -gain1;
//         delay1.process();
//         delay2.process();
//         return output;
//     }

//     void clear() {
//         input = 0;
//         output = 0;
//         _inSum1 = 0;
//         _inSum2 = 0;
//         delay1.clear();
//         delay2.clear();
//     }

//     float input;
//     float gain1, gain2;;
//     float output;
//     float decay1, decay2;
//     InterpDelay delay1, delay2;
// private:
//     float _inSum1, _inSum2;
// };
