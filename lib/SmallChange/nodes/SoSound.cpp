#if HAVE_CONFIG_H
#include <config.h>
#endif

#if HAVE_OPENAL

#include <SmallChange/nodes/SoSound.h>

#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/sensors/SoFieldSensor.h>

#include <AL/altypes.h>
#include <AL/al.h>

#include <SmallChange/misc/ALTools.h>
#include <SmallChange/nodes/SoAudioClip.h>
#include <SmallChange/nodes/SoAudioClipP.h>
#include <SmallChange/nodes/SoAudioClipStreaming.h>
#include <SmallChange/nodes/SoAudioClipStreamingP.h>

#include <Inventor/sensors/SoTimerSensor.h>

#include <math.h>

#include "SmallChange/nodes/SoSoundP.h"

#undef THIS
#define THIS this->sosound_impl

#undef ITHIS
#define ITHIS this->ifacep


SO_NODE_SOURCE(SoSound);

void SoSound::initClass()
{
//  SO_NODE_INIT_CLASS(SoSound, SoNode, "Node");
  SO_NODE_INTERNAL_INIT_CLASS(SoSound);
};

SoSound::SoSound() 
{

  THIS = new SoSoundP(this);

  SO_NODE_CONSTRUCTOR(SoSound);

  SO_NODE_ADD_FIELD(direction, (0.0f, 0.0f, 1.0f));
  SO_NODE_ADD_FIELD(intensity, (1.0f));
  SO_NODE_ADD_FIELD(location, (0.0f, 0.0f, 0.0f));
  SO_NODE_ADD_FIELD(maxBack, (10.0f));
  SO_NODE_ADD_FIELD(maxFront, (10.0f));
  SO_NODE_ADD_FIELD(minBack, (1.0f));
  SO_NODE_ADD_FIELD(minFront, (1.0f));
  SO_NODE_ADD_FIELD(priority, (0.0f));
  SO_NODE_ADD_FIELD(source, (NULL));
  SO_NODE_ADD_FIELD(spatialize, (TRUE));

  THIS->currentAudioClip = NULL;

  THIS->timersensor = NULL;
  THIS->isStreaming = FALSE;
  THIS->asyncStreamingMode = FALSE;

	ALint	error;

  alGenSources(1, &(THIS->sourceId));
	if ((error = alGetError()) != AL_NO_ERROR)
  {
    char errstr[256];
		SoDebugError::postWarning("SoSound::SoSound",
                              "alGenSources failed. %s",
                              GetALErrorString(errstr, error));
    return;
  }


  // use field sensor for filename since we will load an image if
  // filename changes. This is a time-consuming task which should
  // not be done in notify().
  THIS->sourcesensor = new SoFieldSensor(THIS->sourceSensorCBWrapper, THIS);
  THIS->sourcesensor->setPriority(0);
  THIS->sourcesensor->attach(&this->source);

  THIS->workerThread = new SbAudioWorkerThread(THIS->threadCallbackWrapper, THIS, 10);
  THIS->audioBuffer = NULL;

#ifdef HAVE_PTHREAD
  pthread_mutex_init(&THIS->syncmutex, NULL);
#endif

};

SoSound::~SoSound()
{
#ifndef NDEBUG
  fprintf(stderr, "~SoSound()\n");
#endif
  delete THIS->sourcesensor;

  THIS->stopPlaying(TRUE);

  delete THIS->workerThread;

#ifdef HAVE_PTHREAD
  pthread_mutex_destroy(&THIS->syncmutex);
#endif

  if (THIS->audioBuffer != NULL)
    delete[] THIS->audioBuffer;

	ALint	error;

  alDeleteSources(1, &(THIS->sourceId));
	if ((error = alGetError()) != AL_NO_ERROR)
  {
    char errstr[256];
		SoDebugError::postWarning("SoSound::~SoSound",
                              "alDeleteSources() failed. %s",
                              GetALErrorString(errstr, error));
  }
  delete THIS;
};

int SoSoundP::threadCallbackWrapper(void *userdata)
{
  SoSoundP *thisp = (SoSoundP *)userdata;
  return thisp->threadCallback();
};

int SoSoundP::threadCallback()
{
  return this->fillBuffers();
};


