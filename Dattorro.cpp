#include "Dattorro.hpp"
#include <algorithm>

// float scale(float a, float inMin, float inMax, float outMin, float outMax) {
//     return (a - inMin)/(inMax - inMin) * (outMax - outMin) + outMin;
// }

Dattorro1997Tank::Dattorro1997Tank(const float initSampleRate,
                                   const float initMaxLfoDepth,
                                   const float initMaxTimeScale) :
    maxTimeScale(initMaxTimeScale) 
{
    timePadding = initMaxLfoDepth;
    setSampleRate(initSampleRate);

    // leftOutDCBlock.setCutoffFreq(20.0);
    // rightOutDCBlock.setCutoffFreq(20.0);

    // // Equivalent to the same cutoff in the exponential function
    // leftOutDCBlock.setCutoffFreq(55.0);
    // rightOutDCBlock.setCutoffFreq(55.0);

    // Equivalent to the same cutoff in the exponential function
    leftOutDCBlock.setCutoffFreq(-0.004f);
    rightOutDCBlock.setCutoffFreq(-0.004f);

    lfo1.setFrequency(lfo1Freq);
    lfo2.setFrequency(lfo2Freq);
    lfo3.setFrequency(lfo3Freq);
    lfo4.setFrequency(lfo4Freq);

    lfo2.phase = 0.25f;
    lfo3.phase = 0.5f;
    lfo4.phase = 0.75f;

    lfo1.setRevPoint(0.5f);
    lfo2.setRevPoint(0.5f);
    lfo3.setRevPoint(0.5f);
    lfo4.setRevPoint(0.5f);
}

void Dattorro1997Tank::process(const float leftIn, const float rightIn,
                               float* leftOut, float* rightOut) {
    tickApfModulation();

    //decay = frozen ? 1.f : decayParam;
    decay = decayParam;

    leftSum += leftIn;
    rightSum += rightIn;

    leftApf1.input = leftSum;
    leftDelay1.input = leftApf1.process();
    leftDelay1.process();
    leftHighCutFilter.input = leftDelay1.output;
    leftLowCutFilter.input = leftHighCutFilter.process();
    leftApf2.input = (leftDelay1.output * (1.f - fade) + leftLowCutFilter.process() * fade) * decay;
    leftDelay2.input = leftApf2.process();
    leftDelay2.process();

    rightApf1.input = rightSum;
    rightDelay1.input = rightApf1.process();
    rightDelay1.process();
    rightHighCutFilter.input = rightDelay1.output;
    rightLowCutFilter.input =  rightHighCutFilter.process();
    rightApf2.input = (rightDelay1.output * (1.f - fade) + rightLowCutFilter.process() * fade) * decay;
    rightDelay2.input = rightApf2.process();
    rightDelay2.process();

    rightSum = leftDelay2.output * decay;
    leftSum = rightDelay2.output * decay;

    leftOutDCBlock.input = leftApf1.output;
    leftOutDCBlock.input += leftDelay1.tap(scaledOutputTaps[L_DELAY_1_L_TAP_1]);
    leftOutDCBlock.input += leftDelay1.tap(scaledOutputTaps[L_DELAY_1_L_TAP_2]);
    leftOutDCBlock.input -= leftApf2.delay.tap(scaledOutputTaps[L_APF_2_L_TAP]);
    leftOutDCBlock.input += leftDelay2.tap(scaledOutputTaps[L_DELAY_2_L_TAP]);
    leftOutDCBlock.input -= rightDelay1.tap(scaledOutputTaps[R_DELAY_1_L_TAP]);
    leftOutDCBlock.input -= rightApf2.delay.tap(scaledOutputTaps[R_APF_2_L_TAP]);
    leftOutDCBlock.input -= rightDelay2.tap(scaledOutputTaps[R_DELAY_2_L_TAP]);

    rightOutDCBlock.input = rightApf1.output;
    rightOutDCBlock.input += rightDelay1.tap(scaledOutputTaps[R_DELAY_1_R_TAP_1]);
    rightOutDCBlock.input += rightDelay1.tap(scaledOutputTaps[R_DELAY_1_R_TAP_2]);
    rightOutDCBlock.input -= rightApf2.delay.tap(scaledOutputTaps[R_APF_2_R_TAP]);
    rightOutDCBlock.input += rightDelay2.tap(scaledOutputTaps[R_DELAY_2_R_TAP]);
    rightOutDCBlock.input -= leftDelay1.tap(scaledOutputTaps[L_DELAY_1_R_TAP]);
    rightOutDCBlock.input -= leftApf2.delay.tap(scaledOutputTaps[L_APF_2_R_TAP]);
    rightOutDCBlock.input -= leftDelay2.tap(scaledOutputTaps[L_DELAY_2_R_TAP]);

    *leftOut = leftOutDCBlock.process() * 0.5f;
    *rightOut = rightOutDCBlock.process() * 0.5f;

    fade += fadeStep * fadeDir;
    fade = (fade < 0.f) ? 0.f : ((fade > 1.f) ? 1.f : fade);
}

