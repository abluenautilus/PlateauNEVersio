#define DSJ_PLATEAU_HPP

#include "Dattorro.hpp"
#include <vector>

using namespace daisy;
using namespace daisysp;

DaisyVersio hw;

const float sizeMin = 0.0025f;
const float sizeMax = 4.0f;

const float minus18dBGain = 0.12589254f;
const float minus20dBGain = 0.1f;

float wet = 0.5f;
float dry = 0.5f;

float preDelay = 0.f;
float previousPreDelay = 0.f;

float inputDampLow = 0.f;
float inputDampHigh = 10.f;

float reverbDampLow = 0.f;
float reverbDampHigh = 10.f;

float previousInputDampLow = 0.f;
float previousInputDampHigh = 1.f;

float previousReverbDampLow = 0.f;
float previousReverbDampHigh = 1.f;

unsigned int gainMode = 0;
unsigned int gainModeLedTimer = 0;
unsigned int gainModeLedOnTime = 32000;

float leftInput = 0.f;
float rightInput = 0.f;
float leftOutput = 0.f;
float rightOutput = 0.f;

float decay = 0.5f;

float modSpeed = 0.f;

float modDepth = 0.f;

float timeScale = 2.f;
float previousTimeScale = timeScale;

int holdCount = 0;

float inputVolumeModifier = 1.f;
float tempInputVolumeModifier = inputVolumeModifier;

bool buttonState = false;
bool previousButtonState = false;
unsigned int buttonHoldTime = 0;

bool leds = true;

auto *LED0PtrRed = &hw.leds[0].r_;
auto *LED1PtrRed = &hw.leds[1].r_;
auto *LED2PtrRed = &hw.leds[2].r_;
auto *LED3PtrRed = &hw.leds[3].r_;

Dattorro reverb(32000, 16, sizeMax);

// Fast hypobilc tangent function.
const float smoothness = 27.f;
const float threshold = 9.f;
inline float softLimiter(float x) {
    return x * (smoothness + x * x) / (smoothness + threshold * x * x);
}

inline float hardLimit77_8_(float x) {
    return (x > 0.778f) ? 0.778f : ((x < -0.778f) ? -0.778f : x);
}

inline float hardLimit100_(float x) {
    return (x > 1.f) ? 1.f : ((x < -1.f) ? -1.f : x);
}

inline float hardLimit85_(float x) {
    return (x > 0.85f) ? 0.85f : ((x < -0.85f) ? -0.85f : x);
}

inline float amp120_(float x) {
    return x * 1.2f;
}

inline void setAndUpdateGainLeds(float w, float x, float y, float z) {
    LED0PtrRed->Set(w);
    LED1PtrRed->Set(x);
    LED2PtrRed->Set(y);
    LED3PtrRed->Set(z);
    LED0PtrRed->Update();
    LED1PtrRed->Update();
    LED2PtrRed->Update();
    LED3PtrRed->Update();
}

