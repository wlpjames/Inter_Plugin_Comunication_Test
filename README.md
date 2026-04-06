# Inter_Plugin_Comunication_Test

A short demo of two methods of inter-plugin communication with juce, tested on Mac in Logic, Ableton and the AudioPluginHost

1: Reference counted singleton,

Much like the juce::SharedRecourcePointer a class is implemented that can share data between plugin instances within the same process, using shared pointers and a static std::weak_ptr. The data is deleted when there are no more plugin instances holding a shared pointer.

2: juce::MemoryMappedFile

Plugin instances can reference the same memory mapped file and poll for changes on a background thread - this has the benefit of working across processes (and across plugin formats) but may be subject to restriction in the OS, like sandboxed file systems. Depending on the rate of polling the file, it may also have more latency than the first method.
