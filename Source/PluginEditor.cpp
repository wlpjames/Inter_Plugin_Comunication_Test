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
    
    addAndMakeVisible(m_staticComsLabel);
    m_staticComsLabel.setText("In-Process Static Comunication", juce::dontSendNotification);
    m_staticComsLabel.attachToComponent(&m_staticComunicationSlider, false);
    
    addAndMakeVisible(m_staticComunicationSlider);
    m_staticComunicationSlider.onValueChange = [&] ()
    {
        staticPluginComunication->setValue(m_staticComunicationSlider.getValue());
    };
    
    addAndMakeVisible(m_memMappedLabel);
    m_memMappedLabel.setText("Memory Mapped File Comunication", juce::dontSendNotification);
    m_memMappedLabel.attachToComponent(&m_memMappedFileComunicationSlider, false);
    
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
    auto area = getLocalBounds().reduced(50);
    m_staticComunicationSlider       .setBounds(area.removeFromTop(getHeight() / 3).reduced(20));
    m_memMappedFileComunicationSlider.setBounds(area.removeFromBottom(getHeight() / 3).reduced(20));
    
    m_memMappedLabel.setBounds(m_memMappedFileComunicationSlider.getBounds());
    m_staticComsLabel.setBounds(m_staticComunicationSlider.getBounds());
}

void NewProjectAudioProcessorEditor::interPluginValueChanged(float value)
{
    m_staticComunicationSlider.setValue(value, juce::dontSendNotification);
}

void NewProjectAudioProcessorEditor::sharedValueChanged(float value)
{
    m_memMappedFileComunicationSlider.setValue(value, juce::dontSendNotification);
}
