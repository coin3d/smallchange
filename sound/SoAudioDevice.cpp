#include "SoAudioDevice.h"

#include <Inventor/errors/SoDebugError.h>

#include "SoAudioDeviceP.h"

#undef THIS
#define THIS this->soaudiodevice_impl

#undef ITHIS
#define ITHIS this->ifacep

SoAudioDevice::SoAudioDevice()
{
  THIS = new SoAudioDeviceP(this);

  THIS->context = NULL;
	THIS->device = NULL;
  THIS->glRenderAction = NULL;
  THIS->audioRenderAction = NULL;
  THIS->enabled = FALSE;
  THIS->root = NULL;

  THIS->audioRenderAction = new SoAudioRenderAction();

};

SoAudioDevice::~SoAudioDevice()
{
  this->disable();

  if (THIS->audioRenderAction != NULL)
    delete THIS->audioRenderAction;

	//Get active context
//	this->context=alcGetCurrentContext();
	//Get device for active context
//	this->device=alcGetContextsDevice(Context);
	//Release context(s)
	alcDestroyContext(THIS->context);
	//Close device
	alcCloseDevice(THIS->device);

  delete THIS;
};

SbBool SoAudioDevice::init(const SbString &devicetype, const SbString &devicename)
{
  if (devicetype != "OpenAL")
  {
		SoDebugError::postWarning("SoAudioDevice::init",
                              "devicetype != OpenAL - currently OpenAL is the only supported device type for audio rendering");
    return FALSE;
  }

	//Open device
	THIS->device = alcOpenDevice((ALubyte*)devicename.getString());

	if (THIS->device == NULL)
	{
		SoDebugError::postWarning("SoAudioDevice::init",
                              "Failed to initialize OpenAL");
    return FALSE;
  }

	//Create context(s)
	THIS->context=alcCreateContext(THIS->device,NULL);
	//Set active context
	alcMakeContextCurrent(THIS->context);

	// Clear Error Code
	alGetError();
  
  return TRUE;
};
  
void SoAudioDevice::setSceneGraph(SoNode *root)
{
  THIS->root = root;
};

void SoAudioDevice::setGLRenderAction(SoGLRenderAction *ra)
{
  THIS->glRenderAction = ra;
};

void SoAudioDevice::enable()
{
  if (THIS->enabled)
    return; // allreaty enabled

  THIS->enabled = TRUE;

  THIS->glRenderAction->addPreRenderCallback(THIS->prerendercb, THIS);
};

void SoAudioDevice::disable()
{
  if (!THIS->enabled)
    return; // allready disabled
  
  THIS->enabled = FALSE;

  THIS->glRenderAction->removePreRenderCallback(THIS->prerendercb, THIS);

};

void SoAudioDeviceP::prerendercb(void * userdata, SoGLRenderAction * action)
{
  SoAudioDeviceP *thisp = (SoAudioDeviceP *) userdata;
  thisp->audioRenderAction->apply(thisp->root);
};
