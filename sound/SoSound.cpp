#include "SoSound.h"

#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/sensors/SoFieldSensor.h>

#ifdef SOAL_SUB
#include <AL/altypes.h>
#include <AL/al.h>
#else
#include <altypes.h>
#include <al.h>
#endif

#include "ALTools.h"
#include "SoAudioClip.h"
#include "SoAudioClipStreaming.h"

//#include "SbAsyncBuffer.h"

#include <Inventor/sensors/SoTimerSensor.h>


#include <math.h>

SO_NODE_SOURCE(SoSound);

void SoSound::initClass()
{
  SO_NODE_INIT_CLASS(SoSound, SoNode, "Node");
};

SoSound::SoSound() 
{
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

  this->currentAudioClip = NULL;

  this->timersensor = NULL;
  this->isStreaming = FALSE;
  this->asyncStreamingMode = FALSE;

	ALint	error;

  alGenSources(1, &(this->sourceId));
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
  this->sourcesensor = new SoFieldSensor(sourceSensorCB, this);
  this->sourcesensor->setPriority(0);
  this->sourcesensor->attach(&this->source);

  this->workerThread = new SbAudioWorkerThread(SoSound::threadCallbackWrapper, this);
  this->audioBuffer = NULL;

};

SoSound::~SoSound()
{
  delete this->sourcesensor;

  stopPlaying(TRUE);

  delete workerThread;

  if (this->audioBuffer != NULL)
    delete[] this->audioBuffer;

	ALint	error;

  alDeleteSources(1, &(this->sourceId));
	if ((error = alGetError()) != AL_NO_ERROR)
  {
    char errstr[256];
		SoDebugError::postWarning("SoSoundBuffer::xxx",
                              "alDeleteSources() failed. %s",
                              GetALErrorString(errstr, error));
    return;
  }
};

int SoSound::threadCallbackWrapper(void *userdata)
{
  SoSound *thisp = (SoSound *)userdata;
  return thisp->threadCallback();
};

int SoSound::threadCallback()
{
  return this->fillBuffers();
};


