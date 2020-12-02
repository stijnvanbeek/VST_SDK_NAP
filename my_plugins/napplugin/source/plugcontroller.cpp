//------------------------------------------------------------------------
// Project     : VST SDK
//
// Category    : Examples
// Filename    : plugcontroller.cpp
// Created by  : Steinberg, 01/2018
// Description : HelloWorld Example for VST 3
//
//-----------------------------------------------------------------------------
// LICENSE
// (c) 2020, Steinberg Media Technologies GmbH, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation 
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this 
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "../include/plugcontroller.h"
#include "../include/plugids.h"
#include "public.sdk/source/vst/utility/stringconvert.h"

#include "napinfo.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/base/ibstream.h"

#include <parameter.h>
#include <parameterservice.h>
#include <parametertypes.h>
#include <controlthread.h>

namespace Steinberg {
namespace napplugin {

//-----------------------------------------------------------------------------
tresult PLUGIN_API PlugController::initialize (FUnknown* context)
{

	tresult result = EditController::initialize (context);
	if (result == kResultTrue)
	{
		//---Create Parameters------------
		parameters.addParameter (STR16("Bypass"), STR16(""), 1, 0,
		                         Vst::ParameterInfo::kCanAutomate | Vst::ParameterInfo::kIsBypass,
                                 kBypassId);

		nap::ControlThread::get().enqueue([&](){
            auto paramID = kBypassId + 1;
            mParameters = nap::Global::parameters;
            for (auto& napParameter : mParameters)
            {
                Vst::TChar paramName[128];

                if (napParameter->get_type() == RTTI_OF(nap::ParameterFloat))
                {
                    auto napParameterFloat = rtti_cast<nap::ParameterFloat>(napParameter);
                    VST3::StringConvert::convert(napParameterFloat->getDisplayName(), paramName);
                    auto parameter = std::make_unique<Vst::RangeParameter>(paramName, paramID++, STR16(""), napParameterFloat->mMinimum, napParameterFloat->mMaximum, napParameterFloat->mValue);
                    parameters.addParameter(parameter.release());
                }

                if (napParameter->get_type() == RTTI_OF(nap::ParameterInt))
                {
                    auto napParameterInt = rtti_cast<nap::ParameterInt>(napParameter);
                    VST3::StringConvert::convert(napParameterInt->getDisplayName(), paramName);
                    auto parameter = std::make_unique<Vst::RangeParameter>(paramName, paramID++, STR16(""), napParameterInt->mMinimum, napParameterInt->mMaximum, napParameterInt->mValue, 1.f);
                    parameters.addParameter(parameter.release());
                }

                if (napParameter->get_type() == RTTI_OF(nap::ParameterOptionList))
                {
                    auto napParameterOptionList = rtti_cast<nap::ParameterOptionList>(napParameter);
                    VST3::StringConvert::convert(napParameterOptionList->getDisplayName(), paramName);
                    auto parameter = std::make_unique<Vst::StringListParameter>(paramName, paramID++, STR16(""));
                    Vst::TChar optionName[128];
                    for (auto& option : napParameterOptionList->getOptions())
                    {
                        VST3::StringConvert::convert(option, optionName);
                        parameter->appendString(optionName);
                    }
                    parameters.addParameter(parameter.release());
                }
            }
		}, true);
	}

	return kResultTrue;
}

//------------------------------------------------------------------------
tresult PLUGIN_API PlugController::setComponentState (IBStream* state)
{
	// we receive the current state of the component (processor part)
	// we read our parameters and bypass value...
	if (!state)
		return kResultFalse;

	IBStreamer streamer (state, kLittleEndian);

	auto paramId = 1;
	for (auto& napParam : mParameters)
    {
        if (napParam->get_type() == RTTI_OF(nap::ParameterFloat))
        {
            float value;
            if (!streamer.readFloat(value))
                return kResultFalse;
            auto param = parameters.getParameter(paramId++);
            param->setNormalized(param->toNormalized(value));
        }
        if (napParam->get_type() == RTTI_OF(nap::ParameterInt) || napParam->get_type() == RTTI_OF(nap::ParameterOptionList))
        {
            int value;
            if (!streamer.readInt32(value))
                return kResultFalse;
            auto param = parameters.getParameter(paramId++);
            param->setNormalized(param->toNormalized(value));
        }
    }

	// read the bypass
	int32 bypassState;
	if (!streamer.readInt32 (bypassState))
		return kResultFalse;
	setParamNormalized (kBypassId, bypassState ? 1 : 0);

	return kResultOk;
}

//------------------------------------------------------------------------
} // namespace napplugin
} // namespace Steinberg
