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

#include <SmallChange/misc/ALTools.h>

#include <Inventor/SbTime.h>
#include <Inventor/errors/SoDebugError.h>
#include <string.h>

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif

#ifndef HAVE_STRCMPI
#ifdef HAVE_STRCASECMP
#define strcmpi strcasecmp
#endif // HAVE_STRCASECMP
#endif // HAVE_STRCMPI

char *GetALErrorString(char *text, ALint errorcode)
{
  // text should be at least char[255]
  switch (errorcode) {
    case AL_INVALID_NAME:
      sprintf(text, "AL_INVALID_NAME - Illegal name passed as an argument to an AL call");
      break;
#ifdef _WIN32
    case AL_INVALID_ENUM:
#else
    case AL_ILLEGAL_ENUM:
#endif
      sprintf(text, "AL_INVALID_ENUM - Illegal enum passed as an argument to an AL call");
      break;
    case AL_INVALID_VALUE:
      sprintf(text, "AL_INVALID_VALUE - Illegal value passed as an argument to an AL call");
      break;
#ifdef _WIN32
    case AL_INVALID_OPERATION:
#else
    case AL_ILLEGAL_COMMAND:
#endif
      sprintf(text, "AL_INVALID_OPERATION - A function was called at an inappropriate time or in an inappropriate way, causing an illegal state. This can be an incompatible ALenum, object ID, and/or function");
      break;
    case AL_OUT_OF_MEMORY:
      sprintf(text, "AL_OUT_OF_MEMORY - A function could not be completed, because there is not enough memory available.");
      break;
    default:
      sprintf(text, "UNDEFINED ERROR");
      break;
  }
  return text;
}

ALenum getALSampleFormat(int channels, int bitspersample)
{
  ALenum  alformat = 0;;

  if ( (channels==1) && (bitspersample==8) )
    alformat = AL_FORMAT_MONO8;
  else if ( (channels==1) && (bitspersample==16) )
    alformat = AL_FORMAT_MONO16;
  else if ( (channels==2) && (bitspersample==8) )
    alformat = AL_FORMAT_STEREO8;
  else if ( (channels==2) && (bitspersample==16) )
    alformat = AL_FORMAT_STEREO16;

  return alformat;
};

void mono2stereo(short int *buffer, int length) 
{
  // assumes that buffersize = length * sizeof(short int) * 2
  for (int i=length-1; i>=0; i--) {
    buffer[i*2] = buffer[i*2+1] = buffer[i];
  }
}

void stereo2mono(short int *buffer, int length) 
{
  // assumes that buffersize = length * sizeof(short int) * 2
  for (int i=0; i<length; i++) {
    buffer[i] = ((long int)buffer[i*2] + (long int)buffer[i*2+1]) / 2;
  }
}

#ifdef _WIN32

#include <malloc.h>
#include <string.h>
#include <errno.h>

/**********************************************************************
 * Implement dirent-style opendir/readdir/closedir on Window 95/NT
 *
 * Functions defined are opendir(), readdir() and closedir() with the
 * same prototypes as the normal dirent.h implementation.
 *
 * Does not implement telldir(), seekdir(), rewinddir() or scandir(). 
 * The dirent struct is compatible with Unix, except that d_ino is 
 * always 1 and d_off is made up as we go along.
 *
 * The DIR typedef is not compatible with Unix.
 **********************************************************************/

DIR *opendir(const char *dir)
{
  DIR *dp;
  char *filespec;
  long handle;
  int index;
  
  filespec = (char *)malloc(strlen(dir) + 2 + 1);
  strcpy(filespec, dir);
  index = strlen(filespec) - 1;
  if (index >= 0 && (filespec[index] == '/' || filespec[index] == '\\'))
                filespec[index] = '\0';
  strcat(filespec, "/*");
  
  dp = (DIR *) malloc(sizeof(DIR));
  dp->offset = 0;
  dp->finished = 0;
  dp->dir = strdup(dir);
  
  if ((handle = _findfirst(filespec, &(dp->fileinfo))) < 0) {
    if (errno == ENOENT)
      dp->finished = 1;
    else
      return NULL;
  }
  dp->handle = handle;
  free(filespec);
  
  return dp;
}

struct dirent *readdir(DIR * dp)
{
        if (!dp || dp->finished)
                return NULL;

        if (dp->offset != 0) {
                if (_findnext(dp->handle, &(dp->fileinfo)) < 0) {
                        dp->finished = 1;
                        return NULL;
                }
        }
        dp->offset++;

        strncpy(dp->dent.d_name, dp->fileinfo.name, _MAX_FNAME);
        dp->dent.d_ino = 1;
        dp->dent.d_reclen = strlen(dp->dent.d_name);
        dp->dent.d_off = dp->offset;

        return &(dp->dent);
}

int closedir(DIR * dp)
{
  if (!dp)
    return 0;
  _findclose(dp->handle);
  if (dp->dir)
    free(dp->dir);
  if (dp)
    free(dp);
  
  return 0;
}

void rewinddir(DIR * dir_Info)
{
  /* Re-set to the beginning */
  dir_Info->handle = 0;
  dir_Info->offset = 0;
}

#endif // _WIN32

SbBool getFileExtension(char *ext, const char *filename, int maxlen)
{
  char *dotpos = strrchr(filename, '.');
  if ((dotpos == NULL) || (dotpos+1=='\0'))
    return FALSE; // no extension
  else {
    strncpy(ext, dotpos+1, maxlen);
    ext[maxlen-1]=0;
    return TRUE;
  }
};

SbList<SbString> createPlaylistFromDirectory(SbString dirname)
{
  SbList<SbString> playlist;
  DIR * dir = opendir(dirname.getString());
  if (dir == NULL) {
    SoDebugError::postWarning("createPlaylistFromDirectory()",
                              "Couldn't open directory. '%s'",
                              dirname.getString());
  }
  else {
    dirent *de;
    while ( (de = readdir(dir)) != NULL) {
      if (de->d_name[0] != '.') {
        char ext[4];
        if (getFileExtension(ext, de->d_name, 4)) {
          if (strcmpi(ext, "ogg")==0) {
            playlist.append(SbString(de->d_name));
          }
          else if (strcmpi(ext, "wav")==0) {
            playlist.append(SbString(de->d_name));
          }
        }
      }
    }
    closedir(dir);
  }
  return playlist;
};

#endif // HAVE_OPENAL





