#if HAVE_CONFIG_H
#include <config.h>
#endif

#if HAVE_OPENAL

#include <SmallChange/nodes/SoSound.h>
#include "SmallChange/nodes/SoSoundP.h"
#include <SmallChange/misc/ALTools.h>
#include <SmallChange/nodes/SoAudioClip.h>

#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/sensors/SoFieldSensor.h>
#include <Inventor/sensors/SoTimerSensor.h>

#include <AL/altypes.h>
#include <AL/al.h>

#include <math.h>

#undef THIS
#define THIS this->sosound_impl

#undef ITHIS
#define ITHIS this->ifacep


SO_NODE_SOURCE(SoSound);

// fixme 20021006 thammer: should really do individual synchronization instead of global
SbMutex *SoSoundP::syncmutex = NULL;

int SoSoundP::defaultBufferLength = 44100/10;
int SoSoundP::defaultNumBuffers = 5;
SbTime SoSoundP::defaultSleepTime = 0.100; // 100ms


void SoSound::initClass()
{
  SO_NODE_INTERNAL_INIT_CLASS(SoSound);
  SoSoundP::syncmutex = new SbMutex;
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

  THIS->channels = 1; // because spatialize defaults to TRUE and OpenAL only spatializes mono buffers

  THIS->currentAudioClip = NULL;
  THIS->playing = FALSE;

  THIS->timersensor = NULL;
  THIS->useTimerCallback = TRUE;

  ALint  error;

  alGenSources(1, &(THIS->sourceId));
  if ((error = alGetError()) != AL_NO_ERROR) {
    char errstr[256];
    SoDebugError::post("SoSound::SoSound",
                       "alGenSources failed. %s",
                       GetALErrorString(errstr, error));
    return;
  }

  THIS->sourcesensor = new SoFieldSensor(THIS->sourceSensorCBWrapper, THIS);
  THIS->sourcesensor->setPriority(0);
  THIS->sourcesensor->attach(&this->source);

  /*
  THIS->spatializesensor = new SoFieldSensor(THIS->spatializeSensorCBWrapper, THIS);
  THIS->spatializesensor->setPriority(0);
  THIS->spatializesensor->attach(&this->spatialize);
  */

  THIS->workerThread = NULL;
  THIS->exitthread = FALSE;
  THIS->errorInThread = FALSE;
  THIS->audioBuffer = NULL;

  THIS->alBuffers = NULL;
  // this->setBufferInfo(44100/50, 9);
  this->setBufferingProperties(SoSoundP::defaultBufferLength, SoSoundP::defaultNumBuffers,
    SoSoundP::defaultSleepTime);

};

SoSound::~SoSound()
{
#ifdef DEBUG_AUDIO
  fprintf(stderr, "~SoSound()\n");
#endif
  delete THIS->sourcesensor;
  // delete THIS->spatializesensor;

#ifdef DEBUG_AUDIO
  printf("---[");
#endif

  if (THIS->currentAudioClip != NULL)
    THIS->currentAudioClip->unref();
  THIS->currentAudioClip = NULL;

  THIS->stopPlaying();

#ifdef DEBUG_AUDIO
  printf("]\n");
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

  THIS->deleteAlBuffers();

  delete THIS;
}

void SoSoundP::deleteAlBuffers()
{
  if (this->alBuffers != NULL) {
    alDeleteBuffers(this->numBuffers, this->alBuffers);
    delete [] this->alBuffers;
    this->alBuffers = NULL;
  }
}

void SoSound::setDefaultBufferingProperties(int bufferLength, int numBuffers, SbTime sleepTime)
{
  SoSoundP::defaultBufferLength = bufferLength;
  SoSoundP::defaultNumBuffers = numBuffers;
  SoSoundP::defaultSleepTime = sleepTime;
}

void SoSound::setBufferingProperties(int bufferLength, int numBuffers, SbTime sleepTime)
{
  SbMutexAutoLock autoLock(SoSoundP::syncmutex);
 
  THIS->numBuffers = numBuffers;
  THIS->sleepTime = sleepTime;

  if (THIS->bufferLength == bufferLength) 
    return;

  THIS->bufferLength = bufferLength;
  delete[] THIS->audioBuffer;
  THIS->audioBuffer = new int16_t[THIS->bufferLength * 2];
}

void SoSound::getBufferingProperties(int &bufferLength, int &numBuffers, SbTime &sleepTime)
{
  SbMutexAutoLock autoLock(SoSoundP::syncmutex);

  bufferLength = THIS->bufferLength;
  numBuffers = THIS->numBuffers;
  sleepTime = THIS->sleepTime;
}

