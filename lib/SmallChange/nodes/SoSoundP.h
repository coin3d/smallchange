#include <AL/altypes.h>

#include <SmallChange/misc/SbAudioWorkerThread.h>

class SoSoundP
{
public:
  SoSoundP(SoSound * interfaceptr) : ifacep(interfaceptr) {};
  SoSound *ifacep;

  static void sourceSensorCBWrapper(void *, SoSensor *);
  void sourceSensorCB(SoSensor *);

  SbBool stopPlaying(SbBool force = FALSE);
  SbBool startPlaying(SbBool force = FALSE);

  class SoFieldSensor * sourcesensor;
  ALuint sourceId;
  class SoAudioClip *currentAudioClip;
  SbBool isStreaming;
  SbBool asyncStreamingMode;

  static void timercb(void * data, SoSensor *);
  SoTimerSensor * timersensor;
  SbAudioWorkerThread *workerThread;
  short int *audioBuffer;

  static int threadCallbackWrapper(void *userdata);
  int threadCallback();
  int fillBuffers();

  SbTime actualStartTime;

#ifdef HAVE_PTHREAD
  pthread_mutex_t syncmutex;
#endif
};
