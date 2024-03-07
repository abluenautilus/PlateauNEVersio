#define DSJ_PLATEAU_HPP

#include "Dattorro.hpp"

using namespace daisy;
using namespace daisysp;

DaisyVersio hw;

const float minus18dBGain = 0.12589254f;
const float minus20dBGain = 0.1f;

float wet = 1.0f;
float dry = 0.0f;

float diffusion = 1.f;
float tempDiffusion = 1.f;

float inputDampLow = 0.f;
float inputDampHigh = 0.f;

float reverbDampLow = 0.f;
float reverbDampHigh = 0.f;

float previousInputDampLow = 0.f;
float previousInputDampHigh = 0.f;

float previousReverbDampLow = 0.f;
float previousReverbDampHigh = 0.f;

unsigned int gainMode = 0;
unsigned int gainModeLedTimer = 0;
const unsigned int gainModeLedOnTime = 32000;

float leftInput = 0.f;
float rightInput = 0.f;
float leftOutput = 0.f;
float rightOutput = 0.f;

float volumeChange = 0.f;

unsigned int holdCount = 0;

float inputVolumeModifier = 1.f;
float tempInputVolumeModifier = inputVolumeModifier;
// float diffusion = 0.f;
// float tempDiffusion = diffusion;

bool buttonState = false;
bool previousButtonState = false;
unsigned int buttonHoldTime = 0;

unsigned int toneKnobLedTimer = 0;
const unsigned int toneKnobLedOnTime = 32000;
bool toneKnobIsMoving = false;
float toneKnobValue = 0.f;
float previousToneKnobValue = 0.f;

bool lockModDepthTo3_125_ = false;

bool leds = true;

auto *LED0PtrRed = &hw.leds[0].r_;
auto *LED1PtrRed = &hw.leds[1].r_;
auto *LED2PtrRed = &hw.leds[2].r_;
auto *LED3PtrRed = &hw.leds[3].r_;

Dattorro reverb(32000, 16, 4.0f);

bool diffusionEnabled = true;

float preDelay = 0.f;
float previousPreDelay = 0.f;

// Fast hyperbolic tangent function.
inline float softLimiter(const float &x) {
    return x * (27.f + x * x) / (27.f + 9.f * x * x);
}

inline float hardLimit77_8_(const float &x) {
    return (x > 0.778f) ? 0.778f : ((x < -0.778f) ? -0.778f : x);
}

inline float hardLimit100_(const float &x) {
    return (x > 1.f) ? 1.f : ((x < -1.f) ? -1.f : x);
}

inline float hardLimit85_(const float &x) {
    return (x > 0.85f) ? 0.85f : ((x < -0.85f) ? -0.85f : x);
}

inline float amp120_(const float &x) {
    return x * 1.2f;
}

struct KnobOnePoleFilter {
	float tmp = 0.f;

	KnobOnePoleFilter() {
        inline float processLowpass(const float &x);
    }

    float processLowpass(const float &x) {
        tmp = 0.0005f * x + 0.9995f * tmp;
        return tmp;
    }
};


inline void checkIfToneKnobIsMoving(float currentValue) {
    if (((currentValue - previousToneKnobValue) > 0.001f) or ((currentValue - previousToneKnobValue) < -0.001f)) {
        previousToneKnobValue = currentValue;
        toneKnobIsMoving = true;
    } else {
        toneKnobIsMoving = false;
    }

}

KnobOnePoleFilter mixKnobLPF;
KnobOnePoleFilter modDepthKnobLPF;
KnobOnePoleFilter preDelayKnobLPF;
KnobOnePoleFilter timeScaleKnobLPF;
KnobOnePoleFilter toneKnobLPF;

inline void setAndUpdateGainLeds(const float &w, const float &x, const float &y, const float &z) {
    LED0PtrRed->Set(w);
    LED1PtrRed->Set(x);
    LED2PtrRed->Set(y);
    LED3PtrRed->Set(z);
    LED0PtrRed->Update();
    LED1PtrRed->Update();
    LED2PtrRed->Update();
    LED3PtrRed->Update();
}

