
#ifdef SOAL_SUB
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>
#else
#include <al.h>
#include <alc.h>
#include <alut.h>
#endif

class SoAudioDeviceP
{
public:
  SoAudioDeviceP(SoAudioDevice * interfaceptr) : ifacep(interfaceptr) {};
  SoAudioDevice *ifacep;

  static void prerendercb(void * userdata, SoGLRenderAction * action);

//	ALCcontext *context; 20010803 thh
	void *context;
	ALCdevice *device;
  SoGLRenderAction *glRenderAction;
  SoAudioRenderAction *audioRenderAction;
  SoNode *root;

  bool enabled;

};