void Dattorro1997Tank::freeze(bool freezeFlag) {
    frozen = freezeFlag;
    if (frozen) {
        fadeDir = -1.f;
        //decay = 1.f;
    }
    else {
        fadeDir = 1.f;
        //decay = decayParam;
    }
}

void Dattorro1997Tank::setSampleRate(const float newSampleRate) {
    sampleRate = newSampleRate;
    sampleRate = sampleRate > maxSampleRate ? maxSampleRate : sampleRate;
    sampleRate = sampleRate < 1.f ? 1.f : sampleRate;
    sampleRateScale = sampleRate / dattorroSampleRate;

    fadeStep = 1.f / sampleRate;

    leftOutDCBlock.setSampleRate(sampleRate);
    rightOutDCBlock.setSampleRate(sampleRate);

    rescaleTapTimes();
    setTimeScale(timeScale);
    initialiseDelaysAndApfs();
    //clear();
}

#pragma GCC push_options
#pragma GCC optimize ("Ofast")

void Dattorro1997Tank::setTimeScale(float newTimeScale) {
    timeScale = newTimeScale < 0.0001f ? 0.0001f : newTimeScale;

    rescaleApfAndDelayTimes();
}

#pragma GCC pop_options

void Dattorro1997Tank::setDecay(const float newDecay) {
    decayParam = (float)(newDecay > 1.f ? 1.f :
                         (newDecay < 0.f ? 0.f : newDecay));
}

void Dattorro1997Tank::setModSpeed(const float newModSpeed) {
    lfo1.setFrequency(lfo1Freq * newModSpeed);
    lfo2.setFrequency(lfo2Freq * newModSpeed);
    lfo3.setFrequency(lfo3Freq * newModSpeed);
    lfo4.setFrequency(lfo4Freq * newModSpeed);
}

void Dattorro1997Tank::setModDepth(const float newModDepth) {
    modDepth = newModDepth;
    lfoExcursion = newModDepth * lfoMaxExcursion * sampleRateScale;
}

void Dattorro1997Tank::setModShape(const float shape) {
    lfo1.setRevPoint(shape);
    lfo2.setRevPoint(shape);
    lfo3.setRevPoint(shape);
    lfo4.setRevPoint(shape);
}

void Dattorro1997Tank::setHighCutFrequency(const float frequency) {
    leftHighCutFilter.setCutoffFreq(frequency);
    rightHighCutFilter.setCutoffFreq(frequency);
}

void Dattorro1997Tank::setLowCutFrequency(const float frequency) {
    leftLowCutFilter.setCutoffFreq(frequency);
    rightLowCutFilter.setCutoffFreq(frequency);
}

// float diffusion1 = 0.f;
// float diffusion2 = 0.f;

