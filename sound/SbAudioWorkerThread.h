#ifndef SMALLCHANGE_SBAUDIOWOPRKERTHREAD_H
#define SMALLCHANGE_SBAUDIOWOPRKERTHREAD_H

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

#include <Inventor/SbBasic.h>
#include <stdlib.h>

#if HAVE_PTHREAD
#include <pthread.h>
#endif

class SbAudioWorkerThread
{
public:
  SbAudioWorkerThread(int (*user_callback)(void * userdataptr) = NULL, void * userdata = NULL,
    int sleeptime = 0);
  void start(void);
  void stop(void);
  SbBool isActive(void) const;
  void sleep(int milliseconds) const;
  void setThreadLoopSleepTime(int milliseconds);

protected:
  struct sbaudio_thread * threadinfo;
  SbBool usethread;
  volatile SbBool exitthread;
  SbBool isactive;
  int (*user_callback)(void * userdataptr);
  void * userdata;
  int sleeptime; // in milliseconds

  void start_thread(void);
  void stop_thread(void);  

  static void * main_thread(void *);


};

#if HAVE_PTHREAD

class SbAutoLock
{
protected:
  pthread_mutex_t *mutex;
public:
  SbAutoLock(pthread_mutex_t *mutex)
  {
    this->mutex = mutex;
    pthread_mutex_lock(this->mutex);

  };
  ~SbAutoLock()
  {
    pthread_mutex_unlock(this->mutex);
  };
};

#endif // HAVE_PTHREAD

#endif // !SMALLCHANGE_SBAUDIOWOPRKERTHREAD_H
