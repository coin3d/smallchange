#if HAVE_CONFIG_H 
#include <config.h>
#endif

#include <string.h>
#include <assert.h>

#if HAVE_OPENAL

#include <SmallChange/nodes/SoAudioClipStreaming.h>
#include <SmallChange/nodes/SoSoundP.h>

#include <Inventor/errors/SoDebugError.h>

#include <AL/al.h>

#include <SmallChange/misc/ALTools.h>
#include <SmallChange/nodes/SoAudioClipP.h>
#include <SmallChange/nodes/SoAudioClipStreamingP.h>

#undef THIS
#define THIS this->soaudioclipstreaming_impl

#undef ITHIS
#define ITHIS this->ifacep

#define SOAS_PAUSE_BETWEEN_TRACKS 5

#ifndef HAVE_STRCMPI
#ifdef HAVE_STRCASECMP
#define strcmpi strcasecmp
#endif // HAVE_STRCASECMP
#endif // HAVE_STRCMPI

SO_NODE_SOURCE(SoAudioClipStreaming);

void SoAudioClipStreaming::initClass()
{
  // 20011206 thammer, kept for future reference
  // SO_NODE_INIT_CLASS(SoAudioClipStreaming, SoNode, "Node");
  SO_NODE_INTERNAL_INIT_CLASS(SoAudioClipStreaming);
};


SoAudioClipStreaming::SoAudioClipStreaming()
{
  THIS = new SoAudioClipStreamingP(this);

  SO_NODE_CONSTRUCTOR(SoAudioClipStreaming);

//  THIS->asyncMode = FALSE;
  THIS->asyncMode = TRUE;
  THIS->alBuffers = NULL;
  this->setBufferInfo(44100/50, 9);
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

  THIS->currentPlaylistIndex = 0;
  this->setKeepAlive(TRUE);

#ifdef HAVE_PTHREAD
  pthread_mutex_init(&THIS->syncmutex, NULL);
#endif

  THIS->playlistDirty = FALSE;
};

