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

#if HAVE_CONFIG_H
#include <config.h>
#endif

#if HAVE_OPENAL

#include <SmallChange/misc/SbAudioWorkerThread.h>

#include <Inventor/SbTime.h>

#include <assert.h>

#if HAVE_PTHREAD
#include <pthread.h>
#include <Inventor/errors/SoDebugError.h>


struct sbaudio_thread {
  pthread_t threadid;
  pthread_cond_t threadbrake;
  pthread_mutex_t threadbrakemutex;
  pthread_cond_t mainbrake;
  pthread_mutex_t mainbrakemutex;
};


void
sbawt_safe_pthread_cond_wait(pthread_cond_t * cond, pthread_mutex_t * mutex)
{
  pthread_mutex_lock(mutex);
  pthread_cond_wait(cond, mutex);
  pthread_mutex_unlock(mutex);
}

void
sbawt_safe_pthread_cond_signal(pthread_cond_t * cond, pthread_mutex_t * mutex)
{
  pthread_mutex_lock(mutex);
  pthread_cond_signal(cond);
  pthread_mutex_unlock(mutex);
}

#endif // HAVE_PTHREAD


SbAudioWorkerThread::SbAudioWorkerThread(int (*user_callback)(void * userdataptr), void * userdata,
                                         int sleeptime)
{
  this->usethread = TRUE;
  this->threadinfo = NULL;
  this->exitthread = FALSE;
  this->isactive = FALSE;
  this->user_callback = user_callback;
  this->userdata = userdata;
  this->sleeptime = sleeptime; 
}

void
SbAudioWorkerThread::start(void)
{
  assert(!this->isactive);
#if HAVE_PTHREAD
  if (this->usethread) {
    this->start_thread();
  }
#endif // HAVE_PTHREAD
  this->isactive = TRUE;
}

void
SbAudioWorkerThread::stop(void)
{
  if (this->isactive) {
#if HAVE_PTHREAD
    if (this->usethread) {
      this->stop_thread();
    }
#endif
    this->isactive = FALSE;
  }
}

SbBool SbAudioWorkerThread::isActive(void) const
{
  return this->isactive;
};


//
// Initializes the thread and all mutexes needed for synchronization
// between the caller thread and the new thread.
//
void
SbAudioWorkerThread::start_thread(void)
{
#ifdef DEBUG_AUDIO
  fprintf(stderr, "Starting thread...");
#endif

#if HAVE_PTHREAD
  this->threadinfo = new struct sbaudio_thread;
  pthread_cond_init(&this->threadinfo->threadbrake, NULL);
  pthread_mutex_init(&this->threadinfo->threadbrakemutex, NULL);
  pthread_cond_init(&this->threadinfo->mainbrake, NULL);
  pthread_mutex_init(&this->threadinfo->mainbrakemutex, NULL);

  if (pthread_create(&this->threadinfo->threadid,
                     NULL, main_thread, this)) {
    SoDebugError::postWarning("SbAudioWorkerThread::start_thread",
                              "Could not create thread. Using synchronized mode.");
    pthread_cond_destroy(&this->threadinfo->threadbrake);
    pthread_mutex_destroy(&this->threadinfo->threadbrakemutex);
    pthread_cond_destroy(&this->threadinfo->mainbrake);
    pthread_mutex_destroy(&this->threadinfo->mainbrakemutex);
    delete this->threadinfo;
    this->threadinfo = NULL;
    this->usethread = FALSE;
  }
#endif // HAVE_PTHREAD

#ifdef DEBUG_AUDIO
  fprintf(stderr, "Ok\n");
#endif
}

//
// stops filling thread and cleans up all data used to synchronize
//
void
SbAudioWorkerThread::stop_thread(void)
{
#ifdef DEBUG_AUDIO
  fprintf(stderr, "Stopping thread...");
#endif

#if HAVE_PTHREAD
  this->exitthread = TRUE;
  // in case the fill thread is waiting to fill a buffer
  sbawt_safe_pthread_cond_signal(&this->threadinfo->threadbrake, &this->threadinfo->threadbrakemutex);
  // wait for thread to finish up
  pthread_join(this->threadinfo->threadid, NULL);

  // clean up allocated data
  pthread_cond_destroy(&this->threadinfo->threadbrake);
  pthread_mutex_destroy(&this->threadinfo->threadbrakemutex);
  pthread_cond_destroy(&this->threadinfo->mainbrake);
  pthread_mutex_destroy(&this->threadinfo->mainbrakemutex);
  delete this->threadinfo;
#endif // HAVE_PTHREAD

#ifdef DEBUG_AUDIO
  fprintf(stderr, "Ok\n");
#endif
}


void *
SbAudioWorkerThread::main_thread(void * userdata)
{
#ifdef DEBUG_AUDIO
  fprintf(stderr, "Fillthread running\n");
#endif

#if HAVE_PTHREAD
  SbAudioWorkerThread * thisp = (SbAudioWorkerThread*) userdata;

  while (!thisp->exitthread) {

    if (thisp->user_callback != NULL) {
      int retval;
      retval = thisp->user_callback(thisp->userdata);
      if (retval <= 0) 
        thisp->exitthread = TRUE;
    };

    // do not use all available cpu-resources...
    // thisp->sleep(thisp->sleeptime);
    Sleep(thisp->sleeptime);
  };
#endif // HAVE_PTHREAD
  return NULL;
}

void SbAudioWorkerThread::sleep(int milliseconds) const
{
  /*
    fixme 20021003 thammer: this function leaks memory. A lot of it.
    We're going to replace pthreads with coin threads, so I won't fix it...
  */
#if HAVE_PTHREAD
  pthread_cond_t      cond  = PTHREAD_COND_INITIALIZER;
  pthread_mutex_t     mutex = PTHREAD_MUTEX_INITIALIZER;

  struct timespec   ts;

  time_t sec;
  long usec;

  (void) pthread_mutex_lock(&mutex);

  SbTime::getTimeOfDay().getValue(sec, usec);

  /* Convert from timeval to timespec */
  ts.tv_sec  = sec;
  ts.tv_nsec = usec * 1000 + milliseconds*1000000L;

  (void) pthread_cond_timedwait(&cond, &mutex, &ts);

  (void) pthread_mutex_unlock(&mutex);

  pthread_cond_destroy(&cond);
  pthread_mutex_destroy(&mutex);
#endif // HAVE_PTHREAD

};

void SbAudioWorkerThread::setThreadLoopSleepTime(int milliseconds)
{
  this->sleeptime = milliseconds;
};

#endif // HAVE_OPENAL
