//
// Plateu.hpp
// Author: Dale Johnson
// Contact: valley.audio.soft@gmail.com
// Date: 24/6/2018
//
// Copyright 2018 Dale Johnson. Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met: 1. Redistributions of
// source code must retain the above copyright notice, this list of conditions and the following
// disclaimer. 2. Redistributions in binary form must reproduce the above copyright notice, this
// list of conditions and the following disclaimer in the documentation and/or other materials
// provided with the distribution. 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this software without
// specific prior written permission.THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
// CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
// EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE.
//
// Plateau is based on the venerable Dattorro (1997) reverb algorithm.
// Reference: Dattorro, J. (1997). Effect design part 1: Reverberator and other filters, J. Audio
// Eng. Soc, 45(9), 660-684.

#ifndef DSJ_PLATEAU_HPP
#define DSJ_PLATEAU_HPP

#include "Dattorro.hpp"
#include "../dsp/shaping/NonLinear.hpp"
#include "../dsp/modulation/LinearEnvelope.hpp"
#include <vector>

struct Plateau {

    enum InputIds {
        LEFT_INPUT,
        RIGHT_INPUT,

        DRY_CV_INPUT,
        WET_CV_INPUT,
        PRE_DELAY_CV_INPUT,
        INPUT_LOW_DAMP_CV_INPUT,
        INPUT_HIGH_DAMP_CV_INPUT,

        SIZE_CV_INPUT,
        DIFFUSION_CV_INPUT,
        DECAY_CV_INPUT,
        REVERB_HIGH_DAMP_CV_INPUT,
        REVERB_LOW_DAMP_CV_INPUT,

        MOD_SPEED_CV_INPUT,
        MOD_SHAPE_CV_INPUT,
        MOD_DEPTH_CV_INPUT,

        FREEZE_CV_INPUT,
        CLEAR_CV_INPUT,

        NUM_INPUTS
    };

    enum OutputIds {
        LEFT_OUTPUT,
        RIGHT_OUTPUT,
        NUM_OUTPUTS
    };

    enum ParamIds {
        DRY_PARAM,
        WET_PARAM,
        PRE_DELAY_PARAM,
        INPUT_LOW_DAMP_PARAM,
        INPUT_HIGH_DAMP_PARAM,

        SIZE_PARAM,
        DIFFUSION_PARAM,
        DECAY_PARAM,
        REVERB_HIGH_DAMP_PARAM,
        REVERB_LOW_DAMP_PARAM,

        MOD_SPEED_PARAM,
        MOD_SHAPE_PARAM,
        MOD_DEPTH_PARAM,

        FREEZE_PARAM,
        CLEAR_PARAM,
        FREEZE_TOGGLE_PARAM,
        CLEAR_TOGGLE_PARAM,

        DRY_CV_PARAM,
        WET_CV_PARAM,
        INPUT_LOW_DAMP_CV_PARAM,
        INPUT_HIGH_DAMP_CV_PARAM,

        SIZE_CV_PARAM,
        DIFFUSION_CV_PARAM,
        DECAY_CV_PARAM,
        REVERB_HIGH_DAMP_CV_PARAM,
        REVERB_LOW_DAMP_CV_PARAM,

        MOD_SPEED_CV_PARAM,
        MOD_SHAPE_CV_PARAM,
        MOD_DEPTH_CV_PARAM,

        TUNED_MODE_PARAM,
        DIFFUSE_INPUT_PARAM,

        NUM_PARAMS
    };

    enum LightIds {
        FREEZE_LIGHT,
        CLEAR_LIGHT,
        FREEZE_TOGGLE_LIGHT,
        TUNED_MODE_LIGHT,
        DIFFUSE_INPUT_LIGHT,
        NUM_LIGHTS
    };

    // CV scaling
    const float dryMin = 0.f;
    const float dryMax = 1.f;
    const float wetMin = 0.f;
    const float wetMax = 1.f;
    const float preDelayNormSens = 0.1f;
    const float preDelayLowSens = 0.05f;
    const float sizeMin = 0.0025f;
    const float sizeMax = 4.0f;
    const float diffMin = 0.f;
    const float diffMax = 1.f;
    const float decayMin = 0.1f;
    const float decayMax = 0.9999f;
    const float reverbLowDampMin = 0.f;
    const float reverbLowDampMax = 10.f;
    const float reverbHighDampMin = 0.f;
    const float reverbHIghDampMax = 10.f;
    const float modSpeedMin = 0.f;
    const float modSpeedMax = 1.f;
    const float modDepthMin = 0.f;
    const float modDepthMax = 16.f;
    const float modShapeMin = 0.001f;
    const float modShapeMax = 0.999f;

    static constexpr float minus20dBGain = 0.1f;
    static constexpr float minus18dBGain = 0.12589254f;
    static constexpr float zerodBGain = 1.f;

    static constexpr float saturatorPreGain = 0.111f;
    static constexpr float saturatorDrive = 0.95f;
    static constexpr float saturatorPostGain = 9.999f;

    float wet = 0.5f;
    float dry = 1.f;
    float preDelay = 0.f;
    float preDelayCVSens = preDelayNormSens;
    float size = 1.f;
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
    bool diffuseButtonState = false;
    int preDelayCVSensState = 0;
    int inputSensitivityState = 0;
    bool softDriveOutput = 0;

    bool clear = false;
    bool cleared = true;
    bool fadeOut = false;
    bool fadeIn = false;

    float leftInput = 0.f;
    float rightInput = 0.f;
    float leftOutput = 0.f;
    float rightOutput = 0.f;

    Dattorro reverb;
    LinearEnvelope envelope;

    int panelStyle = 0;
    int tuned = 0;
    int diffuseInput = 1;

    Plateau();

    //void process(const ProcessArgs &args) override;
    //Loop function that perfroms the necessary algorithms.

    void getParameters();
    void setLights();
};

#endif
