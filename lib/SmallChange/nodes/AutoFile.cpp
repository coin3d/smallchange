/**************************************************************************\
 *
 *  This file is part of the SmallChange extension library for Coin.
 *  Copyright (C) 1998-2003 by Systems in Motion.  All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  ("GPL") version 2 as published by the Free Software Foundation.
 *  See the file LICENSE.GPL at the root directory of this source
 *  distribution for additional information about the GNU GPL.
 *
 *  For using SmallChange with software that can not be combined with the
 *  GNU GPL, and for taking advantage of the additional benefits of our
 *  support services, please contact Systems in Motion about acquiring
 *  a Coin Professional Edition License.
 *
 *  See <URL:http://www.coin3d.org> for  more information.
 *
 *  Systems in Motion, Teknobyen, Abels Gate 5, 7030 Trondheim, NORWAY.
 *  <URL:http://www.sim.no>.
 *
\**************************************************************************/

/*!
  \class AutoFile AutoFile.h
  \brief The AutoFile class is used to automatically reload files when modified.
  \ingroup nodes

  It works by using a timer sensor that triggers an idle sensor at
  regular intervals. The idle sensor then checks if the loaded file
  has been modified, and triggers a new timer sensor that reloads the
  file after a given delay. A delay is needed to avoid trying to load
  the file while it is being written by another process.  
*/

// *************************************************************************

/*!
  \var SoSFFloat AutoFile::interval
  Sets how often the idle sensor is triggered.
*/

/*!
  \var SoSFFloat AutoFile::delay
  Sets the delay before file is reloaded after a modification is detected.
*/

/*!
  \var SoSFInt32 AutoFile::priority
  Sets the priority of the idle sensor. It is set to the default priority (100) by default.
*/

/*!
  \var SoSFBool AutoFile::active
  Sets whether autoload is active.
*/

// *************************************************************************

#include "AutoFile.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_IO_H
#include <io.h>
#endif

#include <stdlib.h>
#include <stddef.h>

#include <Inventor/sensors/SoTimerSensor.h>
#include <Inventor/sensors/SoIdleSensor.h>
#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/misc/SoChildList.h>

// *************************************************************************

class AutoFileP {
public:
  AutoFileP(AutoFile * master);
  ~AutoFileP();
  
  SbString currname;
  SbString fullname;
  SbBool mtimevalid;
  time_t mtime;
  off_t size;

  SoTimerSensor * toucher;
  SoTimerSensor * rescheduler;
  SoIdleSensor * idler;

  static void idler_cb(void * data, SoSensor *);
  static void rescheduler_cb(void * data, SoSensor *);
  static void toucher_cb(void * data, SoSensor *);

private:
  AutoFile * master;
};

SO_NODE_SOURCE(AutoFile);

#undef PRIVATE
#define PRIVATE(p) (p->pimpl)
#undef PUBLIC
#define PUBLIC(p) (p->master)

// *************************************************************************

/*!
  Constructor.
*/
AutoFile::AutoFile()
{
  SO_NODE_CONSTRUCTOR(AutoFile);
  
  SO_NODE_ADD_FIELD(active, (TRUE));
  SO_NODE_ADD_FIELD(interval, (1.0f));
  SO_NODE_ADD_FIELD(delay, (1.0f));
  SO_NODE_ADD_FIELD(priority, ((int32_t)SoDelayQueueSensor::getDefaultPriority()));
  SO_NODE_ADD_FIELD(stripTopSeparator, (FALSE));

  PRIVATE(this) = new AutoFileP(this);
}

/*!
  Destructor.
*/
AutoFile::~AutoFile()
{
  delete PRIVATE(this);
}

// doc from parent
void
AutoFile::initClass(void)
{
  SO_NODE_INIT_CLASS(AutoFile, SoFile, "File");
}

void 
AutoFile::doAction(SoAction * action)
{
  int numindices;
  const int * indices;
  if (action->getPathCode(numindices, indices) == SoAction::IN_PATH) {
    this->getChildren()->traverseInPath(action, numindices, indices);
  }
  else {
    this->getChildren()->traverse(action); // traverse all children
  }  
}

void 
AutoFile::search(SoSearchAction * action)
{
  inherited::search(action);
  if (action->isFound()) return;
  AutoFile::doAction(action);
}

void 
AutoFile::notify(SoNotList * list)
{
  SoField *f = list->getLastField();
  if (f == &this->active) {
    if (this->active.getValue()) {
      if (!PRIVATE(this)->rescheduler->isScheduled() && 
          !PRIVATE(this)->idler->isScheduled()) {
        PRIVATE(this)->rescheduler->setInterval(SbTime((double) this->interval.getValue()));
        PRIVATE(this)->rescheduler->schedule();
      }
    }
    else {
      if (PRIVATE(this)->idler->isScheduled()) PRIVATE(this)->idler->unschedule();
      if (PRIVATE(this)->rescheduler->isScheduled()) PRIVATE(this)->rescheduler->unschedule();
      if (PRIVATE(this)->toucher->isScheduled()) PRIVATE(this)->toucher->unschedule();
    }
  }

  inherited::notify(list);
}

