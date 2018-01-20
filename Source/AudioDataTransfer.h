/*
  ==============================================================================

    AudioDataTransfer.h
    Created: 20 Jan 2018 3:16:44pm
    Author:  Andrew

  ==============================================================================
*/

#pragma once

/*  
	This file contains utility classes designed for data transfer applications involving a juce::AudioProcessor. As you'll see from the
	use cases described below, this involves asynchronous transfer between an AudioProcessor and an AudioProcessorEditor and synchronous
	transfer within an AudioProcessor. This file does not contain classes relating to data transfer for AudioProcessorParameters.

	Asynchronous use cases:
	----------------------
	- AudioProcessor to GUI
		- AudioProcessor makes latest data available for GUI (AudioProcessorProbe)
			- GUI polls probe when it feels like
			- GUI is notified when there has been a change
		-	AudioProcessor streams all data to GUI (AudioProcessorStreamingProbe, not provided)
			-	Can't see where this would be needed except something like AutoTune's graph - normally you'd process everything you need
				in the AudioProcessor and let the GUI sample what it needs
			-	Would require a separate queue for each observer
		- Trivial cases
			- Atomic data types
	-	AudioProcessor to Processing Thread (AudioProcessorParallelStream, not provided)
		-	Not recommended for plugins (you can miss the cache and the host will be trying to spread lots of plugins across cores anyhow)
	
	Synchronous use cases:
	---------------------
	-	AudioProcessor streams all data to elsewhere in AudioProcessor at different block size (FixedBlockProcessor)
		-	Useful for things like FFTs which need to be processed with a fixed block size
		-	Maybe template <typename FloatType> FixedBlockProcessor?
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

// TODO - check that addListener() & removeListener() never cause the sender to glitch
/**
*   This class is designed to be used by an AudioProcessor which needs to safely and asynchronously transfer non-POD data to one or many 
*	observers. A series of objects of the templated type are written to a lock-free queue for later reading by the observers. This pattern
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
*   The ElementType used for the dataframe must be of fixed size at compile time. For example:
*  
*   struct SimpleDataFrame
*	{
*		alignas(16) float f[1024];
*	};
*/
template <class ElementType>
class AudioProcessorProbe
{
public:

	/** AudioProcessorProbe::Listener class - inherit from this in order to be able to register as a listener. Objects
    *   inheriting from this should add themselves as a listener by calling addListener() on the parent AudioProcessorProbe<> object.
	*	Similarly, the listener should remove itself by calling removeListener() on the parent (typically done during destruction). 
	*/
	class Listener
	{
	public:
		/* Destructor. */
		virtual ~Listener ()
		{
		}

		/** Callback for whenever an AudioProcessorProbe is updated. This callback should never attempt to remove a listener
		*	(bailout checking not implemented). */
		virtual void asyncProbeUpdated (AudioProcessorProbe *const asyncProbeThatUpdated) = 0;
	};

    /* Constructor. Optionally specify the length of the queue. */
	explicit AudioProcessorProbe(const int queueLengthInFrames = 3	/**< Number of frames in queue. Higher values have lower risk of data tearing. */)
        : numFramesInQueue (queueLengthInFrames),
        writeIndex (1),
        readIndex (0),
        frameSize (sizeof (ElementType))
    {
        // Allocate memory for queue
		writeQueue.allocate (numFramesInQueue, false);
        // Intialise frame at read position
		writeQueue[readIndex] = ElementType ();
    }

    /* Destructor. */
    ~AudioProcessorProbe()
	{
		// Naturally, the AudioProcessor shouldn't keep trying to write to this object once it has been destroyed! If it attempts to, then behaviour
		// is undefined.
			
		// We shouldn't need to worry about delayed reads from listeners arriving during destruction because the AudioProcessorEditor should
		// always be destroyed before it's parent AudioProcessor
	}

    /** Writes a data frame to the queue and will thus overwrite anything altered using getWritePointer().
    *   There is no need to call finishedWrite() after this. */
    void writeFrame (ElementType* source)
    {
        jassert (writeIndex != readIndex);
        std::memcpy (&writeQueue[writeIndex], source, frameSize);
        finishedWrite();
    }

    /** Access pointer to write a data frame. Must be followed by a call to finishedWrite() when done. */
	ElementType* getWritePointer()
    {
        jassert (writeIndex != readIndex);
        return &writeQueue[writeIndex];
    }

	/** This should be called when the writer has completed writing a data frame. It is only needed if getWritePointer() has been used to write
	*	data frame. */
    void finishedWrite()
    {
        jassert (writeIndex != readIndex);
        readIndex = writeIndex;
        writeIndex++;
        if (writeIndex == numFramesInQueue)
            writeIndex = 0;
		if (hasListeners())
		{
			// Post callback on message thread to notify listeners
			auto notifyListeners = [this]() { this->notifyAllListeners(); };
			juce::MessageManager::callAsync(notifyListeners);
		}
    }

