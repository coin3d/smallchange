#ifndef COIN_SOAUDIODEVICE_H
#define COIN_SOAUDIODEVICE_H

#include <Inventor/SbString.h>
#include <Inventor/actions/SoGLRenderAction.h>

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

protected:
	class SoAudioDeviceP *soaudiodevice_impl;
  friend class SoAudioDeviceP;
};

#endif