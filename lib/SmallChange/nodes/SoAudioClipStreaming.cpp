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

#ifndef _WIN32 // FIXME: hack to compile under Linux, pederb 2001-12-01
#define strcmpi strcasecmp
#endif

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

  THIS->riffFile = NULL;

  THIS->urlFileType = SoAudioClipStreamingP::AUDIO_UNKNOWN;
  this->setSampleFormat(1, 44100, 16);
  this->loop.setValue(FALSE); 

  THIS->currentUrlIndex = 0;
};

SoAudioClipStreaming::~SoAudioClipStreaming()
{
  // fixme: we cannot delete until the thread has stopped
  // which means the SoSound must be deleted first ...

#ifndef NDEBUG
  fprintf(stderr, "~SoAudioClipStreaming()\n");
#endif

  THIS->deleteAlBuffers();

// 20010821 thammer
//  if (this->soaudioclip_impl->bufferId != 0)
//    delete[] THIS->alBuffers;
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

void SoAudioClipStreamingP::deleteAlBuffers()
{
  if (this->alBuffers != NULL)
  {
    alDeleteBuffers(this->numBuffers, this->alBuffers);
    delete [] this->alBuffers;
    this->alBuffers = NULL;
  }
};

void SoAudioClipStreaming::setBufferInfo(int bufferSize, int numBuffers)
{
  THIS->deleteAlBuffers();

  THIS->bufferSize = bufferSize;
  THIS->numBuffers = numBuffers;
};

void SoAudioClipStreaming::setSampleFormat(int channels, int samplerate, int bitspersample)
{
  THIS->channels = channels;
  THIS->samplerate = samplerate;
  THIS->bitspersample = bitspersample;
};

void SoAudioClipStreaming::getSampleFormat(int &channels, int &samplerate, int &bitspersample)
{
  channels = THIS->channels;
  samplerate = THIS->samplerate;
  bitspersample = THIS->bitspersample;
};

int SoAudioClipStreaming::getNumChannels()
{
  return THIS->channels;
};


void SoAudioClipStreaming::setUserCallback(SbBool (*usercallback)(void *buffer, int length, void * userdataptr),
  void *userdata)
{
  THIS->usercallback = usercallback;
  THIS->userdata = userdata;
}


SbBool SoAudioClipStreamingP::startPlaying(SbBool force)
{
  fillBufferDone = FALSE;
	ALint	error;
  this->deleteAlBuffers();
/*
  20010821 thammer
  if (this->alBuffers != NULL)
  {
    alDeleteBuffers(this->alBuffers)
  }
*/
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

  ALenum	alformat = 0;;
  alformat = getALSampleFormat(this->channels, this->bitspersample);

  int loop;
  short int *buf = new short int[this->bufferSize * this->channels ];
	for (loop = 0; loop < this->numBuffers; loop++)
	{
    this->fillBuffer(buf, this->bufferSize);
//		alBufferData(this->alBuffers[loop], AL_FORMAT_MONO16, buf, this->bufferSize*sizeof(short int), 44100);

		alBufferData(this->alBuffers[loop], alformat, buf, this->bufferSize*sizeof(short int), this->samplerate);
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

  vorbis_info *vi = ov_info(&this->ovOvFile, -1);
  channels = vi->channels;
  samplerate = vi->rate;
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
  if (this->ovFile != NULL) {
    ov_clear(&this->ovOvFile);
    fclose(this->ovFile);
  }
  this->ovFile = NULL;
};

#endif // HAVE_OGGVORBIS

SbBool SoAudioClipStreamingP::openWaveFile(const char *filename)
{
  int size, format;
  this->riffFile = wave_open(filename, size, format, this->channels, this->samplerate, this->bitspersample);

  if (this->riffFile == NULL)
  {
		SoDebugError::postWarning("SoAudioClipStreamingP::openWaveFile",
                              "Couldn't open file '%s'",
                              filename);
    return FALSE;
  }
  else
    return TRUE; // OK

  return TRUE;
}

void SoAudioClipStreamingP::closeWaveFile()
{
  if (this->riffFile != NULL)
    wave_close(this->riffFile);
  this->riffFile = NULL;
};


SbBool SoAudioClipStreaming::loadUrl(void)
{
  THIS->currentUrlIndex = 0;
  return THIS->loadUrl(0);
/*  // similar to SoTexture2::loadFilename()
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

  if (filename.getLength() <= 0) {
    char errstr[256];
		SoDebugError::postWarning("SoAudioClipStreaming::loadUrl",
                              "File not found: '%s'",
                              filename.getString());
    return FALSE;
  }

  char ext[4];
  SbBool ret = getFileExtension(ext, filename.getString(), 4);

  if (strcmpi(ext, "ogg")==0) {
    THIS->urlFileType = SoAudioClipStreamingP::AUDIO_OGGVORBIS;
  }
  else if (strcmpi(ext, "wav")==0) {
    THIS->urlFileType = SoAudioClipStreamingP::AUDIO_WAVPCM;
  }
  else {
    THIS->urlFileType = SoAudioClipStreamingP::AUDIO_UNKNOWN;
  };


  if (THIS->urlFileType == SoAudioClipStreamingP::AUDIO_OGGVORBIS) {
#if HAVE_OGGVORBIS
    return THIS->openOggFile(filename.getString());
#else
		SoDebugError::postWarning("SoAudioClipStreaming::loadUrl",
                              "File is in the Ogg Vorbis file format, but Ogg Vorbis support is missing. '%s'",
                              filename.getString());
#endif // HAVE_OGGVORBIS
  } else if (THIS->urlFileType == SoAudioClipStreamingP::AUDIO_WAVPCM) {
    return THIS->openWaveFile(filename.getString());
  }
  else {
		SoDebugError::postWarning("SoAudioClipStreaming::loadUrl",
                              "File has unknown format. '%s'",
                              filename.getString());
  }

  return FALSE;
  */
};

void SoAudioClipStreaming::unloadUrl()
{
#if HAVE_OGGVORBIS
  if (THIS->urlFileType == SoAudioClipStreamingP::AUDIO_OGGVORBIS)
    THIS->closeOggFile();
#endif // HAVE_OGGVORBIS
  if (THIS->urlFileType == SoAudioClipStreamingP::AUDIO_WAVPCM)
    THIS->closeWaveFile();
};

SbBool SoAudioClipStreamingP::loadUrl(int index)
{
  // similar to SoTexture2::loadFilename()
  // similar to SoAudioClip::loadUrl()

  ITHIS->unloadUrl();

  if (ITHIS->url.getNum() <index)
  {
		SoDebugError::postWarning("SoAudioClipStreaming::loadUrl(index)",
                              "Invalid index %d. There are only %d urls in the url list",
                              index, ITHIS->url.getNum());
    return FALSE; // invalid index
  }
  const char * str = ITHIS->url[index].getString();
  if ( (str == NULL) || (strlen(str)==0) )
    return FALSE; // url is blank

  SbStringList subdirectories;

  subdirectories.append(new SbString("samples"));

  SbString filename = SoInput::searchForFile(SbString(str), SoInput::getDirectories(), subdirectories);

  for (int i = 0; i < subdirectories.getLength(); i++) {
    delete subdirectories[i];
  }

  if (filename.getLength() <= 0) {
    char errstr[256];
		SoDebugError::postWarning("SoAudioClipStreaming::loadUrl(index)",
                              "File not found: '%s'",
                              filename.getString());
    return FALSE;
  }

  char ext[4];
  SbBool ret = getFileExtension(ext, filename.getString(), 4);

  if (strcmpi(ext, "ogg")==0) {
    this->urlFileType = SoAudioClipStreamingP::AUDIO_OGGVORBIS;
  }
  else if (strcmpi(ext, "wav")==0) {
    this->urlFileType = SoAudioClipStreamingP::AUDIO_WAVPCM;
  }
  else {
    this->urlFileType = SoAudioClipStreamingP::AUDIO_UNKNOWN;
  };


  if (this->urlFileType == SoAudioClipStreamingP::AUDIO_OGGVORBIS) {
#if HAVE_OGGVORBIS
    return this->openOggFile(filename.getString());
#else
		SoDebugError::postWarning("SoAudioClipStreaming::loadUrl(index)",
                              "File is in the Ogg Vorbis file format, but Ogg Vorbis support is missing. '%s'",
                              filename.getString());
#endif // HAVE_OGGVORBIS
  } else if (this->urlFileType == SoAudioClipStreamingP::AUDIO_WAVPCM) {
    return this->openWaveFile(filename.getString());
  }
  else {
		SoDebugError::postWarning("SoAudioClipStreaming::loadUrl(index)",
                              "File has unknown format. '%s'",
                              filename.getString());
  }

  return FALSE;
};




SbBool SoAudioClipStreamingP::defaultCallback(void *buffer, int length)
{
  if (this->urlFileType == SoAudioClipStreamingP::AUDIO_OGGVORBIS) {
#if HAVE_OGGVORBIS
    if (this->ovFile == NULL)
      return FALSE; // file not opened successfully
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
#else
    return FALSE; // no ogg vorbis support
#endif // HAVE_OGGVORBIS
  } else if (this->urlFileType == SoAudioClipStreamingP::AUDIO_WAVPCM) {
    if (this->riffFile == NULL)
      return FALSE; // file not opened successfully
    int numread = 0;
    // fixme: read correct channels and bitspersample
    int size=length*this->bitspersample/8;
    // fixme: read wave buffer
    if (fillBufferDone)
    {
      // fill 
      for (int i=0; i<size; i++)
        ((char *)buffer)[i] = 0;
      if (numBuffersLeftToClear>0)
      {
        numBuffersLeftToClear--;
      }
      else
      {
        // try to load next url
        SbBool ret;
        this->currentUrlIndex++;
        if ( (this->currentUrlIndex >= ITHIS->url.getNum()) && (ITHIS->loop.getValue() == TRUE) )
          this->currentUrlIndex = 0; 
        if (this->currentUrlIndex < ITHIS->url.getNum()) {
          ret = loadUrl(this->currentUrlIndex);
          if (!ret)
            return FALSE;
          fillBufferDone = FALSE;
        }
        else
          return FALSE;

        printf("nbltc == 0\n");
      }
    }

    if (!fillBufferDone)
    {
      numread = wave_read(this->riffFile, buffer, size);

      if (numread != size)
      {
        // fill rest of buffer with zeros
        for (int i=numread; i<size; i++)
          ((char *)buffer)[i] = 0;
        this->fillBufferDone = TRUE;
        this->numBuffersLeftToClear = this->numBuffers * 2;
//        this->numBuffersLeftToClear = 20;
        printf("SoAudioClipStreaming::defaultCallback() : fillbuffer done\n");
      }
    }
  } else
    return FALSE; // unknown format

  return TRUE;
};


#endif // HAVE_OPENAL
