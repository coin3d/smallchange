#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <SmallChange/nodes/SoAudioClip.h>
#include <SmallChange/nodes/SoAudioClipP.h>
#include <SmallChange/misc/ALTools.h>

#include <Inventor/errors/SoDebugError.h>

#include <string.h>


// fixme 20021005 thammer: remove later. this is from coin/include/nodes/SoSubNodeP.h
#define SO_NODE_INTERNAL_INIT_CLASS(_class_) \
  do { \
    const char * classname = SO__QUOTE(_class_); \
    PRIVATE_COMMON_INIT_CODE(_class_, &classname[2], &_class_::createInstance, inherited); \
  } while (0)


#undef THIS
#define THIS this->soaudioclip_impl

#undef ITHIS
#define ITHIS this->ifacep

SO_NODE_SOURCE(SoAudioClip);

SbStringList SoAudioClipP::subdirectories = SbStringList();
SbTime SoAudioClipP::pauseBetweenTracks = 2.0f;
SbTime SoAudioClipP::introPause = 0.0f;
int SoAudioClipP::defaultSampleRate = 44100;

void SoAudioClip::initClass()
{
  // 20011206 thammer, kept for debugging purposes
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

  this->isActive.setValue(FALSE);

  THIS->urlsensor = new SoFieldSensor(THIS->urlSensorCBWrapper, THIS);
  THIS->urlsensor->setPriority(0);
  THIS->urlsensor->attach(&this->url);

  THIS->loopsensor = new SoFieldSensor(THIS->loopSensorCBWrapper, THIS);
  THIS->loopsensor->setPriority(0);
  THIS->loopsensor->attach(&this->loop);

  THIS->startTimeSensor = new SoFieldSensor(THIS->startTimeSensorCBWrapper, THIS);
  THIS->startTimeSensor->setPriority(0);
  THIS->startTimeSensor->attach(&this->startTime);

  THIS->stopTimeSensor = new SoFieldSensor(THIS->stopTimeSensorCBWrapper, THIS);
  THIS->stopTimeSensor->setPriority(0);
  THIS->stopTimeSensor->attach(&this->stopTime);

  THIS->loop = FALSE;
  THIS->endOfFile = FALSE;
  THIS->stream = NULL;

  THIS->channels = 0;
  THIS->bitspersample = 0;

  THIS->currentPlaylistIndex = 0;
  THIS->playlistDirty = FALSE;

  THIS->sampleRate = SoAudioClipP::defaultSampleRate;

  this->setFillBufferCallback(THIS->internalFillBufferWrapper, THIS);

  THIS->actualStartTime = 0.0f;
  THIS->totalNumberOfFramesToPlay = 0;

}

SoAudioClip::~SoAudioClip()
{
#ifdef DEBUG_AUDIO
  fprintf(stderr, "~SoAudioClip()\n");
#endif

  THIS->unloadUrl();

  delete THIS->urlsensor;
  delete THIS->loopsensor;
  delete THIS->startTimeSensor;
  delete THIS->stopTimeSensor;
  delete THIS;
}

void SoAudioClip::setDefaultSampleRate(int samplerate)
{
  SoAudioClipP::defaultSampleRate = samplerate;
}

void SoAudioClip::setDefaultPauseBetweenTracks(SbTime pause)
{
  // fixme: use SbTime instead. use both default and node-specific. 20021007 thammer.
  SoAudioClipP::pauseBetweenTracks = pause;
};

void SoAudioClip::setDefaultIntroPause(SbTime pause)
{
  SoAudioClipP::introPause = pause;
}


int SoAudioClip::getSampleRate()
{
  return THIS->sampleRate;
}

int SoAudioClip::getCurrentFrameOffset()
{
  /* fixme 20021007 thammer. Implement this. It is needed to support more than one 
     sound node connected to each audioclip.
  */  
  return 0;
}

void SoAudioClipP::startPlaying()
{
#ifdef DEBUG_AUDIO
  printf("ac:start\n");
#endif
  this->currentPause = SoAudioClipP::introPause;
  this->currentPlaylistIndex = 0;
  this->endOfFile = FALSE;
  ITHIS->isActive.setValue(TRUE);
  this->actualStartTime = 0.0f; // will be set in fillBuffers
  this->totalNumberOfFramesToPlay = 0; // will be increased in fillBuffers
}

void SoAudioClipP::stopPlaying()
{
#ifdef DEBUG_AUDIO
  printf("ac:stop\n");
#endif
  ITHIS->isActive.setValue(FALSE);
  this->closeFile();
}

void SoAudioClip::setFillBufferCallback(FillBufferCallback *callback, void *userdata)
{
  THIS->fillBufferCallback = callback;
  THIS->fillBufferCallbackUserData = userdata;
}

