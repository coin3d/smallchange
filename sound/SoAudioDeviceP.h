
#include <Inventor/SbBasic.h>

#include <AL/al.h>
#include <AL/alc.h>

class SoAudioDeviceP
{
public:
  SoAudioDeviceP(SoAudioDevice * interfaceptr) : ifacep(interfaceptr) {};
  SoAudioDevice *ifacep;

  static void prerendercb(void * userdata, SoGLRenderAction * action);

  void *context;
  ALCdevice *device;
  SoGLRenderAction *glRenderAction;
  SoAudioRenderAction *audioRenderAction;
  SoNode *root;

  SbBool enabled;
  SbBool initOK;

};