void Dattorro1997Tank::setDiffusion(const float diffusion) {
    //diffusion1 = scale(diffusion, 0.0, 10.0, 0.0, maxDiffusion1);
    //diffusion2 = scale(diffusion, 0.0, 10.0, 0.0, maxDiffusion2);

    // leftApf1.setGain(-diffusion1);
    // leftApf2.setGain(diffusion2);
    // rightApf1.setGain(-diffusion1);
    // rightApf2.setGain(diffusion2);

    // It is important that apf1 has inverted diffusion.
    // In the main code the diffusion is always set to the maximum of 0.7f
    leftApf1.setGain(-diffusion);
    leftApf2.setGain(diffusion);
    rightApf1.setGain(-diffusion);
    rightApf2.setGain(diffusion);
}

void Dattorro1997Tank::clear() {
    leftApf1.clear();
    leftDelay1.clear();
    leftHighCutFilter.clear();
    leftLowCutFilter.clear();
    leftApf2.clear();
    leftDelay2.clear();

    rightApf1.clear();
    rightDelay1.clear();
    rightHighCutFilter.clear();;
    rightLowCutFilter.clear();
    rightApf2.clear();
    rightDelay2.clear();

    leftOutDCBlock.clear();
    rightOutDCBlock.clear();

    leftSum = 0.f;
    rightSum = 0.f;
}

int maxScaledOutputTap = 0;

inline int Dattorro1997Tank::calcMaxTime(float delayTime) {
    maxScaledOutputTap = *std::max_element(scaledOutputTaps.begin(),
                                                scaledOutputTaps.end());

    return (int)(sampleRateScale * (delayTime * maxTimeScale + 
                                         maxScaledOutputTap + timePadding));
}

void Dattorro1997Tank::initialiseDelaysAndApfs() {
    // int maxScaledOutputTap = *std::max_element(scaledOutputTaps.begin(),
    //                                             scaledOutputTaps.end());
    // int calcMaxTime = [&](float delayTime) -> int {
    //     return (int)(sampleRateScale * (delayTime * maxTimeScale + 
    //                                      maxScaledOutputTap + timePadding));
    // };

    const int kLeftApf1MaxTime = calcMaxTime(leftApf1Time);
    const int kLeftDelay1MaxTime = calcMaxTime(leftDelay1Time);
    const int kLeftApf2MaxTime = calcMaxTime(leftApf2Time);
    const int kLeftDelay2MaxTime = calcMaxTime(leftDelay2Time);
    const int kRightApf1MaxTime = calcMaxTime(rightApf1Time);
    const int kRightDelay1MaxTime = calcMaxTime(rightDelay1Time);
    const int kRightApf2MaxTime = calcMaxTime(rightApf2Time);
    const int kRightDelay2MaxTime = calcMaxTime(rightDelay2Time);

    leftApf1 = AllpassFilter(kLeftApf1MaxTime);
    leftDelay1 = InterpDelay(kLeftDelay1MaxTime);
    leftApf2 = AllpassFilter(kLeftApf2MaxTime);
    leftDelay2 = InterpDelay(kLeftDelay2MaxTime);
    rightApf1 = AllpassFilter(kRightApf1MaxTime);
    rightDelay1 = InterpDelay(kRightDelay1MaxTime);
    rightApf2 = AllpassFilter(kRightApf2MaxTime);
    rightDelay2 = InterpDelay(kRightDelay2MaxTime);
}

void Dattorro1997Tank::tickApfModulation() {
    leftApf1.delay.setDelayTime(lfo1.process() * lfoExcursion + scaledLeftApf1Time);
    leftApf2.delay.setDelayTime(lfo2.process() * lfoExcursion + scaledLeftApf2Time);
    rightApf1.delay.setDelayTime(lfo3.process() * lfoExcursion + scaledRightApf1Time);
    rightApf2.delay.setDelayTime(lfo4.process() * lfoExcursion + scaledRightApf2Time);
}

float scaleFactor = 0.f;


#pragma GCC push_options
#pragma GCC optimize ("Ofast")