void *SoSoundP::threadCallbackWrapper(void *userdata)
{
  SoSoundP *thisp = (SoSoundP *)userdata;
  return thisp->threadCallback();
}

void *SoSoundP::threadCallback()
{
  while (!this->exitthread) {
    this->fillBuffers();
    cc_sleep(this->workerThreadSleepTime.getValue());
  }
  return NULL;
}

void
SoSoundP::timercb(void * data, SoSensor * s)
{
  SoSoundP * thisp = (SoSoundP*) data;
  thisp->fillBuffers();
}

SbBool SoSoundP::stopPlaying()
{
  #if COIN_DEBUG && DEBUG_AUDIO // debug
    printf("sound:stop[.");
  #endif // debug

  if (!this->playing)
    return TRUE;

  #if COIN_DEBUG && DEBUG_AUDIO // debug
    printf(".");
  #endif // debug

  ALint error;

  // stop timersensor
  if (this->timersensor) {
    if (this->timersensor->isScheduled()) 
      this->timersensor->unschedule();
    delete this->timersensor;
    this->timersensor = NULL;
  }

  // stop thread
  if (this->workerThread!=NULL) {
    this->exitthread = TRUE;
    void *retval = NULL;
    cc_thread_join(this->workerThread, &retval);
    cc_thread_destruct(this->workerThread);
    this->workerThread = NULL;
  }

  #if COIN_DEBUG && DEBUG_AUDIO // debug
    printf(".");
  #endif // debug

  this->errorInThread = FALSE;

  SbBool retval = TRUE;

  // wait until all buffers have been played
  ALint      processed;

#if 0
  // this will (busy) wait for all remaining buffers to play
  // 20021008 thammer
  SbBool done = FALSE;
  while (!done) {
    ALint      state;
    #ifdef _WIN32
      alGetSourcei(this->sourceId, AL_SOURCE_STATE, &state);
    #else
      alGetSourceiv(this->sourceId, AL_SOURCE_STATE, &state);
    #endif

    #ifdef _WIN32
      alGetSourcei(this->sourceId, AL_BUFFERS_PROCESSED, &processed);
    #else
      alGetSourceiv(this->sourceId, AL_BUFFERS_PROCESSED, &processed);
    #endif

    ALint      queued;
    #ifdef _WIN32
      alGetSourcei(this->sourceId, AL_BUFFERS_QUEUED, &queued);
    #else
      alGetSourceiv(this->sourceId, AL_BUFFERS_QUEUED, &queued);
    #endif

    #if COIN_DEBUG && DEBUG_AUDIO // debug
      printf("State: %d, Processed: %d, Queued: %d\n", state, processed, queued);
    #endif // debug

    // if (state == AL_PLAYING)
    if (processed<queued)
      cc_sleep(this->workerThreadSleepTime.getValue());
    else
      done = TRUE;
  }
#endif

  alSourceStop(this->sourceId);
  if ((error = alGetError()) != AL_NO_ERROR) {
    char errstr[256];
    SoDebugError::postWarning("SoSound::stopPlaying",
                              "alSourceStop failed. %s",
                              GetALErrorString(errstr, error));
    retval= FALSE;
  }

  #ifdef _WIN32
    alGetSourcei(this->sourceId, AL_BUFFERS_PROCESSED, &processed);
  #else
    alGetSourceiv(this->sourceId, AL_BUFFERS_PROCESSED, &processed);
  #endif

  // alSourceUnqueueBuffers(this->sourceId, this->numBuffers, this->alBuffers);
  alSourceUnqueueBuffers(this->sourceId, processed, this->alBuffers);
  if ((error = alGetError()) != AL_NO_ERROR) {
    char errstr[256];
    SoDebugError::postWarning("SoSoundP::stopPlaying",
                              "alSourceUnqueueBuffers failed. %s",
                              GetALErrorString(errstr, error));
    retval = FALSE;
  }

  this->deleteAlBuffers();

  this->playing = FALSE;

  #if COIN_DEBUG && DEBUG_AUDIO // debug
    printf(".]\n");
  #endif // debug

  return retval;
}

