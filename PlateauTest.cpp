#define DSJ_PLATEAU_HPP

#include "Dattorro.hpp"
#include <vector>

using namespace daisy;
using namespace daisysp;

DaisyVersio hw;

const float sizeMin = 0.0025f;
const float sizeMax = 4.0f;

const float diffMin = 0.f;
const float diffMax = 1.f;

const float decayMin = 0.1f;
const float decayMax = 0.9999f;

const float reverbDampMin = 0.f;
const float reverbDampMax = 10.f;

const float modSpeedMin = 0.f;
const float modSpeedMax = 20.f;

const float modDepthMin = 0.f;
const float modDepthMax = 16.f;

constexpr float minus20dBGain = 0.1f;
constexpr float zerodBGain = 1.f;

float wet = 0.5f;
float dry = 0.5f;
float wetValue = 0.f;
float previousWetValue = wetValue;

float preDelay = 0.f;
float previousPreDelay = 0.f;

float inputDampLow = 0.f;
float inputDampHigh = 10.f;

float reverbDampLow = 0.f;
float reverbDampHigh = 10.f;

float previousInputDampLow = 0.f;
float previousInputDampHigh = 10.f;

float previousReverbDampLow = 0.f;
float previousReverbDampHigh = 10.f;

bool freezeButtonState = false;
bool freezeToggle = false;
bool freezeToggleButtonState = false;
bool freeze = false;
bool frozen = false;
bool tunedButtonState = false;

int inputSensitivityState = 0;

bool clear = false;
bool cleared = true;
bool fadeOut = false;
bool fadeIn = false;

float leftInput = 0.f;
float rightInput = 0.f;
float leftOutput = 0.f;
float rightOutput = 0.f;

float decay = 0.5f;
float previousDecay = decay;

float modSpeed = 0.f;
float previousModSpeed = modSpeed;

float modDepth = 0.f;
float previousModDepth = modDepth;

float timeScale = 2.f;
float previousTimeScale = timeScale;

float tempCutoff = 0.f;

int holdCount = 0;
int paramCount = 0;

Dattorro reverb;

void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
              AudioHandle::InterleavingOutputBuffer out,
              size_t                                size)
{
    for(size_t i = 0; i < size; i += 2)
    {
        reverb.process(in[i] * minus20dBGain,
                in[i + 1] * minus20dBGain);

        leftOutput = (in[i] * dry) + 
                (reverb.getLeftOutput() * 2 * wet);
        rightOutput = (in[i + 1] * dry) + 
                (reverb.getRightOutput() * 2 * wet);

        if(leftOutput > 10)
            leftOutput = 10;

        if(leftOutput < -10)
            leftOutput = -10;

        if(rightOutput > 10)
            rightOutput = 10;
        
        if(rightOutput < -10)
            rightOutput = -10;

        out[i] = leftOutput;
        out[i + 1] = rightOutput;

        hw.ProcessAnalogControls();

        if(holdCount < 192000)
            ++holdCount;
        else
            hold = true;

        //If reverb is too grungy, add this section into the switch statement below.
        //It was necessary for the time scale, size, to respond smoothly to modulation,
        //hence its placement outside the following switch statement.
        timeScale = hw.GetKnobValue(DaisyVersio::KNOB_5) * 4.f;
            reverb.setTimeScale(timeScale); 

        //Caused some artifacting when in the main while loop, using this method to offload some stress on the CPU.
        //Knobs are generally unstable so none of the following will track smoothly to avoid static/artifacting.
        //If you want things to track nicer then it is recommented to lower the 0.01f, but this may cause some
        //unwanted artifacting. The predelay knob was the main culprit here, 
        switch (paramCount) {
            case 0:
                wetValue = hw.GetKnobValue(DaisyVersio::KNOB_0);
                if(abs(wetValue - previousWetValue) > 0.1f) {
                    dry = 1.f - wetValue;
                    wet = wetValue;
                    previousWetValue = wetValue;
                }
                ++paramCount;
                break;
            case 1:
                decay = hw.GetKnobValue(DaisyVersio::KNOB_4);
                if(abs(decay - previousDecay) > 0.01f) {
                    reverb.setDecay(decay);
                    previousDecay = decay;
                }
                ++paramCount;
                break;
            case 2:
                modSpeed = hw.GetKnobValue(DaisyVersio::KNOB_1) * 10.f;
                if(abs(modSpeed - previousModSpeed) > 0.1f) {
                    reverb.setTankModSpeed(modSpeed);
                    previousModSpeed = modSpeed;
                }
                ++paramCount;
                break;
            case 3:
                modDepth = hw.GetKnobValue(DaisyVersio::KNOB_3) * 16.f;
                if(abs(modDepth - previousModDepth) > 0.1f) {
                    reverb.setTankModDepth(modDepth);
                    previousModDepth = modDepth;
                }
                ++paramCount;
                break;
            case 4:
                preDelay = hw.GetKnobValue(DaisyVersio::KNOB_6) * 4;
                if(abs(preDelay - previousPreDelay) > 0.01f) {
                    reverb.setPreDelay(preDelay);
                    previousPreDelay = preDelay;
                }
                ++paramCount;
                break;
            case 5:
                paramCount = 0;
                break;
        }
    }
}

