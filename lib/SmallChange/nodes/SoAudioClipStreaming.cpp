#include "SoAudioClipStreaming.h"

#include <Inventor/errors/SoDebugError.h>

#ifdef SOAL_SUB
#include <AL/al.h>
#else
#include <al.h>
#endif

#include "ALTools.h"
#include "SoAudioClipP.h"
#include "SoAudioClipStreamingP.h"

#undef THIS
#define THIS this->soaudioclipstreaming_impl

#undef ITHIS
#define ITHIS this->ifacep


SO_NODE_SOURCE(SoAudioClipStreaming);

void SoAudioClipStreaming::initClass()
{
  SO_NODE_INIT_CLASS(SoAudioClipStreaming, SoNode, "Node");
};


SoAudioClipStreaming::SoAudioClipStreaming()
{
  THIS = new SoAudioClipStreamingP(this);

  SO_NODE_CONSTRUCTOR(SoAudioClipStreaming);

  THIS->asyncMode = FALSE;
  THIS->streamingBuffers = NULL;
  THIS->numBuffers = 0;
  this->setBufferInfo(0, 0);
};

SoAudioClipStreaming::~SoAudioClipStreaming()
{
  if (this->soaudioclip_impl->bufferId != NULL)
    delete[] THIS->streamingBuffers;
  delete THIS;
};

void SoAudioClipStreaming::setAsyncMode(SbBool flag)
{
  THIS->asyncMode = flag;
};

SbBool SoAudioClipStreaming::getAsyncMode()
{
  return THIS->asyncMode;
};

int SoAudioClipStreaming::getBufferSize()
{
  return THIS->bufferSize;
};

int SoAudioClipStreaming::getNumBuffers()
{
  return THIS->numBuffers;
};

void SoAudioClipStreaming::setBufferInfo(int bufferSize, int numBuffers)
{
  //fixme: delete old buffers first (AL-buffers) - or du it in startPlaying
  THIS->bufferSize = bufferSize;
  THIS->numBuffers = numBuffers;
};


SbBool SoAudioClipStreamingP::startPlaying(SbBool force)
{
	ALint	error;
  // fixme: delete old buffers
  this->streamingBuffers = new ALuint[this->numBuffers];
  alGenBuffers(this->numBuffers, this->streamingBuffers);
	if ((error = alGetError()) != AL_NO_ERROR)
  {
    char errstr[256];
		SoDebugError::postWarning("SoAudioClipStreamingP::startPlaying",
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
		  SoDebugError::postWarning("SoAudioClipStreamingP::startPlaying",
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

SbBool SoAudioClipStreamingP::stopPlaying(SbBool force)
{
  return TRUE;
};


SbBool SoAudioClipStreamingP::fillBuffer(void *buffer, int size)
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