SbBool SoSoundP::stopPlaying(SbBool force)
{
  ALint error;
  SbBool retval = TRUE;
  SoAudioClip *audioClip = NULL;
  audioClip = (SoAudioClip *)ITHIS->source.getValue();

  if (force || (audioClip->isActive.getValue()))
  { 
    // we were playing, so stop
    // printf("SoSound::stopPlaying stopping \n");
    // stop playing
    alSourceStop(this->sourceId);
	  if ((error = alGetError()) != AL_NO_ERROR)
    {
      char errstr[256];
		  SoDebugError::postWarning("SoSound::stopPlaying",
                                "alSourceStop failed. %s",
                                GetALErrorString(errstr, error));
      retval= FALSE;
    }

    audioClip->isActive.setValue(FALSE);
  }

  // stop timersensor
  if (this->timersensor) {
    if (this->timersensor->isScheduled()) 
      this->timersensor->unschedule();
    delete this->timersensor;
    this->timersensor = NULL;
  }

  if (workerThread->isActive())
    workerThread->stop();

  if (this->isStreaming)
  {
    SoAudioClipStreaming *audioClipStreaming = (SoAudioClipStreaming *)ITHIS->source.getValue();
    audioClipStreaming->soaudioclipstreaming_impl->stopPlaying();

  }

  return retval;
};

SbBool SoSoundP::startPlaying(SbBool force)
{
//  stopPlaying();

  ALint error;
  SoAudioClip *audioClip = (SoAudioClip *)ITHIS->source.getValue();

  if (force || (!audioClip->isActive.getValue()))
  {
   
    this->isStreaming = FALSE;
    if (ITHIS->source.getValue()->isOfType(SoAudioClipStreaming::getClassTypeId()))
    {
      SoAudioClipStreaming *audioClipStreaming = (SoAudioClipStreaming *)ITHIS->source.getValue();
      this->asyncStreamingMode = audioClipStreaming->getAsyncMode();
      this->isStreaming = TRUE;

      // create class-local buffers (used in fillBuffers)
      if (this->isStreaming)
      {
        if (this->audioBuffer == NULL)
        {
          SoAudioClipStreaming *audioClipStreaming = (SoAudioClipStreaming *)this->currentAudioClip;
          this->audioBuffer = new short int[audioClipStreaming->getBufferSize() * audioClipStreaming->getNumChannels()];
        };
      };

      // create AL buffers
      audioClipStreaming->soaudioclipstreaming_impl->startPlaying();

    	  // Queue the buffers on the source
	    alSourceQueueBuffers(this->sourceId, audioClipStreaming->getNumBuffers(), audioClipStreaming->soaudioclipstreaming_impl->alBuffers);
	    if ((error = alGetError()) != AL_NO_ERROR)
      {
        char errstr[256];
		    SoDebugError::postWarning("SoSoundP::startPlaying",
                                  "alSourceQueueBuffers failed. %s",
                                  GetALErrorString(errstr, error));
        return FALSE;
      }

	    alSourcei(this->sourceId,AL_LOOPING, FALSE);
	    if ((error = alGetError()) != AL_NO_ERROR)
      {
        char errstr[256];
		    SoDebugError::postWarning("SoSoundP::sourceSensorCB",
                                  "alSourcei(,AL_LOOPING,) failed. %s",
                                  GetALErrorString(errstr, error));
        return FALSE;
      }

      // 20010803 thh moved alSourcePlay here 

      if (!this->asyncStreamingMode)
      {
        // stop previous timer
        if (this->timersensor) {
          if (this->timersensor->isScheduled()) 
            this->timersensor->unschedule();
          delete this->timersensor;
          this->timersensor = NULL;
        }
        // start new timer
        this->timersensor = new SoTimerSensor(timercb, this);
        this->timersensor->setInterval(SbTime(0.05));
        this->timersensor->schedule();
      }
      else
      {
        // stop existing thread, start new thread
        if (workerThread->isActive())
          workerThread->stop();

        workerThread->start();
      }

    };

    // 20010809 thh moved here
    // we weren't playing, so start playing
    alSourcePlay(this->sourceId);
	  if ((error = alGetError()) != AL_NO_ERROR)
    {
      char errstr[256];
		  SoDebugError::postWarning("SoSoundP::StartPlaying",
                                "alSourcePlay failed. %s",
                                GetALErrorString(errstr, error));
      return FALSE;
    }

  audioClip->isActive.setValue(TRUE);

/*
  moved up 20010803 ThH
    // we weren't playing, so start playing
    alSourcePlay(this->sourceId);
	  if ((error = alGetError()) != AL_NO_ERROR)
    {
      char errstr[256];
		  SoDebugError::postWarning("SoSound::audioRender",
                                "alSourcePlay failed. %s",
                                GetALErrorString(errstr, error));
      return FALSE;
    }
*/
  }
  return TRUE;
};