void Dattorro1997Tank::rescaleApfAndDelayTimes() {
    scaleFactor = timeScale * sampleRateScale;

    scaledLeftApf1Time = leftApf1Time * scaleFactor;
    scaledLeftDelay1Time = leftDelay1Time * scaleFactor;
    scaledLeftApf2Time = leftApf2Time * scaleFactor;
    scaledLeftDelay2Time = leftDelay2Time * scaleFactor;

    scaledRightApf1Time = rightApf1Time * scaleFactor;
    scaledRightDelay1Time = rightDelay1Time * scaleFactor;
    scaledRightApf2Time = rightApf2Time * scaleFactor;
    scaledRightDelay2Time = rightDelay2Time * scaleFactor;

    leftDelay1.setDelayTime(scaledLeftDelay1Time);
    leftDelay2.setDelayTime(scaledLeftDelay2Time);
    rightDelay1.setDelayTime(scaledRightDelay1Time);
    rightDelay2.setDelayTime(scaledRightDelay2Time);
}

#pragma GCC pop_options

void Dattorro1997Tank::rescaleTapTimes() {
    for (size_t i = 0; i < scaledOutputTaps.size(); ++i) {
        scaledOutputTaps[i] = (int)((float)kOutputTaps[i] * sampleRateScale);
    }
}

Dattorro::Dattorro(const float initMaxSampleRate,
                   const float initMaxLfoDepth,
                   const float initMaxTimeScale)
    : tank(initMaxSampleRate, initMaxLfoDepth, initMaxTimeScale)
{
    sampleRate = initMaxSampleRate;
    dattorroScaleFactor = sampleRate / dattorroSampleRate;

    preDelay = InterpDelay(192010, 0.f);

    // 22000 goes outside the range fo the linear function.
    // inputLpf = OnePoleLPFilter(22000.0);
    inputLpf = OnePoleLPFilter(-1.f);
    inputHpf = OnePoleHPFilter(0.f);

    inApf1 = AllpassFilter(dattorroScale(8 * kInApf1Time), dattorroScale(kInApf1Time), inputDiffusion1);
    inApf2 = AllpassFilter(dattorroScale(8 * kInApf2Time), dattorroScale(kInApf2Time), inputDiffusion1);
    inApf3 = AllpassFilter(dattorroScale(8 * kInApf3Time), dattorroScale(kInApf3Time), inputDiffusion2);
    inApf4 = AllpassFilter(dattorroScale(8 * kInApf4Time), dattorroScale(kInApf4Time), inputDiffusion2);

    // leftInputDCBlock.setCutoffFreq(20.0);
    // rightInputDCBlock.setCutoffFreq(20.0);

    // // Equivalent to the same amount in the exponential function
    // leftInputDCBlock.setCutoffFreq(55.0);
    // rightInputDCBlock.setCutoffFreq(55.0);

    // Equivalent to the same amount in the exponential function
    leftInputDCBlock.setCutoffFreq(-0.004f);
    rightInputDCBlock.setCutoffFreq(-0.004f);
}

//float subApfOut = 0.f;

void Dattorro::process(float leftInput, float rightInput) {
    leftInputDCBlock.input = leftInput;
    rightInputDCBlock.input = rightInput;
    //Redundant
    //inputLpf.setCutoffFreq(inputHighCut);
    //inputHpf.setCutoffFreq(inputLowCut);
    inputLpf.input = leftInputDCBlock.process() + rightInputDCBlock.process();
    inputHpf.input = inputLpf.process();
    inputHpf.process();
    preDelay.input = inputHpf.output;
    preDelay.process();
    inApf1.input = preDelay.output;
    inApf2.input = inApf1.process();
    inApf3.input = inApf2.process();
    inApf4.input = inApf3.process();
    // Might be slower than caching and using registers
    //subApfOut = inApf4.process();
    tankFeed = preDelay.output * (1.f - diffuseInput) + inApf4.process() * diffuseInput;

    tank.process(tankFeed, tankFeed, &leftOut, &rightOut);
}

void Dattorro::clear() {
    leftInputDCBlock.clear();
    rightInputDCBlock.clear();

    inputLpf.clear();
    inputHpf.clear();
    preDelay.clear();
    inApf1.clear();
    inApf2.clear();
    inApf3.clear();
    inApf4.clear();

    tank.clear();
}