void * SoAudioClip::fillBuffer(int frameoffset, void *buffer, int numframes, int &channels)
{
  assert (THIS->fillBufferCallback != NULL);

  SbMutexAutoLock autoLock(&THIS->syncmutex);
  if (THIS->actualStartTime == 0.0f)
    THIS->actualStartTime = SbTime::getTimeOfDay();
  void *ret;
  ret = THIS->fillBufferCallback(frameoffset, buffer, numframes, channels, 
                                  THIS->fillBufferCallbackUserData);
  if (ret != NULL) {
    THIS->totalNumberOfFramesToPlay += numframes;
  }

  return ret;
}

void * SoAudioClipP::internalFillBufferWrapper(int frameoffset, void *buffer, int numframes, 
                                               int &channels, void *userdata)
{
  SoAudioClipP *pthis = (SoAudioClipP *)userdata;
  return pthis->internalFillBuffer(frameoffset, buffer, numframes, channels);
};


void * SoAudioClipP::internalFillBuffer(int frameoffset, void *buffer, int numframes, int &channels)
{
  // 20021007 thammer note:
  // this method might be called from a thread different from the thread which created the
  // "main" Coin thread.

  /* FIXME: We should really support different sampling rates and bitspersample. 
     I think it should be the AudioClip's
     responsibility to resample if necessary. 20021007 thammer.
  */

  /* FIXME: Opening a file might take some CPU time, so we should perhaps try doing this
     in non-critical places. Such as when url changes. Perhaps we should even open
     multiple files when url changes. This _might_ improve the current problem we have
     with possible stuttering at the beginning of playing a buffer...
     20021007 thammer.
  */

  if (currentPause>0.0) {
    // deliver a zero'ed, mono buffer
    int outputsize = numframes * 1 * sizeof(int16_t);
    memset(buffer, 0, outputsize);
    // for (int i=0; i<outputsize; i++)
    //   ((char *)buffer)[i] = 0;
    currentPause -= (double)numframes / (double)SoAudioClipP::defaultSampleRate;
    channels = 1;
    return buffer;
  }

  if (this->playlistDirty) {
    this->playlistDirty = FALSE;
    this->closeFile();
    this->currentPlaylistIndex = 0;
  }

  if ( (!this->endOfFile) && (this->stream==NULL) ) {
    if ( this->loop && (this->currentPlaylistIndex >= this->playlist.getLength()) ) 
      this->currentPlaylistIndex = 0;
    int startindex = this->currentPlaylistIndex;
    SbBool ret = FALSE;
    while ( (!ret) && (this->currentPlaylistIndex < this->playlist.getLength()) ) {
      ret = openFile(this->currentPlaylistIndex);
      if (!ret) {
        this->currentPlaylistIndex++;
        if ( this->loop && (this->currentPlaylistIndex >= this->playlist.getLength()) &&
             (this->currentPlaylistIndex != startindex) ) 
          this->currentPlaylistIndex = 0;
      }
    } 

    if (!ret)
      this->endOfFile = TRUE;
  }

  if (this->endOfFile) {
    // deliver a zero'ed, mono buffer
    int outputsize = numframes * 1 * sizeof(int16_t);
    memset(buffer, 0, outputsize);
    channels=1;
    return NULL;
  }


  assert(this->stream!=NULL);

  assert(bitspersample == sizeof(int16_t) * 8);

  int inputsize = numframes * this->channels * this->bitspersample / 8;

  int numread = inputsize;
  s_stream_get_buffer(this->stream, buffer, &numread, NULL);

  if (numread != inputsize) {
    closeFile();
    // fill rest of buffer with zeros
    for (int i=numread; i<inputsize; i++)
      ((char *)buffer)[i] = 0;

    this->currentPlaylistIndex++;
    if ( (this->currentPlaylistIndex<this->playlist.getLength()) && this->loop )
      this->currentPause = SoAudioClipP::pauseBetweenTracks;
  }

  channels = this->channels;
  return buffer;
}


void SoAudioClipP::loadUrl()
{
  SbMutexAutoLock autoLock(&this->syncmutex);

  this->unloadUrl();

  for (int i=0; i<ITHIS->url.getNum(); i++) {
    const char * str = ITHIS->url[i].getString();
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

    this->playlist.append(filename);
  }
}

void SoAudioClipP::unloadUrl()
{
  this->playlistDirty = TRUE;
  this->playlist.truncate(0);
  this->closeFile();
}

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
  this->loadUrl();
}

//
// called when loop changes
//
void
SoAudioClipP::loopSensorCBWrapper(void * data, SoSensor *)
{
  SoAudioClipP * thisp = (SoAudioClipP*) data;
  thisp->loopSensorCB(NULL);
}

//
// called when loop changes
//
void
SoAudioClipP::loopSensorCB(SoSensor *)
{
  SbMutexAutoLock autoLock(&this->syncmutex);
  this->loop = ITHIS->loop.getValue();
}

