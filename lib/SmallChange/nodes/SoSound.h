#ifndef COIN_SOSOUND_H
#define COIN_SOSOUND_H

#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/fields/SoSfVec3f.h>
#include <Inventor/fields/SoSfFloat.h>
#include <Inventor/fields/SoSfNode.h>
#include <Inventor/fields/SoSfBool.h>

#include <actions/SoAudioRenderAction.h>

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

  virtual void audioRender(SoAudioRenderAction *action);

protected:
  class SoSoundP *sosound_impl;
  friend class SoSoundP;

private:

  virtual ~SoSound();

};

#endif COIN_SOSOUND_H