#ifndef COIN_SOAUDIOCLIPSTREAMING_H
#define COIN_SOAUDIOCLIPSTREAMING_H

#include <SmallChange/nodes/SoAudioClip.h>

class SoAudioClipStreaming : public SoAudioClip
{
  SO_NODE_HEADER(SoAudioClipStreaming);

  friend class SoSound;
public:
  static void initClass(void);
  SoAudioClipStreaming();

  void setAsyncMode(SbBool flag=FALSE);
  SbBool getAsyncMode();
  void setBufferInfo(int bufferSize, int numBuffers);
  int getBufferSize();
  int getNumBuffers();
  void setUserCallback(int (*user_callback)(void *buffer, int length, void * userdataptr),
    void *userdata=NULL);

protected:
	virtual ~SoAudioClipStreaming();

protected:
  class SoAudioClipStreamingP *soaudioclipstreaming_impl;
  friend class SoAudioClipStreamingP;
  friend class SoSoundP;
};

#endif // COIN_SOAUDIOCLIPSTREAMING_H