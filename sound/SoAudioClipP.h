#ifndef DOXYGEN_SKIP_THIS

#include <Inventor/sensors/SoFieldSensor.h>
#include <Inventor/threads/SbMutex.h>

#include <simage.h>

class SoAudioClipP 
{
public:
  SoAudioClipP(SoAudioClip * interfaceptr) : ifacep(interfaceptr) {};
  SoAudioClip *ifacep;

  static void urlSensorCBWrapper(void *, SoSensor *);
  void urlSensorCB(SoSensor *);

  static void loopSensorCBWrapper(void *, SoSensor *);
  void loopSensorCB(SoSensor *);

  static void startTimeSensorCBWrapper(void *, SoSensor *);
  void startTimeSensorCB(SoSensor *);

  static void stopTimeSensorCBWrapper(void *, SoSensor *);
  void stopTimeSensorCB(SoSensor *);

  void * internalFillBuffer(int frameoffset, void *buffer, 
                            int numframes, int &channels);

  SoAudioClip::FillBufferCallback *fillBufferCallback;
  static void * internalFillBufferWrapper(int frameoffset, void *buffer, 
                                          int numframes, int &channels, 
                                          void *userdata);

  void loadUrl(void); 
  void unloadUrl(void);

  void startPlaying();
  void stopPlaying();
  
  void *fillBufferCallbackUserData;

  SoFieldSensor * urlsensor;
  SoFieldSensor * loopsensor;
  SoFieldSensor * startTimeSensor;
  SoFieldSensor * stopTimeSensor;

  static SbStringList subdirectories;
  static SbTime pauseBetweenTracks;
  static SbTime introPause;
  static int defaultSampleRate;

  int sampleRate;

  SbTime currentPause;

  SbBool openFile(int playlistIndex);
  SbBool openFile(const char *filename);
  void closeFile();

  s_stream * stream;

  int channels;
  int samplerate;
  int bitspersample;

  SbList<SbString> playlist;
  SbBool playlistDirty;
  int currentPlaylistIndex;

  SbBool loop;
  SbBool endOfFile;

  SbMutex syncmutex;

  SbTime actualStartTime;
  int totalNumberOfFramesToPlay;

};

#endif // DOXYGEN_SKIP_THIS
