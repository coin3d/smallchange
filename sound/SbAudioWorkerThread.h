#ifndef COIN_SBAUDIOWOPRKERTHREAD_H
#define COIN_SBAUDIOWOPRKERTHREAD_H

#include <Inventor/SbBasic.h>
#include <stdlib.h>

class SbAudioWorkerThread
{
public:
  SbAudioWorkerThread(int (*user_callback)(void * userdataptr) = NULL, void * userdata = NULL);
  void start(void);
  void stop(void);
  SbBool isActive(void) const;

protected:
  struct sbaudio_thread * threadinfo;
  SbBool usethread;
  volatile SbBool exitthread;
  SbBool isactive;
  int (*user_callback)(void * userdataptr);
  void * userdata;

  void start_thread(void);
  void stop_thread(void);  

  static void * main_thread(void *);


};

#endif
