#include "SoAudioDevice.h"

#include <Inventor/errors/SoDebugError.h>


SoAudioDevice::SoAudioDevice()
{

  this->context = NULL;
	this->device = NULL;
  this->glRenderAction = NULL;
  this->audioRenderAction = NULL;
  this->enabled = FALSE;
  this->root = NULL;

  this->audioRenderAction = new SoAudioRenderAction();

};

SoAudioDevice::~SoAudioDevice()
{
  disable();

  if (this->audioRenderAction != NULL)
    delete this->audioRenderAction;

	//Get active context
//	this->context=alcGetCurrentContext();
	//Get device for active context
//	this->device=alcGetContextsDevice(Context);
	//Release context(s)
	alcDestroyContext(this->context);
	//Close device
	alcCloseDevice(this->device);
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
	this->device = alcOpenDevice((ALubyte*)devicename.getString());

	if (this->device == NULL)
	{
		SoDebugError::postWarning("SoAudioDevice::init",
                              "Failed to initialize OpenAL");
    return FALSE;
  }

	//Create context(s)
	this->context=alcCreateContext(this->device,NULL);
	//Set active context
	alcMakeContextCurrent(this->context);

	// Clear Error Code
	alGetError();
  
  return TRUE;
};
  
void SoAudioDevice::setSceneGraph(SoNode *root)
{
  this->root = root;
};

void SoAudioDevice::setGLRenderAction(SoGLRenderAction *ra)
{
  this->glRenderAction = ra;
};

void SoAudioDevice::enable()
{
  if (enabled)
    return; // allreaty enabled

  enabled = TRUE;

  glRenderAction->addPreRenderCallback(prerendercb, this);
};

void SoAudioDevice::disable()
{
  if (!enabled)
    return; // allready disabled
  
  enabled = FALSE;

  glRenderAction->removePreRenderCallback(prerendercb, this);

};

void SoAudioDevice::prerendercb(void * userdata, SoGLRenderAction * action)
{
  SoAudioDevice *thisp = (SoAudioDevice *) userdata;
  thisp->audioRenderAction->apply(thisp->root);
};

/*
  struct AudioRenderCallbackStruct
  {
    SoNode * root;
    SoAudioRenderAction *ara;
    AudioRenderCallbackStruct(SoNode * root=NULL, SoAudioRenderAction *ara=NULL)
    {
      init(root, ara);
    }
    void init(SoNode * root=NULL, SoAudioRenderAction *ara=NULL)
    {
      this->root = root; this->ara = ara;
    }
  };

  AudioRenderCallbackStruct callbackData;
*/



