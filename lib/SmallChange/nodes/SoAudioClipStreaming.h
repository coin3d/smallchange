#ifndef COIN_SOAUDIOCLIPSTREAMING_H
#define COIN_SOAUDIOCLIPSTREAMING_H

#include <nodes/SoAudioClip.h>

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

protected:
	virtual ~SoAudioClipStreaming();

protected:
  class SoAudioClipStreamingP *soaudioclipstreaming_impl;
  friend class SoAudioClipStreamingP;
  friend class SoSoundP;
};

#endif // COIN_SOAUDIOCLIPSTREAMING_H