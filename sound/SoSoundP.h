#include <AL/altypes.h>

#include <Inventor/threads/SbMutex.h>
#include <Inventor/C/threads/thread.h>
#include <Inventor/SbTime.h>
#include <Inventor/sensors/SoSensor.h>

class SoTimerSensor;

class SoSoundP
{
public:
  SoSoundP(SoSound * interfaceptr) : ifacep(interfaceptr) {};
  SoSound *ifacep;

  static void sourceSensorCBWrapper(void *, SoSensor *);
  void sourceSensorCB(SoSensor *);

  SbBool stopPlaying();
  SbBool startPlaying();

  static void timercb(void * data, SoSensor *);

  static void * threadCallbackWrapper(void *userdata);
  void * threadCallback();
  void fillBuffers();

  void deleteAlBuffers();


  class SoFieldSensor * sourcesensor;
  ALuint sourceId;
  class SoAudioClip *currentAudioClip;
  SbBool playing;
  SbBool useTimerCallback;

  SoTimerSensor * timersensor;
  cc_thread *workerThread;
  volatile SbBool exitthread;
  volatile SbBool errorInThread;

  static SbMutex *syncmutex;

  int16_t *audioBuffer;
  int channels;
  SbTime sleepTime;
  SbTime workerThreadSleepTime;

  static int defaultBufferLength;
  static int defaultNumBuffers;
  static SbTime defaultSleepTime;

  ALuint *alBuffers;
  int bufferLength; // bytesize = bufferLength*bitspersample/8*channels
  int numBuffers;

};
