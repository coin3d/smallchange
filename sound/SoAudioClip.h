#ifndef COIN_SOAUDIOCLIP_H
#define COIN_SOAUDIOCLIP_H

#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/fields/SoMFString.h>
#include <Inventor/fields/SoSFString.h>
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/fields/SoSFFloat.h>
#include <Inventor/fields/SoSFTime.h>
#include <Inventor/SbTime.h>
#include <Inventor/lists/SbStringList.h>

#include <SmallChange/actions/SoAudioRenderAction.h>

class SoAudioClip : public SoNode
{
  typedef SoNode inherited;
  SO_NODE_HEADER(SoAudioClip);

  friend class SoAudioRenderAction;

public:

  typedef void *FillBufferCallback(int frameoffset, void *buffer, int numframes, int &channels, 
                                   void * userdataptr);

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

  static void  setSubdirectories(const SbList<SbString> &subdirectories);
  static const SbStringList & getSubdirectories();
  static void setDefaultPauseBetweenTracks(SbTime pause);
  static void setDefaultSampleRate(int samplerate);
  static void setDefaultIntroPause(SbTime pause);

  int getSampleRate();
  int getCurrentFrameOffset();
  void *fillBuffer(int frameoffset, void *buffer, int numframes, int &channels);

  void setFillBufferCallback(FillBufferCallback *callback, void *userdata=NULL);

protected:
  virtual void audioRender(SoAudioRenderAction *action);

protected:
  class SoAudioClipP *soaudioclip_impl;
  friend class SoAudioClipP;

  virtual ~SoAudioClip();

};

#endif // COIN_SOAUDIOCLIP_H