SbBool SoSoundP::startPlaying()
{
  #if COIN_DEBUG && DEBUG_AUDIO // debug
    printf("sound:start[.");
  #endif // debug

  if (this->playing)
    return TRUE;

  #if COIN_DEBUG && DEBUG_AUDIO // debug
    printf(".");
  #endif // debug

  ALint error;

  // Create new buffers

  this->alBuffers = new ALuint[this->numBuffers];
  alGenBuffers(this->numBuffers, this->alBuffers);
  if ((error = alGetError()) != AL_NO_ERROR) {
    char errstr[256];
    SoDebugError::postWarning("SoSoundP::startPlaying",
                              "alGenBuffers failed. %s",
                              GetALErrorString(errstr, error));
    return FALSE;
  }

  // Fill buffer with data

  ALenum  alformat = 0;;
  alformat = getALSampleFormat(this->channels, 16);

  int bufferno;
  void *ret;
  for (bufferno = 0; bufferno < this->numBuffers; bufferno++) {
    // FIXME: must fill buffer without risking deadlocks. Investigate. 20021007 thammer.
    int frameoffset, channels;
    frameoffset=0;
    ret = this->currentAudioClip->fillBuffer(frameoffset, this->audioBuffer, 
      this->bufferLength, channels);

  #if COIN_DEBUG && DEBUG_AUDIO // debug
    printf(".");
  #endif // debug

    if (ret == NULL) {
      break; // eof before done filling buffers
    }

    if ( (channels==1) && (this->channels==2) )
      mono2stereo(this->audioBuffer, this->bufferLength);
    else if ( (channels==2) && (this->channels==1) )
      stereo2mono(this->audioBuffer, this->bufferLength);
    
    alBufferData(this->alBuffers[bufferno], alformat, this->audioBuffer, 
      this->bufferLength * sizeof(int16_t) * this->channels, 
      this->currentAudioClip->getSampleRate());
    if ((error = alGetError()) != AL_NO_ERROR) {
      char errstr[256];
      SoDebugError::postWarning("SoSoundP::startPlaying",
                                "alBufferData failed. %s",
                                GetALErrorString(errstr, error));
      return FALSE;
    }
  }


  // Queue the buffers on the source
  
  //alSourceQueueBuffers(this->sourceId, this->numBuffers, this->alBuffers);
  alSourceQueueBuffers(this->sourceId, bufferno, this->alBuffers);
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
    SoDebugError::postWarning("SoSoundP::startPlaying",
                              "alSourcei(,AL_LOOPING,) failed. %s",
                              GetALErrorString(errstr, error));
    return FALSE;
  }

  // Start timer or thread
  if (this->useTimerCallback) {
    // stop previous timer
    if (this->timersensor) {
      if (this->timersensor->isScheduled()) 
        this->timersensor->unschedule();
      delete this->timersensor;
      this->timersensor = NULL;
    }
    // start new timer
    this->timersensor = new SoTimerSensor(timercb, this);
    this->timersensor->setInterval(this->sleepTime);
    this->timersensor->schedule();
  }
  else {
    // stop existing thread, start new thread
    if (this->workerThread!=NULL) {
      this->exitthread = TRUE;
      void *retval = NULL;
      cc_thread_join(this->workerThread, &retval);
      cc_thread_destruct(this->workerThread);
      this->workerThread = NULL;
    }

    this->workerThreadSleepTime = this->sleepTime;
    this->errorInThread = FALSE;
    this->exitthread = FALSE;
    this->workerThread = cc_thread_construct(this->threadCallbackWrapper, this);
  }

  alSourcePlay(this->sourceId);
  if ((error = alGetError()) != AL_NO_ERROR) {
    char errstr[256];
    SoDebugError::postWarning("SoSoundP::StartPlaying",
                              "alSourcePlay failed. %s",
                              GetALErrorString(errstr, error));
    return FALSE;
  }

  this->playing = TRUE;

  #if COIN_DEBUG && DEBUG_AUDIO // debug
    printf(".]\n");
  #endif // debug

  return TRUE;
}

