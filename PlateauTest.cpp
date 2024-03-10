#define DSJ_PLATEAU_HPP

#include "Dattorro.hpp"
#include "signalsmith/envelopes.h"
#include "signalsmith/delay.h"

using namespace daisy;
using namespace daisysp;

uint32_t *sdramPtr = (uint32_t *)0xC0000000;

struct ExponentialRelease {
	float releaseSlew;
	float output = 1;
	
	ExponentialRelease(float releaseSamples = 1280) {
		// The exact value is `1 - exp(-1/releaseSamples)`
		// but this is a decent approximation
		releaseSlew = 1/(releaseSamples + 1);
	}
	
	float step(float input) {
		// Move towards input
		output += (input - output)*releaseSlew;
		output = std::min(output, input);
		return output;
	}
};

struct LimiterAttackHoldRelease {
    // Desired dB limit is -10dB -> e^(-10dB / 20) = x = 0.6065
	float limit = 0.6065f;
	float attackMs = 150;
	float holdMs = 0;
	float releaseMs = 600;
	
	signalsmith::envelopes::PeakHold<float> peakHold{0};
	signalsmith::envelopes::BoxStackFilter<float> smoother{0};
	// We don't need fractional delays, so this could be nearest-sample
	signalsmith::delay::Delay<float> delay;
	ExponentialRelease release; // see the previous example code
	
	int attackSamples = 0;
	void configure(float sampleRate) {
		attackSamples = attackMs*0.001*sampleRate;
		int holdSamples = holdMs*0.001*sampleRate;
		float releaseSamples = releaseMs*0.001*sampleRate;
		release = ExponentialRelease(releaseSamples);

		peakHold.resize(attackSamples + holdSamples);
		smoother.resize(attackSamples, 3);
		smoother.reset(1);
		
		delay.resize(attackSamples + 1);
	}
	int latencySamples() {
		return attackSamples;
	}
	
	inline float gain(const float &v) {
		float maxGain = 1;
		if (std::abs(v) > limit) {
			maxGain = limit/std::abs(v);
		}
        
		return smoother(release.step(-peakHold(-maxGain)));

	}

	float sample(const float &v) {
		return delay.write(v).read(attackSamples) * gain(v);
	}
};

DaisyVersio hw;

LimiterAttackHoldRelease limiter;

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

uint32_t gainMode = 0;
unsigned int gainModeLedTimer = 32001;
const unsigned int gainModeLedOnTime = 32000;

float leftInput = 0.f;
float rightInput = 0.f;
float leftOutput = 0.f;
float rightOutput = 0.f;

float volumeChange = 0.f;

unsigned int holdCount = 0;

float inputVolumeModifier = 1.f;
float tempInputVolumeModifier = inputVolumeModifier;

bool buttonState = false;
bool previousButtonState = false;
unsigned int buttonHoldTime = 0;

unsigned int toneKnobLedTimer = 32001;
const unsigned int toneKnobLedOnTime = 32000;
bool toneKnobIsMoving = false;
float toneKnobValue = 0.f;
float previousToneKnobValue = 0.f;
float toneKnobZeroLockValue = 0.f;
float previousToneKnobZeroLockValue = 0.f;

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
float modDepthZeroLockValue = 0.f;

float led1 = 0.f;
float led2 = 0.f;
float led3 = 0.f;
float led4 = 0.f;

float knobValue0 = 0.f;
float knobValue1 = 0.f;
float knobValue2 = 0.f;
float knobValue3 = 0.f;
float knobValue4 = 0.f;
float knobValue5 = 0.f;
float knobValue6 = 0.f;

unsigned int saveTimer = 0;
bool saveToggle = false; 
unsigned int saveTime = 32000;

bool clear = false;
bool triggerClear = false;

// // Persistence
// struct Settings {
//     uint32_t gainMode;
//     float inputDampLow;
//     float inputDampHigh;
//     float reverbDampLow;
//     float reverbDampHigh;
//     float diffusion;
//     float inputAmplification;
//     float outputAmplification;
//     bool operator!=(const Settings& a) {
//         return (a.gainMode != gainMode)
//         and (a.inputDampLow != inputDampLow)
//         and (a.inputDampHigh != inputDampHigh)
//         and (a.reverbDampLow != reverbDampLow)
//         and (a.reverbDampHigh != reverbDampHigh)
//         and (a.diffusion != diffusion)
//         and (a.inputAmplification != inputAmplification)
//         and (a.outputAmplification != outputAmplification);
//     }
// }
//  __attribute__((aligned(4)));