int SoSoundP::fillBuffers()
{
  SbAutoLock autoLock(&(this->syncmutex)); // synchronize with main thread

	ALint			processed;

  //  SoAudioClipStreaming *audioClipStreaming = (SoAudioClipStreaming *)this->source.getValue();
  SoAudioClipStreaming *audioClipStreaming = (SoAudioClipStreaming *)this->currentAudioClip;
	// Get status
#ifdef _WIN32
  alGetSourcei(this->sourceId, AL_BUFFERS_PROCESSED, &processed);
#else
  alGetSourceiv(this->sourceId, AL_BUFFERS_PROCESSED, &processed);
#endif

  // for debugging
	ALint			queued;
#ifdef _WIN32
	alGetSourcei(this->sourceId, AL_BUFFERS_QUEUED, &queued);
#else
	alGetSourceiv(this->sourceId, AL_BUFFERS_QUEUED, &queued);
#endif
//  printf("Processed: %d, Queued: %d\n", processed, queued);


	ALboolean bFinishedPlaying = AL_FALSE;
	ALuint			BufferID;
	ALint			error;

	if (processed > 0)
	{

		while (processed)
		{

//      printf(">");
			alSourceUnqueueBuffers(this->sourceId, 1, &BufferID);
	    if ((error = alGetError()) != AL_NO_ERROR)
      {
        char errstr[256];
		    SoDebugError::postWarning("SoSound::audioRender",
                                  "alSourceUnqueueBuffers failed. %s",
                                  GetALErrorString(errstr, error));
        return 0;
      }

      // fill buffer
      audioClipStreaming->soaudioclipstreaming_impl->fillBuffer(this->audioBuffer, audioClipStreaming->getBufferSize());


      // send buffer to OpenAL
      int channels;
      int samplerate;
      int bitspersample;
      audioClipStreaming->getSampleFormat(channels, samplerate, bitspersample);

	    ALenum	alformat = 0;;
      alformat = getALSampleFormat(channels, bitspersample);

			alBufferData(BufferID, alformat, this->audioBuffer, 
        audioClipStreaming->getBufferSize() * sizeof(short int) * audioClipStreaming->getNumChannels(), 
        samplerate);
	    if ((error = alGetError()) != AL_NO_ERROR)
      {
        char errstr[256];
		    SoDebugError::postWarning("SoSound::audioRender",
                                  "alBufferData failed. %s",
                                  GetALErrorString(errstr, error));
        return 0;
      }

			// Queue buffer
			alSourceQueueBuffers(this->sourceId, 1, &BufferID);
	    if ((error = alGetError()) != AL_NO_ERROR)
      {
        char errstr[256];
		    SoDebugError::postWarning("SoSound::audioRender",
                                  "alSourceQueueBuffers failed. %s",
                                  GetALErrorString(errstr, error));
        return 0;
      }

			processed--;
		}
    // delete buffer;
  }

  // check to see if we're still playing
  // if not, make sure to start over again
  // if we're not playing, it's because the buffers have not been filled up (unqueued, filled, queued)
  // fast enough, so the source has plaued the last buffer in the queue and changed state from playing to stopped.

	ALint			state;
#ifdef _WIN32
	alGetSourcei(this->sourceId, AL_SOURCE_STATE, &state);
#else
	alGetSourceiv(this->sourceId, AL_SOURCE_STATE, &state);
#endif
  if (state != AL_PLAYING)
  {
    if (state == AL_STOPPED)
    {
      alSourcePlay(this->sourceId);
	    if ((error = alGetError()) != AL_NO_ERROR)
      {
        char errstr[256];
		    SoDebugError::postWarning("SoSoundP::StartPlaying",
                                  "alSourcePlay failed. %s",
                                  GetALErrorString(errstr, error));
        return FALSE;
      }
#ifndef NDEBUG
      fprintf(stderr, "state == AL_STOPPED. Had to restart source\n");
#endif
    }
    else
    {
      char statestr[20];
      switch (state) {
      case AL_INITIAL : sprintf(statestr, "initial"); break;
      case AL_PLAYING : sprintf(statestr, "playing"); break;
      case AL_PAUSED : sprintf(statestr, "paused"); break;
      case AL_STOPPED : sprintf(statestr, "stopped"); break;
      default : sprintf(statestr, "unknown"); break;
      };
#ifndef NDEBUG
      fprintf(stderr, "state == %s. Don't know what to do about it...\n", statestr);
#endif
    }
  };


  return 1;
};

