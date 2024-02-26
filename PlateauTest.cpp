#define DSJ_PLATEAU_HPP

#include "Dattorro.hpp"
#include "dsp/shaping/NonLinear.hpp"
#include "dsp/modulation/LinearEnvelope.hpp"
#include <vector>

using namespace daisy;
using namespace daisysp;

DaisyVersio hw;

const float preDelayNormSens = 0.1f;
const float preDelayLowSens = 0.05f;
const float sizeMin = 0.0025f;
const float sizeMax = 4.0f;
const float diffMin = 0.f;
const float diffMax = 1.f;
const float decayMin = 0.1f;
const float decayMax = 0.9999f;

const float reverbDampMin = 0.f;
const float reverbDampMax = 10.f;

const float modSpeedMin = 0.f;
const float modSpeedMax = 1.f;

const float modDepthMin = 0.f;
const float modDepthMax = 16.f;

constexpr float minus20dBGain = 0.1f;
constexpr float minus18dBGain = 0.12589254f;
constexpr float zerodBGain = 1.f;

constexpr float saturatorPreGain = 0.111f;
constexpr float saturatorDrive = 0.95f;
constexpr float saturatorPostGain = 9.999f;

float wet = 0.0f;
float dry = 1.f;
float wetValue = 0.f;



float preDelay = 0.f;
float preDelayCVSens = preDelayNormSens;
float timeScale = 1.f;
float diffusion = 1.f;
float decay = 0.f;
float inputSensitivity = minus18dBGain;
float inputDampLow = 0.f;
float inputDampHigh = 10.f;
float reverbDampLow = 0.f;
float reverbDampHigh = 10.f;
float modSpeed = 0.1f;
float modShape = 0.5f;
float modDepth = 0.f;

bool freezeButtonState = false;
bool freezeToggle = false;
bool freezeToggleButtonState = false;
bool freeze = false;
bool frozen = false;
bool tunedButtonState = false;
bool diffuseButtonState = true;
int preDelayCVSensState = 0;
int inputSensitivityState = 0;
bool softDriveOutput = 0;

bool clear = false;
bool cleared = true;
bool fadeOut = false;
bool fadeIn = false;

Dattorro reverb;
LinearEnvelope envelope;

int panelStyle = 0;
int tuned = 0;
int diffuseInput = 1;

float leftInput = 0.f;
float rightInput = 0.f;
float leftOutput = 0.f;
float rightOutput = 0.f;

float tempDecay = 0.f;
float tempModSpeed = 0.f;
float tempModDepth = 0.f;
float tempCutoff = 0.f;
float tempSize = 0.f;
float tempDiffusion = 0.f;


void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
              AudioHandle::InterleavingOutputBuffer out,
              size_t                                size)
{
    for(size_t i = 0; i < size; i += 2)
    {
        reverb.process(in[i] * 10 * minus20dBGain * inputSensitivity * envelope._value,
                    in[i + 1] * 10 * minus20dBGain * inputSensitivity * envelope._value);

        leftOutput = reverb.getLeftOutput();
        rightOutput = reverb.getRightOutput();

        if(leftOutput > 10 || leftOutput < -10)
            leftOutput = 0;

        if(rightOutput > 10 || rightOutput < -10)
            rightOutput = 0;

        out[i] = (in[i] * dry) + 
                (leftOutput * wet * envelope._value);
        out[i + 1] = (in[i + 1] * dry) + 
                (rightOutput * wet * envelope._value);
    }
}

int main(void)
{
	hw.Init();

    reverb.setSampleRate(48000);
    envelope.setSampleRate(48000);
    envelope.setTime(0.004f);
    envelope._value = 1.f;

    reverb.setTimeScale(timeScale);
    // Clamps predelay between 0 and 1
    reverb.setPreDelay(((preDelay > 0.f) * (preDelay < 1.f) * preDelay) + (preDelay > 1.f));
    reverb.setInputFilterLowCutoffPitch(inputDampLow);
    reverb.setInputFilterHighCutoffPitch(inputDampHigh);
    reverb.enableInputDiffusion(diffuseInput > 0.5f);
    reverb.setDecay(decay);
    reverb.setTankDiffusion(diffusion);
    reverb.setTankFilterLowCutFrequency(reverbDampLow);
    reverb.setTankFilterHighCutFrequency(reverbDampHigh);
    reverb.setTankModSpeed(modSpeed);
    reverb.setTankModDepth(modDepth);
    reverb.setTankModShape(modShape);

	hw.SetAudioBlockSize(1); // Number of samples handled per callback
	//hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

    
    hw.StartAudio(AudioCallback);
	hw.StartAdc();

	while(1) {
        hw.ProcessAnalogControls();
        hw.UpdateExample(); // Control the LED colors using the knobs and gate inputs
        hw.UpdateLeds();

        wetValue = hw.GetKnobValue(DaisyVersio::KNOB_0);

        dry = 1.f - wetValue;
        wet = wetValue;

        tempModSpeed = hw.GetKnobValue(DaisyVersio::KNOB_1);
        if(tempModSpeed > modSpeedMax)
            tempModSpeed = modSpeedMax;
        if(tempModSpeed < modSpeedMin)
            tempModSpeed = modSpeedMin;
        reverb.setTankModSpeed(tempModSpeed);

        tempCutoff = hw.GetKnobValue(DaisyVersio::KNOB_2) * 10;
        if(tempCutoff > reverbDampMax)
            tempCutoff = reverbDampMax;
        if(tempCutoff < reverbDampMin)
            tempCutoff = reverbDampMin;
        reverb.setTankFilterLowCutFrequency(tempCutoff);
        // reverb.setTankFilterHighCutFrequency(tempCutoff);

        tempModDepth = hw.GetKnobValue(DaisyVersio::KNOB_3) * 16;
        if(tempModDepth > modDepthMax)
            tempModDepth = modDepthMax;
        if(tempModDepth < modDepthMin)
            tempModDepth = modDepthMin;
        reverb.setTankModDepth(tempModDepth);

        tempDecay = hw.GetKnobValue(DaisyVersio::KNOB_4);
        if(tempDecay > decayMax)
            tempDecay = decayMax;
        if(tempDecay < decayMin)
            tempDecay = decayMin;
        reverb.setDecay(tempDecay);

        tempSize = hw.GetKnobValue(DaisyVersio::KNOB_5) * 4;
        if(tempSize > sizeMax)
            tempSize = sizeMax;
        if(tempSize < sizeMin)
            tempSize = sizeMin;
        reverb.setTimeScale(tempSize);

        tempDiffusion = hw.GetKnobValue(DaisyVersio::KNOB_6);
        if(tempDiffusion > diffMax)
            tempDiffusion = diffMax;
        if(tempDiffusion < diffMin)
            tempDiffusion = diffMin;
        reverb.setTankDiffusion(tempDiffusion);
    }
}