// Settings& operator* (const Settings& settings) { return *settings; }
// PersistentStorage<Settings> storage(hw.seed.qspi);

// inline void saveData() {

//     //
//     // Save settings to QSPI
//     //

//     Settings &localSettings = storage.GetSettings();
//     localSettings.gainMode = gainMode;
//     localSettings.inputDampLow = inputDampLow;
//     localSettings.inputDampHigh = inputDampHigh;
//     localSettings.reverbDampLow = reverbDampLow;
//     localSettings.reverbDampHigh = reverbDampHigh;
//     localSettings.diffusion = diffusion;
//     localSettings.inputAmplification = inputAmplification;
//     localSettings.outputAmplification = outputAmplification;

//     storage.Save();
// }

// inline void loadData() {

//     //
//     // Load settings from QSPI
//     //

//     Settings &localSettings = storage.GetSettings();
//     gainMode = localSettings.gainMode;
//     inputDampLow = localSettings.inputDampLow;
//     inputDampHigh = localSettings.inputDampHigh;
//     reverbDampLow = localSettings.reverbDampLow;
//     reverbDampHigh = localSettings.reverbDampHigh;
//     diffusion = localSettings.diffusion;
//     inputAmplification = localSettings.inputAmplification;
//     outputAmplification = localSettings.outputAmplification;

// }