//
// called when startTime changes
//
void
SoAudioClipP::startTimeSensorCBWrapper(void * data, SoSensor *)
{
  SoAudioClipP * thisp = (SoAudioClipP*) data;
  thisp->startTimeSensorCB(NULL);
}

//
// called when startTime changes
//
void
SoAudioClipP::startTimeSensorCB(SoSensor *)
{
  SbMutexAutoLock autoLock(&this->syncmutex);

  SbTime now = SbTime::getTimeOfDay();
  SbTime start = ITHIS->startTime.getValue();

  if (now>=start) {
    if (!ITHIS->isActive.getValue())
      this->startPlaying();
  }
}

//
// called when stopTime changes
//
void
SoAudioClipP::stopTimeSensorCBWrapper(void * data, SoSensor *)
{
  SoAudioClipP * thisp = (SoAudioClipP*) data;
  thisp->stopTimeSensorCB(NULL);
}

//
// called when stopTime changes
//
void
SoAudioClipP::stopTimeSensorCB(SoSensor *)
{
  SbMutexAutoLock autoLock(&this->syncmutex);

  SbTime now = SbTime::getTimeOfDay();
  SbTime start = ITHIS->startTime.getValue();
  SbTime stop = ITHIS->stopTime.getValue();

  if ( (now>=stop) && (stop>start) ) 
  {
    // we shouldn't be playing now
    if  (ITHIS->isActive.getValue()) 
      this->stopPlaying();
    return; 
  }
}

void SoAudioClip::setSubdirectories(const SbList<SbString> &subdirectories)
{
  int i;
  for (i = 0; i < SoAudioClipP::subdirectories.getLength(); i++) {
    delete SoAudioClipP::subdirectories[i];
  }
  for (i = 0; i < subdirectories.getLength(); i++) {
    SoAudioClipP::subdirectories.append(new SbString(subdirectories[i]));
  }
}

const SbStringList & SoAudioClip::getSubdirectories()
{
  return SoAudioClipP::subdirectories;
}

void SoAudioClip::audioRender(SoAudioRenderAction *action)
{
  SbMutexAutoLock autoLock(&THIS->syncmutex);

  SbTime now = SbTime::getTimeOfDay();
  SbTime start = this->startTime.getValue();
  SbTime stop = this->stopTime.getValue();

#if COIN_DEBUG && DEBUG_AUDIO // debug
  SbString start_str = start.format("%D %h %m %s");
  SbString stop_str = stop.format("%D %h %m %s");
  SbString now_str = now.format("%D %h %m %s");
#endif // debug

  // 20021007 thammer removed: if ( (now<start) || ( (now>=stop) && (stop>start)) )

  if ( (now>=stop) && (stop>start) ) 
  {
    // we shouldn't be playing now
    if  (this->isActive.getValue())
      THIS->stopPlaying();
    return; 
  }

  // ( (now<stop) || (stop<=start) )

  if (THIS->endOfFile == TRUE) {
    if  (this->isActive.getValue()) {
      // FIXME: perhaps add some additional slack, the size of one buffer? 20021008 thammer.
      if ( (now-THIS->actualStartTime) > 
           ((float)THIS->totalNumberOfFramesToPlay / (float)SoAudioClipP::defaultSampleRate) )
        // we have played through the clip once, so we shouldn't play anymore
        THIS->stopPlaying(); 
    }
    return; 
  }

  if (now>=start) {
    if (!this->isActive.getValue())
      THIS->startPlaying();
  }
}

SbBool SoAudioClipP::openFile(int playlistIndex)
{
  assert ( (playlistIndex<this->playlist.getLength()) && (playlistIndex>=0) );

  return this->openFile(this->playlist[playlistIndex].getString());
}


SbBool SoAudioClipP::openFile(const char *filename)
{
  closeFile();

  this->stream = s_stream_open(filename, NULL);
  if (this->stream == NULL) {
    SoDebugError::postWarning("SoAudioClipP::openFile",
                              "Couldn't open file '%s'",
                              filename);
    return FALSE;
  }

  s_params * params;
  params = s_stream_params(stream);

  this->channels = 0;
  this->bitspersample = 16;
  this->samplerate = 0;
  if (params != NULL) {
    s_params_get(params,
                 "channels", S_INTEGER_PARAM_TYPE, &this->channels, NULL);
    s_params_get(params,
                 "samplerate", S_INTEGER_PARAM_TYPE, &this->samplerate, NULL);
  }

  #ifdef DEBUG_AUDIO
  printf("Wave file '%s' opened successfully\n", filename);
  #endif

  return TRUE; // OK
}

void SoAudioClipP::closeFile()
{
  if (this->stream != NULL) {
    s_stream_close(this->stream);
    s_stream_destroy(this->stream);
    this->stream = NULL;
  }
}
