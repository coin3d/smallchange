#ifndef COIN_SOAUDIOCLIPSTREAMING_H
#define COIN_SOAUDIOCLIPSTREAMING_H

#include "SoAudioClip.h"

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

protected:
	virtual ~SoAudioClipStreaming();
  virtual SbBool fillBuffer(void *buffer, int size);
  SbBool stopPlaying(SbBool force = FALSE);
  SbBool startPlaying(SbBool force = FALSE);

  SbBool asyncMode;
  ALuint *streamingBuffers;
  int bufferSize;
  int numBuffers;

};

#endif // COIN_SOAUDIOCLIPSTREAMING_H