// Fast hyperbolic tangent function.
inline float softLimiter(const float &x) {
   return limiter.sample(x);
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

KnobOnePoleFilter mixKnobLPF;
KnobOnePoleFilter modDepthKnobLPF;
KnobOnePoleFilter preDelayKnobLPF;
KnobOnePoleFilter timeScaleKnobLPF;
KnobOnePoleFilter toneKnobLPF;
KnobOnePoleFilter toneKnobZeroLockLPF;

unsigned int lockModDepthTime = 320000;
unsigned int bufferClearTriggerWindow = 8000;

unsigned int genericLedOnTime = 32000;
unsigned int genericLedTimer = genericLedOnTime + 1;

inline void checkIfToneKnobIsMoving(float currentValue) {
    if (((currentValue - previousToneKnobValue) > 0.001f) or ((currentValue - previousToneKnobValue) < -0.001f)) {
        previousToneKnobValue = currentValue;
        toneKnobIsMoving = true;
    } else {
        toneKnobIsMoving = false;
    }

}

//These pointers are necessary to speed up the code, otherwise severe crackling occurs.
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

inline void prepareLeds(const float &w, const float &x, const float &y, const float &z) {
    led1 = w;
    led2 = x;
    led3 = y;
    led4 = z;
}

inline void prepareGenericLed() {
    if(genericLedTimer < genericLedOnTime) {
        ++genericLedTimer;
        prepareLeds(1.f,1.f,1.f,1.f);
    } else if (genericLedTimer == genericLedOnTime) {
        genericLedTimer = genericLedOnTime + 1;
        prepareLeds(0.f,0.f,0.f,0.f);
    }
}

inline void checkButton() {
    previousButtonState = buttonState;
    hw.tap.Debounce();
    buttonState = hw.tap.Pressed();
}

inline void checkSwitches() {
    switchState0 = SWITCH0Ptr->Read();
    switchState1 = SWITCH1Ptr->Read();
}

inline void processSwitches() {
    if(switchState0 == 2) {
        if(switchState1 == 1) {
            inputDampHigh = toneKnobZeroLockValue;
            if(((inputDampHigh - previousInputDampHigh) < 0.01f) and ((inputDampHigh - previousInputDampHigh) > -0.01f)) {
                previousInputDampHigh = inputDampHigh;
                reverb.setInputFilterHighCutoffPitch(-1.f - (-1.f * inputDampHigh));
                if(toneKnobIsMoving) {
                    toneKnobLedTimer = 0;
                }
                if(toneKnobLedTimer < toneKnobLedOnTime) {
                    ++toneKnobLedTimer;
                    prepareLeds(0.f,inputDampHigh,0.f,0.f);
                } else if (toneKnobLedTimer == toneKnobLedOnTime) {
                    toneKnobLedTimer = toneKnobLedOnTime + 1;
                    prepareLeds(0.f,0.f,0.f,0.f);
                }
            }
        } else if (switchState1 == 2) {
            reverbDampHigh = toneKnobZeroLockValue;
            if(((reverbDampHigh - previousReverbDampHigh) < 0.01f) and ((reverbDampHigh - previousReverbDampHigh) > -0.01f)) {
                previousReverbDampHigh = reverbDampHigh;
                reverb.setTankFilterHighCutFrequency(-1.f - (-1.f * reverbDampHigh));
                if(toneKnobIsMoving) {
                    toneKnobLedTimer = 0;
                }
                if(toneKnobLedTimer < toneKnobLedOnTime) {
                    ++toneKnobLedTimer;
                    prepareLeds(0.f,0.f,0.f,reverbDampHigh);
                } else if (toneKnobLedTimer == toneKnobLedOnTime) {
                    toneKnobLedTimer = toneKnobLedOnTime + 1;
                    prepareLeds(0.f,0.f,0.f,0.f);
                }
            }
        }
    } else if(switchState0 == 1) {
        if(switchState1 == 1) {
            inputDampLow = toneKnobZeroLockValue;
            if(((inputDampLow - previousInputDampLow) < 0.01f) and ((inputDampLow - previousInputDampLow) > -0.01f)) {
                previousInputDampLow = inputDampLow;
                reverb.setInputFilterLowCutoffPitch(-1.f * inputDampLow);
                if(toneKnobIsMoving) {
                    toneKnobLedTimer = 0;
                }   
                if(toneKnobLedTimer < toneKnobLedOnTime) {
                    ++toneKnobLedTimer;
                    prepareLeds(inputDampLow,0.f,0.f,0.f);
                } else if (toneKnobLedTimer == toneKnobLedOnTime) {
                    toneKnobLedTimer = toneKnobLedOnTime + 1;
                    prepareLeds(0.f,0.f,0.f,0.f);
                }
            }
        } else if (switchState1 == 2) {
            reverbDampLow = toneKnobZeroLockValue;
            if(((reverbDampLow - previousReverbDampLow) < 0.01f) and ((reverbDampLow - previousReverbDampLow) > -0.01f)) {
                previousReverbDampLow = reverbDampLow;
                reverb.setTankFilterLowCutFrequency(-1.f * reverbDampLow);
                if(toneKnobIsMoving) {
                    toneKnobLedTimer = 0;
                }
                if(toneKnobLedTimer < toneKnobLedOnTime) {
                    ++toneKnobLedTimer;
                    prepareLeds(0.f,0.f,reverbDampLow,0.f);
                } else if (toneKnobLedTimer == toneKnobLedOnTime) {
                    toneKnobLedTimer = toneKnobLedOnTime + 1;
                    prepareLeds(0.f,0.f,0.f,0.f);
                }
            }
        }
    }

    if(switchState1 == 0) {
        switch(switchState0) {
            case 0:
                tempDiffusion = toneKnobZeroLockValue;
                if(((tempDiffusion - diffusion) < 0.01f) and ((tempDiffusion - diffusion) > -0.01f)) {
                    diffusion = tempDiffusion;
                    if(diffusion == 0.f) {
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
                        prepareLeds(diffusion,diffusion,diffusion,diffusion);
                    } else if (toneKnobLedTimer == toneKnobLedOnTime) {
                        toneKnobLedTimer = toneKnobLedOnTime + 1;
                        prepareLeds(0.f,0.f,0.f,0.f);
                    }
                }
                break;
            case 1:
                tempInputAmplification = toneKnobZeroLockValue;
                if(((tempInputAmplification - inputAmplification) < 0.01f) and ((tempInputAmplification - inputAmplification) > -0.01f)) {
                    inputAmplification = tempInputAmplification;
                    if(toneKnobIsMoving) {
                        toneKnobLedTimer = 0;
                    }
                    if(toneKnobLedTimer < toneKnobLedOnTime) {
                        ++toneKnobLedTimer;
                        prepareLeds(inputAmplification,inputAmplification,inputAmplification,inputAmplification);
                    } else if (toneKnobLedTimer == toneKnobLedOnTime) {
                        toneKnobLedTimer = toneKnobLedOnTime + 1;
                        prepareLeds(0.f,0.f,0.f,0.f);
                    }
                }
                break;
            case 2:
                tempOutputAmplification = toneKnobZeroLockValue;
                if(((tempOutputAmplification - outputAmplification) < 0.01f) and ((tempOutputAmplification - outputAmplification) > -0.01f)) {
                    outputAmplification = tempOutputAmplification;
                    if(toneKnobIsMoving) {
                        toneKnobLedTimer = 0;
                    }
                    if(toneKnobLedTimer < toneKnobLedOnTime) {
                        ++toneKnobLedTimer;
                        prepareLeds(outputAmplification,outputAmplification,outputAmplification,outputAmplification);
                    } else if (toneKnobLedTimer == toneKnobLedOnTime) {
                        toneKnobLedTimer = toneKnobLedOnTime + 1;
                        prepareLeds(0.f,0.f,0.f,0.f);
                    }
                }
                break;
        }
    }
}


// On falling edge, if button or cv is released within 0.25s
// buffers are cleared, otherwise if button is released before 10
// seconds, gain mode is changed. If button is held for 10 seconds
// mod depth is locked/unlocked to 3.125%.
inline void processButton() {
    if(buttonState) {
        if (buttonHoldTime == lockModDepthTime) {
            buttonHoldTime = lockModDepthTime + 1;
            genericLedTimer = 0;
            lockModDepthTo3_125_ = !lockModDepthTo3_125_;
        }
    } else {
        if(buttonHoldTime < bufferClearTriggerWindow) {
            if(previousButtonState) {
                genericLedTimer = 0;
                triggerClear = true;
            }
        } else if(buttonHoldTime < lockModDepthTime) {
            gainModeLedTimer = 0;
            ++gainMode;
        }
        buttonHoldTime = 0;
    }
}

inline void incrementButtonHoldCounterAudioRate() {
    if(buttonState) {
        ++buttonHoldTime;
    }
}

inline void interpolatingDelayHold() {
    if(holdCount < 192000)
        ++holdCount;
    else
        hold = 1.f;
}

inline void processAllParameters() {

    // Putting this here for larger audio block sizes.
    // Quick knob update times means less noise
    hw.ProcessAnalogControls();
    knobValue0 = KNOB0Ptr->Value();
    knobValue1 = KNOB1Ptr->Value();
    knobValue2 = KNOB2Ptr->Value();
    knobValue3 = KNOB3Ptr->Value();
    knobValue4 = KNOB4Ptr->Value();
    knobValue5 = KNOB5Ptr->Value();
    knobValue6 = KNOB6Ptr->Value();

    // If the tone knob is not moving and the mode LEDs are not shining, show audio IO levels on LEDs
    if ((gainModeLedTimer > gainModeLedOnTime) and (!toneKnobIsMoving)) {
        prepareLeds(leftInput * minus20dBGain, rightInput * minus20dBGain, leftOutput, rightOutput);
    }

    // Tone knob parameters smoothly lock to 0 to avoid any clicking when disabling diffusion and unwanted low/high cuts
    toneKnobValue = toneKnobLPF.processLowpass(knobValue2);
    checkIfToneKnobIsMoving(toneKnobValue);
    toneKnobZeroLockValue = toneKnobZeroLockLPF.processLowpass((knobValue2 >= 0.01f) * knobValue2);
    if(toneKnobZeroLockValue < 1.0e-030) {
        toneKnobZeroLockValue = 0.f;
    }

    // Mix knob locks to zero and one. Mix knob is very susceptible to noise along with pre-delay, mod depth, and time scale
    // These knobs are thus ran through one pole LPFs. It is important these 1 pole LPFs are evaluated at audio rate.
    wet = mixKnobLPF.processLowpass((knobValue0 > 0.99f) * 1.f + (knobValue0 >= 0.01f) * knobValue0 * (knobValue0 <= 0.99f));
    dry = 1.f - wet;

    // As with mix, mod speed need not be locked to zero. Mod speed is not succeptible to noise
    reverb.setTankModSpeed(0.5f + (knobValue1 * 100.f));

    // Mod depth value also smoothly locks to zero to avoid any clicking
    modDepthValue = modDepthKnobLPF.processLowpass((knobValue3 >= 0.01f) * knobValue3) * 16.0f;
    if(modDepthValue < 1.0e-030) {
        modDepthValue = 0.f;
    }

    // Ability to lock mod depth to the equivalent default 3.125% of VCV rack
    if(lockModDepthTo3_125_) {
        reverb.setTankModDepth(0.5f);
    } else {
        reverb.setTankModDepth(modDepthValue);
    }

    // Minimum value in VCV rack is 0.1f. In addition, the decay setting is not succeptible to noise.
    reverb.setDecay(0.1f + (knobValue4 * 0.8999f));

    // Time scale is very succeptible to noise. It need not be locked to zero though.
    reverb.setTimeScale(timeScaleKnobLPF.processLowpass(knobValue5) * 4.f);
    
    // Pre-delay knob is smoothly locked to zero and out of all controls is most succeptible to noise
    preDelay = preDelayKnobLPF.processLowpass((knobValue6 >= 0.01f) * knobValue6) * 4.f;
    if(preDelay < 1.0e-030) {
        preDelay = 0.f;
    }
    reverb.setPreDelay(preDelay);

    processSwitches();
}

inline void saveCounterAudioRate() {
    if(saveTimer < saveTime) {
        ++saveTimer;
        saveToggle = false;
    } else {
        saveTimer = 0;
        saveToggle = true;
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
            prepareLeds(0.f,0.f,0.f,1.f);
        } else if (gainModeLedTimer == gainModeLedOnTime) {
            ++gainModeLedTimer;
            prepareLeds(0.f,0.f,0.f,0.f);
        }
        break;
    case 1:
        leftOutput = softLimiter(leftOutput);
        rightOutput = softLimiter(rightOutput);
        leftOutput = hardLimit77_8_(leftOutput);
        rightOutput = hardLimit77_8_(rightOutput);
        if(gainModeLedTimer < gainModeLedOnTime) {
            ++gainModeLedTimer;
            prepareLeds(0.f,0.f,1.f,0.f);
        } else if (gainModeLedTimer == gainModeLedOnTime) {
            ++gainModeLedTimer;
            prepareLeds(0.f,0.f,0.f,0.f);
        }
        break;
    case 2:
        leftOutput = hardLimit100_(leftOutput);
        rightOutput = hardLimit100_(rightOutput);
        if(gainModeLedTimer < gainModeLedOnTime) {
            ++gainModeLedTimer;
            prepareLeds(0.f,0.f,1.f,1.f);
        } else if (gainModeLedTimer == gainModeLedOnTime) {
            ++gainModeLedTimer;
            prepareLeds(0.f,0.f,0.f,0.f);
        }
        break;
    case 3:
        leftOutput = hardLimit85_(leftOutput);
        rightOutput = hardLimit85_(rightOutput);
        if(gainModeLedTimer < gainModeLedOnTime) {
            ++gainModeLedTimer;
            prepareLeds(0.f,1.f,0.f,0.f);
        } else if (gainModeLedTimer == gainModeLedOnTime) {
            ++gainModeLedTimer;
            prepareLeds(0.f,0.f,0.f,0.f);
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
            prepareLeds(0.f,1.f,0.f,1.f);
        } else if (gainModeLedTimer == gainModeLedOnTime) {
            ++gainModeLedTimer;
            prepareLeds(0.f,0.f,0.f,0.f);
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
            prepareLeds(0.f,1.f,1.f,0.f);
        } else if (gainModeLedTimer == gainModeLedOnTime) {
            ++gainModeLedTimer;
            prepareLeds(0.f,0.f,0.f,0.f);
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
            prepareLeds(0.f,1.f,1.f,1.f);
        } else if (gainModeLedTimer == gainModeLedOnTime) {
            ++gainModeLedTimer;
            prepareLeds(0.f,0.f,0.f,0.f);
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
            prepareLeds(1.f,0.f,0.f,0.f);
        } else if (gainModeLedTimer == gainModeLedOnTime) {
            ++gainModeLedTimer;
            prepareLeds(0.f,0.f,0.f,0.f);
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
            prepareLeds(1.f,0.f,0.f,1.f);
        } else if (gainModeLedTimer == gainModeLedOnTime) {
            ++gainModeLedTimer;
            prepareLeds(0.f,0.f,0.f,0.f);
        }
        break;
    case 9:
        leftOutput = amp120_(leftOutput);
        rightOutput = amp120_(rightOutput);
        leftOutput = hardLimit100_(leftOutput);
        rightOutput = hardLimit100_(rightOutput);
        if(gainModeLedTimer < gainModeLedOnTime) {
            ++gainModeLedTimer;
            prepareLeds(1.f,0.f,1.f,0.f);
        } else if (gainModeLedTimer == gainModeLedOnTime) {
            ++gainModeLedTimer;
            prepareLeds(0.f,0.f,0.f,0.f);
        }
        break;
    case 10:
        leftOutput = amp120_(leftOutput);
        rightOutput = amp120_(rightOutput);
        leftOutput = hardLimit85_(leftOutput);
        rightOutput = hardLimit85_(rightOutput);
        if(gainModeLedTimer < gainModeLedOnTime) {
            ++gainModeLedTimer;
            prepareLeds(1.f,0.f,1.f,1.f);
        } else if (gainModeLedTimer == gainModeLedOnTime) {
            ++gainModeLedTimer;
            prepareLeds(0.f,0.f,0.f,0.f);
        }
        break;
    case 11:
        gainMode = 0;
        break;
    }
}

