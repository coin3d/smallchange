#ifndef DOXYGEN_SKIP_THIS

#include <Inventor/sensors/SoFieldSensor.h>

class SoAudioClipP
{
public:
  SoAudioClipP(SoAudioClip * interfaceptr) : ifacep(interfaceptr) {};
  SoAudioClip *ifacep;

  unsigned int size;
  float frequency;
  ALuint	bufferId;
  int readstatus;
  SoFieldSensor * urlsensor;
  static void urlSensorCBWrapper(void *, SoSensor *);
  void urlSensorCB(SoSensor *);
};

#endif // DOXYGEN_SKIP_THIS