void SoSoundP::fillBuffers()
{
  SbMutexAutoLock autoLock(SoSoundP::syncmutex);

  ALint      processed;

  // Get status
#ifdef _WIN32
  alGetSourcei(this->sourceId, AL_BUFFERS_PROCESSED, &processed);
#else
  alGetSourceiv(this->sourceId, AL_BUFFERS_PROCESSED, &processed);
#endif

  ALint      queued;
#ifdef _WIN32
  alGetSourcei(this->sourceId, AL_BUFFERS_QUEUED, &queued);
#else
  alGetSourceiv(this->sourceId, AL_BUFFERS_QUEUED, &queued);
#endif

#if COIN_DEBUG && DEBUG_AUDIO // debug
  printf("Processed: %d, Queued: %d\n", processed, queued);
#endif // debug

  ALuint BufferID;
  ALint error;
  void *ret;

/*
  if (queued<=0) {
    // no buffers were queued, so there's nothing to do
    // this should only happen after audioclip::fillBuffer() returns NULL to indicate an EOF,
    // and sound::fillBuffers() does not queue new buffers after that
    #if COIN_DEBUG && 1 // debug
      printf("No more buffers queued (we're probably stopping soon)\n");
    #endif // debug
    return; 
  }
*/

  while (processed>0) {

    // FIXME: perhaps we should reread processed in the while loop too. 
    // This might make buffer underruns less frequent. 20021007 thammer

    // unqueue one buffer

    alSourceUnqueueBuffers(this->sourceId, 1, &BufferID);
    if ((error = alGetError()) != AL_NO_ERROR) {
      char errstr[256];
      SoDebugError::post("SoSound::fillBuffers",
                         "alSourceUnqueueBuffers failed. %s",
                         GetALErrorString(errstr, error));
      this->errorInThread = TRUE;
      return;
    }

    // fill buffer

    int frameoffset, channels;
    frameoffset=0;
    ret = this->currentAudioClip->fillBuffer(frameoffset, this->audioBuffer, 
      this->bufferLength, channels);

/*    if (ret == NULL)
      return; // AudioClip has reached EOF (or an error), so we shouldn't fill any more buffers
*/
    if ( (channels==1) && (this->channels==2) )
      mono2stereo(this->audioBuffer, this->bufferLength);
    else if ( (channels==2) && (this->channels==1) )
      stereo2mono(this->audioBuffer, this->bufferLength);

    // send buffer to OpenAL

    ALenum  alformat = 0;;
    alformat = getALSampleFormat(this->channels, 16);

    alBufferData(BufferID, alformat, this->audioBuffer, 
      this->bufferLength * sizeof(int16_t) * this->channels, 
      this->currentAudioClip->getSampleRate());
    if ((error = alGetError()) != AL_NO_ERROR) {
      char errstr[256];
      SoDebugError::post("SoSound::fillBuffers",
                         "alBufferData failed. %s",
                         GetALErrorString(errstr, error));
      this->errorInThread = TRUE;
      return;
    }

    // Queue buffer
    alSourceQueueBuffers(this->sourceId, 1, &BufferID);
    if ((error = alGetError()) != AL_NO_ERROR) {
      char errstr[256];
      SoDebugError::post("SoSound::fillBuffers",
                         "alSourceQueueBuffers failed. %s",
                         GetALErrorString(errstr, error));
      this->errorInThread = TRUE;
      return;
    }

    processed--;
  }

  // Check to see if we're still playing
  // if not, make sure to start over again.
  // If we're not playing, it's because the buffers have not been filled up 
  // (unqueued, filled, queued) fast enough, so the source has played the last 
  // buffer in the queue and changed state from playing to stopped.

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
        SoDebugError::post("SoSoundP::fillBuffers",
                           "alSourcePlay failed. %s",
                           GetALErrorString(errstr, error));
        this->errorInThread = TRUE;
        return;
      }
      SoDebugError::postWarning("SoSoundP::fillBuffers",
                                "Buffer underrun. The audio source had to be restarted. "
                                "Try increasing buffer size (current: %d frames), "
                                "and/or increasing number of buffers (current: %d buffers), "
                                "and/or decreasing sleeptime (current: %0.3fs)", 
                                this->bufferLength, this->numBuffers, this->sleepTime.getValue());
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
      // 20021007 thammer fixme: deal with this properly!
#if COIN_DEBUG && DEBUG_AUDIO // debug
      fprintf(stderr, "state == %s. Don't know what to do about it...\n", statestr);
#endif
    }
  }
}