/*!
  Overridden to detect when file is read.
*/
SbBool
AutoFile::readNamedFile(SoInput * in)
{
  SbBool ret = inherited::readNamedFile(in);
  

  if (this->stripTopSeparator.getValue()) {
    SoChildList * children = this->getChildren();
    SoNode * child = children->getLength() == 1 ?
      (*children)[0] : NULL;
    if (child && child->getTypeId() == SoSeparator::getClassTypeId()) {
      SoSeparator * sep = (SoSeparator*) child;
      sep->ref();
      children->truncate(0);
      for (int i = 0; i < sep->getNumChildren(); i++) {
        children->append(sep->getChild(i));
      }
      sep->removeAllChildren();
      sep->unref();
    }
  }
  return ret;
}


#undef PRIVATE

// *************************************************************************

AutoFileP::AutoFileP(AutoFile * master)
  : master(master)
{
  this->currname.makeEmpty();
  this->fullname.makeEmpty();
  this->mtimevalid = FALSE;

  this->idler = new SoIdleSensor(idler_cb, this);
  this->toucher = new SoTimerSensor(toucher_cb, this);
  this->toucher->setInterval(SbTime((double) PUBLIC(this)->delay.getValue()));

  this->rescheduler = new SoTimerSensor(rescheduler_cb, this);
  this->rescheduler->setInterval(SbTime((double) PUBLIC(this)->interval.getValue()));
  this->rescheduler->schedule(); // go
}

AutoFileP::~AutoFileP()
{
  delete this->idler;
  delete this->rescheduler;
  delete this->toucher;
}

void
AutoFileP::idler_cb(void * data, SoSensor *)
{
  AutoFileP * thisp = (AutoFileP*) data;
 
  if (!PUBLIC(thisp)->active.getValue()) return;
 
  if (PUBLIC(thisp)->name.getValue().getLength()) {
    // test for new name
    if (PUBLIC(thisp)->name.getValue() != thisp->currname) {
      thisp->currname = PUBLIC(thisp)->name.getValue();
      thisp->fullname = PUBLIC(thisp)->getFullName();
      if (thisp->fullname.getLength()) {
        struct stat statbuf;
        if (stat(thisp->fullname.getString(), &statbuf) == 0) {
          thisp->mtimevalid = TRUE;
          thisp->mtime = statbuf.st_mtime;
          thisp->size = statbuf.st_size;
        }
      }
    }
    // if old file, see if file has changed
    /* FIXME: On the Win32 platform, several optimizations are possible:
       1) stat returns time in seconds. Use GetFileTime() to get time
          in 100-nanosecond intervals. The actual resolution depends on
          the file system used. NT FAT has a 2-second resolution on write
          times. thammer has measured the resolution on NTFS to 10ms.
          We should check the resolution for FAT and FAT32 too.
       2) Use FindFirstChangeNotification and WaitForSingleObject in
          a separate thread to detect changes in the file. The contents
          of the file could be read in the thread (to minimize risk of
          failing to open file because it's being written to again), and
          a flag could be set. The interval-based polling could check 
          this flag, and read file from buffer instead of from file if 
          flag is set.
       2002-11-29 thammer
     */
    else if (thisp->fullname.getLength()) {
      struct stat statbuf;
      if (stat(thisp->fullname.getString(), &statbuf) == 0) {
        if (thisp->mtimevalid) {
          if ( (thisp->mtime != statbuf.st_mtime) || (thisp->size != statbuf.st_size) ) {
            thisp->mtime = statbuf.st_mtime;
            thisp->size = statbuf.st_size;
            if (!thisp->toucher->isScheduled()) {
              // wait before loading, in case file is being written.
              thisp->toucher->setInterval(SbTime((double) PUBLIC(thisp)->delay.getValue()));
              thisp->toucher->schedule();
            }
          }
        }
        else {
          thisp->mtimevalid = TRUE;
          thisp->mtime = statbuf.st_mtime;
          thisp->size = statbuf.st_size;
        }
      }
    }
  }

  if (!thisp->rescheduler->isScheduled()) {
    thisp->rescheduler->setInterval(SbTime((double) PUBLIC(thisp)->interval.getValue()));
    thisp->rescheduler->schedule();
  }
}

void 
AutoFileP::rescheduler_cb(void * data, SoSensor *)
{
  AutoFileP * thisp = (AutoFileP*) data;
  if (PUBLIC(thisp)->active.getValue()) {
    if (!thisp->idler->isScheduled()) {
      thisp->idler->setPriority(PUBLIC(thisp)->priority.getValue());
      thisp->idler->schedule();
    }
  }
  thisp->rescheduler->unschedule();
}

void 
AutoFileP::toucher_cb(void * data, SoSensor *)
{
  AutoFileP * thisp = (AutoFileP*) data;
  if (PUBLIC(thisp)->active.getValue()) {
    struct stat statbuf;
    if (stat(thisp->fullname.getString(), &statbuf) == 0) {
      // only load if size and mtime hasn't changed 
      if (statbuf.st_size == thisp->size && statbuf.st_mtime == thisp->mtime) {
        PUBLIC(thisp)->name.touch(); // force reload
      }
    }
  }
  thisp->toucher->unschedule();
}

// *************************************************************************

#undef PUBLIC