SoAudioClipStreaming::~SoAudioClipStreaming()
{
  // fixme: we cannot delete until the thread has stopped
  // which means the SoSound must be deleted first ...

#ifdef DEBUG_AUDIO
  fprintf(stderr, "~SoAudioClipStreaming()\n");
#endif

  THIS->deleteAlBuffers();

#ifdef HAVE_PTHREAD
  pthread_mutex_destroy(&THIS->syncmutex);
#endif

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
  if (this->alBuffers != NULL) {
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
  fillerPause = FALSE;
  introPause = TRUE;
  numBuffersLeftToClear = this->numBuffers * 2;
  currentPlaylistIndex = 0;

  /*
  fixme 20011129 thammer. The use of intropause is just a temp solution to a problem
  with "hickups" at the beginning of playing a streaming buffer. This needs to be investigated
  further and fixed properly.
  */

  ALint  error;
  this->deleteAlBuffers();

#if 0
  // Kept for future reference until this code has been debugged properly
  /*
  20010821 thammer
  if (this->alBuffers != NULL) {
    alDeleteBuffers(this->alBuffers)
  }
*/
#endif
  // Create new buffers
  this->alBuffers = new ALuint[this->numBuffers];
  alGenBuffers(this->numBuffers, this->alBuffers);
  if ((error = alGetError()) != AL_NO_ERROR) {
    char errstr[256];
    SoDebugError::postWarning("SoAudioClipStreamingP::startPlaying",
                              "alGenBuffers failed. %s",
                              GetALErrorString(errstr, error));
    return FALSE;
  }

  // Fill buffer with data

  ALenum  alformat = 0;;
  alformat = getALSampleFormat(this->channels, this->bitspersample);

  int loop;
  short int *buf = new short int[this->bufferSize * this->channels ];
//  short int *buf = new short int[this->bufferSize ];
  for (loop = 0; loop < this->numBuffers; loop++) {
    this->fillBuffer(buf, this->bufferSize);

    alBufferData(this->alBuffers[loop], alformat, buf, this->bufferSize*sizeof(short int)*this->channels, this->samplerate);
    if ((error = alGetError()) != AL_NO_ERROR) {
      char errstr[256];
      SoDebugError::postWarning("SoAudioClipStreamingP::startPlaying",
                                "alBufferData failed. %s",
                                GetALErrorString(errstr, error));
      delete buf;
      return FALSE;
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
  if (this->ovFile == NULL) {
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
  channels = filechannels = vi->channels;
  samplerate = vi->rate;
  bitspersample = 16;

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
//  this->riffFile = wave_open(filename, size, format, this->filechannels, this->samplerate, this->bitspersample);
  this->riffFile = wave_open(filename, size, format, this->channels, this->samplerate, this->bitspersample);

  if (this->riffFile == NULL) {
    SoDebugError::postWarning("SoAudioClipStreamingP::openWaveFile",
                              "Couldn't open file '%s'",
                              filename);
    return FALSE;
  } 
  else {
    #ifdef DEBUG_AUDIO
    printf("Wave file '%s' opened successfully\n", filename);
    #endif

    return TRUE; // OK
  };

}

void SoAudioClipStreamingP::closeWaveFile()
{
  if (this->riffFile != NULL)
    wave_close(this->riffFile);
  this->riffFile = NULL;
};


SbBool SoAudioClipStreaming::loadUrl(void)
{
#if HAVE_PTHREAD
//  SbAutoLock autoLock(&(THIS->syncmutex)); // synchronize with callback
  SbAutoLock autoLock(&(SoSoundP::syncmutex)); // synchronize with callback
#endif

  this->unloadUrl();

  SoAudioClipStreamingP::PlaylistItem item;
  for (int i=0; i<this->url.getNum(); i++) {
    const char * str = this->url[i].getString();
    if ( (str == NULL) || (strlen(str)==0) )
      continue; // ignore empty url

    SbString filename = SoInput::searchForFile(SbString(str), SoInput::getDirectories(), 
      SoAudioClip::getSubdirectories());

    if (filename.getLength() <= 0) {
		  SoDebugError::postWarning("SoAudioClipStreaming::loadUrl(index)",
                                "File not found: '%s'",
                                filename.getString());
      continue; // ignore invalid file
    }

    item.filename = filename;

    char ext[4];
    SbBool ret = getFileExtension(ext, filename.getString(), 4);

    if (strcmpi(ext, "wav")==0) {
      item.filetype = SoAudioClipStreamingP::AUDIO_WAVPCM;
    }
#if HAVE_OGGVORBIS
    else if (strcmpi(ext, "ogg")==0) {
      item.filetype = SoAudioClipStreamingP::AUDIO_OGGVORBIS;
    }
#endif // HAVE_OGGVORBIS
    else {
      item.filetype = SoAudioClipStreamingP::AUDIO_UNKNOWN;
    };

    THIS->playlist.append(item);
  };

  return TRUE;
};

void SoAudioClipStreaming::unloadUrl()
{
  THIS->playlistDirty = TRUE;
  THIS->playlist.truncate(0);

  THIS->closeFiles();
};

SbBool SoAudioClipStreamingP::openFile(int playlistIndex)
{
  assert ( (playlistIndex<this->playlist.getLength()) && (playlistIndex>=0) );

  if ( !((playlistIndex<this->playlist.getLength()) && (playlistIndex>=0)) )
    return FALSE; // invalid index

  if (this->playlist[playlistIndex].filetype == SoAudioClipStreamingP::AUDIO_OGGVORBIS) {
#if HAVE_OGGVORBIS
    return this->openOggFile(this->playlist[playlistIndex].filename.getString());
#endif // HAVE_OGGVORBIS
  } else if (playlist[playlistIndex].filetype == SoAudioClipStreamingP::AUDIO_WAVPCM) {
    return this->openWaveFile(this->playlist[playlistIndex].filename.getString());
  }
  else {
    SoDebugError::postWarning("SoAudioClipStreaming::loadUrl(index)",
                              "File has unknown format. '%s'",
                              this->playlist[playlistIndex].filename.getString());
    return FALSE;
  }
  return FALSE;
}




SbBool SoAudioClipStreamingP::defaultCallback(void *buffer, int length)
{
#if HAVE_PTHREAD
//  SbAutoLock autoLock(&(this->syncmutex)); // synchronize with loadUrl
#endif

  int numread = 0;
  int size=length*this->bitspersample/8 * this->channels;
  SbBool ret;

  // fixme 20011201 thammer. only 16 bits supported. handle other formats gracefully

  if (introPause) {
    // deliver some empty buffers
    for (int i=0; i<size; i++)
      ((char *)buffer)[i] = 0;
    if (numBuffersLeftToClear>0)
      numBuffersLeftToClear--;
    else 
      introPause = FALSE;
    return TRUE;
  }

  if (this->playlistDirty) {
    this->playlistDirty = FALSE;
    this->closeFiles();
    this->currentPlaylistIndex = 0;

    if (this->playlist.getLength() > 0) {
      ret = openFile(this->currentPlaylistIndex);
      if (!ret) {
        if (!this->keepAlive)
          return FALSE;
        else
          fillerPause = TRUE;
      }
      else
        fillerPause = FALSE;
    }
    else
      fillerPause = TRUE;
  }

  if (fillerPause) {
    // deliver some empty buffers
    for (int i=0; i<size; i++)
      ((char *)buffer)[i] = 0;

    if (numBuffersLeftToClear>0) {
      numBuffersLeftToClear--;
    }
    else {
      // we've played the pause, now try loading next file (if any)
      this->currentPlaylistIndex++;
      if ( (this->currentPlaylistIndex >= this->playlist.getLength()) && (ITHIS->loop.getValue() == TRUE) )
        this->currentPlaylistIndex = 0; 
      if (this->currentPlaylistIndex < this->playlist.getLength()) {
        ret = openFile(this->currentPlaylistIndex);
        if (!ret) {
          if (!this->keepAlive)
            return FALSE;
        }
        else
          fillerPause = FALSE; // success, so start playing
      }
      else {
        if (!this->keepAlive)
          return FALSE; // all done ....
      }

#ifdef DEBUG_AUDIO
      // printf("nbltc == 0\n");
#endif
    }
    return TRUE;
  }
  else { // fillerPause
    if (this->playlist[this->currentPlaylistIndex].filetype == SoAudioClipStreamingP::AUDIO_OGGVORBIS) {
#if HAVE_OGGVORBIS
      if (this->ovFile == NULL) {
        if (!this->keepAlive)
          return FALSE; // file not opened successfully
        else {
          // try next file...
          // fixme 20011201 thammer, should really clear buffer too
          this->fillerPause = TRUE;
          this->numBuffersLeftToClear = 0;
        }
      }
      else {
        int ret;
        char *ptr = (char *)buffer;
        while (numread<size) {
          ret=ov_read(&this->ovOvFile, ptr+numread, size-numread, 0, 2, 1, &(this->ovCurrentSection));
          numread+=ret;
          if (ret == 0) {
            break;
          };
        };
      }
#if 0
        // 20011206 thammer, kept for future reference
/*        else
        { // assuming filechannels == 2
          short int *inbuf = new short int[size];
          char *ptr = (char *)inbuf;
          while (numread<size*2)
          {
            ret=ov_read(&this->ovOvFile, ptr+numread, size*2-numread, 0, 2, 1, &(this->ovCurrentSection));
            numread+=ret;
            if (ret == 0)
            {
              break;
            };
          };
          short int *outbuf = (short int *) ( ((char *)buffer)+1 );
          for (int i=0; i<size/2-4; i++)
            outbuf[i] = inbuf[i];
          delete[] inbuf;
        }
*/
        /*
        char *ptr = (char *)buffer;
        if (this->filechannels==2)
          ptr = new char[size*2];
        while (numread<size*this->filechannels)
        {
          ret=ov_read(&this->ovOvFile, ptr+numread, size*this->filechannels-numread, 0, 2, 1, &(this->ovCurrentSection));
          numread+=ret;
          if (ret == 0)
          {
            break;
          };
        };
        if (this->filechannels==2)
        {
          short int *org = (short int *)buffer;
          short int *newbuf = (short int *)ptr;
          for (int i=0; i<size; i++)

//            org[i] = (newbuf[i*2] + newbuf[i*2+1])/2.0;
          delete[] ptr;
        };
*/
#endif // 0
#endif // HAVE_OGGVORBIS
    } 
    else if (this->playlist[this->currentPlaylistIndex].filetype == SoAudioClipStreamingP::AUDIO_WAVPCM) {
      // fixme 20011201 thammer, check for keepalive
      if (this->riffFile == NULL)
        return FALSE; // file not opened successfully

      numread = wave_read(this->riffFile, buffer, size);
    }
    else
      return FALSE; // unknown format

    if (numread != size) {
      // fill rest of buffer with zeros
      for (int i=numread; i<size; i++)
        ((char *)buffer)[i] = 0;
      this->fillerPause = TRUE;
      this->numBuffersLeftToClear = this->numBuffers * SOAS_PAUSE_BETWEEN_TRACKS;
#ifdef DEBUG_AUDIO
      printf("SoAudioClipStreaming::defaultCallback() : fillerPause\n");
#endif
    }
  };

  return TRUE;
};

void SoAudioClipStreaming::setKeepAlive(SbBool alive)
{
  THIS->keepAlive = alive;
};

void SoAudioClipStreamingP::closeFiles()
{
#if HAVE_OGGVORBIS
  if (this->urlFileType == SoAudioClipStreamingP::AUDIO_OGGVORBIS)
    this->closeOggFile();
#endif // HAVE_OGGVORBIS
  if (this->urlFileType == SoAudioClipStreamingP::AUDIO_WAVPCM)
    this->closeWaveFile();
}

#endif // HAVE_OPENAL