auto *SWITCH0Ptr = &hw.sw[0];
auto *SWITCH1Ptr = &hw.sw[1];
unsigned char switchState0 = 0;
unsigned char switchState1 = 0;

auto *KNOB0Ptr = &hw.knobs[0];
auto *KNOB1Ptr = &hw.knobs[1];
auto *KNOB2Ptr = &hw.knobs[2];
auto *KNOB3Ptr = &hw.knobs[3];
auto *KNOB4Ptr = &hw.knobs[4];
auto *KNOB5Ptr = &hw.knobs[5];
auto *KNOB6Ptr = &hw.knobs[6];

float outputAmplification = 1.0f;
float tempOutputAmplification = outputAmplification;

float inputAmplification = 0.0f;
float tempInputAmplification = inputAmplification;

float modDepthValue = 0.f;

inline void getParameters() {
    previousButtonState = buttonState;
    hw.tap.Debounce();
    buttonState = hw.tap.Pressed();

    switchState0 = SWITCH0Ptr->Read();
    switchState1 = SWITCH1Ptr->Read();
    
    hw.ProcessAnalogControls();


    toneKnobValue = toneKnobLPF.processLowpass(KNOB2Ptr->Value());
    checkIfToneKnobIsMoving(toneKnobValue);

    wet = mixKnobLPF.processLowpass(KNOB0Ptr->Value());
    dry = 1.f - wet;

    reverb.setTankModSpeed(1.f + (KNOB1Ptr->Value() * 100.f));

    modDepthValue = KNOB3Ptr->Value();

    // Instead of stopping at minimum 0.5f, user has the option to lock to 0.5f
    // or turn it all the way to 0. This is useful in conjunction with zero diffusion.
    if(lockModDepthTo3_125_) {
        reverb.setTankModDepth(0.5f);
    } else {
        if(modDepthValue < 0.01f) {
            reverb.setTankModDepth(0.f);
        } else {
            reverb.setTankModDepth(modDepthKnobLPF.processLowpass(modDepthValue) * 16.0f);
        }
    }

    reverb.setDecay(0.1f + (KNOB4Ptr->Value() * 0.8999f));

    reverb.setTimeScale(timeScaleKnobLPF.processLowpass(KNOB5Ptr->Value()) * 4.f);
    
    preDelay = preDelayKnobLPF.processLowpass(KNOB6Ptr->Value());
    // if(((preDelay - previousPreDelay) > 0.001f) or ((preDelay - previousPreDelay) < -0.001f)) {
    //     previousPreDelay = preDelay;
        if(preDelay < 0.01f) {
            reverb.setPreDelay(0.f);
        } else {
            reverb.setPreDelay(preDelay * 4.f);
        }
        
    //}

    // reverb.setPreDelay(preDelayKnobLPF.processLowpass(KNOB6Ptr->Value()) * 4.f);

    if(switchState0 == 2) {
        if(switchState1 == 1) {
            inputDampHigh = toneKnobValue;
            if(((inputDampHigh - previousInputDampHigh) < 0.01f) and ((inputDampHigh - previousInputDampHigh) > -0.01f)) {
                previousInputDampHigh = inputDampHigh;
                reverb.setInputFilterHighCutoffPitch(-1.f - (-1.f * inputDampHigh));
                if(toneKnobIsMoving) {
                    toneKnobLedTimer = 0;
                }
                if(toneKnobLedTimer < toneKnobLedOnTime) {
                    ++toneKnobLedTimer;
                    setAndUpdateGainLeds(0.f,inputDampHigh,0.f,0.f);
                } else if (toneKnobLedTimer == toneKnobLedOnTime) {
                    toneKnobLedTimer = toneKnobLedOnTime + 1;
                    setAndUpdateGainLeds(0.f,0.f,0.f,0.f);
                }
                // if(leds) {
                //     LED1PtrRed->Set(inputDampHigh);
                //     LED1PtrRed->Update();
                // }
            }
        } else if (switchState1 == 2) {
            reverbDampHigh = toneKnobValue;
            if(((reverbDampHigh - previousReverbDampHigh) < 0.01f) and ((reverbDampHigh - previousReverbDampHigh) > -0.01f)) {
                previousReverbDampHigh = reverbDampHigh;
                reverb.setTankFilterHighCutFrequency(-1.f - (-1.f * reverbDampHigh));
                if(toneKnobIsMoving) {
                    toneKnobLedTimer = 0;
                }
                if(toneKnobLedTimer < toneKnobLedOnTime) {
                    ++toneKnobLedTimer;
                    setAndUpdateGainLeds(0.f,0.f,0.f,reverbDampHigh);
                } else if (toneKnobLedTimer == toneKnobLedOnTime) {
                    toneKnobLedTimer = toneKnobLedOnTime + 1;
                    setAndUpdateGainLeds(0.f,0.f,0.f,0.f);
                }
                // if(leds) {
                //     LED3PtrRed->Set(reverbDampHigh);
                //     LED3PtrRed->Update();
                // }
            }
        }
    } else if(switchState0 == 1) {
        if(switchState1 == 1) {
            inputDampLow = toneKnobValue;
            if(((inputDampLow - previousInputDampLow) < 0.01f) and ((inputDampLow - previousInputDampLow) > -0.01f)) {
                previousInputDampLow = inputDampLow;
                reverb.setInputFilterLowCutoffPitch(-1.f * inputDampLow);
                if(toneKnobIsMoving) {
                    toneKnobLedTimer = 0;
                }   
                if(toneKnobLedTimer < toneKnobLedOnTime) {
                    ++toneKnobLedTimer;
                    setAndUpdateGainLeds(inputDampLow,0.f,0.f,0.f);
                } else if (toneKnobLedTimer == toneKnobLedOnTime) {
                    toneKnobLedTimer = toneKnobLedOnTime + 1;
                    setAndUpdateGainLeds(0.f,0.f,0.f,0.f);
                }
                // if(leds) {
                //     LED0PtrRed->Set(inputDampLow);
                //     LED0PtrRed->Update();
                // }
            }
        } else if (switchState1 == 2) {
            reverbDampLow = toneKnobValue;
            if(((reverbDampLow - previousReverbDampLow) < 0.01f) and ((reverbDampLow - previousReverbDampLow) > -0.01f)) {
                previousReverbDampLow = reverbDampLow;
                reverb.setTankFilterLowCutFrequency(-1.f * reverbDampLow);
                if(toneKnobIsMoving) {
                    toneKnobLedTimer = 0;
                }
                if(toneKnobLedTimer < toneKnobLedOnTime) {
                    ++toneKnobLedTimer;
                    setAndUpdateGainLeds(0.f,0.f,reverbDampLow,0.f);
                } else if (toneKnobLedTimer == toneKnobLedOnTime) {
                    toneKnobLedTimer = toneKnobLedOnTime + 1;
                    setAndUpdateGainLeds(0.f,0.f,0.f,0.f);
                }
                // if(leds) {
                //     LED2PtrRed->Set(reverbDampLow);
                //     LED2PtrRed->Update();
                // }
            }
        }
    }

    if(buttonState) {
        gainModeLedTimer = 0;
        ++buttonHoldTime;
        if(buttonHoldTime == 128000) {
            toneKnobLedTimer = 0;
            lockModDepthTo3_125_ = !lockModDepthTo3_125_;
        }
    } else {
        if(buttonHoldTime < 8000) {
            if(previousButtonState) {
                ++gainMode;
            }
        }
        buttonHoldTime = 0;
        if(switchState1 == 0) {
            switch(switchState0) {
                case 0:
                    tempDiffusion = toneKnobValue;
                    if(((tempDiffusion - diffusion) < 0.01f) and ((tempDiffusion - diffusion) > -0.01f)) {
                        diffusion = tempDiffusion;
                        if(diffusion < 0.01f) {
                            if(diffusionEnabled) {
                                diffusionEnabled = false;
                                reverb.enableInputDiffusion(diffusionEnabled);
                            }
                        } else {
                            if(!diffusionEnabled) {
                                diffusionEnabled = true;
                                reverb.enableInputDiffusion(diffusionEnabled);
                            }
                            reverb.setTankDiffusion(diffusion * 0.7f);
                        }
                        if(toneKnobIsMoving) {
                            toneKnobLedTimer = 0;
                        }
                        if(toneKnobLedTimer < toneKnobLedOnTime) {
                            ++toneKnobLedTimer;
                            setAndUpdateGainLeds(diffusion,diffusion,diffusion,diffusion);
                        } else if (toneKnobLedTimer == toneKnobLedOnTime) {
                            toneKnobLedTimer = toneKnobLedOnTime + 1;
                            setAndUpdateGainLeds(0.f,0.f,0.f,0.f);
                        }
                    }
                    break;
                case 1:
                    tempInputAmplification = toneKnobValue;
                    if(((tempInputAmplification - inputAmplification) < 0.01f) and ((tempInputAmplification - inputAmplification) > -0.01f)) {
                        inputAmplification = tempInputAmplification;
                        if(toneKnobIsMoving) {
                            toneKnobLedTimer = 0;
                        }
                        if(toneKnobLedTimer < toneKnobLedOnTime) {
                            ++toneKnobLedTimer;
                            setAndUpdateGainLeds(inputAmplification,inputAmplification,inputAmplification,inputAmplification);
                        } else if (toneKnobLedTimer == toneKnobLedOnTime) {
                            toneKnobLedTimer = toneKnobLedOnTime + 1;
                            setAndUpdateGainLeds(0.f,0.f,0.f,0.f);
                        }
                    }
                    break;
                case 2:
                    tempOutputAmplification = toneKnobValue;
                    if(((tempOutputAmplification - outputAmplification) < 0.01f) and ((tempOutputAmplification - outputAmplification) > -0.01f)) {
                        outputAmplification = tempOutputAmplification;
                        if(toneKnobIsMoving) {
                            toneKnobLedTimer = 0;
                        }
                        if(toneKnobLedTimer < toneKnobLedOnTime) {
                            ++toneKnobLedTimer;
                            setAndUpdateGainLeds(outputAmplification,outputAmplification,outputAmplification,outputAmplification);
                        } else if (toneKnobLedTimer == toneKnobLedOnTime) {
                            toneKnobLedTimer = toneKnobLedOnTime + 1;
                            setAndUpdateGainLeds(0.f,0.f,0.f,0.f);
                        }
                    }
                    break;
            }
        }
    }
}

