#ifndef SMALLCHANGE_ALTOOLS_H
#define SMALLCHANGE_ALTOOLS_H

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

#include <Inventor/lists/SbList.h>
#include <Inventor/SbString.h>
#include <Inventor/SbVec3f.h>

#include <AL/altypes.h>

inline void SbVec3f2ALfloat3(ALfloat *dest, const SbVec3f &source)
{
  source.getValue(dest[0], dest[1], dest[2]);
};

char *GetALErrorString(char *text, ALint errorcode);

ALenum getALSampleFormat(int channels, int bitspersample);

/*
class SbAutoLock
{
protected:
  SbMutex *mutex;
public:
  SbAutoLock(SbMutex *mutex) {
    this->mutex = mutex;
    this->mutex->lock();
  }
  ~SbAutoLock() {
    this->mutex->unlock();
  }
};
*/

// fixme: this is from coin/include/nodes/SoSubNodeP.h
#define SO_NODE_INTERNAL_INIT_CLASS(_class_) \
  do { \
    const char * classname = SO__QUOTE(_class_); \
    PRIVATE_COMMON_INIT_CODE(_class_, &classname[2], &_class_::createInstance, inherited); \
  } while (0)

// fixme: this is from Apache

void mono2stereo(short int *buffer, int length);
void stereo2mono(short int *buffer, int length);


#ifdef _WIN32

/*
 * Structures and types used to implement opendir/readdir/closedir
 * on Windows 95/NT.
 */

#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

/* struct dirent - same as Unix */
struct dirent {
    long d_ino;                    /* inode (always 1 in WIN32) */
    off_t d_off;                /* offset to this dirent */
    unsigned short d_reclen;    /* length of d_name */
    char d_name[_MAX_FNAME+1];    /* filename (null terminated) */
};

/* typedef DIR - not the same as Unix */
typedef struct {
    long handle;                /* _findfirst/_findnext handle */
    short offset;                /* offset into directory */
    short finished;             /* 1 if there are not more files */
    struct _finddata_t fileinfo;  /* from _findfirst/_findnext */
    char *dir;                  /* the dir we are reading */
    struct dirent dent;         /* the dirent to return */
} DIR;

DIR *opendir(const char *dir);
struct dirent *readdir(DIR * dp);
int closedir(DIR * dp);
void rewinddir(DIR * dir_Info);

#endif // _WIN32

SbBool getFileExtension(char *ext, const char *filename, int maxlen);
SbList<SbString> createPlaylistFromDirectory(SbString dirname);

#endif // HAVE_OPENAL

#endif // !SMALLCHANGE_ALTOOLS_H