	/** Copies current data frame at read index into the destination.
	*	Direct access to the current data frame isn't allowed as any non atomic access risks data tearing. By using a lock-free queue
	*	and copying out in one operation, the risk of data tearing is extremely low (an assertion will throw if this does happen).
	*	Observers/listeners should pre-allocate a member variable of ElementType to copy into if performance is critical.
	*/
    void copyFrame (ElementType* destination)
    {
        jassert (writeIndex != readIndex);
        std::memcpy (destination, &writeQueue[readIndex], frameSize);
    }

	/*	Indicates whether the probe has any listeners.
	*	In the case where all observers are listeners, this can be used by the sender to choose whether to suspend non-essential processing
	*	(e.g. don't bother computing an FFT for a GUI if there are no GUIs attached). This is of no use if the observers call copyFrame() 
	*	of their own volition (e.g. during a timerCallback).
	*/
	inline bool hasListeners () const
	{
		return listeners.size() > 0;
	}

	/** Adds a listener to the list. A listener can only be added once, so if the listener is already in the list,
	*	this method has no effect.
	*/
	void addListener(Listener *const listener			/*<< Listener to add. */)
	{
		listeners.add(listener);
	}

	/** Removes a listener from the list. If the listener wasn't in the list, this has no effect.
	*	If this call is causing errors when called from a destructor then you probably haven't checked to make sure your
	*	AudioProcessorProbe* is not already a nullptr.
	*/
	void removeListener(Listener *const listener		/*<< Listener to remove. */)
	{
		listeners.remove(listener);
	}

private:

	/** Iterates through all listeners, calling AsyncProbeUpdated(). */
	void notifyAllListeners ()
	{
		listeners.call (&Listener::asyncProbeUpdated, this);
	}

    const int numFramesInQueue; // In units of frame size
    int writeIndex, readIndex;  // In units of frame size
    int frameSize;              // In bytes
	ListenerList <Listener> listeners;
	HeapBlock <ElementType> writeQueue;
};


/**
*	This abstract class is used to provide the capability to stream data to elsewhere in an AudioProcessor at a fixed block size which
*	is independent of the block size of the source stream. Derivations need to implement the performProcessing() method. It is assumed that
*	processing will take place on the same thread as AudioProcessor::processBlock(), there is no consideration given to thread safety.
*	Note that this class only operates on float based audio data.
*/
class FixedBlockProcessor
{
public:

	virtual ~FixedBlockProcessor() = default;

    /** Prepare the FixedBlockProcessor. Note that this only sets the number of channels. The blocksize of spec is typically different
     *  to that used here, so call setFixedBlockSize() instead. */
    virtual void prepare (const dsp::ProcessSpec& spec)
    {
        jassert (spec.numChannels > 0);
        numChannels = spec.numChannels;
        resizeBuffer();
    }

	/** Gets the number of channels. */
    int getNumChannels() const
    {
        return numChannels;
    }

	/** Sets the block size used - may be independent of the stream being copied. */
    void setFixedBlockSize (const int size		/**< Block size */)
    {
        jassert (size > 0);
        blockSize = size;
        resizeBuffer();
    }

	/** Gets the block size. */
    int getFixedBlockSize() const
    {
        return blockSize;
    }

	/** Appends data to the buffer. */
	void appendData (const int channel,		/**< Channel to which data is going to be appended */
		             const int numSamples,	/**< Number of samples to be appended */
		             const float* data		/**< Pointer to source data */
                    )
    {
        jassert (numChannels > 0);
        jassert (blockSize > 0);
        jassert (channel >= 0 && channel < numChannels);
        jassert (numSamples > 0);


        if (data != nullptr)
        {
            if (currentIndex[channel] + numSamples < blockSize)
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
                const auto rem = blockSize - currentIndex[channel];
                buffer.copyFrom (channel, currentIndex[channel], data, rem);
                performProcessing (channel);
                currentIndex[channel] = 0;
                samplesRemaining -= rem;
                dataOffset += rem;

                // Loop full blocks (processing as we go)
                while (samplesRemaining >= blockSize)
                {
                    buffer.copyFrom (channel, currentIndex[channel], data + dataOffset, blockSize);
                    performProcessing (channel);
                    samplesRemaining -= blockSize;
                    dataOffset += blockSize;
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
    HeapBlock<int> currentIndex;

private:

	/** Resizes buffer. */
    void resizeBuffer()
    {
        buffer.setSize (numChannels, blockSize, false, true);
        currentIndex.allocate (numChannels, true);
    }

	int numChannels = 0;
	int blockSize = 0;
};