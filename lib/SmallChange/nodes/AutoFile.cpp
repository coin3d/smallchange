/**************************************************************************\
 * Copyright (c) Kongsberg Oil & Gas Technologies AS
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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

  Sets the delay before file is reloaded after a modification is
  detected.
*/

/*!
  \var SoSFInt32 AutoFile::priority

  Sets the priority of the idle sensor. It is set to the default
  priority (100) by default.
*/

/*!
  \var SoSFBool AutoFile::active

  Sets whether autoload is active.
*/

/*!
  \var SoSFBool AutoFile::stripTopSeparator

  ** OBSOLETED, mortene 20031217: **

  I have now changed the semantics of SoFile, so it will no longer
  always have an SoSeparator as its root, and never add an extra such
  node at the top. The contents of an SoFile / AutoFile will always
  match exactly what is found in the file.

  FIXME: this means the stripTopSeparator field is not really needed
  any more, and it should therefore be removed in a design
  cleanup. Will not do that yet, as we have software out at customers
  which have files with AutoFile nodes using this field (and they will
  then stop loading). Also, it's a change of semantics, which may be
  dangerous.

  The best way to proceed is perhaps to make a clean break and rename
  this node to SmAutoFile and then kill the old cruft. Before that
  happens, should also kill at least the "delay" field, possibly also
  the "interval" field (should use some kind of proper monitor
  technique instead of a lame polling technique) -- see related
  FIXMEs.


  ** OLD DOCS: **

  If set to \c TRUE, the root SoSeparator node of the loaded iv-file
  will be stripped off. This is useful for letting appearance settings
  in the loaded sub-scene graph, like e.g. materials or textures,
  influence the rest of the scene graph.

  Default value is \c FALSE.
*/

// *************************************************************************

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "AutoFile.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_IO_H
#include <io.h>
#endif

#include <cstdlib>
#include <cstddef>

#include <Inventor/sensors/SoTimerSensor.h>
#include <Inventor/sensors/SoIdleSensor.h>
#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/misc/SoChildList.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/C/tidbits.h>

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

  SbBool debug;

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
  // FIXME: as far as I can tell, this method is exactly the same as
  // its superclass -- can we remove it? 20031217 mortene.

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

  if (f == &this->stripTopSeparator) {
    static SbBool first = TRUE;
    if (first) {
      SoDebugError::postWarning("AutoFile::notify",
                                "AutoFile::stripTopSeparator field has "
                                "been obsoleted");
      first = FALSE;
    }
  }

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

// Overridden to strip off top SoSeparator, if requested.
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
  this->debug = coin_getenv("SMALLCHANGE_AUTOFILE_DEBUG") ? TRUE : FALSE;

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
      if (thisp->debug) {
        SoDebugError::postInfo("AutoFileP::idler_cb", "loading file '%s'",
                               thisp->fullname.getString());
      }
      if (thisp->fullname.getLength()) {
        struct stat statbuf;
        // FIXME: stat() errors are just silently ignored. Should do
        // proper reporting. 20031217 mortene.
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
      // FIXME: stat() errors are just silently ignored. Should do
      // proper reporting. 20031217 mortene.
      if (stat(thisp->fullname.getString(), &statbuf) == 0) {
        if (thisp->mtimevalid) {
          if ( (thisp->mtime != statbuf.st_mtime) || (thisp->size != statbuf.st_size) ) {
            thisp->mtime = statbuf.st_mtime;
            thisp->size = statbuf.st_size;
            if (!thisp->toucher->isScheduled()) {
              // wait before loading, in case file is being written.
              //
              // FIXME: this looks like a crap and shaky hack. What if
              // the file is larger than what can be written for the
              // delay time? What if a file write is interrupted by
              // some other task on the system for longer than the
              // delay time?
              //
              // And of course it is bad design to push the
              // responsibility of guessing how much time to wait onto
              // the client code.
              //
              // I presume there is some manner which one can find out
              // when a file write has been completed through system
              // APIs. That code might be harder to write, but this is
              // just ugly and should be fixed.
              //
              // 20031217 mortene.
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
    // FIXME: stat() errors are just silently ignored. Should do
    // proper reporting. 20031217 mortene.
    if (stat(thisp->fullname.getString(), &statbuf) == 0) {
      // only load if size and mtime hasn't changed
      if (statbuf.st_size == thisp->size && statbuf.st_mtime == thisp->mtime) {

        if (thisp->debug) {
          SoDebugError::postInfo("AutoFileP::toucher_cb", "reloading '%s'",
                                 thisp->fullname.getString());
        }

        PUBLIC(thisp)->name.touch(); // force reload
      }
    }
  }
  thisp->toucher->unschedule();
}

// *************************************************************************

#undef PUBLIC
