#ifndef SMALLCHANGE_SOAUDIOCLIP_H
#define SMALLCHANGE_SOAUDIOCLIP_H

/**************************************************************************\
 *
 *  This file is part of the SmallChange extension library for Coin.
 *  Copyright (C) 1998-2002 by Systems in Motion.  All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  version 2 as published by the Free Software Foundation.  See the
 *  file LICENSE.GPL at the root directory of this source distribution
 *  for more details.
 *
 *  If you desire to use SmallChange with software that is incompatible
 *  licensewise with the GPL, and / or you would like to take
 *  advantage of the additional benefits with regard to our support
 *  services, please contact Systems in Motion about acquiring a Coin
 *  Professional Edition License.  See <URL:http://www.coin3d.org> for
 *  more information.
 *
 *  Systems in Motion, Prof Brochs gate 6, 7030 Trondheim, NORWAY
 *  <URL:http://www.sim.no>, <mailto:support@sim.no>
 *
\**************************************************************************/

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
  double getBufferDuration();
  SbBool getPlayedOnce();
  void   setPlayedOnce(SbBool played=TRUE);
  SbBool isBufferOK();
  static void  setSubdirectories(const SbList<SbString> &subdirectories);
  static const SbStringList & getSubdirectories();

protected:

  virtual ~SoAudioClip();

  virtual SbBool loadUrl(void); 
  virtual void unloadUrl(void);

  static SbStringList subdirectories;
protected:
  class SoAudioClipP *soaudioclip_impl;
  friend class SoAudioClipP;
  friend class SoSoundP;
};

#endif // !SMALLCHANGE_SOAUDIOCLIP_H
