#ifndef COIN_SOSOUNDBUFFER_H
#define COIN_SOSOUNDBUFFER_H

// 20010806 ThH: Unused so far, use SoSound instead

#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/fields/SoMFString.h>

#include <AL/altypes.h>

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