#include "SbAudioWorkerThread.h"

#include <assert.h>

#if HAVE_PTHREAD
#include <pthread.h>
#include <Inventor/errors/SoDebugError.h>

struct sbaudio_thread {
  pthread_t threadid;
//  pthread_mutex_t * buffermutex;
  pthread_cond_t threadbrake;
  pthread_mutex_t threadbrakemutex;
  pthread_cond_t mainbrake;
  pthread_mutex_t mainbrakemutex;
};


inline void
safe_pthread_cond_wait(pthread_cond_t * cond, pthread_mutex_t * mutex)
{
  pthread_mutex_lock(mutex);
  pthread_cond_wait(cond, mutex);
  pthread_mutex_unlock(mutex);
}

inline void
safe_pthread_cond_signal(pthread_cond_t * cond, pthread_mutex_t * mutex)
{
  pthread_mutex_lock(mutex);
  pthread_cond_signal(cond);
  pthread_mutex_unlock(mutex);
}

#endif // HAVE_PTHREAD


SbAudioWorkerThread::SbAudioWorkerThread(int (*user_callback)(void * userdataptr), void * userdata)
{
  this->usethread = TRUE;
  this->threadinfo = NULL;
  this->exitthread = FALSE;
  this->isactive = FALSE;
  this->user_callback = user_callback;
  this->userdata = userdata;
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
  printf("Starting thread...");
#if HAVE_PTHREAD
  int i;
  this->threadinfo = new struct sbaudio_thread;
//  this->threadinfo->buffermutex = new pthread_mutex_t[this->numbuffers];
//  for (i = 0; i < this->numbuffers; i++) {
//    pthread_mutex_init(&this->threadinfo->buffermutex[i], NULL);
//  }
  pthread_cond_init(&this->threadinfo->threadbrake, NULL);
  pthread_mutex_init(&this->threadinfo->threadbrakemutex, NULL);
  pthread_cond_init(&this->threadinfo->mainbrake, NULL);
  pthread_mutex_init(&this->threadinfo->mainbrakemutex, NULL);

  if (pthread_create(&this->threadinfo->threadid,
                     NULL, main_thread, this)) {
    SoDebugError::postWarning("SbAudioWorkerThread::start_thread",
                              "Could not create thread. Using synchronized mode.");
//    for (i = 0; i < this->numbuffers; i++) {
//      pthread_mutex_destroy(&this->threadinfo->buffermutex[i]);
//    }
//    delete this->threadinfo->buffermutex;
    pthread_cond_destroy(&this->threadinfo->threadbrake);
    pthread_mutex_destroy(&this->threadinfo->threadbrakemutex);
    pthread_cond_destroy(&this->threadinfo->mainbrake);
    pthread_mutex_destroy(&this->threadinfo->mainbrakemutex);
    delete this->threadinfo;
    this->threadinfo = NULL;
    this->usethread = FALSE;
  }
#endif // HAVE_PTHREAD
  printf("Ok\n");
}

//
// stops filling thread and cleans up all data used to synchronize
//
void
SbAudioWorkerThread::stop_thread(void)
{
  printf("Stopping thread...");
#if HAVE_PTHREAD
  this->exitthread = TRUE;
  // in case the fill thread is waiting to fill a buffer
  safe_pthread_cond_signal(&this->threadinfo->threadbrake, &this->threadinfo->threadbrakemutex);
  // wait for thread to finish up
  pthread_join(this->threadinfo->threadid, NULL);

  // clean up allocated data
//  for (int i = 0; i < this->numbuffers; i++) {
//    pthread_mutex_destroy(&this->threadinfo->buffermutex[i]);
//  }
//  delete this->threadinfo->buffermutex;
  pthread_cond_destroy(&this->threadinfo->threadbrake);
  pthread_mutex_destroy(&this->threadinfo->threadbrakemutex);
  pthread_cond_destroy(&this->threadinfo->mainbrake);
  pthread_mutex_destroy(&this->threadinfo->mainbrakemutex);
  delete this->threadinfo;
#endif // HAVE_PTHREAD
  printf("Ok\n");
}


void *
SbAudioWorkerThread::main_thread(void * userdata)
{
  printf("Fillthread running\n");
#if HAVE_PTHREAD
  SbAudioWorkerThread * thisp = (SbAudioWorkerThread*) userdata;

  while (!thisp->exitthread) {

    if (thisp->user_callback != NULL)
    {
      int retval;
    // thh: should probably have some mutex here .....
      retval = thisp->user_callback(thisp->userdata);
      if (retval < 0) 
        thisp->exitthread = TRUE;
    };
/*
    int i = thisp->nexttofill;
    pthread_mutex_lock(&thisp->threadinfo->buffermutex[i]);

    if (thisp->bufferstatus[i] == STATUS_UNFILLED) {
      thisp->bufferstatus[i] = STATUS_BUSY; // not really needed...
      thisp->fillretval[i] =
        thisp->buffer_fill_routine(thisp->buffer[i], thisp->userdata);
      thisp->nexttofill++;
      if (thisp->nexttofill >= thisp->numbuffers) thisp->nexttofill = 0;
      thisp->bufferstatus[i] = STATUS_FILLED;
      pthread_mutex_unlock(&thisp->threadinfo->buffermutex[i]);
      // in case the main thread is waiting for a buffer
      safe_pthread_cond_signal(&thisp->threadinfo->mainbrake, &thisp->threadinfo->mainbrakemutex);
      // check to see if we should exit thread
      if (thisp->fillretval[i] < 0) thisp->exitthread = TRUE;
    }
    else {
      pthread_mutex_unlock(&thisp->threadinfo->buffermutex[i]);
      // in case the main thread is waiting for the buffer
      safe_pthread_cond_signal(&thisp->threadinfo->mainbrake, &thisp->threadinfo->mainbrakemutex);
      // suspend this thread until a buffer has been read
      safe_pthread_cond_wait(&thisp->threadinfo->threadbrake,
                             &thisp->threadinfo->threadbrakemutex);
    }
*/
  }
#endif // HAVE_PTHREAD
  return NULL;
}

