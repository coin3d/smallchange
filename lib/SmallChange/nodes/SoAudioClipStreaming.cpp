#if HAVE_CONFIG_H 
#include <config.h>
#endif

#include <string.h>

#if HAVE_OPENAL

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
//  SO_NODE_INIT_CLASS(SoAudioClipStreaming, SoNode, "Node");
  SO_NODE_INTERNAL_INIT_CLASS(SoAudioClipStreaming);
};


SoAudioClipStreaming::SoAudioClipStreaming()
{
  THIS = new SoAudioClipStreamingP(this);

  SO_NODE_CONSTRUCTOR(SoAudioClipStreaming);

//  THIS->asyncMode = FALSE;
  THIS->asyncMode = TRUE;
  THIS->alBuffers = NULL;
  this->setBufferInfo(44100/40, 7);
  THIS->usercallback = THIS->defaultCallbackWrapper;
  THIS->userdata = THIS;

#ifdef HAVE_OGGVORBIS
  THIS->ovFile = NULL;
  THIS->ovCurrentSection = 0;
#endif // HAVE_OGGVORBIS

  THIS->urlFileType = SoAudioClipStreamingP::AUDIO_UNKNOWN;
  this->loop.setValue(FALSE); 
};

SoAudioClipStreaming::~SoAudioClipStreaming()
{
  // fixme: we cannot delete until the thread has stopped
  // which means the SoSound must be deleted first ...

  printf("~SoAudioClipStreaming()\n");

  if (this->soaudioclip_impl->bufferId != 0)
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
  // Create new buffers
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

  // Fill buffer with data

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
  if (this->usercallback != 0)
    return this->usercallback(buffer, size, this->userdata);
  return FALSE;
}

SbBool SoAudioClipStreamingP::defaultCallbackWrapper(void *buffer, int length, void *userdata)
{
  SoAudioClipStreamingP *pthis = (SoAudioClipStreamingP *)userdata;
  return pthis->defaultCallback(buffer, length);
};

#if HAVE_OGGVORBIS

SbBool SoAudioClipStreamingP::openOggFile(const char *filename)
{
  this->ovFile = fopen(filename, "rb");
  if (this->ovFile == NULL)
  {
		SoDebugError::postWarning("SoAudioClipStreamingP::openOggFile",
                              "Couldn't open file '%s'",
                              filename);
    return FALSE;
  }

  if(ov_open(this->ovFile, &this->ovOvFile, NULL, 0) < 0) {
		SoDebugError::postWarning("SoAudioClipStreamingP::openOggFile",
                              "Input does not appear to be an Ogg bitstream, filename '%s'",
                              filename);
    return FALSE;
  }

/*
  {
    char **ptr=ov_comment(&vf,-1)->user_comments;
    vorbis_info *vi=ov_info(&vf,-1);
    while(*ptr){
      fprintf(stderr,"%s\n",*ptr);
      ++ptr;
    }
    fprintf(stderr,"\nBitstream is %d channel, %ldHz\n",vi->channels,vi->rate);
    fprintf(stderr,"\nDecoded length: %ld samples\n",
	    (long)ov_pcm_total(&vf,-1));
    fprintf(stderr,"Encoded by: %s\n\n",ov_comment(&vf,-1)->vendor);
  }
*/
  return TRUE;
}

void SoAudioClipStreamingP::closeOggFile()
{
  ov_clear(&this->ovOvFile);
  fclose(this->ovFile);
};

#endif // HAVE_OGGVORBIS

SbBool SoAudioClipStreaming::loadUrl(void)
{
  // similar to SoTexture2::loadFilename()
  // similar to SoAudioClip::loadUrl()

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

  // fixme: check the file extension

  THIS->urlFileType = SoAudioClipStreamingP::AUDIO_OGGVORBIS;

#if HAVE_OGGVORBIS
  if (THIS->urlFileType == SoAudioClipStreamingP::AUDIO_OGGVORBIS)
    return THIS->openOggFile(filename.getString());
#endif // HAVE_OGGVORBIS

  return FALSE;
};

void SoAudioClipStreaming::unloadUrl()
{
#if HAVE_OGGVORBIS
  if (THIS->urlFileType == SoAudioClipStreamingP::AUDIO_OGGVORBIS)
    THIS->closeOggFile();
#endif // HAVE_OGGVORBIS
};

SbBool SoAudioClipStreamingP::defaultCallback(void *buffer, int length)
{
#if HAVE_OGGVORBIS
  if (this->urlFileType == SoAudioClipStreamingP::AUDIO_OGGVORBIS) {
    int numread = 0;
    int ret;
    char *ptr = (char *)buffer;
    int size = length*2;
    while (numread<size)
    {
      ret=ov_read(&this->ovOvFile, ptr+numread, size-numread, 0, 2, 1, &(this->ovCurrentSection));
      numread+=ret;
      if (ret == 0)
        return FALSE;
    };
  };
#endif // HAVE_OGGVORBIS
  return TRUE;
};


#endif // HAVE_OPENAL
