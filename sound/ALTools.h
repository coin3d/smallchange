#ifndef ALTools_H
#define ALTools_H

#include <Inventor/SbVec3f.h>

#ifdef SOAL_SUB
#include <AL/altypes.h>
#else
#include <altypes.h>
#endif

inline void SbVec3f2ALfloat3(ALfloat *dest, const SbVec3f &source)
{
  source.getValue(dest[0], dest[1], dest[2]);
};

char *GetALErrorString(char *text, ALint errorcode);


#endif