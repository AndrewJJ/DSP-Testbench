/*
  ==============================================================================

    AudioDataTransfer.h
    Created: 20 Jan 2018 3:16:44pm
    Author:  Andrew

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <list>

/*  
	This file contains utility classes designed for data transfer applications involving real time audio.

	Asynchronous use cases:
	----------------------
	- AudioProcessor to GUI
		- AudioProcessor makes latest data available for GUI (AudioProbe)
			- GUI polls probe when it feels like
			- GUI is notified when there has been a change
		-	AudioProcessor streams all data to GUI (AudioProcessorStreamingProbe, not provided)
			-	Can't see where this would be needed except something like AutoTune's graph - normally you'd process everything you need
				in the AudioProcessor and let the GUI sample what it needs
			-	Would require a separate queue for each observer
		- Trivial cases
			- Atomic data types
	-	AudioProcessor to Processing Thread
		-	Not recommended for plugins (you can miss the cache and the host will be trying to spread lots of plugins across cores anyhow)
	
	Synchronous use cases:
	---------------------
	-	AudioProcessor streams all data to elsewhere in AudioProcessor at different block size (FixedBlockProcessor)
		-	Useful for things like FFTs which need to be processed with a fixed block size
	-	Trivial cases
		-	AudioProcessor makes latest data available for AudioProcessor (just use state variables)
		-	AudioProcessor streams all data to elsewhere in AudioProcessor at same block size (just copy data directly)

    Notes on asynchronous implementation:
	---------------------------------------------------
	- Single writer is assumed (i.e. in AudioProcessor)
	- Multiple observers/listeners assumed (need to be able to handle zero or more AudioProcessorEditors)
	- Don't need to copy data in (as write index doesn't need to be advanced until ready)
	- Need to copy out data in blocks (don't allow pointers or references as this may result in data tearing if accessing routines iterate slowly)
    - Each observer needs it's own pre-allocated buffer to copy the data into (observer creates this itself so memory allocation not triggered on realtime thread)

*/


/** These definitions improve readability for our lambda-based listener/callback implementation. */
using ListenerCallback = std::function<void()>;
using ListenerRemovalCallback = std::function<void()>;


/**
*   This class is designed to be used by a real time audio process which needs to safely and asynchronously transfer non-POD data to one or many 
*	observers. A series of objects of the template type are written to a lock-free queue for later reading by the observers. This pattern
*	is useful for communication between an AudioProcessor and it's AudioProcessorEditor(s) because we can't guarantee how many editors exist
*	at any point in time.
*
*   The class is best understood as a probe, i.e. observers can simply read the most recently written data frame whenever it
*   suits them. There is no guarantee that all written frames will be read by an observer, they simply sample whatever is
*   there at the time. Optionally, an observer can register as a listener so that it is notified whenever data has been written.
*
*   Data is written one frame at a time. It is also copied out one frame at a time by observers. The use of a copy method
*   for reading minimises the risk of data tearing. Tearing can only occur if the write method overwrites the read
*   position during the non-atomic copy operation. Hence the risk is a function of the queue length.
*  
*   The FrameType used for the data frame must be of fixed size at compile time. For example:
*  
*   struct SimpleDataFrame
*	{
*		alignas(16) float f[1024];
*	};
*/
template <class FrameType>
class AudioProbe
{
public:

    /* Constructor. Optionally specify the length of the queue. */
	explicit AudioProbe(const int queueLengthInFrames = 3	/**< Number of frames in queue. Higher values have lower risk of data tearing. */)
        : numFramesInQueue (queueLengthInFrames),
        writeIndex (1),
        readIndex (0),
        frameSize (sizeof (FrameType))
    {
        // Allocate memory for queue
		writeQueue.allocate (numFramesInQueue, false);
        // Intialise frame at read position
		writeQueue[readIndex] = FrameType ();
    }

    /* 
     * Naturally, the AudioProcessor shouldn't keep trying to write to this object once it has been destroyed! If it attempts to, then behaviour
     * is undefined. For audio plugins, we shouldn't need to worry about delayed reads from listeners arriving during destruction because the
     * AudioProcessorEditor should always be destroyed before it's parent AudioProcessor.
     */
    ~AudioProbe()
	{
	    masterReference.clear();
	}

