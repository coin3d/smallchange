#ifndef COIN_SOAUDIOCLIPSTREAMING_H
#define COIN_SOAUDIOCLIPSTREAMING_H

#include <SmallChange/nodes/SoAudioClip.h>

class SoAudioClipStreaming : public SoAudioClip
{
  typedef SoAudioClip inherited;
  SO_NODE_HEADER(SoAudioClipStreaming);

  friend class SoSound;
public:
  static void initClass(void);
  SoAudioClipStreaming();

  void setAsyncMode(SbBool flag=FALSE);
  SbBool getAsyncMode();
  void setBufferInfo(int bufferSize, int numBuffers);
  void setSampleFormat(int channels = 1, int samplerate = 44100, int bitspersample=16);
  void getSampleFormat(int &channels, int &samplerate, int &bitspersample);
  int getNumChannels();
  int getBufferSize();
  int getNumBuffers();
  void setUserCallback(int (*user_callback)(void *buffer, int length, void * userdataptr),
    void *userdata=NULL);

protected:
	virtual ~SoAudioClipStreaming();

  virtual SbBool loadUrl(void); 
  virtual void unloadUrl(void);

protected:
  class SoAudioClipStreamingP *soaudioclipstreaming_impl;
  friend class SoAudioClipStreamingP;
  friend class SoSoundP;
};

#endif // COIN_SOAUDIOCLIPSTREAMING_H