#ifndef COIN_SOAUDIOCLIP_H
#define COIN_SOAUDIOCLIP_H

#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/fields/SoMFString.h>
#include <Inventor/fields/SoSFString.h>
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/fields/SoSFFloat.h>
#include <Inventor/fields/SoSFTime.h>


#ifdef SOAL_SUB
#include <AL/altypes.h>
#else
#include <altypes.h>
#endif

class SoAudioClip : public SoNode
{
  SO_NODE_HEADER(SoAudioClip);

  friend class SoSound;

public:

  static void initClass();
  SoAudioClip();

  SoSFString description;
  SoSFBool   loop;
  SoSFFloat  pitch;
  SoSFTime   startTime;
  SoSFTime   stopTime;
  SoMFString url;

  SoSFTime   duration_changed; //  eventOut
  SoSFBool   isActive; //  eventOut

protected:

  class SoFieldSensor * urlsensor;
  static void urlSensorCB(void *, SoSensor *);
  SbBool loadUrl(void); 

  unsigned int size;
  float frequency;
  ALuint	bufferId;
  int readstatus;
//  static ALuint buffers[SoAudioClip_MAXNUMBUFFERS]; 

  virtual ~SoAudioClip();

};

#endif // COIN_SOAUDIOCLIP_H