    /** Writes a data frame to the queue and will thus overwrite anything altered using getWritePointer(). */
    void writeFrame (const FrameType* source)
    {
        jassert (writeIndex != readIndex);
        std::memcpy (&writeQueue[writeIndex], source, frameSize);
        finishedWrite();
    }

	/** Copies current data frame at read index into the destination.
	*	Direct access to the current data frame isn't allowed as any non atomic access risks data tearing. By using a lock-free queue
	*	and copying out in one operation, the risk of data tearing is extremely low (an assertion will throw if this does happen).
	*	Observers/listeners should pre-allocate a member variable of ElementType to copy into if performance is critical. */
    void copyFrame (FrameType* destination)
    {
        jassert (writeIndex != readIndex);
        std::memcpy (destination, &writeQueue[readIndex], frameSize);
    }

	/**
	 *  Allows a listener to add a lambda function as a callback. This function is called on the audio thread so it should not
	 *  perform any allocations, acquire any locks or do anything else which might cause blocking. So either use an atomic flag as
	 *  a semaphore or implement a lock-free function call queue for signalling back to other threads. The callback should also
	 *  make sure that it's owner still exists before it attempts to do anything (e.g. check a WeakReference to itself).
	 *
     *  Returns a ListenerRemovalCallback which allows the listener to de-register the callback that was just added. It's probably
     *  worth initialising any ListenerRemovalCallback class members used to hold this return value to an empty function.
     */
    ListenerRemovalCallback addListenerCallback (ListenerCallback &&listenerCallback)
    {
        WeakReference<AudioProbe<FrameType>> weakThis = this;
        auto thisListener = listenerCallbacks.emplace (listenerCallbacks.begin(), listenerCallback);
        return [this, weakThis, thisListener] ()
        {
            // Check the WeakReference because the callback may live longer than this AudioProbe
            if (weakThis)
                listenerCallbacks.erase (thisListener);
        };
    }

	/**
    *   Indicates whether the probe has any listeners. In the case where all observers are listeners, this can be used by the sender to
																																	  
    *   choose whether to suspend non-essential processing (e.g. don't bother computing an FFT for a GUI if there are no GUIs attached).
    *   This is of no use if the observers call copyFrame() of their own volition (e.g. during a timerCallback).
    */
    [[nodiscard]] inline bool hasListeners() const
	{
		return !listenerCallbacks.empty();
	}

private:

	/** This is called when the writer has completed writing a data frame. */
    void finishedWrite()
    {
        jassert (writeIndex != readIndex);
        readIndex = writeIndex;
        writeIndex++;
        if (writeIndex == numFramesInQueue)
            writeIndex = 0;
        
        // Perform registered callbacks
        for (auto &&callback : listenerCallbacks)
        {
            if (callback)
                callback();
        }
    }

    const int numFramesInQueue; // In units of frame size
    int writeIndex, readIndex;  // In units of frame size
    int frameSize;              // In bytes
    std::list<ListenerCallback> listenerCallbacks{};
	HeapBlock <FrameType> writeQueue;

    typename WeakReference<AudioProbe<FrameType>>::Master masterReference;
    friend class WeakReference<AudioProbe<FrameType>>;

public:
    // Declare non-copyable, non-movable
    AudioProbe (const AudioProbe&) = delete;
    AudioProbe& operator= (const AudioProbe&) = delete;
    AudioProbe (AudioProbe&& other) = delete;
    AudioProbe& operator=(AudioProbe&& other) = delete;
};

/**
*	This abstract class is used to provide the capability to stream data to elsewhere in a real time audio process at
*   a fixed block size which is independent of the block size of the source stream. Derivations need to implement the
*	performProcessing() method. It is assumed that processing will take place on the same thread as AudioProcessor::processBlock(),
*	there is no consideration given to thread safety.
*	 
*	Note that this class only operates on float based audio data.
*/
class FixedBlockProcessor
{
public:

    /** We set a maximum block size at construction but allow later modification to a smaller size if necessary. */
    explicit FixedBlockProcessor (const int maximumBlockSize)
        : maxBlockSize (maximumBlockSize),
          currentBlockSize (maximumBlockSize)
    { }

	virtual ~FixedBlockProcessor() = default;

