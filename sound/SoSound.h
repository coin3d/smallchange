#ifndef COIN_SOSOUND_H
#define COIN_SOSOUND_H

#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/fields/SoSfVec3f.h>
#include <Inventor/fields/SoSfFloat.h>
#include <Inventor/fields/SoSfNode.h>
#include <Inventor/fields/SoSfBool.h>

#ifdef SOAL_SUB
#include <AL/altypes.h>
#else
#include <altypes.h>
#endif

#include "SoAudioRenderAction.h"
#include "SbAudioWorkerThread.h"

class SoTimerSensor;

class SbAsyncBuffer;

//#define NUMBUFFERS 10

class SoSound : 
  public SoNode
{
  SO_NODE_HEADER(SoSound);

  friend class SoAudioRenderAction;

public:

  static void initClass();
  SoSound();

  SoSFVec3f  direction;
  SoSFFloat  intensity;
  SoSFVec3f  location;
  SoSFFloat  maxBack;
  SoSFFloat  maxFront;
  SoSFFloat  minBack;
  SoSFFloat  minFront;
  SoSFFloat  priority;
  SoSFNode   source;
  SoSFBool   spatialize;

protected:

//  virtual void GLRender(SoGLRenderAction *action);
  virtual void audioRender(SoAudioRenderAction *action);
  static void sourceSensorCB(void *, SoSensor *);

  SbBool stopPlaying(SbBool force = FALSE);
  SbBool startPlaying(SbBool force = FALSE);
//  static int fill_buffer(void * buffer, void * userdata);

  class SoFieldSensor * sourcesensor;
  ALuint sourceId;
  class SoAudioClip *currentAudioClip;
  SbBool isStreaming;
  SbBool asyncStreamingMode;

//  SbAsyncBuffer * buffer;
//  unsigned char * currentdata;

private:

  virtual ~SoSound();

  // to be removed
//  ALuint audio_buffers[NUMBUFFERS];
  static void timercb(void * data, SoSensor *);
  SoTimerSensor * timersensor;
  SbAudioWorkerThread *workerThread;
  short int *audioBuffer;

  static int threadCallbackWrapper(void *userdata);
  int threadCallback();
  int fillBuffers();
};

#endif COIN_SOSOUND_H