#include "SoAudioClip.h"

#include <Inventor/errors/SoDebugError.h>
#include <Inventor/sensors/SoFieldSensor.h>

#ifdef SOAL_SUB
#include <AL/al.h>
#include <AL/alut.h>
#else
#include <al.h>
#include <alut.h>
#endif

#include <string.h>

#include "ALTools.h"

SO_NODE_SOURCE(SoAudioClip);

//ALuint SoAudioClip = {};

void SoAudioClip::initClass()
{
  SO_NODE_INIT_CLASS(SoAudioClip, SoNode, "Node");
};

SoAudioClip::SoAudioClip()
{
  SO_NODE_CONSTRUCTOR(SoAudioClip);

  SO_NODE_ADD_FIELD(description, (""));
  SO_NODE_ADD_FIELD(loop, (FALSE));
  SO_NODE_ADD_FIELD(pitch, (1.0f));
  SO_NODE_ADD_FIELD(startTime, (0.0f));
  SO_NODE_ADD_FIELD(stopTime, (0.0f));
  SO_NODE_ADD_FIELD(url, (""));
  SO_NODE_ADD_FIELD(duration_changed, (0.0f)); //  eventOut
  SO_NODE_ADD_FIELD(isActive, (FALSE)); //  eventOut

  this->size = 0;
  this->frequency = 0;
  this->bufferId = 0; // no buffer (NULL), see alIsBuffer(...)
  this->readstatus = 0; // ?

  // use field sensor for url since we will load an image if
  // url changes. This is a time-consuming task which should
  // not be done in notify().
  this->urlsensor = new SoFieldSensor(urlSensorCB, this);
  this->urlsensor->setPriority(0);
  this->urlsensor->attach(&this->url);

};

SoAudioClip::~SoAudioClip()
{
  if (alIsBuffer(bufferId))
	  alDeleteBuffers(1, &bufferId);

  delete this->urlsensor;
};

SbBool SoAudioClip::loadUrl(void)
{
  // similar to SoTexture2::loadFilename()

  if (this->url.getNum() <1)
    return FALSE; // no url specified
  const char * str = this->url[0].getString();
  if ( (str == NULL) || (strlen(str)==0) )
    return FALSE; // url is blank

  SbStringList subdirectories;

  subdirectories.append(new SbString("samples"));

  SbString filename = SoInput::searchForFile(SbString(str), SoInput::getDirectories(), subdirectories);

  for (int i = 0; i < subdirectories.getLength(); i++) {
    delete subdirectories[i];
  }


  ALint	error;

  // Delete previous buffer

  if (alIsBuffer(bufferId))
	  alDeleteBuffers(1, &bufferId);

  // Generate new buffer

  alGenBuffers(1, &bufferId);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
    char errstr[256];
		SoDebugError::postWarning("SoAudioClip::loadUrl",
                              "alGenBuffers failed. %s",
                              GetALErrorString(errstr, error));
		return FALSE;
	}

	ALsizei size,freq;
	ALenum	format;
	ALvoid	*data;
	ALboolean loop;

/*
  ALsizei format;
  ALsizei size;
  ALsizei bits;
  ALsizei freq;

  void *data; //  = (void *) new char[44100*10*2];
*/
	// Load .wav
  alutLoadWAVFile(const_cast<ALbyte *>(str), &format, &data, &size, &freq, &loop);
//	alutLoadWAV(str, &data, &format, &size, &bits, &freq);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
    char errstr[256];
		SoDebugError::postWarning("SoAudioClip::loadUrl",
                              "Couldn't load file %s. %s",
//                              text.getString(), 
                              str, 
                              GetALErrorString(errstr, error));
		return FALSE;
	}

	// Copy wav data into buffer
	alBufferData(bufferId, format, data, size, freq);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
    char errstr[256];
		SoDebugError::postWarning("SoAudioClip::GLRender",
                              "alBufferData failed for data read from file %s. %s",
//                              text.getString(),
                              str,
                              GetALErrorString(errstr, error));
		return FALSE;
	}

	// Unload .wav
	alutUnloadWAV(format,data,size,freq);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
    char errstr[256];
		SoDebugError::postWarning("SoAudioClip::GLRender",
                              "alutUnloadWAV failed for data read from file %s. %s",
//                              text.getString(),
                              str,
                              GetALErrorString(errstr, error));
		return FALSE;
	}

//  delete data; // 20010803 thh

  return TRUE;
};

//
// called when filename changes
//
void
SoAudioClip::urlSensorCB(void * data, SoSensor *)
{
  SoAudioClip * thisp = (SoAudioClip*) data;

//   printf("SoAudioClip::urlSensorCB()\n");

  if (thisp->url.getNum()>0)
  {
    const char * str = thisp->url[0].getString();
    if ( (str != NULL) && (strlen(str)>0) )
    {

      if (thisp->loadUrl())
      {
        thisp->readstatus = 1;
      }
      else
      {
        SoDebugError::postWarning("SoAudioClip::urlSensorCB",
                                  "Sound file could not be read: %s",
                                  str);
//                                  text.getString());
        thisp->readstatus = 0;
      }
    }
  }
}
