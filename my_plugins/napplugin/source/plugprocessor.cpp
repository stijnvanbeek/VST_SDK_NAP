//------------------------------------------------------------------------
// Project     : VST SDK
//
// Category    : Examples
// Filename    : plugprocessor.cpp
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

#include "../include/plugprocessor.h"
#include "../include/plugids.h"
#include "napinfo.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/vst/ivstevents.h"

#include <utility/errorstate.h>
#include <utility/fileutils.h>
#include <sceneservice.h>
#include <audio/service/audioservice.h>
#include <audio/service/advancedaudioservice.h>
#include <pythonscriptservice.h>
#include <parameter.h>
#include <parameterservice.h>

namespace Steinberg {
namespace HelloWorld {

// Initialize static member
std::unique_ptr<nap::ControlThread> PlugProcessor::sControlThread = nullptr;

    //-----------------------------------------------------------------------------
PlugProcessor::PlugProcessor ()
{
	// register its editor class
	setControllerClass (MyControllerUID);
}


PlugProcessor::~PlugProcessor()
{
    sControlThread->removeCore(*mCore);
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API PlugProcessor::initialize (FUnknown* context)
{
	//---always initialize the parent-------
	tresult result = AudioEffect::initialize (context);
	if (result != kResultTrue)
		return kResultFalse;

	//--- create midi input
    addEventInput (STR16("MIDI in"), 1);

	//---create Audio In/Out busses------
	// we want a stereo Input and a Stereo Output
	addAudioInput (STR16("AudioInput"), Vst::SpeakerArr::kStereo);
	addAudioOutput (STR16("AudioOutput"), Vst::SpeakerArr::kStereo);

    nap::utility::ErrorState error;

    std::set<nap::rtti::TypeInfo> serviceTypes = { RTTI_OF(nap::SceneService), RTTI_OF(nap::audio::AudioService), RTTI_OF(nap::audio::AdvancedAudioService), RTTI_OF(nap::MidiService), RTTI_OF(nap::ParameterService) };
//    std::set<nap::rtti::TypeInfo> serviceTypes = { RTTI_OF(nap::SceneService), RTTI_OF(nap::audio::AudioService), RTTI_OF(nap::audio::AdvancedAudioService), RTTI_OF(nap::PythonScriptService), RTTI_OF(nap::MidiService), RTTI_OF(nap::ParameterService) };

    // Initialize engine
    mCore = std::make_unique<nap::Core>();
    if (!mCore->initializeEngine(nap::dataPath, nap::pythonHomePath, serviceTypes, error))
    {
        error.fail("unable to initialize engine");
        return kResultTrue;
    }
    // Initialize the various services
    if (!mCore->initializeServices(error))
    {
        mCore->shutdownServices();
        error.fail("Failed to initialize services");
        return kResultFalse;
    }

    if (!mCore->initializePython(error))
        return kResultFalse;

    mAudioService = mCore->getService<nap::audio::AudioService>();
    if (mAudioService == nullptr)
    {
        error.fail("Couldn't find audio service");
        return kResultFalse;
    }

    mMidiService = mCore->getService<nap::MidiService>();
    if (mMidiService == nullptr)
    {
        error.fail("Couldn't find midi service");
        return kResultFalse;
    }

    auto resourceManager = mCore->getResourceManager();
    if (!resourceManager->loadFile(nap::utility::getFileName(nap::dataPath), error))
    {
        error.fail("Failed to load json file");
        return kResultFalse;
    }

    mParameters = resourceManager->findObject<nap::ParameterGroup>("Parameters");
    if (nap::Global::parameters == nullptr)
        nap::Global::parameters = mParameters.get();

    mCore->start();

    if (sControlThread == nullptr)
    {
        sControlThread = std::make_unique<nap::ControlThread>([](double){});
        sControlThread->start();
    }

    sControlThread->addCore(*mCore);

	return kResultTrue;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API PlugProcessor::setBusArrangements (Vst::SpeakerArrangement* inputs,
                                                            int32 numIns,
                                                            Vst::SpeakerArrangement* outputs,
                                                            int32 numOuts)
{
    auto& nodeManager = mAudioService->getNodeManager();
    nodeManager.setInputChannelCount(2);
    nodeManager.setOutputChannelCount(2);

    return AudioEffect::setBusArrangements (inputs, numIns, outputs, numOuts);
//	return kResultFalse;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API PlugProcessor::setupProcessing (Vst::ProcessSetup& setup)
{
	// here you get, with setup, information about:
	// sampleRate, processMode, maximum number of samples per audio block
    auto& nodeManager = mAudioService->getNodeManager();
    nodeManager.setSampleRate(setup.sampleRate);
	nodeManager.setInternalBufferSize(setup.maxSamplesPerBlock);

	return AudioEffect::setupProcessing (setup);
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API PlugProcessor::setActive (TBool state)
{
	if (state) // Initialize
	{
		// Allocate Memory Here
		// Ex: algo.create ();
	}
	else // Release
	{
		// Free Memory if still allocated
		// Ex: if(algo.isCreated ()) { algo.destroy (); }
	}
	return AudioEffect::setActive (state);
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API PlugProcessor::process (Vst::ProcessData& data)
{
	//--- Read inputs parameter changes-----------
	if (data.inputParameterChanges)
	{
		int32 numParamsChanged = data.inputParameterChanges->getParameterCount ();
		for (int32 index = 0; index < numParamsChanged; index++)
		{
			Vst::IParamValueQueue* paramQueue =
			    data.inputParameterChanges->getParameterData (index);
			if (paramQueue)
			{
				Vst::ParamValue value;
				int32 sampleOffset;
				int32 numPoints = paramQueue->getPointCount ();
				switch (paramQueue->getParameterId ())
				{
					case HelloWorldParams::kParamVolId:
						if (paramQueue->getPoint (numPoints - 1, sampleOffset, value) ==
						    kResultTrue)
							mParam1 = value;
						break;
					case HelloWorldParams::kParamOnId:
						if (paramQueue->getPoint (numPoints - 1, sampleOffset, value) ==
						    kResultTrue)
							mParam2 = value > 0 ? 1 : 0;
						break;
					case HelloWorldParams::kBypassId:
						if (paramQueue->getPoint (numPoints - 1, sampleOffset, value) ==
						    kResultTrue)
							mBypass = (value > 0.5f);
						break;
				}
			}
		}
	}

	// --- Process note events
	auto events = data.inputEvents;
    if (events)
    {
        int32 count = events->getEventCount ();
        for (int32 i = 0; i < count; i++)
        {
            Vst::Event e;
            events->getEvent (i, e);
            switch (e.type)
            {
                case Vst::Event::kNoteOnEvent:
                {
                    auto midiEvent = std::make_unique<nap::MidiEvent>(nap::MidiEvent::Type::noteOn, e.noteOn.pitch, e.noteOn.velocity * 127);
                    mMidiService->enqueueEvent(std::move(midiEvent));
//                    notes[eventPos++] = e.sampleOffset;
//                    notes[eventPos++] = e.noteOn.pitch;
//                    notes[eventPos++] = e.noteOn.velocity * 127;
                    break;
                }
                case Vst::Event::kNoteOffEvent:
                {
                    auto midiEvent = std::make_unique<nap::MidiEvent>(nap::MidiEvent::Type::noteOff, e.noteOn.pitch, 0);
                    mMidiService->enqueueEvent(std::move(midiEvent));
//                    notes[eventPos++] = e.sampleOffset;
//                    notes[eventPos++] = e.noteOff.pitch;
//                    notes[eventPos++] = 0;
                    break;
                }
                default:
                    continue;
            }
        }
    }

    //--- Process Audio---------------------
	//--- ----------------------------------
	if (data.numOutputs == 0)
	{
		// nothing to do
		return kResultOk;
	}

	if (data.numSamples > 0)
	{
		// Process Algorithm
		// Ex: algo.process (data.inputs[0].channelBuffers32, data.outputs[0].channelBuffers32,
		// data.numSamples);
		mAudioService->onAudioCallback(data.numInputs > 0 ? data.inputs[0].channelBuffers32 : nullptr, data.outputs[0].channelBuffers32, data.numSamples);
	}
	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API PlugProcessor::setState (IBStream* state)
{
	if (!state)
		return kResultFalse;

	// called when we load a preset or project, the model has to be reloaded

	IBStreamer streamer (state, kLittleEndian);

	float savedParam1 = 0.f;
	if (streamer.readFloat (savedParam1) == false)
		return kResultFalse;

	int32 savedParam2 = 0;
	if (streamer.readInt32 (savedParam2) == false)
		return kResultFalse;

	int32 savedBypass = 0;
	if (streamer.readInt32 (savedBypass) == false)
		return kResultFalse;

	mParam1 = savedParam1;
	mParam2 = savedParam2 > 0 ? 1 : 0;
	mBypass = savedBypass > 0;

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API PlugProcessor::getState (IBStream* state)
{
	// here we need to save the model (preset or project)

	float toSaveParam1 = mParam1;
	int32 toSaveParam2 = mParam2;
	int32 toSaveBypass = mBypass ? 1 : 0;

	IBStreamer streamer (state, kLittleEndian);
	streamer.writeFloat (toSaveParam1);
	streamer.writeInt32 (toSaveParam2);
	streamer.writeInt32 (toSaveBypass);

	return kResultOk;
}

//------------------------------------------------------------------------
} // namespace
} // namespace Steinberg
