/*
  ==============================================================================

    InterPluginManager.h
    Created: 5 Apr 2026 8:02:14pm
    Author:  Billy James

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

///The singleton needs to be thread safe - and reference counted to that deletion can work
/// an alternative to this can be the juce::SharedRecourcePointer
class StaticPluginComunicationManager
{
public:
    
    class Listener
    {
        public:
        virtual ~Listener() {};
        virtual void interPluginValueChanged(float value)=0;
    };
    
    void setValue(float newValue)
    {
        m_value = newValue;
        m_listenerList.call([&] (auto& l)
        {
            l.interPluginValueChanged(m_value);
        });
    }
    
    void addListener(Listener* newListener)
    {
        m_listenerList.add(newListener);
        newListener->interPluginValueChanged(m_value);
    }
    
    void removeListener(Listener* listenerToRemove)
    {
        m_listenerList.remove(listenerToRemove);
    }
    
    // Each plugin calls this to get (or create) the shared instance and store it
    // somewhere
    static std::shared_ptr<StaticPluginComunicationManager> getShared()
    {
        static juce::SpinLock instanceLock;
        juce::SpinLock::ScopedLockType lock(instanceLock);

        // Try to promote the weak reference to a shared one
        if (auto existing = s_weakInstance.lock())
            return existing;

        // Create a new instance
        auto newInstance = std::make_shared<StaticPluginComunicationManager>();
        s_weakInstance = newInstance;
        return newInstance;
    }
    
private:
    juce::ListenerList<Listener> m_listenerList;
    
    //a fill in for whatever data will be shared
    float m_value;
    
    // Weak — does NOT keep the manager alive
    inline static std::weak_ptr<StaticPluginComunicationManager> s_weakInstance;
};



//==============================================================================
class MMFComunicaionManager
{
public:
    
    ///memory mapped file communication manager
    struct SharedData
    {
        static constexpr uint32_t MAGIC = 0xABCD1234;
        static constexpr size_t   SIZE  = 4096; // map a full page, plenty of headroom

        uint32_t                 magic         { 0 };
        std::atomic<uint32_t>    changeCounter { 0 };
        std::atomic<uint32_t>    instanceCount { 0 }; // track living instances
        
        //the actual data being managed (in this case only a float value)
        std::atomic<float> value { 0.0f };
    };

    MMFComunicaionManager()
    {
        auto file = getSharedFile();

        //ensure the file has a minimum saze for the data
        if (!file.existsAsFile() || file.getSize() < (juce::int64) SharedData::SIZE)
        {
            juce::FileOutputStream stream (file);
            
            if (stream.openedOk())
            {
                stream.setPosition (SharedData::SIZE - 1);
                stream.writeByte (0); // write one byte at the end
                                      // OS fills the gap with zeros,
                                      // giving us a file of exactly SIZE bytes
            }
        }

        m_mappedFile = std::make_unique<juce::MemoryMappedFile> (
            file,
            juce::MemoryMappedFile::readWrite,
            false
        );

        jassert (m_mappedFile->getData() != nullptr);
        jassert (m_mappedFile->getSize() >= SharedData::SIZE);

        m_data = static_cast<SharedData*> (m_mappedFile->getData());

        if (m_data->magic != SharedData::MAGIC)
        {
            // Fresh or corrupted file — initialise from scratch
            new (m_data) SharedData();
            m_data->magic = SharedData::MAGIC;
        }

        m_data->instanceCount.fetch_add (1);

        startPollingThread();
    }

    ~MMFComunicaionManager()
    {
        stopPollingThread();

        if (m_data != nullptr)
        {
            auto remaining = m_data->instanceCount.fetch_sub (1);

            if (remaining == 1) // we were the last one
            {
                m_data->~SharedData();
                m_mappedFile.reset();
                getSharedFile().deleteFile();
                return;
            }
        }

        m_mappedFile.reset();
    }

    //==========================================================================
    void setValue (float newValue)
    {
        if (m_data == nullptr) return;

        m_data->value.store (newValue);
        m_data->changeCounter.fetch_add (1);
    }

    float getValue() const
    {
        if (m_data == nullptr) return 0.0f;
        return m_data->value.load();
    }

    //==========================================================================
    class Listener
    {
    public:
        virtual ~Listener() = default;
        virtual void sharedValueChanged (float newValue) = 0;
    };

    void addListener (Listener* l)
    {
        m_listeners.add (l);
        l->sharedValueChanged (getValue()); // sync immediately on add
    }

    void removeListener (Listener* l) { m_listeners.remove (l); }

private:
    //==========================================================================
    static juce::File getSharedFile()
    {
        return juce::File::getSpecialLocation (juce::File::tempDirectory)
                          .getChildFile ("my_plugin_shared.dat");
    }

    //==========================================================================
    void startPollingThread()
    {
        m_pollThread = std::thread ([this]
        {
            uint32_t lastCounter = 0;

            while (!m_shouldStop.load())
            {
                if (m_data != nullptr)
                {
                    auto currentCounter = m_data->changeCounter.load();

                    if (currentCounter != lastCounter)
                    {
                        lastCounter = currentCounter;
                        auto value  = m_data->value.load();

                        juce::MessageManager::callAsync ([this, value]
                        {
                            m_listeners.call ([&] (auto& l)
                            {
                                l.sharedValueChanged (value);
                            });
                        });
                    }
                }

                std::this_thread::sleep_for (std::chrono::milliseconds (16));
            }
        });
    }

    void stopPollingThread()
    {
        m_shouldStop.store (true);
        if (m_pollThread.joinable())
            m_pollThread.join();
    }

    //==========================================================================
    std::unique_ptr<juce::MemoryMappedFile> m_mappedFile;
    SharedData*                             m_data { nullptr };

    juce::ListenerList<Listener>            m_listeners;
    std::thread                             m_pollThread;
    std::atomic<bool>                       m_shouldStop { false };
};
