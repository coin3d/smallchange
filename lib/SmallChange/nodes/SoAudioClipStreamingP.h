

#ifdef SOAL_SUB
#include <AL/altypes.h>
#include <AL/al.h>
#include <AL/alut.h>
#else
#include <al.h>
#include <alut.h>
#include <altypes.h>
#endif

class SoAudioClipStreamingP
{
public:
  SoAudioClipStreamingP(SoAudioClipStreaming * interfaceptr) : ifacep(interfaceptr) {};
  SoAudioClipStreaming *ifacep;

  // fixme: fillbuffer should probably be in SoSudioClipStreaming, and subclassed
  // - or (probably better because of the use of coin typeinfo in sosound)
  // user registers a callback for filling the streaming buffer....
  virtual SbBool fillBuffer(void *buffer, int size);
  SbBool stopPlaying(SbBool force = FALSE);
  SbBool startPlaying(SbBool force = FALSE);

  SbBool asyncMode;
  ALuint *streamingBuffers;
  int bufferSize;
  int numBuffers;

  SbBool (*usercallback)(void *buffer, int length, void * userdataptr);
  void *userdata;
};