#ifndef COIN_SOAUDIOCLIP_H
#define COIN_SOAUDIOCLIP_H

#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/fields/SoMFString.h>
#include <Inventor/fields/SoSFString.h>
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/fields/SoSFFloat.h>
#include <Inventor/fields/SoSFTime.h>

class SoAudioClip : public SoNode
{
  typedef SoNode inherited;
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

  SbBool setBuffer(void *buffer, int length, int channels, int bitspersample, int samplerate);

protected:

  virtual ~SoAudioClip();

  virtual SbBool loadUrl(void); 
  virtual void unloadUrl(void);


protected:
	class SoAudioClipP *soaudioclip_impl;
	friend class SoAudioClipP;
  friend class SoSoundP;
};

#endif // COIN_SOAUDIOCLIP_H