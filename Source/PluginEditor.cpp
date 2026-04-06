/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NewProjectAudioProcessorEditor::NewProjectAudioProcessorEditor (NewProjectAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (400, 300);
    
    //store a shared ptr to the data
    staticPluginComunication = StaticPluginComunicationManager::getShared();
    staticPluginComunication->addListener(this);
    
    addAndMakeVisible(m_staticComunicationSlider);
    m_staticComunicationSlider.onValueChange = [&] ()
    {
        staticPluginComunication->setValue(m_staticComunicationSlider.getValue());
    };
    
    
    memMappedFilePluginComunication.addListener(this);
    addAndMakeVisible(m_memMappedFileComunicationSlider);
    m_memMappedFileComunicationSlider.onValueChange = [&] ()
    {
        memMappedFilePluginComunication.setValue(m_memMappedFileComunicationSlider.getValue());
    };
}

NewProjectAudioProcessorEditor::~NewProjectAudioProcessorEditor()
{
    //remember to remove the listener!!
    staticPluginComunication->removeListener(this);
    memMappedFilePluginComunication.removeListener(this);
}

//==============================================================================
void NewProjectAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
}

void NewProjectAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    m_staticComunicationSlider       .setBounds(area.removeFromTop(getHeight() / 3));
    m_memMappedFileComunicationSlider.setBounds(area.removeFromBottom(getHeight() / 3));
}

void NewProjectAudioProcessorEditor::interPluginValueChanged(float value)
{
    m_staticComunicationSlider.setValue(value, juce::dontSendNotification);
}

void NewProjectAudioProcessorEditor::sharedValueChanged(float value)
{
    m_memMappedFileComunicationSlider.setValue(value, juce::dontSendNotification);
}