void
SoSoundP::timercb(void * data, SoSensor * s)
{
  SoSoundP * thisp = (SoSoundP*) data;
  thisp->fillBuffers();
}


void SoSound::audioRender(SoAudioRenderAction *action)
{
  ALint error;
  // FIXME: allow for other types of source nodes (like MovieTexture)

  // Get SoAudioClip::startTime and see if it's time to start
  if (!this->source.getValue())
    return;

  SoAudioClip *audioClip = NULL;
  
  
  audioClip = (SoAudioClip *)this->source.getValue();

  SbTime now = SbTime::getTimeOfDay();
  SbTime start = audioClip->startTime.getValue();
  SbTime stop = audioClip->stopTime.getValue();
  
  SbString start_str = start.format("%D %h %m %s");
  SbString stop_str = stop.format("%D %h %m %s");
  SbString now_str = now.format("%D %h %m %s");

//  if ((now<start) && (!audioClip->isActive))
  if ( (now<start) || (now>=stop) )
  {
    // we shouldn't be playing now
    THIS->stopPlaying();
    return; 
  }

  // If we got this far, then  ( start <= now < stop ) and we should be playing
// moved down  THIS->startPlaying(); // if we're not playing, playing will start

  // audio source is now playing, update OpenAL parameters
 
  SbVec3f pos, worldpos;
  ALfloat alfloat3[3];

  pos = this->location.getValue();
  SoModelMatrixElement::get(action->getState()).multVecMatrix(pos, worldpos); 

  float x, y, z;
  worldpos.getValue(x, y, z);

//  printf("(%0.2f, %0.2f, %0.2f)-----------------------\n", x, y, z);

  SbVec3f2ALfloat3(alfloat3, worldpos);

	// Position ...
	alSourcefv(THIS->sourceId, AL_POSITION, alfloat3);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
    char errstr[256];
		SoDebugError::postWarning("SoSound::audioRender",
                              "alSourcefv(,AL_POSITION,) failed. %s",
                              GetALErrorString(errstr, error));
		return;
	}

	// Velocity ...
/*  SbVec3f2ALfloat3(alfloat3, velocity.getValue());

	alSourcefv(this->sourceId, AL_VELOCITY, alfloat3);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
    char errstr[256];
		SoDebugError::postWarning("SoSound::GLRender",
                              "alSourcefv(,AL_VELOCITY,) failed. %s",
                              GetALErrorString(errstr, error));
		return;
	}
*/

  // Gain / intensity
	alSourcef(THIS->sourceId,AL_GAIN, this->intensity.getValue());
	if ((error = alGetError()) != AL_NO_ERROR)
  {
    char errstr[256];
		SoDebugError::postWarning("SoSound::audioRender",
                              "alSourcef(,AL_GAIN,) failed. %s",
                              GetALErrorString(errstr, error));
    return;
  }

  THIS->startPlaying(); // if we're not playing, playing will start


  // FIXME: check if anything has changed in the audioClip (like buffer, loop-points, etc, and react accordingly)
}

