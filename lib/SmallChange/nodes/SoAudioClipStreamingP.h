
#include "SmallChange/misc/SbAudioWorkerThread.h"

#include <AL/altypes.h>
#include <AL/al.h>

#if HAVE_OGGVORBIS

#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#endif // HAVE_OGGVORBIS

#include <SmallChange/misc/tariff.h>

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
  int bufferSize; // bytesize = buffersize*bits/8*channels
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

  riff_t *riffFile;

  SbBool openWaveFile(const char *filename);
  void closeWaveFile();

  int channels;
  int samplerate;
  int bitspersample;
  int filechannels;

  enum urlFileTypes { AUDIO_UNKNOWN = 0, AUDIO_WAVPCM, AUDIO_OGGVORBIS };
  urlFileTypes urlFileType;

  SbBool fillerPause;
  ALint numBuffersLeftToClear;
  SbBool introPause;

  SbBool keepAlive;

#ifdef HAVE_PTHREAD
  pthread_mutex_t syncmutex;
#endif

  struct PlaylistItem
  {
    SbString filename;
    urlFileTypes filetype;
    PlaylistItem(const SbString &filename = SbString(""), urlFileTypes filetype = AUDIO_UNKNOWN)
    { this->filename = filename; this->filetype = filetype; };
    PlaylistItem(const PlaylistItem &c)
    { operator=(c); };
    const PlaylistItem & operator= (const PlaylistItem &c)
    { filename = c.filename; filetype = c.filetype;  return *this; };
  };

  SbList<PlaylistItem> playlist;
  SbBool playlistDirty;

  int currentPlaylistIndex;
  SbBool openFile(int playlistIndex);
  void closeFiles();
};
