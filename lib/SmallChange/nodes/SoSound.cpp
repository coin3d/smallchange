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

#ifdef HAVE_PTHREAD
pthread_mutex_t SoSoundP::syncmutex = NULL;
#endif // HAVE_PTHREAD

void SoSound::initClass()
{
  SO_NODE_INTERNAL_INIT_CLASS(SoSound);
  pthread_mutex_init(&SoSoundP::syncmutex, NULL);
  // fixme 20011207 thammer, what about destroying the mutex ?
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

  ALint  error;

  alGenSources(1, &(THIS->sourceId));
  if ((error = alGetError()) != AL_NO_ERROR) {
    char errstr[256];
    SoDebugError::postWarning("SoSound::SoSound",
                              "alGenSources failed. %s",
                              GetALErrorString(errstr, error));
    return;
  }

  THIS->sourcesensor = new SoFieldSensor(THIS->sourceSensorCBWrapper, THIS);
  THIS->sourcesensor->setPriority(0);
  THIS->sourcesensor->attach(&this->source);

  THIS->workerThread = new SbAudioWorkerThread(THIS->threadCallbackWrapper, THIS, 10);
  THIS->audioBuffer = NULL;

#ifdef HAVE_PTHREAD
#if 0
  // 20011207 thammer, kept for futute debugging
  pthread_mutex_init(&SoSoundP::syncmutex, NULL);
#endif
#endif

};