// Is mutating the output this way a mortal sin?
inline void gainControl(float &leftOutput, float &rightOutput) {
    switch (gainMode) {
    case 0:
        leftOutput = softLimiter(leftOutput);
        rightOutput = softLimiter(rightOutput);
        if(gainModeLedTimer < gainModeLedOnTime) {
            ++gainModeLedTimer;
            setAndUpdateGainLeds(0.f,0.f,0.f,1.f);
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
            setAndUpdateGainLeds(0.f,0.f,1.f,0.f);
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
            setAndUpdateGainLeds(0.f,0.f,1.f,1.f);
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
            setAndUpdateGainLeds(0.f,1.f,0.f,0.f);
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
            setAndUpdateGainLeds(0.f,1.f,0.f,1.f);
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
            setAndUpdateGainLeds(0.f,1.f,1.f,0.f);
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
            setAndUpdateGainLeds(0.f,1.f,1.f,1.f);
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
            setAndUpdateGainLeds(1.f,0.f,0.f,0.f);
        } else if (gainModeLedTimer == gainModeLedOnTime) {
            ++gainModeLedTimer;
            setAndUpdateGainLeds(0.f,0.f,0.f,0.f);
        }
        break;
    case 8:
        leftOutput = amp120_(leftOutput);
        rightOutput = amp120_(rightOutput);
        leftOutput = hardLimit85_(leftOutput);
        rightOutput = hardLimit85_(rightOutput);
        leftOutput = softLimiter(leftOutput);
        rightOutput = softLimiter(rightOutput);
        if(gainModeLedTimer < gainModeLedOnTime) {
            ++gainModeLedTimer;
            setAndUpdateGainLeds(1.f,0.f,0.f,1.f);
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
            setAndUpdateGainLeds(1.f,0.f,1.f,0.f);
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
            setAndUpdateGainLeds(1.f,0.f,1.f,1.f);
        } else if (gainModeLedTimer == gainModeLedOnTime) {
            ++gainModeLedTimer;
            setAndUpdateGainLeds(0.f,0.f,0.f,0.f);
        }
        break;
    case 11:
        gainMode = 0;
        break;
    }
}