SbBool SoSound::stopPlaying(SbBool force)
{
  ALint error;
  SbBool retval = TRUE;
  SoAudioClip *audioClip = NULL;
  audioClip = (SoAudioClip *)this->source.getValue();

  if (force || (audioClip->isActive.getValue()))
  { 
    // we were playing, so stop
    printf("SoSound::stopPlaying stopping \n");
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

/*    // stop the tread
    if (this->buffer) {
      if (this->currentdata) {
        this->buffer->unlockBuffer();
        this->currentdata = NULL;
      }
      delete this->buffer;
      this->buffer = NULL;
    }
*/
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

  // fixme: delete audio buffers
  if (this->isStreaming)
  {
    SoAudioClipStreaming *audioClipStreaming = (SoAudioClipStreaming *)this->source.getValue();
    audioClipStreaming->stopPlaying();
  }

  return retval;
};

//#define BUFFERSIZE 8820

SbBool SoSound::startPlaying(SbBool force)
{
//  stopPlaying();

  // fixme: check if it's a streaming node before starting thread
  ALint error;
  SoAudioClip *audioClip = (SoAudioClip *)this->source.getValue();

  if (force || (!audioClip->isActive.getValue()))
  {
   
    this->isStreaming = FALSE;
    if (this->source.getValue()->isOfType(SoAudioClipStreaming::getClassTypeId()))
    {
      SoAudioClipStreaming *audioClipStreaming = (SoAudioClipStreaming *)this->source.getValue();
      this->asyncStreamingMode = audioClipStreaming->asyncMode;
      this->isStreaming = TRUE;

      // create class-local buffers (used in fillBuffers)
      if (this->isStreaming)
      {
        if (this->audioBuffer == NULL)
        {
          SoAudioClipStreaming *audioClipStreaming = (SoAudioClipStreaming *)this->currentAudioClip;
          this->audioBuffer = new short int[audioClipStreaming->bufferSize];
        };
      };

      // create AL buffers
      audioClipStreaming->startPlaying();

    	  // Queue the buffers on the source
	    alSourceQueueBuffers(this->sourceId, audioClipStreaming->numBuffers, audioClipStreaming->streamingBuffers);
	    if ((error = alGetError()) != AL_NO_ERROR)
      {
        char errstr[256];
		    SoDebugError::postWarning("SoSoundBuffer::xxx",
                                  "alSourceQueueBuffers failed. %s",
                                  GetALErrorString(errstr, error));
        return FALSE;
      }

      // 20010803 thh moved here
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
    audioClip->isActive.setValue(TRUE);
  }
  return TRUE;
};

int SoSound::fillBuffers()
{

	ALint			processed;

  //  SoAudioClipStreaming *audioClipStreaming = (SoAudioClipStreaming *)this->source.getValue();
  SoAudioClipStreaming *audioClipStreaming = (SoAudioClipStreaming *)this->currentAudioClip;
	// Get status
	alGetSourcei(this->sourceId, AL_BUFFERS_PROCESSED, &processed);

	ALuint buffersreturned = 0;
	ALboolean bFinishedPlaying = AL_FALSE;
	ALuint buffersinqueue = audioClipStreaming->numBuffers;
	ALuint			BufferID;
	ALint			error;

	if (processed > 0)
	{
		buffersreturned += processed;
		printf("Buffers Completed is %d   \r", buffersreturned);
 
//    short int *buffer = new short int[audioClipStreaming->bufferSize];

		while (processed)
		{
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
      audioClipStreaming->fillBuffer(audioBuffer, audioClipStreaming->bufferSize);

      // send buffer to OpenAL
			alBufferData(BufferID, AL_FORMAT_MONO16, audioBuffer, audioClipStreaming->bufferSize*sizeof(short int), 44100);
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

  return 1;
};

void
SoSound::timercb(void * data, SoSensor * s)
{
  SoSound * thisp = (SoSound*) data;
  thisp->fillBuffers();
/*
  // used only when isStreaming and not in asyncStreamingMode
	ALint			processed;
  SoSound * thisp = (SoSound*) data;
  SoAudioClipStreaming *audioClipStreaming = (SoAudioClipStreaming *)thisp->source.getValue();

	// Get status
	alGetSourcei(thisp->sourceId, AL_BUFFERS_PROCESSED, &processed);

	ALuint buffersreturned = 0;
	ALboolean bFinishedPlaying = AL_FALSE;
	ALuint buffersinqueue = audioClipStreaming->numBuffers;
	ALuint			BufferID;
	ALint			error;

	if (processed > 0)
	{
		buffersreturned += processed;
		printf("Buffers Completed is %d   \r", buffersreturned);
 
    // fixme: use class local variable so we don't have to new each time
    short int *buffer = new short int[audioClipStreaming->bufferSize];

		while (processed)
		{
			alSourceUnqueueBuffers(thisp->sourceId, 1, &BufferID);
	    if ((error = alGetError()) != AL_NO_ERROR)
      {
        char errstr[256];
		    SoDebugError::postWarning("SoSound::audioRender",
                                  "alSourceUnqueueBuffers failed. %s",
                                  GetALErrorString(errstr, error));
        return;
      }

      // fill buffer
      audioClipStreaming->fillBuffer(buffer, audioClipStreaming->bufferSize);

      // send buffer to OpenAL
			alBufferData(BufferID, AL_FORMAT_MONO16, buffer, audioClipStreaming->bufferSize*sizeof(short int), 44100);
	    if ((error = alGetError()) != AL_NO_ERROR)
      {
        char errstr[256];
		    SoDebugError::postWarning("SoSound::audioRender",
                                  "alBufferData failed. %s",
                                  GetALErrorString(errstr, error));
        return;
      }

			// Queue buffer
			alSourceQueueBuffers(thisp->sourceId, 1, &BufferID);
	    if ((error = alGetError()) != AL_NO_ERROR)
      {
        char errstr[256];
		    SoDebugError::postWarning("SoSound::audioRender",
                                  "alSourceQueueBuffers failed. %s",
                                  GetALErrorString(errstr, error));
        return;
      }

			processed--;
		}
    delete buffer;
  }

  return;
*/
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
  if (now<start)
  {
    // we shouldn't be playing now
    stopPlaying();
    return; 
  }

  if (now>=stop)
  {
    stopPlaying();
    return;
  }

  // If we got this far, then  ( start <= now < stop ) and we should be playing

  startPlaying(); // if we're not playing, playing will start

  // audio source is now playing, update OpenAL parameters
 
  SbVec3f pos, worldpos;
  ALfloat alfloat3[3];

  pos = this->location.getValue();
  SoModelMatrixElement::get(action->getState()).multVecMatrix(pos, worldpos); 

  float x, y, z;
  worldpos.getValue(x, y, z);

  printf("(%0.2f, %0.2f, %0.2f)\n", x, y, z);

  SbVec3f2ALfloat3(alfloat3, worldpos);

	// Position ...
	alSourcefv(this->sourceId, AL_POSITION, alfloat3);
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
	alSourcef(this->sourceId,AL_GAIN, this->intensity.getValue());
	if ((error = alGetError()) != AL_NO_ERROR)
  {
    char errstr[256];
		SoDebugError::postWarning("SoSoundBuffer::xxx",
                              "alSourcef(,AL_GAIN,) failed. %s",
                              GetALErrorString(errstr, error));
    return;
  }

  // FIXME: check if anything has changed in the audioClip (like buffer, loop-points, etc, and react accordingly)
}

//
// called when source changes
//
void
SoSound::sourceSensorCB(void * data, SoSensor *)
{
  ALint error;
  SoSound * thisp = (SoSound*) data;

//  printf("SoSound::sourceSensorCB()\n");

  if (!thisp->source.getValue())
    return;

  SoAudioClip *audioClip = (SoAudioClip *)thisp->source.getValue();
  // FIXME: use RTTI instead, to see what kind of node it is (might be a movietexture node)
  // ... or perhaps OI has a convenience method for just that (SoNode::getNodeId ??)

  if (audioClip == thisp->currentAudioClip)
    return; 
    // for some obscure reason, the sensor was called, even though the field haven't changed .....
    // FIXME: ask mortene about this --^

  thisp->currentAudioClip = audioClip;

  if (thisp->isStreaming)
  { 
    if (thisp->audioBuffer != NULL)
      delete[] thisp->audioBuffer;
    SoAudioClipStreaming *audioClipStreaming = (SoAudioClipStreaming *)thisp->currentAudioClip;
    thisp->audioBuffer = new short int[audioClipStreaming->bufferSize];
  };

  // FIXME: should probably sync with streaming thread (if we're doing async streaming)
  // especially important for the buffer...

  alSourceStop(thisp->sourceId);
	if ((error = alGetError()) != AL_NO_ERROR)
  {
    char errstr[256];
		SoDebugError::postWarning("SoSound::sourceSensorCB",
                              "alSourceStop failed. %s",
                              GetALErrorString(errstr, error));
  }

	alSourcef(thisp->sourceId,AL_PITCH, audioClip->pitch.getValue());
	if ((error = alGetError()) != AL_NO_ERROR)
  {
    char errstr[256];
		SoDebugError::postWarning("SoSoundBuffer::xxx",
                              "alSourcef(,AL_PITCH,) failed. %s",
                              GetALErrorString(errstr, error));
    return;
  }

	alSourcef(thisp->sourceId,AL_GAIN, thisp->intensity.getValue());
	if ((error = alGetError()) != AL_NO_ERROR)
  {
    char errstr[256];
		SoDebugError::postWarning("SoSoundBuffer::xxx",
                              "alSourcef(,AL_GAIN,) failed. %s",
                              GetALErrorString(errstr, error));
    return;
  }

  if (thisp->source.getValue()->isOfType(SoAudioClipStreaming::getClassTypeId()))
  {

  }
  else
  {

	  alSourcei(thisp->sourceId, AL_BUFFER, audioClip->bufferId);
	  if ((error = alGetError()) != AL_NO_ERROR)
    {
      char errstr[256];
		  SoDebugError::postWarning("SoSound::sourceSensorCB",
                                "alSourcei(,AL_BUFFER,) failed. %s",
                                GetALErrorString(errstr, error));
      return;
    }
  };

	alSourcei(thisp->sourceId,AL_LOOPING, audioClip->loop.getValue());
	if ((error = alGetError()) != AL_NO_ERROR)
  {
    char errstr[256];
		SoDebugError::postWarning("SoSoundBuffer::xxx",
                              "alSourcei(,AL_LOOPING,) failed. %s",
                              GetALErrorString(errstr, error));
    return;
  }

}

// void SoSound::GLRender(SoGLRenderAction *action)
