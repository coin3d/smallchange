#include <SmallChange/nodes/SoAudioClipStreaming.h>

#include <Inventor/errors/SoDebugError.h>

#include <AL/al.h>

#include <SmallChange/misc/ALTools.h>
#include <SmallChange/nodes/SoAudioClipP.h>
#include <SmallChange/nodes/SoAudioClipStreamingP.h>

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
  THIS->alBuffers = NULL;
  THIS->numBuffers = 0;
  this->setBufferInfo(0, 0);
  THIS->usercallback = NULL;
  THIS->userdata = NULL;
};

SoAudioClipStreaming::~SoAudioClipStreaming()
{
  // fixme: we cannot delete until the thread has stopped
  // which means the SoSound must be deleted first ...

  printf("~SoAudioClipStreaming()\n");

  if (this->soaudioclip_impl->bufferId != NULL)
    delete[] THIS->alBuffers;
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

void SoAudioClipStreaming::setUserCallback(SbBool (*usercallback)(void *buffer, int length, void * userdataptr),
  void *userdata)
{
  THIS->usercallback = usercallback;
  THIS->userdata = userdata;
}


SbBool SoAudioClipStreamingP::startPlaying(SbBool force)
{
	ALint	error;
  // fixme: delete old buffers
  this->alBuffers = new ALuint[this->numBuffers];
  alGenBuffers(this->numBuffers, this->alBuffers);
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
		alBufferData(this->alBuffers[loop], AL_FORMAT_MONO16, buf, this->bufferSize*sizeof(short int), 44100);
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
  if (this->usercallback != NULL)
    return this->usercallback(buffer, size, this->userdata);
};
