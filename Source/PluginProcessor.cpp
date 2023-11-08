/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BasicMultiTapDelayAudioProcessor::BasicMultiTapDelayAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), treeState(*this, nullptr, "PARAMETERS", createParameterLayout())
#endif
{
    treeState.addParameterListener("delayTimeID1", this);
    treeState.addParameterListener("delayTimeID2", this);
    treeState.addParameterListener("delayTimeID3", this);
    treeState.addParameterListener("delayTimeID4", this);
    treeState.addParameterListener("delayFeedbackID", this);
    treeState.addParameterListener("wetLevelID", this);
    treeState.addParameterListener("dryLevelID", this);
}

BasicMultiTapDelayAudioProcessor::~BasicMultiTapDelayAudioProcessor()
{
    treeState.removeParameterListener("delayTimeID1", this);
    treeState.removeParameterListener("delayTimeID2", this);
    treeState.removeParameterListener("delayTimeID3", this);
    treeState.removeParameterListener("delayTimeID4", this);
    treeState.removeParameterListener("delayFeedbackID", this);
    treeState.removeParameterListener("wetLevelID", this);
    treeState.removeParameterListener("dryLevelID", this);
}

//==============================================================================

juce::AudioProcessorValueTreeState::ParameterLayout BasicMultiTapDelayAudioProcessor::createParameterLayout()
{
    std::vector <std::unique_ptr<juce::RangedAudioParameter>> params;
    
    params.reserve(3);
    
    juce::NormalisableRange<float> delayTimeRange (0.f, 2000.f, 1.f, 1.f, false);
    params.push_back(std::make_unique<juce::AudioParameterFloat>("delayTimeID1", "time 1", delayTimeRange, 250.f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("delayTimeID2", "time 2", delayTimeRange, 350.f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("delayTimeID3", "time 3", delayTimeRange, 450.f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("delayTimeID4", "time 4", delayTimeRange, 550.f));
    
    juce::NormalisableRange<float> delayFeedbackRange (0.f, 100.f, 1.f, 1.f, false);
    params.push_back(std::make_unique<juce::AudioParameterFloat>("delayFeedbackID", "feedback", delayFeedbackRange, 30.f));
    
    juce::NormalisableRange<float> wetLevelRange (-60.f, 12.f, 1.f, 1.f, false);
    params.push_back(std::make_unique<juce::AudioParameterFloat>("wetLevelID", "wetLevel", wetLevelRange, -3.f));
    
    juce::NormalisableRange<float> dryLevelRange (-60.f, 12.f, 1.f, 1.f, false);
    params.push_back(std::make_unique<juce::AudioParameterFloat>("dryLevelID", "dryLevel", dryLevelRange, -3.f));
        
    return { params.begin(), params.end() };
}

void BasicMultiTapDelayAudioProcessor::parameterChanged(const juce::String &parameterID, float newValue)
{
    if (parameterID == "delayTimeID1")
    {
        params.tap1_mSec = newValue;
        multiTapDelay.setMultiTapParameters(params);
    }
    
    if (parameterID == "delayTimeID2")
    {
        params.tap2_mSec = newValue;
        multiTapDelay.setMultiTapParameters(params);
    }
    
    if (parameterID == "delayTimeID3")
    {
        params.tap3_mSec = newValue;
        multiTapDelay.setMultiTapParameters(params);
    }
    
    if (parameterID == "delayTimeID4")
    {
        params.tap4_mSec = newValue;
        multiTapDelay.setMultiTapParameters(params);
    }
    
    if (parameterID == "delayFeedbackID")
    {
        params.feedback_Pct = newValue;
        multiTapDelay.setMultiTapParameters(params);
    }
    
    if (parameterID == "wetLevelID")
    {
        params.wetLevel_dB = newValue;
        multiTapDelay.setMultiTapParameters(params);
    }
    
    if (parameterID == "dryLevelID")
    {
        params.dryLevel_dB = newValue;
        multiTapDelay.setMultiTapParameters(params);
    }
}


const juce::String BasicMultiTapDelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BasicMultiTapDelayAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool BasicMultiTapDelayAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool BasicMultiTapDelayAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double BasicMultiTapDelayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BasicMultiTapDelayAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int BasicMultiTapDelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BasicMultiTapDelayAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String BasicMultiTapDelayAudioProcessor::getProgramName (int index)
{
    return {};
}

void BasicMultiTapDelayAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void BasicMultiTapDelayAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    multiTapDelay.reset(sampleRate);

    multiTapDelay.createDelayBuffers(sampleRate, 2000);

    params.tap1_mSec = treeState.getRawParameterValue("delayTimeID1")->load();
    params.tap2_mSec = treeState.getRawParameterValue("delayTimeID2")->load();
    params.tap3_mSec = treeState.getRawParameterValue("delayTimeID3")->load();
    params.tap4_mSec = treeState.getRawParameterValue("delayTimeID4")->load();

    params.feedback_Pct = treeState.getRawParameterValue("delayFeedbackID")->load();

    params.dryLevel_dB = treeState.getRawParameterValue("dryLevelID")->load();
    params.wetLevel_dB = treeState.getRawParameterValue("wetLevelID")->load();
    
    multiTapDelay.setMultiTapParameters(params);
}

void BasicMultiTapDelayAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BasicMultiTapDelayAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void BasicMultiTapDelayAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    int numSamples = buffer.getNumSamples();
    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());


    for (int sample = 0; sample < numSamples; ++sample)
    {
        auto* xnL = buffer.getReadPointer(0);
        auto* ynL = buffer.getWritePointer(0);
        
        auto* xnR = buffer.getReadPointer(1);
        auto* ynR = buffer.getWritePointer(1);
        
        float inputs[2] = {xnL[sample], xnR[sample]};
        float outputs[2] = {0.0, 0.0};
        multiTapDelay.processAudioFrame(inputs, outputs, 2, 2);
        
        ynL[sample] = outputs[0];
        ynR[sample] = outputs[1];
    }
    
}

//==============================================================================
bool BasicMultiTapDelayAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* BasicMultiTapDelayAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor (*this);
}

//==============================================================================
void BasicMultiTapDelayAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void BasicMultiTapDelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BasicMultiTapDelayAudioProcessor();
}
