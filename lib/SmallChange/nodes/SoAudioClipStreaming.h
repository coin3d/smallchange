#ifndef SMALLCHANGE_SOAUDIOCLIPSTREAMING_H
#define SMALLCHANGE_SOAUDIOCLIPSTREAMING_H

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

#include <SmallChange/nodes/SoAudioClip.h>

class SoAudioClipStreaming : public SoAudioClip
{
  typedef SoAudioClip inherited;
  SO_NODE_HEADER(SoAudioClipStreaming);

  friend class SoSound;
public:
  static void initClass(void);
  SoAudioClipStreaming();

  void setAsyncMode(SbBool flag=FALSE);
  SbBool getAsyncMode();
  void setBufferInfo(int bufferSize, int numBuffers);
  void setSampleFormat(int channels = 1, int samplerate = 44100, int bitspersample=16);
  void getSampleFormat(int &channels, int &samplerate, int &bitspersample);
  int getNumChannels();
  int getBufferSize();
  int getNumBuffers();
  void setUserCallback(int (*user_callback)(void *buffer, int length, void * userdataptr),
    void *userdata=NULL);
  void setKeepAlive(SbBool alive=TRUE);

protected:
  virtual ~SoAudioClipStreaming();

  virtual SbBool loadUrl(void); 
  virtual void unloadUrl(void);

protected:
  class SoAudioClipStreamingP *soaudioclipstreaming_impl;
  friend class SoAudioClipStreamingP;
  friend class SoSoundP;
};

#endif // SMALLCHANGE_SOAUDIOCLIPSTREAMING_H