//unsigned int counter = 0;
void AudioCallback(AudioHandle::InputBuffer in,
              AudioHandle::OutputBuffer out,
              size_t size)
{
    for(size_t i = 0; i < size; i += 1) { 


        processAllParameters();

        incrementButtonHoldCounterAudioRate();

        interpolatingDelayHold();

        saveCounterAudioRate();

        prepareGenericLed();

        leftInput = in[0][i] * 10.f;
        rightInput = in[1][i] * 10.f;

        reverb.process(leftInput * minus18dBGain * minus20dBGain * (1.0f + inputAmplification * 7.f),
                    rightInput * minus18dBGain * minus20dBGain * (1.0f + inputAmplification * 7.f));

        leftOutput = ((leftInput * dry * 0.1f) + 
                    (reverb.getLeftOutput() * wet * 2.f)) * (0.25f + outputAmplification * 0.75f);
        rightOutput = ((rightInput * dry * 0.1f) + 
                    (reverb.getRightOutput() * wet * 2.f)) * (0.25f + outputAmplification * 0.75f);

        gainControl(leftOutput, rightOutput);

        out[0][i] = leftOutput;
        out[1][i] = rightOutput;
    }
};

float maxLoad = 0.f;

int main(void)
{
	hw.Init(true);

    limiter.configure(32000);

    // LEDs indicate we are starting up
    hw.leds[0].Set(1, 0, 0);
    hw.leds[1].Set(1, 0, 0);
    hw.leds[2].Set(1, 0, 0);
    hw.leds[3].Set(1, 0, 0);
    hw.UpdateLeds();

    for(int i = 0; i < 50; i++) {
        for(int j = 0; j < 144000; j++) {
            sdramData[i][j] = 0.f;
        }
    }

    // // Setup default settings and load saved data
    // Settings defaults;
    // defaults.gainMode = 0;
    // defaults.inputDampLow = 0.f;
    // defaults.inputDampHigh = 0.f;
    // defaults.reverbDampLow = 0.f;
    // defaults.reverbDampHigh = 0.f;
    // defaults.diffusion = 0.f;
    // defaults.inputAmplification = 0.f;
    // defaults.outputAmplification = 0.f;
    // storage.Init(defaults);
    // storage.RestoreDefaults();
    // loadData();

    reverb.setSampleRate(32000);

    reverb.setTimeScale(4.0f);
    reverb.setPreDelay(0.f);
    
    reverb.setInputFilterLowCutoffPitch(-1.f * inputDampLow);
    reverb.setInputFilterHighCutoffPitch(-1.f - (-1.f * inputDampHigh));
    reverb.enableInputDiffusion(true);
    reverb.setDecay(0.5f);
    reverb.setTankDiffusion(diffusion * 0.7f);
    reverb.setTankFilterLowCutFrequency(-1.f * reverbDampLow);
    reverb.setTankFilterHighCutFrequency(-1.f - (-1.f * reverbDampHigh));
    reverb.setTankModSpeed(0.5f);
    reverb.setTankModDepth(8.f);
    reverb.setTankModShape(0.5f);

    hw.SetAudioBlockSize(32);
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

	// LEDs indicate that we are ready to go
    hw.leds[0].Set(1, 0, 0);
    hw.leds[1].Set(1, 1, 0);
    hw.leds[2].Set(0, 1, 0);
    hw.leds[3].Set(0, 0, 1);
    hw.UpdateLeds();
    System::Delay(500);
    hw.leds[0].Set(0, 0, 0);
    hw.leds[1].Set(0, 0, 0);
    hw.leds[2].Set(0, 0, 0);
    hw.leds[3].Set(0, 0, 0);
    hw.UpdateLeds();

	while(1) {
        checkSwitches();
        // Process switches occurs at audio rate in the callback
        checkButton();
        processButton();
        // The button LED counter occurs at audio rate
        setAndUpdateGainLeds(led1, led2, led3, led4);
        // if(saveToggle) {
        //     saveData();
        //     saveToggle = false;
        // }

        // Clear buffers. Fingers crossed it works.
        if(triggerClear) {
            reverb.clear();
            triggerClear = false;
        }
    }
}