SoSound::~SoSound()
{
#ifdef DEBUG_AUDIO
  fprintf(stderr, "~SoSound()\n");
#endif
  delete THIS->sourcesensor;

  THIS->stopPlaying(TRUE);

  delete THIS->workerThread;

#ifdef HAVE_PTHREAD
#if 0
  // 20011207 thammer, kept for future debugging
  pthread_mutex_destroy(&SoSoundP::syncmutex);
#endif
#endif

  if (THIS->audioBuffer != NULL)
    delete[] THIS->audioBuffer;

  ALint  error;

  alDeleteSources(1, &(THIS->sourceId));
  if ((error = alGetError()) != AL_NO_ERROR) {
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

  if (force || (audioClip->isActive.getValue())) { 
    // we were playing, so stop
    // printf("SoSound::stopPlaying stopping \n");
    // stop playing
    alSourceStop(this->sourceId);
    if ((error = alGetError()) != AL_NO_ERROR) {
      char errstr[256];
      SoDebugError::postWarning("SoSound::stopPlaying",
                                "alSourceStop failed. %s",
                                GetALErrorString(errstr, error));
      retval= FALSE;
    }

    // fixme debugging
    // unqueue buffer
    if (!this->isStreaming) {
      alSourceUnqueueBuffers(this->sourceId, 1, &audioClip->soaudioclip_impl->bufferId);
      if ((error = alGetError()) != AL_NO_ERROR) {
        char errstr[256];
        SoDebugError::postWarning("SoSound::stopPlaying",
                                  "alSourceUnqueueBuffers failed. %s",
                                  GetALErrorString(errstr, error));
        return 0;
      }
      else
        printf("Buffer unqueued.\n");

      alSourcei(this->sourceId, AL_BUFFER, 0);
      if ((error = alGetError()) != AL_NO_ERROR) {
        char errstr[256];
        SoDebugError::postWarning("SoSoundP::stopPlaying",
                                  "alSourcei(,AL_BUFFER, 0) failed. %s",
                                  GetALErrorString(errstr, error));
        return FALSE;
      }
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

  if (this->isStreaming) {
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

  if (force || (!audioClip->isActive.getValue())) {
   
    this->isStreaming = FALSE;
    if (ITHIS->source.getValue()->isOfType(SoAudioClipStreaming::getClassTypeId())) {
      SoAudioClipStreaming *audioClipStreaming = (SoAudioClipStreaming *)ITHIS->source.getValue();
      this->asyncStreamingMode = audioClipStreaming->getAsyncMode();
      this->isStreaming = TRUE;

      // create class-local buffers (used in fillBuffers)
      if (this->isStreaming) {
        if (this->audioBuffer == NULL) {
          SoAudioClipStreaming *audioClipStreaming = (SoAudioClipStreaming *)this->currentAudioClip;
          this->audioBuffer = new short int[audioClipStreaming->getBufferSize() * audioClipStreaming->getNumChannels()];
        };
      };

      // create AL buffers
      audioClipStreaming->soaudioclipstreaming_impl->startPlaying();

        // Queue the buffers on the source
      alSourceQueueBuffers(this->sourceId, audioClipStreaming->getNumBuffers(), audioClipStreaming->soaudioclipstreaming_impl->alBuffers);
      if ((error = alGetError()) != AL_NO_ERROR) {
        char errstr[256];
        SoDebugError::postWarning("SoSoundP::startPlaying",
                                  "alSourceQueueBuffers failed. %s",
                                  GetALErrorString(errstr, error));
        return FALSE;
      }

      alSourcei(this->sourceId,AL_LOOPING, FALSE);
      if ((error = alGetError()) != AL_NO_ERROR) {
        char errstr[256];
        SoDebugError::postWarning("SoSoundP::sourceSensorCB",
                                  "alSourcei(,AL_LOOPING,) failed. %s",
                                  GetALErrorString(errstr, error));
        return FALSE;
      }

      // 20010803 thh moved alSourcePlay here 

      if (!this->asyncStreamingMode) {
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
      else {
        // stop existing thread, start new thread
        if (workerThread->isActive())
          workerThread->stop();

        workerThread->start();
      }

    }
    else {
      // FIXME 20011130 thammer, this is just for debugging! copied from sourcesensorcb
      // when we're not streaming, fill buffer once
      alSourcei(this->sourceId, AL_BUFFER, audioClip->soaudioclip_impl->bufferId);
      if ((error = alGetError()) != AL_NO_ERROR) {
        char errstr[256];
        SoDebugError::postWarning("SoSoundP::startPlaying",
                                  "alSourcei(,AL_BUFFER,) failed. %s",
                                  GetALErrorString(errstr, error));
        return FALSE;
      }
    }

    ALint      state;
  #ifdef _WIN32
    alGetSourcei(this->sourceId, AL_SOURCE_STATE, &state);
  #else
    alGetSourceiv(this->sourceId, AL_SOURCE_STATE, &state);
  #endif


    // 20010809 thh moved here
    // we weren't playing, so start playing
    alSourcePlay(this->sourceId);
    if ((error = alGetError()) != AL_NO_ERROR) {
      char errstr[256];
      SoDebugError::postWarning("SoSoundP::StartPlaying",
                                "alSourcePlay failed. %s",
                                GetALErrorString(errstr, error));
      return FALSE;
    }

    this->actualStartTime = SbTime::getTimeOfDay();

    audioClip->isActive.setValue(TRUE);

  }
  return TRUE;
};

int SoSoundP::fillBuffers()
{
#if HAVE_PTHREAD
  SbAutoLock autoLock(&(SoSoundP::syncmutex)); // synchronize with main thread
#endif

  ALint      processed;

  SoAudioClipStreaming *audioClipStreaming = (SoAudioClipStreaming *)this->currentAudioClip;
  // Get status
#ifdef _WIN32
  alGetSourcei(this->sourceId, AL_BUFFERS_PROCESSED, &processed);
#else
  alGetSourceiv(this->sourceId, AL_BUFFERS_PROCESSED, &processed);
#endif

  // for debugging
  ALint      queued;
#ifdef _WIN32
  alGetSourcei(this->sourceId, AL_BUFFERS_QUEUED, &queued);
#else
  alGetSourceiv(this->sourceId, AL_BUFFERS_QUEUED, &queued);
#endif
//  printf("Processed: %d, Queued: %d\n", processed, queued);


  ALuint      BufferID;
  ALint      error;
  int retval = 1;

  if (processed > 0) {
    while (processed) {

      alSourceUnqueueBuffers(this->sourceId, 1, &BufferID);
      if ((error = alGetError()) != AL_NO_ERROR) {
        char errstr[256];
        SoDebugError::postWarning("SoSound::fillBuffers",
                                  "alSourceUnqueueBuffers failed. %s",
                                  GetALErrorString(errstr, error));
        return 0;
      }

      // fill buffer
      retval = audioClipStreaming->soaudioclipstreaming_impl->fillBuffer(this->audioBuffer, audioClipStreaming->getBufferSize());

      if (retval <= 0) {
        printf("retval<=0\n");
        audioClipStreaming->setPlayedOnce();
        return 0; // 20011206 thammer. 0 worked ok, but will terminate thread, better to let audioRender do that
        // should play AL_BUFFERS_QUEUED empty buffers, then return 0 so that the thread finishes
        // the sound should finish also, so we could probably just call stopPlaying()
      }


      // send buffer to OpenAL
      int channels;
      int samplerate;
      int bitspersample;
      audioClipStreaming->getSampleFormat(channels, samplerate, bitspersample);

      ALenum  alformat = 0;;
      alformat = getALSampleFormat(channels, bitspersample);

      alBufferData(BufferID, alformat, this->audioBuffer, 
        audioClipStreaming->getBufferSize() * sizeof(short int) * audioClipStreaming->getNumChannels(), 
        samplerate);
      if ((error = alGetError()) != AL_NO_ERROR) {
        char errstr[256];
        SoDebugError::postWarning("SoSound::fillBuffers",
                                  "alBufferData failed. %s",
                                  GetALErrorString(errstr, error));
        return 0;
      }

      // Queue buffer
      alSourceQueueBuffers(this->sourceId, 1, &BufferID);
      if ((error = alGetError()) != AL_NO_ERROR) {
        char errstr[256];
        SoDebugError::postWarning("SoSound::fillBuffers",
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

  ALint      state;
#ifdef _WIN32
  alGetSourcei(this->sourceId, AL_SOURCE_STATE, &state);
#else
  alGetSourceiv(this->sourceId, AL_SOURCE_STATE, &state);
#endif
  if (state != AL_PLAYING) {
    if (state == AL_STOPPED) {
      alSourcePlay(this->sourceId);
      if ((error = alGetError()) != AL_NO_ERROR) {
        char errstr[256];
        SoDebugError::postWarning("SoSoundP::StartPlaying",
                                  "alSourcePlay failed. %s",
                                  GetALErrorString(errstr, error));
        return FALSE;
      }
#ifdef DEBUG_AUDIO
      fprintf(stderr, "state == AL_STOPPED. Had to restart source\n");
#endif
    }
    else {
      char statestr[20];
      switch (state) {
      case AL_INITIAL : sprintf(statestr, "initial"); break;
      case AL_PLAYING : sprintf(statestr, "playing"); break;
      case AL_PAUSED : sprintf(statestr, "paused"); break;
      case AL_STOPPED : sprintf(statestr, "stopped"); break;
      default : sprintf(statestr, "unknown"); break;
      };
#ifdef DEBUG_AUDIO
      fprintf(stderr, "state == %s. Don't know what to do about it...\n", statestr);
#endif
    }
  };


  return retval;
};

void
SoSoundP::timercb(void * data, SoSensor * s)
{
  SoSoundP * thisp = (SoSoundP*) data;
  thisp->fillBuffers();
}


void SoSound::audioRender(SoAudioRenderAction *action)
{

#if HAVE_PTHREAD
  SbAutoLock autoLock(&(SoSoundP::syncmutex)); // synchronize with fillBuffers
#endif

  ALint error;
  // FIXME: allow for other types of source nodes (like MovieTexture)

  // Get SoAudioClip::startTime and see if it's time to start
  if (!this->source.getValue())
    return;

  SoAudioClip *audioClip = NULL;
  audioClip = (SoAudioClip *)this->source.getValue();

  if (!audioClip->isBufferOK())
    return; // the source buffer is invalid, and there's not much we can do about it

  SbTime now = SbTime::getTimeOfDay();
  SbTime start = audioClip->startTime.getValue();
  SbTime stop = audioClip->stopTime.getValue();
  
  SbString start_str = start.format("%D %h %m %s");
  SbString stop_str = stop.format("%D %h %m %s");
  SbString now_str = now.format("%D %h %m %s");

  if ( (now<start) || ( (now>=stop) && (stop>start)) )
  {
    // we shouldn't be playing now
    THIS->stopPlaying();
    return; 
  }

  if (audioClip->getPlayedOnce() == TRUE) {
    if  (audioClip->isActive.getValue())
      THIS->stopPlaying(); // fillThread (streaming) probably set the playedOnce flag, 
                           // and we'll stop the sound here
    return; // we have played through the clip once, so we shouldn't play anymore
  }

//  if ( (audioClip->isActive.getValue()) && (stop <= start))
  if ( (audioClip->isActive.getValue()) && !(THIS->isStreaming) )
  {
    // check to see if we're looping. 
    // If we're not, check if the sample has been played completely
    // if it has, there's no point in plaing any more, is there?????
    // fixme: 20011116 thammer, think about this. Could some other node be depending on
    // exact playing time ??
    ALint looping = FALSE;
#ifdef _WIN32 // FIXME: hack to compile under Linux, pederb 2001-12-01
    alGetSourcei(THIS->sourceId,AL_LOOPING, &looping);
#endif
    if ((error = alGetError()) != AL_NO_ERROR)
    {
      char errstr[256];
      SoDebugError::postWarning("SoSoundP::audioRender",
                                "alSourcei(,AL_LOOPING,) failed. %s",
                                GetALErrorString(errstr, error));
      return;
    }

    if (!looping) {
      if (now - THIS->actualStartTime >= audioClip->getBufferDuration()) {
        // fixme 20011206 thammer, perhaps we need some slack...
        printf("SoSoundP::audioRender - "
                                  "Stopping non-looping sound because the whole buffer has been played\n");
        THIS->stopPlaying();
        audioClip->setPlayedOnce();
        return;
      }
    }
  };

  // If we got this far, then  ( start <= now < stop ) || (stop <= start <= now) and we should be playing
  // if stop <= start, the stop time is ignored

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
  if ((error = alGetError()) != AL_NO_ERROR) {
    char errstr[256];
    SoDebugError::postWarning("SoSound::audioRender",
                              "alSourcefv(,AL_POSITION,) failed. %s",
                              GetALErrorString(errstr, error));
    return;
  }

  // Velocity ...
/*  SbVec3f2ALfloat3(alfloat3, velocity.getValue());

  alSourcefv(this->sourceId, AL_VELOCITY, alfloat3);
  if ((error = alGetError()) != AL_NO_ERROR) {
    char errstr[256];
    SoDebugError::postWarning("SoSound::GLRender",
                              "alSourcefv(,AL_VELOCITY,) failed. %s",
                              GetALErrorString(errstr, error));
    return;
  }
*/

  // Gain / intensity
  alSourcef(THIS->sourceId,AL_GAIN, this->intensity.getValue());
  if ((error = alGetError()) != AL_NO_ERROR) {
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
    // 20011202 thammer: isn't that because we're notified also if the source
    // gets any fields changed!

#if HAVE_PTHREAD
  SbAutoLock autoLock(&(SoSoundP::syncmutex)); // synchronize with fill-thread
#endif

  this->currentAudioClip = audioClip;

  if (!audioClip->isBufferOK())
    return; // the source buffer is invalid, and there's not much we can do about it

  if (this->isStreaming)
  { 
    if (this->audioBuffer != NULL)
      delete[] this->audioBuffer;
    SoAudioClipStreaming *audioClipStreaming = (SoAudioClipStreaming *)this->currentAudioClip;
    this->audioBuffer = new short int[audioClipStreaming->getBufferSize() * audioClipStreaming->getNumChannels()];
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

#if 0
/*
//  fixme 20011130 thammer, removed for debugging purposes. see startplaying

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
*/
#endif // 0

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

#endif // HAVE_OPENAL
