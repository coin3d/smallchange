#if HAVE_CONFIG_H
#include <config.h>
#endif

#if HAVE_OPENAL

#include <SmallChange/nodes/SoAudioClip.h>

#include <Inventor/errors/SoDebugError.h>

#include <AL/altypes.h>
#include <AL/al.h>

#include <string.h>

#include <SmallChange/misc/ALTools.h>
#include <SmallChange/nodes/SoAudioClipP.h>

#undef THIS
#define THIS this->soaudioclip_impl

#undef ITHIS
#define ITHIS this->ifacep

SO_NODE_SOURCE(SoAudioClip);

//ALuint SoAudioClip = {};

void SoAudioClip::initClass()
{
  // SO_NODE_INIT_CLASS(SoAudioClip, SoNode, "Node");
  SO_NODE_INTERNAL_INIT_CLASS(SoAudioClip);
};

SoAudioClip::SoAudioClip()
{
  THIS = new SoAudioClipP(this);

  SO_NODE_CONSTRUCTOR(SoAudioClip);

  SO_NODE_ADD_FIELD(description, (""));
  SO_NODE_ADD_FIELD(loop, (FALSE));
  SO_NODE_ADD_FIELD(pitch, (1.0f));
  SO_NODE_ADD_FIELD(startTime, (0.0f));
  SO_NODE_ADD_FIELD(stopTime, (0.0f));
  SO_NODE_ADD_FIELD(url, (""));
  SO_NODE_ADD_FIELD(duration_changed, (0.0f)); //  eventOut
  SO_NODE_ADD_FIELD(isActive, (FALSE)); //  eventOut

  THIS->size = 0;
  THIS->bufferId = 0; // no buffer (NULL), see alIsBuffer(...)
  THIS->readstatus = 0; // ?

  // use field sensor for url since we will load an image if
  // url changes. This is a time-consuming task which should
  // not be done in notify().
  THIS->urlsensor = new SoFieldSensor(THIS->urlSensorCBWrapper, THIS);
  THIS->urlsensor->setPriority(0);
  THIS->urlsensor->attach(&this->url);

};

SoAudioClip::~SoAudioClip()
{
#ifndef NDEBUG
  fprintf(stderr, "~SoAudioClip()\n");
#endif
  if (alIsBuffer(THIS->bufferId))
	  alDeleteBuffers(1, &THIS->bufferId);

  this->unloadUrl();

  delete THIS->urlsensor;
  delete THIS;
};

SbBool SoAudioClip::setBuffer(void *buffer, int length, int channels, int bitspersample, int samplerate)
{
  // for setting data directly (doesn't use url)

  ALint	error;

  // Delete previous buffer

  if (alIsBuffer(THIS->bufferId))
	  alDeleteBuffers(1, &THIS->bufferId);

  // Generate new buffer

  alGenBuffers(1, &THIS->bufferId);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
    char errstr[256];
		SoDebugError::postWarning("SoAudioClip::setBuffer",
                              "alGenBuffers failed. %s",
                              GetALErrorString(errstr, error));
		return FALSE;
	}

	ALenum	alformat = 0;;

  alformat = getALSampleFormat(channels, bitspersample);


	// Copy wav data into buffer
	alBufferData(THIS->bufferId, alformat, (ALvoid *)buffer, length*bitspersample/8*channels,
    samplerate);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
    char errstr[256];
		SoDebugError::postWarning("SoAudioClip::setBuffer",
                              "alBufferData failed for data read from file. %s",
                              GetALErrorString(errstr, error));
		return FALSE;
	}

  return TRUE;
};

SbBool SoAudioClip::loadUrl(void)
{
  // similar to SoTexture2::loadFilename()

  this->unloadUrl();

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

  if (alIsBuffer(THIS->bufferId))
	  alDeleteBuffers(1, &THIS->bufferId);

  // Generate new buffer

  alGenBuffers(1, &THIS->bufferId);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
    char errstr[256];
		SoDebugError::postWarning("SoAudioCli::loadUrl",
                              "alGenBuffers failed. %s",
                              GetALErrorString(errstr, error));
		return FALSE;
	}

  SbBool ret;
  char *buffer;
  int size;
  int format;
  int channels;
  int samplerate;
  int bitspersample;
    
  ret = readWaveFile(filename.getString(), buffer, size, format, 
                    channels, samplerate, bitspersample);
  if (!ret) {
		SoDebugError::postWarning("SoAudioClip::loadUrl",
                              "Couldn't load file %s.",
                              filename.getString());
    return FALSE;
  }

  ALsizei alsize = size;
  ALsizei alfreq = samplerate;
	ALvoid	*aldata = (ALvoid	*)buffer;
	ALenum	alformat = 0;;

  if (format == 1)
    alformat = getALSampleFormat(channels, bitspersample);


	// Copy wav data into buffer
	alBufferData(THIS->bufferId, alformat, aldata, alsize, alfreq);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
    char errstr[256];
		SoDebugError::postWarning("SoAudioClip::loadUrl",
                              "alBufferData failed for data read from file %s. %s",
//                              text.getString(),
                              str,
                              GetALErrorString(errstr, error));
		return FALSE;
	}

  freeWaveDataBuffer(buffer);

/*
  ALsizei size,freq;
	ALenum	format;
	ALvoid	*data;
	ALboolean loop;

  // fixme: use filename.getString() instead of str

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
	alBufferData(THIS->bufferId, format, data, size, freq);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
    char errstr[256];
		SoDebugError::postWarning("SoAudioClip::loadUrl",
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
		SoDebugError::postWarning("SoAudioClipP::LoadUrl",
                              "alutUnloadWAV failed for data read from file %s. %s",
//                              text.getString(),
                              str,
                              GetALErrorString(errstr, error));
		return FALSE;
	}
*/
//  delete data; // 20010803 thh

  return TRUE;
};

void SoAudioClip::unloadUrl()
{
};

//
// called when filename changes
//
void
SoAudioClipP::urlSensorCBWrapper(void * data, SoSensor *)
{
  SoAudioClipP * thisp = (SoAudioClipP*) data;
  thisp->urlSensorCB(NULL);
}

//
// called when filename changes
//
void
SoAudioClipP::urlSensorCB(SoSensor *)
{
//   printf("SoAudioClip::urlSensorCB()\n");

  if (ITHIS->url.getNum()>0)
  {
    const char * str = ITHIS->url[0].getString();
    if ( (str != NULL) && (strlen(str)>0) )
    {

      if (ITHIS->loadUrl())
      {
        this->readstatus = 1;
      }
      else
      {
        SoDebugError::postWarning("SoAudioClipP::urlSensorCB",
                                  "Sound file could not be read: %s",
                                  str);
//                                  text.getString());
        this->readstatus = 0;
      }
    }
  }
}

#endif // HAVE_OPENAL
