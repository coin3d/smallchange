#include "SoAudioClipStreaming.h"

#include <Inventor/errors/SoDebugError.h>

#ifdef SOAL_SUB
#include <AL/al.h>
#else
#include <al.h>
#endif

#include "ALTools.h"

SO_NODE_SOURCE(SoAudioClipStreaming);

void SoAudioClipStreaming::initClass()
{
  SO_NODE_INIT_CLASS(SoAudioClipStreaming, SoNode, "Node");
};


SoAudioClipStreaming::SoAudioClipStreaming()
{
  SO_NODE_CONSTRUCTOR(SoAudioClipStreaming);

  this->asyncMode = FALSE;
  this->streamingBuffers = NULL;
  this->numBuffers = 0;
  setBufferInfo(0, 0);
};

SoAudioClipStreaming::~SoAudioClipStreaming()
{
  if (this->bufferId != NULL)
    delete[] this->streamingBuffers;
};

void SoAudioClipStreaming::setAsyncMode(SbBool flag)
{
  this->asyncMode = flag;
};

SbBool SoAudioClipStreaming::getAsyncMode()
{
  return this->asyncMode;
};

void SoAudioClipStreaming::setBufferInfo(int bufferSize, int numBuffers)
{
  //fixme: delete old buffers first (AL-buffers) - or du it in startPlaying
  this->bufferSize = bufferSize;
  this->numBuffers = numBuffers;
};


SbBool SoAudioClipStreaming::startPlaying(SbBool force)
{
	ALint	error;
  // fixme: delete old buffers
  this->streamingBuffers = new ALuint[this->numBuffers];
  alGenBuffers(this->numBuffers, this->streamingBuffers);
	if ((error = alGetError()) != AL_NO_ERROR)
  {
    char errstr[256];
		SoDebugError::postWarning("SoAudioClipStreaming::startPlaying",
                              "alGenBuffers failed. %s",
                              GetALErrorString(errstr, error));
    return FALSE;
  }

  int loop;
  short int *buf = new short int[this->bufferSize];
	for (loop = 0; loop < this->numBuffers; loop++)
	{
    this->fillBuffer(buf, this->bufferSize);
		alBufferData(this->streamingBuffers[loop], AL_FORMAT_MONO16, buf, this->bufferSize*sizeof(short int), 44100);
	  if ((error = alGetError()) != AL_NO_ERROR)
    {
      char errstr[256];
		  SoDebugError::postWarning("SoAudioClipStreaming::startPlaying",
                                "alBufferData failed. %s",
                                GetALErrorString(errstr, error));
      delete buf;
      return FALSE;
      // return;
    }
	}
  delete buf;
  return TRUE;
};

SbBool SoAudioClipStreaming::stopPlaying(SbBool force)
{
  return TRUE;
};


SbBool SoAudioClipStreaming::fillBuffer(void *buffer, int size)
{
  short int *ibuffer = (short int *)buffer;
  static double freq = 880.0;
  static int counter = 0;
  freq -=10.0;
  for (int i=0; i< size; i++)
  {
    if ((counter+i)%4100<2050)
      ibuffer[i] = 0.0;
    else
      ibuffer[i] = 32000.0*sin( ((float)i)/44100.0*2*3.14159265358979323846264383*freq);
  }
  counter+=size;

  return TRUE;
};