void AudioCallback(AudioHandle::InputBuffer in,
              AudioHandle::OutputBuffer out,
              size_t size)
{
    for(size_t i = 0; i < size; i += 1)
    {
        
        hw.ProcessAnalogControls();

        wet = hw.knobs[0].Value();
        dry = 1.f - wet;

        hw.knobs[5].SetCoeff(0.0001f);
        timeScale = hw.knobs[5].Value();
        if(((timeScale - previousTimeScale) > 0.00001f) or ((timeScale - previousTimeScale) < -0.00001f)) {
            previousTimeScale = timeScale;
            timeScale *= timeScale * 4.f;
            reverb.setTimeScale(timeScale);
        }

        preDelay = hw.knobs[6].Value();
        if(((preDelay - previousPreDelay) > 0.01f) or ((preDelay - previousPreDelay) < -0.01f)) {
            reverb.setPreDelay(preDelay * 4.f);
            previousPreDelay = preDelay;
        }

        decay = 0.1f + (hw.knobs[4].Value() * 0.8999f);
        decay = 1.f - decay;
        decay = 1.f - decay * decay;
        reverb.setDecay(decay);

        if(hw.sw[0].Read() == 2) {
            if(hw.sw[1].Read() == 1) {
                hw.knobs[2].SetCoeff(0.001f);
                inputDampHigh = hw.knobs[2].Value();
                if(((inputDampHigh - previousInputDampHigh) < 0.1f) and ((inputDampHigh - previousInputDampHigh) > -0.1f)) {
                    previousInputDampHigh = inputDampHigh;
                    reverb.setInputFilterHighCutoffPitch(13.75f + (20000.f - (20000.f * inputDampHigh)));
                    if(leds) {
                        LED1PtrRed->Set(inputDampHigh);
                        LED1PtrRed->Update();
                    }
                }
            } else if (hw.sw[1].Read() == 2) {
                hw.knobs[2].SetCoeff(0.001f);
                reverbDampHigh = hw.knobs[2].Value();
                if(((reverbDampHigh - previousReverbDampHigh) < 0.1f) and ((reverbDampHigh - previousReverbDampHigh) > -0.1f)) {
                    previousReverbDampHigh = reverbDampHigh;
                    reverb.setTankFilterHighCutFrequency(13.75f + (20000.f - (20000.f * reverbDampHigh)));
                    if(leds) {
                        LED3PtrRed->Set(reverbDampHigh);
                        LED3PtrRed->Update();
                    }
                }
            }
        } else if(hw.sw[0].Read() == 1) {
            if(hw.sw[1].Read() == 1) {
                hw.knobs[2].SetCoeff(0.001f);
                inputDampLow = hw.knobs[2].Value();
                if(((inputDampLow - previousInputDampLow) < 0.1f) and ((inputDampLow - previousInputDampLow) > -0.1f)) {
                    previousInputDampLow = inputDampLow;
                    reverb.setInputFilterLowCutoffPitch(13.75f + (20000.f * inputDampLow));
                    if(leds) {
                        LED0PtrRed->Set(inputDampLow);
                        LED0PtrRed->Update();
                    }
                }
            } else if (hw.sw[1].Read() == 2) {
                hw.knobs[2].SetCoeff(0.001f);
                reverbDampLow = hw.knobs[2].Value();
                if(((reverbDampLow - previousReverbDampLow) < 0.1f) and ((reverbDampLow - previousReverbDampLow) > -0.1f)) {
                    previousReverbDampLow = reverbDampLow;
                    reverb.setTankFilterLowCutFrequency(13.75f + (20000.f * reverbDampLow));
                    if(leds) {
                        LED2PtrRed->Set(reverbDampLow);
                        LED2PtrRed->Update();
                    }
                }
            }
        }
        
        modSpeed = 1.f + (hw.knobs[1].Value() * 100.f);
        reverb.setTankModSpeed(modSpeed);

        modDepth = 0.5f + (hw.knobs[3].Value() * 16.f);
        reverb.setTankModDepth(modDepth);

        hw.tap.Debounce();
        previousButtonState = buttonState;
        buttonState = hw.SwitchPressed();
        if(buttonState) {
            hw.knobs[2].SetCoeff(0.001f);
            tempInputVolumeModifier = hw.knobs[2].Value();
            if(((tempInputVolumeModifier - inputVolumeModifier) < 0.01f) and ((tempInputVolumeModifier - inputVolumeModifier) > -0.1f)) {
                inputVolumeModifier = tempInputVolumeModifier;
            }
            gainModeLedTimer = 0;
            ++buttonHoldTime;
        } else {
            if(buttonHoldTime < 8000) {
                if(previousButtonState) {
                    ++gainMode;
                }
            }
            buttonHoldTime = 0;
        }

        leftInput = in[0][i];
        rightInput = in[1][i];

        reverb.process(leftInput * minus18dBGain * inputVolumeModifier,
                    rightInput * minus18dBGain * inputVolumeModifier);

        leftOutput = (leftInput * dry * inputVolumeModifier) + 
                    (reverb.getLeftOutput() * wet * 0.833f);
        rightOutput = (rightInput * dry * inputVolumeModifier) + 
                    (reverb.getRightOutput() * wet * 0.833f);

        switch (gainMode)
        {
        case 0:
            leftOutput = softLimiter(leftOutput);
            rightOutput = softLimiter(rightOutput);
            if(gainModeLedTimer < gainModeLedOnTime) {
                ++gainModeLedTimer;
                setAndUpdateGainLeds(0.f,0.f,0.f,inputVolumeModifier);
            } else if (gainModeLedTimer == gainModeLedOnTime) {
                ++gainModeLedTimer;
                setAndUpdateGainLeds(0.f,0.f,0.f,0.f);
            }
            break;
        case 1:
            leftOutput = softLimiter(leftOutput);
            rightOutput = softLimiter(rightOutput);
            leftOutput = hardLimit77_8_(leftOutput);
            rightOutput = hardLimit77_8_(rightOutput);
            if(gainModeLedTimer < gainModeLedOnTime) {
                ++gainModeLedTimer;
                setAndUpdateGainLeds(0.f,0.f,inputVolumeModifier,0.f);
            } else if (gainModeLedTimer == gainModeLedOnTime) {
                ++gainModeLedTimer;
                setAndUpdateGainLeds(0.f,0.f,0.f,0.f);
            }
            break;
        case 2:
            leftOutput = hardLimit100_(leftOutput);
            rightOutput = hardLimit100_(rightOutput);
            if(gainModeLedTimer < gainModeLedOnTime) {
                ++gainModeLedTimer;
                setAndUpdateGainLeds(0.f,0.f,inputVolumeModifier,inputVolumeModifier);
            } else if (gainModeLedTimer == gainModeLedOnTime) {
                ++gainModeLedTimer;
                setAndUpdateGainLeds(0.f,0.f,0.f,0.f);
            }
            break;
        case 3:
            leftOutput = hardLimit85_(leftOutput);
            rightOutput = hardLimit85_(rightOutput);
            if(gainModeLedTimer < gainModeLedOnTime) {
                ++gainModeLedTimer;
                setAndUpdateGainLeds(0.f,inputVolumeModifier,0.f,0.f);
            } else if (gainModeLedTimer == gainModeLedOnTime) {
                ++gainModeLedTimer;
                setAndUpdateGainLeds(0.f,0.f,0.f,0.f);
            }
            break;
        case 4:
            leftOutput = hardLimit100_(leftOutput);
            rightOutput = hardLimit100_(rightOutput);
            leftOutput = amp120_(leftOutput);
            rightOutput = amp120_(rightOutput);
            leftOutput = softLimiter(leftOutput);
            rightOutput = softLimiter(rightOutput);
            if(gainModeLedTimer < gainModeLedOnTime) {
                ++gainModeLedTimer;
                setAndUpdateGainLeds(0.f,inputVolumeModifier,0.f,inputVolumeModifier);
            } else if (gainModeLedTimer == gainModeLedOnTime) {
                ++gainModeLedTimer;
                setAndUpdateGainLeds(0.f,0.f,0.f,0.f);
            }
            break;
        case 5:
            leftOutput = hardLimit85_(leftOutput);
            rightOutput = hardLimit85_(rightOutput);
            leftOutput = amp120_(leftOutput);
            rightOutput = amp120_(rightOutput);
            leftOutput = softLimiter(leftOutput);
            rightOutput = softLimiter(rightOutput);
            if(gainModeLedTimer < gainModeLedOnTime) {
                ++gainModeLedTimer;
                setAndUpdateGainLeds(0.f,inputVolumeModifier,inputVolumeModifier,0.f);
            } else if (gainModeLedTimer == gainModeLedOnTime) {
                ++gainModeLedTimer;
                setAndUpdateGainLeds(0.f,0.f,0.f,0.f);
            }
            break;
        case 6:
            leftOutput = amp120_(leftOutput);
            rightOutput = amp120_(rightOutput);
            leftOutput = softLimiter(leftOutput);
            rightOutput = softLimiter(rightOutput);
            leftOutput = hardLimit77_8_(leftOutput);
            rightOutput = hardLimit77_8_(rightOutput);
            if(gainModeLedTimer < gainModeLedOnTime) {
                ++gainModeLedTimer;
                setAndUpdateGainLeds(0.f,inputVolumeModifier,inputVolumeModifier,inputVolumeModifier);
            } else if (gainModeLedTimer == gainModeLedOnTime) {
                ++gainModeLedTimer;
                setAndUpdateGainLeds(0.f,0.f,0.f,0.f);
            }
            break;
        case 7:
            leftOutput = amp120_(leftOutput);
            rightOutput = amp120_(rightOutput);
            leftOutput = hardLimit100_(leftOutput);
            rightOutput = hardLimit100_(rightOutput);
            leftOutput = softLimiter(leftOutput);
            rightOutput = softLimiter(rightOutput);
            if(gainModeLedTimer < gainModeLedOnTime) {
                ++gainModeLedTimer;
                setAndUpdateGainLeds(inputVolumeModifier,0.f,0.f,0.f);
            } else if (gainModeLedTimer == gainModeLedOnTime) {
                ++gainModeLedTimer;
                setAndUpdateGainLeds(0.f,0.f,0.f,0.f);
            }
            break;
            leftOutput = amp120_(leftOutput);
            rightOutput = amp120_(rightOutput);
            leftOutput = hardLimit85_(leftOutput);
            rightOutput = hardLimit85_(rightOutput);
            leftOutput = softLimiter(leftOutput);
            rightOutput = softLimiter(rightOutput);
            if(gainModeLedTimer < gainModeLedOnTime) {
                ++gainModeLedTimer;
                setAndUpdateGainLeds(inputVolumeModifier,0.f,0.f,inputVolumeModifier);
            } else if (gainModeLedTimer == gainModeLedOnTime) {
                ++gainModeLedTimer;
                setAndUpdateGainLeds(0.f,0.f,0.f,0.f);
            }
            break;
        case 9:
            leftOutput = amp120_(leftOutput);
            rightOutput = amp120_(rightOutput);
            leftOutput = hardLimit100_(leftOutput);
            rightOutput = hardLimit100_(rightOutput);
            if(gainModeLedTimer < gainModeLedOnTime) {
                ++gainModeLedTimer;
                setAndUpdateGainLeds(inputVolumeModifier,0.f,inputVolumeModifier,0.f);
            } else if (gainModeLedTimer == gainModeLedOnTime) {
                ++gainModeLedTimer;
                setAndUpdateGainLeds(0.f,0.f,0.f,0.f);
            }
            break;
        case 10:
            leftOutput = amp120_(leftOutput);
            rightOutput = amp120_(rightOutput);
            leftOutput = hardLimit85_(leftOutput);
            rightOutput = hardLimit85_(rightOutput);
            if(gainModeLedTimer < gainModeLedOnTime) {
                ++gainModeLedTimer;
                setAndUpdateGainLeds(inputVolumeModifier,0.f,inputVolumeModifier,inputVolumeModifier);
            } else if (gainModeLedTimer == gainModeLedOnTime) {
                ++gainModeLedTimer;
                setAndUpdateGainLeds(0.f,0.f,0.f,0.f);
            }
            break;
        case 11:
            gainMode = 0;
            break;
        }

        out[0][i] = leftOutput;
        out[1][i] = rightOutput;

        if(holdCount < 192000)
            ++holdCount;
        else
            hold = true;
    }
}