void AudioCallback(AudioHandle::InputBuffer in,
              AudioHandle::OutputBuffer out,
              size_t size)
{
    for(size_t i = 0; i < size; i += 1) {

        getParameters();

        if(holdCount < 192000)
            ++holdCount;
        else
            hold = 1.f;

        leftInput = in[0][i] * 10.f;
        rightInput = in[1][i] * 10.f;

        reverb.process(leftInput * minus18dBGain * minus20dBGain * (1.0f + inputAmplification * 7.f),
                    rightInput * minus18dBGain * minus20dBGain * (1.0f + inputAmplification * 7.f));

        // volumeChange = (1-0.5*diffusion)*(1+(3-3*reverb.tank.decay)*diffusion);

        leftOutput = ((leftInput * dry * 0.1f) + 
                    (reverb.getLeftOutput() * wet * 2.f /* * volumeChange*/)) * (0.25f + outputAmplification * 0.75f);
        rightOutput = ((rightInput * dry * 0.1f) + 
                    (reverb.getRightOutput() * wet * 2.f /* * volumeChange*/)) * (0.25f + outputAmplification * 0.75f);

        gainControl(leftOutput, rightOutput);

        out[0][i] = leftOutput;
        out[1][i] = rightOutput;
    }
};

int main(void)
{
	hw.Init(true);

    for(int i = 0; i < 50; i++) {
        for(int j = 0; j < 144000; j++) {
            sdramData[i][j] = 0.f;
        }
    }

    hw.leds[0].Set(0, 0, 0);
    hw.leds[0].Set(0, 0, 0);
    hw.leds[0].Set(0, 0, 0);
    hw.leds[0].Set(0, 0, 0);
    hw.UpdateLeds();

    reverb.setSampleRate(32000);

    reverb.setTimeScale(4.0f);
    reverb.setPreDelay(0.f);
    
    reverb.setInputFilterLowCutoffPitch(-1.f * inputDampLow);
    reverb.setInputFilterHighCutoffPitch(-1.f - (-1.f * inputDampHigh));
    reverb.enableInputDiffusion(false);
    reverb.setDecay(0.5f);
    reverb.setTankDiffusion(0.f);
    reverb.setTankFilterLowCutFrequency(-1.f * reverbDampLow);
    reverb.setTankFilterHighCutFrequency(-1.f - (-1.f * reverbDampHigh));
    reverb.setTankModSpeed(0.5f);
    reverb.setTankModDepth(8.f);
    reverb.setTankModShape(0.5f);

    hw.SetAudioBlockSize(1);
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_32KHZ);
    hw.seed.audio_handle.SetPostGain(1.0f);
    hw.seed.audio_handle.SetOutputCompensation(1.0f);

    hw.knobs[0].SetCoeff(0.001f);
    hw.knobs[1].SetCoeff(0.01f);
    hw.knobs[2].SetCoeff(0.01f);
    hw.knobs[3].SetCoeff(0.001f);
    hw.knobs[4].SetCoeff(0.01f);
    hw.knobs[5].SetCoeff(0.01f);
    hw.knobs[6].SetCoeff(0.001f);
    
    hw.StartAudio(AudioCallback);

    hw.StartAdc();

	while(1) {}
}