void SoSound::audioRender(SoAudioRenderAction *action)
{

  SbMutexAutoLock autoLock(SoSoundP::syncmutex);

  if (THIS->currentAudioClip == NULL)
    return;

  // SoSFBool * isactive = (SoSFBool *)THIS->currentAudioClip->getField("isActive");

  if ( (!THIS->playing) && (!THIS->currentAudioClip->isActive.getValue()) )
    return;

  if (THIS->errorInThread) {
    THIS->stopPlaying();
    return;
  }

  if ( THIS->playing && (!THIS->currentAudioClip->isActive.getValue()) ) {
    THIS->stopPlaying();
    return;
  }

  // if we got here then we're either allready playing, or we should be

  ALint error;
  SbVec3f pos, worldpos;
  ALfloat alfloat3[3];

  pos = this->location.getValue();
  SoModelMatrixElement::get(action->getState()).multVecMatrix(pos, worldpos); 

#if COIN_DEBUG && 0 // debug
  float x, y, z;
  worldpos.getValue(x, y, z);
  printf("(%0.2f, %0.2f, %0.2f)-----------------------\n", x, y, z);
#endif // debug

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

#if 0
  // 20021007 thammer note: if we ever want to implement velocity (supported by OpenAL)
  // then this is how it should be done
  // get alfloat3 from THIS->velocity
  SbVec3f2ALfloat3(alfloat3, velocity.getValue());

  alSourcefv(this->sourceId, AL_VELOCITY, alfloat3);
  if ((error = alGetError()) != AL_NO_ERROR) {
    char errstr[256];
    SoDebugError::postWarning("SoSound::GLRender",
                              "alSourcefv(,AL_VELOCITY,) failed. %s",
                              GetALErrorString(errstr, error));
    return;
  }
#endif

  // Gain / intensity
  alSourcef(THIS->sourceId,AL_GAIN, this->intensity.getValue());
  if ((error = alGetError()) != AL_NO_ERROR) {
    char errstr[256];
    SoDebugError::postWarning("SoSound::audioRender",
                              "alSourcef(,AL_GAIN,) failed. %s",
                              GetALErrorString(errstr, error));
    return;
  }

  int newchannels = this->spatialize.getValue() ? 1 : 2;
  if (THIS->channels != newchannels) {
    if (THIS->playing)
      THIS->stopPlaying();

    THIS->channels = newchannels;
  }

  if ( (!THIS->playing) && THIS->currentAudioClip->isActive.getValue() ) 
    THIS->startPlaying();
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
  #if COIN_DEBUG && DEBUG_AUDIO // debug
    printf("(S)");
  #endif // debug

  SbMutexAutoLock autoLock(SoSoundP::syncmutex);

  SoNode *node = (SoNode *)ITHIS->source.getValue();

  if (!node->isOfType(SoAudioClip::getClassTypeId())) {
    SoDebugError::postWarning("SoSoundP::sourceSensorCB",
                              "Unknown source node type");
    if (this->currentAudioClip != NULL) {
      this->currentAudioClip->unref();
      this->stopPlaying();
    }
    this->currentAudioClip = NULL;
    return;
  }

  SoAudioClip *audioClip = (SoAudioClip *)node;
  if (audioClip != this->currentAudioClip) {
    if (this->currentAudioClip != NULL) {
      this->currentAudioClip->unref();
      this->currentAudioClip = NULL;
      this->stopPlaying();
    }
    if (audioClip!=NULL)
      audioClip->ref();
    this->currentAudioClip = audioClip;
  }

  if (this->currentAudioClip == NULL)
    return;

#if 0
  // FIXME: support pitch. To do this, AC must be modified so that the calculation
  // of duration (see AC::audioRender, close to ac.endOfFile) must be improved.
  // 20021008 thammer
  ALint error;
  alSourcef(this->sourceId, AL_PITCH, this->currentAudioClip->pitch.getValue());
  if ((error = alGetError()) != AL_NO_ERROR)
  {
    char errstr[256];
    SoDebugError::postWarning("SoSoundP::sourceSensorCB",
                              "alSourcef(,AL_PITCH,) failed. %s",
                              GetALErrorString(errstr, error));
    return;
  }
#endif // pitch support

  // 20021008 thammer: added this block. perhaps we should also start playing here?
  if ( this->playing && (!this->currentAudioClip->isActive.getValue()) ) {
    this->stopPlaying();
  }

}

/*
//
// called when spatialize changes
//
void
SoSoundP::spatializeSensorCBWrapper(void * data, SoSensor *)
{
  SoSoundP * thisp = (SoSoundP*) data;
  thisp->spatializeSensorCB(NULL);
}

//
// called when spatialize changes
//
void
SoSoundP::spatializeSensorCB(SoSensor *)
{
  SbMutexAutoLock autoLock(SoSoundP::syncmutex);

  if (this->channels != ITHIS->spatialize.getValue()) {
    SbBool wasplaying = this->playing;
    if (wasplaying)
      this->stopPlaying();

    this->channels = ITHIS->spatialize.getValue() ? 1 : 2;
    
    if (wasplaying)
      this->startPlaying(FALSE);
  }
}
*/

#endif // HAVE_OPENAL
