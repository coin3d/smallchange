

#include <AL/altypes.h>
#include <AL/al.h>

#if HAVE_OGGVORBIS

#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#endif // HAVE_OGGVORBIS

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
  ALuint *alBuffers;
  int bufferSize;
  int numBuffers;

  SbBool (*usercallback)(void *buffer, int length, void * userdataptr);
  static SbBool defaultCallbackWrapper(void *buffer, int length, void *userdata);
  SbBool defaultCallback(void *buffer, int length);
  void *userdata;

#if HAVE_OGGVORBIS
  FILE *ovFile;
  OggVorbis_File ovOvFile;
  int ovCurrentSection;

  SbBool openOggFile(const char *filename);
  void closeOggFile();
#endif // HAVE_OGGVORBIS

  enum urlFileTypes { AUDIO_UNKNOWN = 0, AUDIO_WAVPCM, AUDIO_OGGVORBIS };
  urlFileTypes urlFileType;

};