#ifndef SMALLCHANGE_SOSOUNDP_H
#define SMALLCHANGE_SOSOUNDP_H

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

#include <AL/altypes.h>

#include <SmallChange/misc/SbAudioWorkerThread.h>

class SoSoundP
{
public:
  SoSoundP(SoSound * interfaceptr) : ifacep(interfaceptr) {};
  SoSound *ifacep;

  static void sourceSensorCBWrapper(void *, SoSensor *);
  void sourceSensorCB(SoSensor *);

  SbBool stopPlaying(SbBool force = FALSE);
  SbBool startPlaying(SbBool force = FALSE);

  class SoFieldSensor * sourcesensor;
  ALuint sourceId;
  class SoAudioClip *currentAudioClip;
  SbBool isStreaming;
  SbBool asyncStreamingMode;

  static void timercb(void * data, SoSensor *);
  SoTimerSensor * timersensor;
  SbAudioWorkerThread *workerThread;
  short int *audioBuffer;

  static int threadCallbackWrapper(void *userdata);
  int threadCallback();
  int fillBuffers();

  SbTime actualStartTime;

#ifdef HAVE_PTHREAD
  static pthread_mutex_t syncmutex;
  static int refcount;
#endif
};

#endif // !SMALLCHANGE_SOSOUNDP_H
