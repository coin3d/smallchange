#ifndef COIN_SOAUDIODEVICE_H
#define COIN_SOAUDIODEVICE_H

#include <Inventor/SbString.h>
#include <Inventor/actions/SoGLRenderAction.h>

#ifdef SOAL_SUB
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>
#else
#include <al.h>
#include <alc.h>
#include <alut.h>
#endif

#include "SoAudioRenderAction.h"

class SoAudioDevice
{
public:

  SoAudioDevice();
  virtual ~SoAudioDevice();

  virtual SbBool init(const SbString &devicetype, const SbString &devicename);
  void setSceneGraph(SoNode *root);
  void setGLRenderAction(SoGLRenderAction *ra);
  
  virtual void enable();
  virtual void disable();

  static void prerendercb(void * userdata, SoGLRenderAction * action);

protected:
//	ALCcontext *context; 20010803 thh
	void *context;
	ALCdevice *device;
  SoGLRenderAction *glRenderAction;
  SoAudioRenderAction *audioRenderAction;
  SoNode *root;

  bool enabled;
};

#endif