    /** Prepare the FixedBlockProcessor. Note that this only sets the number of channels. The blocksize of spec is typically different
     *  to that used here, so call setFixedBlockSize() instead. */
    virtual void prepare (const dsp::ProcessSpec& spec)
    {
        jassert (spec.numChannels > 0);
        numChannels = static_cast<int> (spec.numChannels);
        resizeBuffer();
    }

	/** Gets the number of channels. */
    [[nodiscard]] int getNumChannels() const
    {
        return numChannels;
    }

	/** Sets the current block size used. Current size is initialised to maximum size. If you set a smaller size,
	 *  then performProcessing() will be called when the smaller, current block size is reached. If the size is reduced
	 *  while there is already data in the buffer, then that data will be cleared and will not be processed.
	 */
    void modifyCurrentBlockSize (const int size		/**< This block size must be <= to the maximum, larger values will be truncated to max. */)
    {
        jassert (size > 0);
        jassert (size <= maxBlockSize);
        currentBlockSize = jmin (size, maxBlockSize);
        resetFrame();
    }

	/** Gets the maximum block size. */
    [[nodiscard]] int getMaximumBlockSize() const
    {
        return maxBlockSize;
    }

    /** Gets the current block size. */
    [[nodiscard]] int getCurrentBlockSize() const
    {
        return currentBlockSize;
    }

    /** Resets the current write to the start of the frame (for each channel). */
    void resetFrame()
    {
        for (auto ch = 0 ; ch < numChannels; ++ch)
            currentIndex[ch] = 0;
    }

	/** Appends data to the buffer. */
	void appendData (const int channel,		/**< Channel to which data is going to be appended */
		             const int numSamples,	/**< Number of samples to be appended */
		             const float* data		/**< Pointer to source data */
                    )
    {
        jassert (numChannels > 0);  // If this assert fires then you probably haven't called prepare()
        jassert (currentBlockSize > 0);
        jassert (channel >= 0 && channel < numChannels);
        jassert (numSamples > 0);   // If this assert fires then you probably haven't called prepare()

        if (data != nullptr)
        {
            if (currentIndex[channel] + numSamples < currentBlockSize)
            {
                // Data can't fill the channel buffer, so write what we have
                buffer.copyFrom (channel, currentIndex[channel], data, numSamples);
                currentIndex[channel] += numSamples;
            }
            else
            {
                // We have more data than can fit in the channel buffer
                auto samplesRemaining = numSamples;
                auto dataOffset = 0;

                // Write remainder of channel buffer and process
                const auto rem = currentBlockSize - currentIndex[channel];
                buffer.copyFrom (channel, currentIndex[channel], data, rem);
                performProcessing (channel);
                currentIndex[channel] = 0;
                samplesRemaining -= rem;
                dataOffset += rem;

                // Loop full blocks (processing as we go)
                while (samplesRemaining >= currentBlockSize)
                {
                    buffer.copyFrom (channel, currentIndex[channel], data + dataOffset, currentBlockSize);
                    performProcessing (channel);
                    samplesRemaining -= currentBlockSize;
                    dataOffset += currentBlockSize;
                }

                if (samplesRemaining > 0)
                {
                    // Write remainder of input
                    buffer.copyFrom (channel, currentIndex[channel], data + dataOffset, samplesRemaining);
                    currentIndex[channel] = samplesRemaining;
                }
            }
        }
    }

	/** Abstract function which is called whenever the fixed size buffer has been filled. */
	virtual void performProcessing (const int channel	/**< Channel for which processing is to be performed */ ) = 0;
   
protected:

	/** Internal buffer */
	AudioSampleBuffer buffer;

	/** Used to hold current position in buffer (per channel) */
    HeapBlock<int> currentIndex { };

private:

	/** Resizes buffer - this always allocates sufficient memory to hold the maximum block size. */
    void resizeBuffer()
    {
        buffer.setSize (numChannels, maxBlockSize, false, true, true);
        currentIndex.allocate (numChannels, true);
    }

	int numChannels = 0;
    int maxBlockSize = 0;
	int currentBlockSize = 0;

public:
    // Declare non-copyable, non-movable
    FixedBlockProcessor (const FixedBlockProcessor&) = delete;
    FixedBlockProcessor& operator= (const FixedBlockProcessor&) = delete;
    FixedBlockProcessor (FixedBlockProcessor&& other) = delete;
    FixedBlockProcessor& operator=(FixedBlockProcessor&& other) = delete;
};