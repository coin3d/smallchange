

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

  virtual SbBool fillBuffer(void *buffer, int size);
  SbBool stopPlaying(SbBool force = FALSE);
  SbBool startPlaying(SbBool force = FALSE);

  void deleteAlBuffers();

  SbBool asyncMode;
  ALuint *alBuffers;
  int bufferSize;
  int numBuffers;

  SbBool (*usercallback)(void *buffer, int length, void * userdataptr);
  static SbBool defaultCallbackWrapper(void *buffer, int length, void *userdata);
  SbBool defaultCallback(void *buffer, int length);
  void *userdata;

#ifdef HAVE_OGGVORBIS
  FILE *ovFile;
  OggVorbis_File ovOvFile;
  int ovCurrentSection;

  SbBool openOggFile(const char *filename);
  void closeOggFile();
#endif // HAVE_OGGVORBIS

  int channels;
  int samplerate;
  int bitspersample;

  enum urlFileTypes { AUDIO_UNKNOWN = 0, AUDIO_WAVPCM, AUDIO_OGGVORBIS };
  urlFileTypes urlFileType;

};
