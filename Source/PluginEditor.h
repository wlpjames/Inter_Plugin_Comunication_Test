/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "InterPluginManager.h"

//==============================================================================
/**
*/
class NewProjectAudioProcessorEditor
: public juce::AudioProcessorEditor
, public StaticPluginComunicationManager::Listener
, public MMFComunicaionManager::Listener
{
public:
    NewProjectAudioProcessorEditor (NewProjectAudioProcessor&);
    ~NewProjectAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    ///implement pure virtual function of PluginComunicationManager::Listener
    void interPluginValueChanged(float value) override;
    
    //call back from the mem mapped file comunication
    void sharedValueChanged(float value) override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    NewProjectAudioProcessor& audioProcessor;
    
    std::shared_ptr<StaticPluginComunicationManager> staticPluginComunication;
    MMFComunicaionManager memMappedFilePluginComunication;
    
    juce::Slider m_staticComunicationSlider;
    juce::Slider m_memMappedFileComunicationSlider;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NewProjectAudioProcessorEditor)
};