//
// called when source changes
//
void
SoSoundP::sourceSensorCBWrapper(void * data, SoSensor *)
{
  SoSoundP * thisp = (SoSoundP*) data;
  thisp->sourceSensorCB(NULL);
}
//
// called when source changes
//
void
SoSoundP::sourceSensorCB(SoSensor *)
{

  SbAutoLock autoLock(&(this->syncmutex)); // synchronize with fill-thread
//  printf("SoSound::sourceSensorCB()\n");
  ALint error;

  if (!ITHIS->source.getValue())
    return;

  if ( !ITHIS->source.getValue()->isOfType(SoAudioClipStreaming::getClassTypeId()) 
    && !ITHIS->source.getValue()->isOfType(SoAudioClip::getClassTypeId()) )
  {
		SoDebugError::postWarning("SoSound::sourceSensorCB",
                              "Unknown source node type");
    return;
  };

  SoAudioClip *audioClip = (SoAudioClip *)ITHIS->source.getValue();

  if (audioClip == this->currentAudioClip)
    return; 
    // for some obscure reason, the sensor was called, even though the field hasn't changed .....
    // FIXME: ask mortene about this --^

  this->currentAudioClip = audioClip;

  if (this->isStreaming)
  { 
    if (this->audioBuffer != NULL)
      delete[] this->audioBuffer;
    SoAudioClipStreaming *audioClipStreaming = (SoAudioClipStreaming *)this->currentAudioClip;
    this->audioBuffer = new short int[audioClipStreaming->getBufferSize()  * audioClipStreaming->getNumChannels()];
  };


  alSourceStop(this->sourceId);
	if ((error = alGetError()) != AL_NO_ERROR)
  {
    char errstr[256];
		SoDebugError::postWarning("SoSound::sourceSensorCB",
                              "alSourceStop failed. %s",
                              GetALErrorString(errstr, error));
  }

	alSourcef(this->sourceId,AL_PITCH, audioClip->pitch.getValue());
	if ((error = alGetError()) != AL_NO_ERROR)
  {
    char errstr[256];
		SoDebugError::postWarning("SoSoundP::sourceSensorCB",
                              "alSourcef(,AL_PITCH,) failed. %s",
                              GetALErrorString(errstr, error));
    return;
  }

	alSourcef(this->sourceId,AL_GAIN, ITHIS->intensity.getValue());
	if ((error = alGetError()) != AL_NO_ERROR)
  {
    char errstr[256];
		SoDebugError::postWarning("SoSoundP::sourceSensorCB",
                              "alSourcef(,AL_GAIN,) failed. %s",
                              GetALErrorString(errstr, error));
    return;
  }

  if (ITHIS->source.getValue()->isOfType(SoAudioClipStreaming::getClassTypeId()))
  {
    // if we're streaming, the buffers will be filled in fillBuffer()
  }
  else
  {
    // when we're not streaming, fill buffer once
	  alSourcei(this->sourceId, AL_BUFFER, audioClip->soaudioclip_impl->bufferId);
	  if ((error = alGetError()) != AL_NO_ERROR)
    {
      char errstr[256];
		  SoDebugError::postWarning("SoSoundP::sourceSensorCB",
                                "alSourcei(,AL_BUFFER,) failed. %s",
                                GetALErrorString(errstr, error));
      return;
    }
  };

	alSourcei(this->sourceId,AL_LOOPING, audioClip->loop.getValue());
	if ((error = alGetError()) != AL_NO_ERROR)
  {
    char errstr[256];
		SoDebugError::postWarning("SoSoundP::sourceSensorCB",
                              "alSourcei(,AL_LOOPING,) failed. %s",
                              GetALErrorString(errstr, error));
    return;
  }

}

// void SoSound::GLRender(SoGLRenderAction *action)




      // fixme: if we run out of buffers, processed
      // will allways be 0
      // should probably check queued also / instead

      // 20010809 ThH - update: (from simple2.cpp
//  buffernode->loop.setValue(TRUE); // this fixes silence-bug for the case when 
  // all bufers have been played without new ones being filled
      // Hmm.. no, this doesn't work after all. When looping is on, one cannot unqueue buffers 
      // (and processed will allways be 0)

      // fill buffer
      // as an experiment, moved outside unqueue/queue
//      audioClipStreaming->soaudioclipstreaming_impl->fillBuffer(audioBuffer, audioClipStreaming->getBufferSize());

#endif // HAVE_OPENAL