int main(void)
{
	hw.Init(true);

    hw.leds[0].Set(0, 0, 0);
    hw.leds[0].Set(0, 0, 0);
    hw.leds[0].Set(0, 0, 0);
    hw.leds[0].Set(0, 0, 0);
    hw.UpdateLeds();

    reverb.setSampleRate(32000);

    reverb.setTimeScale(4.0f);
    reverb.setPreDelay(0.345f);
    reverb.setInputFilterLowCutoffPitch(13.75f + (20000.f * 0.f));
    reverb.setInputFilterHighCutoffPitch(13.75f + (20000.f - (20000.f * 0.f)));
    reverb.enableInputDiffusion(true);
    reverb.setDecay(0.847f);
    reverb.setTankDiffusion(0.7f);
    reverb.setTankFilterLowCutFrequency(13.75f + (20000.f * 0.f));
    reverb.setTankFilterHighCutFrequency(13.75f + (20000.f - (20000.f * 0.f)));
    reverb.setTankModSpeed(1.f);
    reverb.setTankModDepth(0.5f);
    reverb.setTankModShape(0.5f);

    hw.SetAudioBlockSize(1); // Number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_32KHZ);
    
    hw.StartAudio(AudioCallback);

    hw.StartAdc();

	// while(1) {
    //     //hw.UpdateLeds();
    // }
}