#pragma GCC push_options
#pragma GCC optimize ("Ofast")

void Dattorro::setTimeScale(float timeScale) {
    timeScale = timeScale < 0.0001f ? 0.0001f : timeScale;
    tank.setTimeScale(timeScale);
}

#pragma GCC pop_options
#pragma GCC push_options
#pragma GCC optimize ("Ofast")

void Dattorro::setPreDelay(float t) {
    preDelay.setDelayTime(t * sampleRate);
}

// void Dattorro::setPreDelay(float t) {
//     preDelay.setDelayTime(t);
// }

#pragma GCC pop_options

void Dattorro::setSampleRate(float newSampleRate) {
    sampleRate = newSampleRate;
    tank.setSampleRate(sampleRate);
    dattorroScaleFactor = sampleRate / dattorroSampleRate;
    setPreDelay(preDelayTime);
    inApf1.delay.setDelayTime(dattorroScale(kInApf1Time));
    inApf2.delay.setDelayTime(dattorroScale(kInApf2Time));
    inApf3.delay.setDelayTime(dattorroScale(kInApf3Time));
    inApf4.delay.setDelayTime(dattorroScale(kInApf4Time));

    leftInputDCBlock.setSampleRate(sampleRate);
    rightInputDCBlock.setSampleRate(sampleRate);
    inputLpf.setSampleRate(sampleRate);
    inputHpf.setSampleRate(sampleRate);

    //clear();
}

void Dattorro::freeze(bool freezeFlag) {
    tank.freeze(freezeFlag);
}

#pragma GCC push_options
#pragma GCC optimize ("Ofast")

void Dattorro::setInputFilterLowCutoffPitch(float inputLowCut) {
    // inputLowCut = 440.0 * std::pow(2.0, pitch - 5.0);
    //inputLowCut = pitch;
    inputHpf.setCutoffFreq(inputLowCut);
}

#pragma GCC pop_options
#pragma GCC push_options
#pragma GCC optimize ("Ofast")

void Dattorro::setInputFilterHighCutoffPitch(float inputHighCut) {
    // inputHighCut = 440.0 * std::pow(2.0, pitch - 5.0);
    //inputHighCut = pitch;
    inputLpf.setCutoffFreq(inputHighCut);
}

#pragma GCC pop_options

void Dattorro::enableInputDiffusion(bool enable) {
    diffuseInput = enable ? 1.f : 0.f;
}

void Dattorro::setDecay(float newDecay) {
    decay = newDecay;
    tank.setDecay(decay);
}

void Dattorro::setTankDiffusion(const float diffusion) {
    tank.setDiffusion(diffusion);
}

#pragma GCC push_options
#pragma GCC optimize ("Ofast")

void Dattorro::setTankFilterHighCutFrequency(const float pitch) {
    // auto frequency = 440.0 * std::pow(2.0, pitch - 5.0);
    // tank.setHighCutFrequency(440.0 * std::pow(2.0, pitch - 5.0));
    tank.setHighCutFrequency(pitch);
}

#pragma GCC pop_options
#pragma GCC push_options
#pragma GCC optimize ("Ofast")

void Dattorro::setTankFilterLowCutFrequency(const float pitch) {
    // auto frequency = 440.0 * std::pow(2.0, pitch - 5.0);
    // tank.setLowCutFrequency(440.0 * std::pow(2.0, pitch - 5.0));
    tank.setLowCutFrequency(pitch);
}

#pragma GCC pop_options

void Dattorro::setTankModSpeed(const float modSpeed) {
    tank.setModSpeed(modSpeed);
}

void Dattorro::setTankModDepth(const float modDepth) {
    tank.setModDepth(modDepth);
}

void Dattorro::setTankModShape(const float modShape) {
    tank.setModShape(modShape);
}

float Dattorro::getLeftOutput() const {
    return leftOut;
}

float Dattorro::getRightOutput() const {
    return rightOut;
}

float Dattorro::dattorroScale(float delayTime) {
    return delayTime * dattorroScaleFactor;
}

