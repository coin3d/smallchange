#ifndef COIN_SOSOUNDBUFFER_H
#define COIN_SOSOUNDBUFFER_H

#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/fields/SoMFString.h>

#ifdef SOAL_SUB
#include <AL/altypes.h>
#else
#include <AL/altypes.h>
#endif

class SoSoundBuffer : public SoNode
{
  SO_NODE_HEADER(SoSoundBuffer);

  friend class SoSoundSource;

public:

  static void initClass();
  SoSoundBuffer();

  SoMFString url; // only filename supported at this time

protected:

  class SoFieldSensor * urlsensor;
  static void urlSensorCB(void *, SoSensor *);
  SbBool loadUrl(void); 

  unsigned int size;
  float frequency;
  ALuint	bufferId;
  int readstatus;
//  static ALuint buffers[SOSOUNDBUFFER_MAXNUMBUFFERS]; 

private:

  virtual ~SoSoundBuffer();

};

#endif // COIN_SOSOUNDBUFFER_H