int main(void)
{
	hw.Init(true);

    reverb.setSampleRate(32000);

	hw.SetAudioBlockSize(1); // Number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_32KHZ);
    
    hw.StartAudio(AudioCallback);

    reverb.setTimeScale(timeScale);
    reverb.setPreDelay(preDelay);
    
    reverb.setInputFilterLowCutoffPitch(inputDampLow);
    reverb.setInputFilterHighCutoffPitch(inputDampHigh);

    reverb.enableInputDiffusion(true);
    reverb.setDecay(decay);
    reverb.setTankDiffusion(1.f);
    reverb.setTankFilterLowCutFrequency(reverbDampLow);
    reverb.setTankFilterHighCutFrequency(reverbDampHigh);

    reverb.setTankModSpeed(modSpeed);
    reverb.setTankModDepth(modDepth);

    reverb.setTankModShape(0.5);

    hw.StartAdc();

    hw.leds[1].Set(0, 0, inputDampHigh/10.f);
    hw.leds[0].Set(0, 0, inputDampLow/10.f);
    hw.leds[3].Set(0, 0, reverbDampHigh/10.f);
    hw.leds[2].Set(0, 0, reverbDampLow/10.f);

	while(1) {

        // if(hw.sw[0].Read() == 2) {
        //     if(hw.sw[1].Read() == 1) {
        //         inputDampHigh = hw.GetKnobValue(DaisyVersio::KNOB_2) * 10;
        //         if(abs(inputDampHigh - previousInputDampHigh) < 0.1) {
        //             if(inputDampHigh > reverbDampMax)
        //                 inputDampHigh = reverbDampMax;
        //             if(inputDampHigh < reverbDampMin)
        //                 inputDampHigh = reverbDampMin;
        //             previousInputDampHigh = inputDampHigh;
        //             reverb.setInputFilterHighCutoffPitch(inputDampHigh);
        //             hw.leds[1].Set(0, 0, inputDampHigh/10.f);
        //         }
        //     } else if (hw.sw[1].Read() == 2) {
        //         reverbDampHigh = hw.GetKnobValue(DaisyVersio::KNOB_2) * 10;
        //         if(abs(reverbDampHigh - previousReverbDampHigh) < 0.1) {
        //             if(reverbDampHigh > reverbDampMax)
        //                 reverbDampHigh = reverbDampMax;
        //             if(reverbDampHigh < reverbDampMin)
        //                 reverbDampHigh = reverbDampMin;
        //             previousReverbDampHigh = reverbDampHigh;
        //             reverb.setTankFilterHighCutFrequency(reverbDampHigh);
        //             hw.leds[3].Set(0, 0, reverbDampHigh/10.f);
        //         }
        //     }
        // } else if(hw.sw[0].Read() == 1) {
        //     if(hw.sw[1].Read() == 1) {
        //         inputDampLow = hw.GetKnobValue(DaisyVersio::KNOB_2) * 10;
        //         if(abs(inputDampLow - previousInputDampLow) < 0.1) {
        //             if(inputDampLow > reverbDampMax)
        //                 inputDampLow = reverbDampMax;
        //             if(inputDampLow < reverbDampMin)
        //                 inputDampLow = reverbDampMin;
        //             previousInputDampLow = inputDampLow;
        //             reverb.setInputFilterLowCutoffPitch(inputDampLow);
        //             hw.leds[0].Set(0, 0, inputDampLow/10.f);
        //         }
        //     } else if (hw.sw[1].Read() == 2) {
        //         reverbDampLow = hw.GetKnobValue(DaisyVersio::KNOB_2) * 10;
        //         if(abs(reverbDampLow - previousReverbDampLow) < 0.1) {
        //             if(reverbDampLow > reverbDampMax)
        //                 reverbDampLow = reverbDampMax;
        //             if(reverbDampLow < reverbDampMin)
        //                 reverbDampLow = reverbDampMin;
        //             previousReverbDampLow = reverbDampLow;
        //             reverb.setTankFilterLowCutFrequency(reverbDampLow);
        //             hw.leds[2].Set(0, 0, reverbDampLow/10.f);
        //         }
        //     }
        // }
        // //  else if(hw.sw[0].Read() == 1) { 
        // //     inputDampLow = hw.GetKnobValue(DaisyVersio::KNOB_2) * 10;
        // //     if(abs(inputDampLow - previousInputDampLow) < 0.1) {
        // //         if(inputDampLow > reverbDampMax)
        // //             inputDampLow = reverbDampMax;
        // //         if(inputDampLow < reverbDampMin)
        // //             inputDampLow = reverbDampMin;
        // //         previousInputDampLow = inputDampLow;
        // //         reverb.setInputFilterLowCutoffPitch(inputDampLow);
        // //         hw.leds[1].Set(0, 0, inputDampLow/20.f);
        // //     }
        // // }
        // // reverb.setTankFilterHighCutFrequency(inputDampHigh);
    }
}
