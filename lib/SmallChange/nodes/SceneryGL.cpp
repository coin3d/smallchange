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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_WINDOWS_H
#include <windows.h>
#else /* !HAVE_WINDOWS_H */
#define APIENTRY
#endif /* !HAVE_WINDOWS_H */

#include <limits.h>
#include <assert.h>
#include <stdlib.h> // atoi()
#include <stdio.h>
#include <math.h> // fmod()

#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif // HAVE_DLFCN_H

#ifdef HAVE_OPENGL_GL_H
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

/* this source file is shared between SIM Scenery and SmallChange, hence the
 * strange conditional includes below */
#ifdef SS_MAJOR_VERSION
/* we are building in the SIM Scenery source repository */
#include "SbList.h"
#include "SbHash.h"
#include "SbVec3.h"
#include "SbBox3.h"
#include "SbPlane.h"

#include <sim/scenery/scenery.h>
#include <sim/scenery/SceneryGL.h>
#include <sim/cbase/debugerror.h>
#else /* !SS_MAJOR_VERSION */
/* we are building in the SmallChange source repository */
#include <Inventor/C/basic.h>
#include "../misc/SbList.h"
#include "../misc/SbHash.h"
#include "../misc/SbVec3.h"
#include "../misc/SbBox3.h"
#include "../misc/SbPlane.h"
#include <SmallChange/misc/SceneryGlue.h>
#include <SmallChange/nodes/SceneryGL.h>
#include <Inventor/errors/SoDebugError.h>
#endif /* !SS_MAJOR_VERSION */

/* ********************************************************************** */

// the number of frames a texture can be unused before being recycled
#define MAX_UNUSED_COUNT 200

#define VA_INTERLEAVED 1

#ifdef SS_SCENERY_H
#define debugerror sc_debugerror_post
#else
/* scenery.h has not been included directly */
#define debugerror SoDebugError::post
#define ss_render_get_elevation_measures sc_ssglue_render_get_elevation_measures
#define ss_render_get_texture_measures sc_ssglue_render_get_texture_measures
#define ss_render_get_texture_image sc_ssglue_render_get_texture_image
#define ss_render_get_undef_array sc_ssglue_render_get_undef_array
#endif

/* ********************************************************************** */

#ifndef GL_TEXTURE0
#define GL_TEXTURE0                       0x84C0
#endif /* !GL_TEXTURE0 */

#ifndef GL_TEXTURE1
#define GL_TEXTURE1                       0x84C1
#endif /* !GL_TEXTURE1 */

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE                  0x812F
#endif /* !GL_CLAMP_TO_EDGE */

#ifndef GL_OCCLUSION_TEST_HP
#define GL_OCCLUSION_TEST_HP              0x8165
#endif /* !GL_OCCLUSION_TEST_HP */

#ifndef GL_OCCLUSION_TEST_RESULT_HP
#define GL_OCCLUSION_TEST_RESULT_HP       0x8166
#endif /* !GL_OCCLUSION_TEST_RESULT_HP */

/* ********************************************************************** */
/* GL setup */

// prototypes
typedef void (APIENTRY * glPolygonOffset_f)(GLfloat factor, GLfloat units);

typedef void (APIENTRY * glGenTextures_f)(GLsizei n, GLuint * textures);
typedef void (APIENTRY * glBindTexture_f)(GLenum target, GLuint texture);
typedef void (APIENTRY * glTexImage2D_f)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid * pixels);
typedef void (APIENTRY * glDeleteTextures_f)(GLsizei n, const GLuint * textures);

typedef void (APIENTRY * glMultiTexCoord2f_f)(GLenum target, GLfloat x, GLfloat y);
typedef void (APIENTRY * glClientActiveTexture_f)(GLenum texture);

// 1.1
typedef void (APIENTRY * glEnableClientState_f)(GLenum cap);
typedef void (APIENTRY * glDisableClientState_f)(GLenum cap);
typedef void (APIENTRY * glVertexPointer_f)(GLint size, GLenum type, GLsizei stride, const GLvoid * ptr);
typedef void (APIENTRY * glNormalPointer_f)(GLenum type, GLsizei stride, const GLvoid * ptr);
typedef void (APIENTRY * glTexCoordPointer_f)(GLint dims, GLenum type, GLsizei stride, const GLvoid * ptr);
typedef void (APIENTRY * glDrawArrays_f)(GLenum mode, GLint first, GLsizei count);
typedef void (APIENTRY * glDrawElements_f)(GLenum mode, GLsizei count, GLenum type, const GLvoid * ptr);
// 1.2
typedef void (APIENTRY * glDrawRangeElements_f)( GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices );


// FIXME: there is a lot of duplicated effort in the OpenGL capability
// probing below, as this is already implemented for Coin --
// better. The best fix would probably be to separate out the GL glue
// part of Coin into a sub-library which can be used from any project,
// then use that.
//
// 20040426 mortene.
//
// UPDATE 20040713 mortene: I'm getting real errors from this, on a
// platform where the NVIDIA drivers are used for onscreen contexts,
// but for some reason the Microsoft OpenGL 1.1 software renderer is
// used for offscreen contexts. MS OGL 1.1 doesn't support
// GL_CLAMP_TO_EDGE, which causes textures (and all geometry with
// them) to simply disappear.

// state container definition
struct sc_GL {
  int initialized;

  // polygon offset (for 
  // glPolygonOffsetEnable_f glPolygonOffsetEnable(TRUE, styles);
  glPolygonOffset_f glPolygonOffset;

  // texturing
  glGenTextures_f glGenTextures;
  glBindTexture_f glBindTexture;
  glTexImage2D_f glTexImage2D;
  glDeleteTextures_f glDeleteTextures;

  // mutitexturing
  glMultiTexCoord2f_f glMultiTexCoord2f;
  glClientActiveTexture_f glClientActiveTexture;

  // vertexarrays
  glEnableClientState_f glEnableClientState;
  glDisableClientState_f glDisableClientState;
  glVertexPointer_f glVertexPointer;
  glNormalPointer_f glNormalPointer;
  glTexCoordPointer_f glTexCoordPointer;
  glDrawArrays_f glDrawArrays;
  glDrawElements_f glDrawElements;
  glDrawRangeElements_f glDrawRangeElements;

  // normalmaps

  // occlusion

  GLenum CLAMP_TO_EDGE;

  int USE_BYTENORMALS;
  int SUGGEST_BYTENORMALS;

  // features / extensions
  int HAVE_MULTITEXTURES;
  int HAVE_VERTEXARRAYS;
  int SUGGEST_VERTEXARRAYS;
  int HAVE_NORMALMAPS;
  int HAVE_OCCLUSIONTEST;
  int USE_OCCLUSIONTEST;
};

static SbHash<struct sc_GL *, unsigned int> * glctxhash = NULL;

static
struct sc_GL *
GLi(const unsigned int ctxid)
{
  if (glctxhash == NULL) {
    glctxhash = new SbHash<struct sc_GL *, unsigned int>;
    // FIXME: leak (should be deallocated on exit). 20040714 mortene.
  }

  struct sc_GL * gli = NULL;
  const int found = glctxhash->get(ctxid, gli);
  if (!found) {
    struct sc_GL tmp = {
      FALSE,    // "has been initialized by sc_probe_gl()" flag

      NULL,     // glPolygonOffset

      NULL,     // glGenTextures
      NULL,     // glBindTexture
      NULL,     // glTexImage2D
      NULL,     // glDeleteTextures

      NULL,     // glMultiTexCoord2f
      NULL,     // glClientActiveTexture

      NULL,     // glEnableClientState
      NULL,     // glDisableClientState
      NULL,     // glVertexPointer
      NULL,     // glNormalPointer
      NULL,     // glTexCoordPointer
      NULL,     // glDrawArrays
      NULL,     // glDrawElements
      NULL,     // glDrawRangeElements

      GL_CLAMP, // clamp_to_edge
      TRUE,     // USE_BYTENORMALS
      TRUE,     // SUGGEST_BYTENORMALS

      FALSE,    // HAVE_MULTITEXTURES
      FALSE,    // HAVE_VERTEXARRAYS
      FALSE,    // SUGGEST_VERTEXARRAYS
      FALSE,    // HAVE_NORMALMAPS
      FALSE,    // HAVE_OCCLUSIONTEST
      FALSE     // USE_OCCLUSIONTEST
    };

    // FIXME: there's got to be a more elegant way to alloc and init?
    // 20040714 mortene.
    gli = new struct sc_GL(tmp);

    // FIXME: leak. Never taken out again. 20040714 mortene.
    const int newentry = glctxhash->put(ctxid, gli);
    assert(newentry);
  }
  return gli;
}

// *************************************************************************

#define GL_FUNCTION_SETTER(func) \
static void \
sc_set_##func(unsigned int ctxid, void * fptr) \
{ \
  GLi(ctxid)->func = (func##_f) fptr; \
}

GL_FUNCTION_SETTER(glPolygonOffset)

/* texture objects */
GL_FUNCTION_SETTER(glGenTextures)
GL_FUNCTION_SETTER(glBindTexture)
GL_FUNCTION_SETTER(glTexImage2D)
GL_FUNCTION_SETTER(glDeleteTextures)

/* multi-texturing */
GL_FUNCTION_SETTER(glMultiTexCoord2f)
GL_FUNCTION_SETTER(glClientActiveTexture)

/* vertex arrays */
GL_FUNCTION_SETTER(glEnableClientState)
GL_FUNCTION_SETTER(glDisableClientState)
GL_FUNCTION_SETTER(glVertexPointer)
GL_FUNCTION_SETTER(glNormalPointer)
GL_FUNCTION_SETTER(glTexCoordPointer)
GL_FUNCTION_SETTER(glDrawArrays)
GL_FUNCTION_SETTER(glDrawElements)
GL_FUNCTION_SETTER(glDrawRangeElements)

#undef GL_FUNCTION_SETTER

void
sc_set_have_clamp_to_edge(unsigned int ctxid, int enable)
{
  if ( enable ) {
    GLi(ctxid)->CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE;
  } else {
    GLi(ctxid)->CLAMP_TO_EDGE = GL_CLAMP;
  }
}

int
sc_get_have_clamp_to_edge(unsigned int ctxid)
{
  return GLi(ctxid)->CLAMP_TO_EDGE;
}

void
sc_set_use_bytenormals(unsigned int ctxid, int enable)
{
  GLi(ctxid)->USE_BYTENORMALS = (enable != FALSE) ? TRUE : FALSE;
}

int
sc_get_use_bytenormals(unsigned int ctxid)
{
  return GLi(ctxid)->USE_BYTENORMALS;
}
 
void
sc_set_use_occlusion_test(unsigned int ctxid, int enable)
{
  GLi(ctxid)->USE_OCCLUSIONTEST = (enable != FALSE) ? TRUE : FALSE;
}

int
sc_get_use_occlusion_test(unsigned int ctxid)
{
  return GLi(ctxid)->USE_OCCLUSIONTEST;
}


int
sc_found_multitexturing(unsigned int ctxid)
{
  return GLi(ctxid)->HAVE_MULTITEXTURES;
}

int
sc_found_vertexarrays(unsigned int ctxid)
{
  return GLi(ctxid)->HAVE_VERTEXARRAYS;
}

int
sc_suggest_vertexarrays(unsigned int ctxid)
{
  return GLi(ctxid)->SUGGEST_VERTEXARRAYS;
}

int
sc_suggest_bytenormals(unsigned int ctxid)
{
  return GLi(ctxid)->SUGGEST_BYTENORMALS;
}

/* ********************************************************************** */

#ifdef HAVE_WINDOWS_H
#define APP_HANDLE_TYPE HINSTANCE
#define APP_HANDLE() GetModuleHandle("opengl32.dll")
#define APP_HANDLE_CLOSE(handle) /* nada */
#define GL_PROC_ADDRESS1(proc) (void *) wglGetProcAddress(#proc)
#define GL_PROC_ADDRESS2(proc) (void *) GetProcAddress(NULL, #proc)
#define GL_PROC_ADDRESS3(proc) (void *) GetProcAddress(handle, #proc)
#else
#define HAVE_DLFCN_H
#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#define APP_HANDLE_TYPE void *
#define APP_HANDLE() dlopen(NULL, RTLD_NOW)
#define APP_HANDLE_CLOSE(handle) dlclose(handle)
#define GL_PROC_ADDRESS1(proc) dlsym(handle, #proc)
#define GL_PROC_ADDRESS2(proc) NULL
#define GL_PROC_ADDRESS3(proc) NULL
#else
#error system not supported (for probing dynamic GL symbols)
#endif
#endif

#define GL_PROC_SEARCH(ptr, name) \
  ptr = NULL; \
  if ( !ptr ) ptr = GL_PROC_ADDRESS1(name); \
  if ( !ptr ) ptr = GL_PROC_ADDRESS1(name##ARB); \
  if ( !ptr ) ptr = GL_PROC_ADDRESS2(name); \
  if ( !ptr ) ptr = GL_PROC_ADDRESS2(name##ARB); \
  if ( !ptr ) ptr = GL_PROC_ADDRESS3(name); \
  if ( !ptr ) ptr = GL_PROC_ADDRESS3(name##ARB)

void
sc_probe_gl(const unsigned int ctxid, sc_msghandler_fp msghandler)
{
  // FIXME: should assert on having a current GL context here...

  struct sc_GL * GL = GLi(ctxid);
  if (GL->initialized) { return; }

  const int bufsize = 1024*16;
  char * buf = (char *) malloc(bufsize); /* *this* should be long enough */

  APP_HANDLE_TYPE handle = APP_HANDLE();
  void * ptr = NULL;

  // probe GL for extensions
  const char * vendor = (const char *) glGetString(GL_VENDOR);
  const char * version = (const char *) glGetString(GL_VERSION);

  if ( version == NULL ) {
    // no current context
    if ( msghandler )
      msghandler("PROBE: no current GL context - glGetString(GL_VERSION) returned NULL");
    else {
      debugerror("sc_probe_gl", "no current GL context - glGetString(GL_VERSION) returned NULL");
    }
    return;
  }

  // BYTE NORMALS:
  // The 3Dlabs driver has problems with normals given with glNormal3bv(), but
  // not with normals given with glNormal3f().  We therefore suggest doing
  // conversion to floats before sending to GL for 3Dlabs graphics cards.

  GL->SUGGEST_BYTENORMALS = TRUE;
  if ( strcmp(vendor, "3Dlabs") == 0 ) {
    // float normals doesn't bug where byte normals does
    GL->SUGGEST_BYTENORMALS = FALSE;
    if ( msghandler ) {
      msghandler("PROBE: detected 3Dlabs card - disabling bytepacked normals\n");
    }
  }

  int major = 0, minor = 0;
  sscanf(version, "%d.%d", &major, &minor);
  assert(major >= 1); // forget about major
  if ( msghandler ) {
    sprintf(buf, "PROBE: GL version %d.%d\n", major, minor);
    msghandler(buf);
  }

  const char * exts = (const char *) glGetString(GL_EXTENSIONS);
  if ( exts != NULL && msghandler != NULL ) {
    assert(strlen(exts) < (bufsize - 32));
    sprintf(buf, "PROBE: extensions: \"%s\"\n", exts);
    msghandler(buf);
  } else if ( msghandler ) {
    msghandler("PROBE: no GL_EXTENSIONS string\n");
  }

  GL->HAVE_MULTITEXTURES = FALSE;
  if ( (minor >= 3) || (exts && strstr(exts, "GL_ARB_multitexture ")) ) {
    // multi-texturing is available frmo OpenGL 1.3 and up
    GL->HAVE_MULTITEXTURES = TRUE;
    if ( msghandler ) {
      msghandler("PROBE: detected multitexturing support\n");
    }
  }

  // normal maps will make rendering with normals instead of textures
  // much nicer (no popping), and will combine better with non-lighted
  // textures.  (to be implemented)
  GL->HAVE_NORMALMAPS = FALSE;
  if ( (minor >= 10) || (exts && strstr(exts, "GL_whatever_normal_maps ")) ) {
    GL->HAVE_NORMALMAPS = TRUE;
  }

  // occlusion testing can probably optimize rendering quite a bit
  // (to be implemented)
  GL->HAVE_OCCLUSIONTEST = FALSE;
  GL->USE_OCCLUSIONTEST = FALSE;
  if ( exts && strstr(exts, "GL_HP_occlusion_test ") ) {
    GL->HAVE_OCCLUSIONTEST = TRUE;
    GL->USE_OCCLUSIONTEST = TRUE;
    if ( msghandler ) {
      msghandler("PROBE: installed hardware occlusion test support\n");
    }
  } else {
    if ( msghandler ) {
      msghandler("PROBE: hardware occlusion test not supported\n");
    }
  }

  assert(vendor);
  if ( strcmp(vendor, "ATI Technologies Inc.") == 0 ) {
    GL->USE_OCCLUSIONTEST = FALSE;
    if ( GL->HAVE_OCCLUSIONTEST && msghandler ) {
      msghandler("PROBE: we don't trust ATI's occlusion test - disabling\n");
    }
    // A case with false positive results for the GL_HP_occlusion_test,
    // causing nothing to be rendered, was found on an ATI graphics board
    // (Windows laptop), so we disable occlusion testing for ATI cards
    // by default (better safe than sorry at this stage), and declare
    // this feature for working on more or less a version-by-version
    // case.
  }

  GL->CLAMP_TO_EDGE = GL_CLAMP;
  if ( (minor >= 2) ||
       (exts &&
        (strstr(exts, "GL_EXT_texture_edge_clamp ") ||
         strstr(exts, "GL_SGIS_texture_edge_clamp "))) ) {
    GL->CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE;
    if ( msghandler ) {
      msghandler("PROBE: detected texture_edge_clamp\n");
    }
  }

  sc_set_glPolygonOffset(ctxid, NULL);
  if ( minor >= 1 ) {
    GL_PROC_SEARCH(ptr, glPolygonOffset);
    if ( msghandler ) {
      sprintf(buf, "PROBE: glPolygonOffset = %p\n", ptr);
      msghandler(buf);
    }
    sc_set_glPolygonOffset(ctxid, ptr);
  }

  sc_set_glGenTextures(ctxid, NULL);
  sc_set_glBindTexture(ctxid, NULL);
  sc_set_glTexImage2D(ctxid, NULL);
  sc_set_glDeleteTextures(ctxid, NULL);
  if ( minor >= 1 ) {
    GL_PROC_SEARCH(ptr, glGenTextures);
    if ( msghandler ) {
      sprintf(buf, "PROBE: glGenTextures = %p\n", ptr);
      msghandler(buf);
    }
    assert(ptr);
    sc_set_glGenTextures(ctxid, ptr);

    GL_PROC_SEARCH(ptr, glBindTexture);
    if ( msghandler ) {
      sprintf(buf, "PROBE: glBindTexture = %p\n", ptr);
      msghandler(buf);
    }
    assert(ptr);
    sc_set_glBindTexture(ctxid, ptr);

    GL_PROC_SEARCH(ptr, glTexImage2D);
    if ( msghandler ) {
      sprintf(buf, "PROBE: glTexImage2D = %p\n", ptr);
      msghandler(buf);
    }
    assert(ptr);
    sc_set_glTexImage2D(ctxid, ptr);

    GL_PROC_SEARCH(ptr, glDeleteTextures);
    if ( msghandler ) {
      sprintf(buf, "PROBE: glDeleteTextures = %p\n", ptr);
      msghandler(buf);
    }
    assert(ptr);
    sc_set_glDeleteTextures(ctxid, ptr);
  }

  sc_set_glMultiTexCoord2f(ctxid, NULL);
  sc_set_glClientActiveTexture(ctxid, NULL);
  if ( GL->HAVE_MULTITEXTURES ) {
    GL_PROC_SEARCH(ptr, glMultiTexCoord2f);
    if ( msghandler ) {
      sprintf(buf, "PROBE: glMultiTexCoord2f = %p\n", ptr);
      msghandler(buf);
    }
    sc_set_glMultiTexCoord2f(ctxid, ptr);
    if ( ptr ) {
      // multi-texturing + vertex-arrays
      GL_PROC_SEARCH(ptr, glClientActiveTexture);
      if ( msghandler ) {
        sprintf(buf, "PROBE: glClientActiveTexture = %p\n", ptr);
        msghandler(buf);
      }
      sc_set_glClientActiveTexture(ctxid, ptr);
      if ( msghandler ) {
        msghandler("PROBE: installed multi-texturing support\n");
      }
    } else {
      // Apparently, Windows software rendering can report TRUE on
      // multitexturing, withough having glMultiTexCoord2f available
      GL->HAVE_MULTITEXTURES = FALSE;
      sc_set_glClientActiveTexture(ctxid, NULL);
      if ( msghandler ) {
        msghandler("PROBE: disabled multi-texturing support\n");
      }
    }
  }
  else {
    if ( msghandler ) {
      msghandler("PROBE: multi-texturing not supported\n");
    }
  }

  sc_set_glEnableClientState(ctxid, NULL);
  sc_set_glDisableClientState(ctxid, NULL);
  sc_set_glVertexPointer(ctxid, NULL);
  sc_set_glNormalPointer(ctxid, NULL);
  sc_set_glTexCoordPointer(ctxid, NULL);
  sc_set_glDrawArrays(ctxid, NULL);
  sc_set_glDrawElements(ctxid, NULL);
  sc_set_glDrawRangeElements(ctxid, NULL);
  if ( minor >= 1 ) {
    GL_PROC_SEARCH(ptr, glEnableClientState);
    if ( msghandler ) {
      sprintf(buf, "PROBE: glEnableClientState = %p\n", ptr);
      msghandler(buf);
    }
    assert(ptr);
    sc_set_glEnableClientState(ctxid, ptr);

    GL_PROC_SEARCH(ptr, glDisableClientState);
    if ( msghandler ) {
      sprintf(buf, "PROBE: glDisableClientState = %p\n", ptr);
      msghandler(buf);
    }
    assert(ptr);
    sc_set_glDisableClientState(ctxid, ptr);

    GL_PROC_SEARCH(ptr, glVertexPointer);
    if ( msghandler ) {
      sprintf(buf, "PROBE: glVertexPointer = %p\n", ptr);
      msghandler(buf);
    }
    assert(ptr);
    sc_set_glVertexPointer(ctxid, ptr);

    GL_PROC_SEARCH(ptr, glNormalPointer);
    if ( msghandler ) {
      sprintf(buf, "PROBE: glNormalPointer = %p\n", ptr);
      msghandler(buf);
    }
    assert(ptr);
    sc_set_glNormalPointer(ctxid, ptr);

    GL_PROC_SEARCH(ptr, glTexCoordPointer);
    if ( msghandler ) {
      sprintf(buf, "PROBE: glTexCoordPointer = %p\n", ptr);
      msghandler(buf);
    }
    assert(ptr);
    sc_set_glTexCoordPointer(ctxid, ptr);

    GL_PROC_SEARCH(ptr, glDrawArrays);
    if ( msghandler ) {
      sprintf(buf, "PROBE: glDrawArrays = %p\n", ptr);
      msghandler(buf);
    }
    assert(ptr);
    sc_set_glDrawArrays(ctxid, ptr);

    GL_PROC_SEARCH(ptr, glDrawElements);
    if ( msghandler ) {
      sprintf(buf, "PROBE: glDrawElements = %p\n", ptr);
      msghandler(buf);
    }
    assert(ptr);
    sc_set_glDrawElements(ctxid, ptr);

    if ( minor >= 2 ) {
      GL_PROC_SEARCH(ptr, glDrawRangeElements);
      if ( msghandler ) {
        sprintf(buf, "PROBE: glDrawRangeElements = %p\n", ptr);
        msghandler(buf);
      }
      sc_set_glDrawRangeElements(ctxid, ptr);
    } else {
      GL_PROC_SEARCH(ptr, glDrawRangeElements);
      if ( msghandler ) {
        sprintf(buf, "PROBE: glDrawRangeElements = %p\n", ptr);
        msghandler(buf);
      }
      sc_set_glDrawRangeElements(ctxid, ptr);
    }

    if ( GL->glVertexPointer != NULL ) {
      GL->HAVE_VERTEXARRAYS = TRUE;
      if ( msghandler ) {
        msghandler("PROBE: installed vertex-arrays support\n");
      }
    } else {
      GL->HAVE_VERTEXARRAYS = FALSE;
      if ( msghandler ) {
        msghandler("PROBE: vertex-arrays should have been supported\n");
      }
    }
  }
  else {
    GL->HAVE_VERTEXARRAYS = FALSE;
    if ( msghandler ) {
      msghandler("PROBE: vertex-arrays not supported\n");
    }
  }
    
  // Vertex arrays are available in OpenGL 1.1 and up.
  // It is currently the preferred rendering loop.
  GL->SUGGEST_VERTEXARRAYS = GL->HAVE_VERTEXARRAYS;

  APP_HANDLE_CLOSE(handle);

  free(buf);

  GL->initialized = TRUE;
}

#undef APP_HANDLE_TYPE
#undef APP_HANDLE
#undef APP_HANDLE_CLOSE
#undef GL_PROC_ADDRESS1
#undef GL_PROC_ADDRESS2
#undef GL_PROC_SEARCH

/* ********************************************************************** */

struct RenderStateP {
  RenderStateP(void)
  {
    this->vertexcount = 0;
    this->dataset = -1;
    this->blockinfo = NULL;

    this->scenerytexid = 0;

    this->glcontextid = UINT_MAX;
    this->glcontextidset = FALSE;

    this->activetexturecontext = UINT_MAX;
  }

  ~RenderStateP()
  {
    // FIXME: should control that the hash is empty at this point, or
    // there will be undestructed subhashes, with undestructed texture
    // structs. 20040602 mortene.
  }

  SbHash<SbHash<class TexInfo *, unsigned int> *, unsigned int> contexthashes;
  SbList<int> cullstate;

  unsigned int glcontextid;
  int glcontextidset;

  unsigned int activetexturecontext;

  // local block info
  float tscale[2];
  float toffset[2];
  float invtsizescale[2]; // (1.0f / blocksize) * tscale[n]
  unsigned int scenerytexid;

  // picking
  int intersected;
  float intersection[3];
  int dataset;
  ss_render_block_cb_info * blockinfo;

  // internal vertex array state data
  int vertexcount;

  // interleaved
  float vertices[10*3];
  unsigned char normals[10*3];
  float fnormals[10*3];
  float texture1[10*2];
  float texture2[10*2];

  // post-block loop
  SbList<float> vertexarray;
  SbList<signed char> normalarray;
  SbList<float> texcoord1array;
  SbList<float> texcoord2array;
  SbList<unsigned int> idxarray;
  SbList<int> lenarray;

  // debugging
  SbList<float> debuglist;
};

#define PRIVATE(s) ((s)->pimpl)

/* ********************************************************************** */
/* texture management */

struct texture_info {
  GLuint id;
  unsigned char * data;
  int texwidth;
  int texheight;
  int components;
  int wraps;
  int wrapt;
  float quality;
  GLboolean isbound; // if glBindTexture()+glTexImage2D() has been done
};

static void *
sc_default_texture_construct(RenderState * state, unsigned char * data,
                             int texw, int texh, int nc,
                             int wraps, int wrapt, float q)
{
  texture_info * info = new texture_info;
  assert(info != NULL);

  info->id = 0;
  info->data = data;
  info->texwidth = texw;
  info->texheight = texw;
  info->components = nc;
  info->wraps = wraps;
  info->wrapt = wrapt;
  info->quality = q;
  info->isbound = GL_FALSE;

  const unsigned int ctxid = PRIVATE(state)->glcontextid;
  assert(GLi(ctxid)->glGenTextures);

  GLi(ctxid)->glGenTextures(1, &info->id);

  return info;
}

static void
sc_default_texture_activate(RenderState * state, void * handle)
{
  texture_info * info = (texture_info *) handle;
  assert(info != NULL);
  
  const unsigned int ctxid = PRIVATE(state)->glcontextid;
  const struct sc_GL * GL = GLi(ctxid);

  assert(GL->glBindTexture);
  assert(GL->glTexImage2D);

  GL->glBindTexture(GL_TEXTURE_2D, info->id);

  // When texture has been passed to GL driver, it will be sufficient
  // with just the glBindTexture() call on successive activations.
  //
  // FIXME: on some systems (with various GL drivers on MSWindows), we
  // get a crash on the glTexImage2D() call below on the second
  // invocation. That shouldn't happen as long as the info->data
  // pointer is still valid (it might not be). Investigate
  // why. 20040607 mortene.
  if (info->isbound) { return; }

  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, info->wraps);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, info->wrapt);
#if 1
  // FIXME:  Consider mipmapping.  I'm not sure it's needed for
  // anything but the top block, since the texture is LODed along
  // with the terrain blocks already.  However, for sattelite view
  // of the top block, it would definitely be a good idea.
  // 20031123 larsa
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#else
  // for non bi-linear filtering (for hard texel-edges)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#endif

  // void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid * pixels);

  GL->glTexImage2D(GL_TEXTURE_2D, 0, info->components,
                   info->texwidth, info->texheight, 0,
                   GL_RGBA, GL_UNSIGNED_BYTE, info->data);

  info->isbound = GL_TRUE;

  // glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}

static void
sc_default_texture_release(RenderState * state, void * handle)
{
  texture_info * info = (texture_info *) handle;
  assert(info);

  const unsigned int ctxid = PRIVATE(state)->glcontextid;
  const struct sc_GL * GL = GLi(ctxid);
  assert(GL->glDeleteTextures);
  GL->glDeleteTextures(1, &info->id);
  delete info;
}

typedef void * sc_texture_construct_f(RenderState * state, unsigned char * data, int texw, int texh, int nc, int wraps, int wrapt, float q);
typedef void sc_texture_activate_f(RenderState * state, void * handle);
typedef void sc_texture_release_f(RenderState * state, void * handle);

static sc_texture_construct_f * texture_construct = sc_default_texture_construct;
static sc_texture_activate_f * texture_activate = sc_default_texture_activate;
static sc_texture_release_f * texture_release = sc_default_texture_release;

// FIXME: not used as part of public API, so not really any point in
// this round-about way of setting up callback functions? 20040714 mortene.
static void
sc_set_texture_functions(sc_texture_construct_f * construct,
                         sc_texture_activate_f * activate,
                         sc_texture_release_f * release)
{
  texture_construct = construct;
  texture_activate = activate;
  texture_release = release;
}

/* ********************************************************************** */

typedef void * sc_texture_create(void);

class TexInfo {
public:
  TexInfo() {
    this->clienttexdata = NULL;
  }
  void * clienttexdata;
  int unusedcount;
};

/* ********************************************************************** */

void
sc_renderstate_construct(RenderState * state)
{
  // construct lists

  state->clipplanes = NULL;
  state->numclipplanes = 0;

  state->dotex = TRUE;

  state->renderpass = FALSE;

  PRIVATE(state) = new struct RenderStateP;
}

void
sc_renderstate_destruct(RenderState * state)
{
  sc_delete_all_textures(state);

  delete PRIVATE(state);
  PRIVATE(state) = NULL;

  // clipplanes are not owned by RenderState - managed elsewhere
}

/* ********************************************************************** */

void
sc_set_current_context_id(RenderState * state, unsigned int context)
{
//   printf("sc_set_current_context_id(%p, %d)\n", state, context);

  PRIVATE(state)->glcontextid = context;
  PRIVATE(state)->glcontextidset = TRUE;
}

void
sc_unset_current_context(RenderState * state)
{
//   printf("sc_unset_current_context(%p)\n", state);

  PRIVATE(state)->glcontextid = UINT_MAX;
  PRIVATE(state)->glcontextidset = FALSE;
}

static SbHash<TexInfo *, unsigned int> *
sc_get_context_texhash(RenderState * state)
{
  assert(PRIVATE(state)->glcontextidset);
  const unsigned int key = PRIVATE(state)->glcontextid;

  SbHash<TexInfo *, unsigned int> * texhash = NULL;

  const int found = PRIVATE(state)->contexthashes.get(key, texhash);

  if (!found) {
    // debug
//     printf("making new hash on context %u (for RenderState %p)\n", key, state);
    texhash = new SbHash<TexInfo *, unsigned int>;
    PRIVATE(state)->contexthashes.put(key, texhash);
  }

  return texhash;
}

/* ********************************************************************** */

static TexInfo *
sc_find_texture(RenderState * state, unsigned int key)
{
  TexInfo * tex = NULL;
  return sc_get_context_texhash(state)->get(key, tex) ? tex : NULL;
}

static TexInfo *
sc_place_texture_in_hash(RenderState * state, unsigned int key, void * clienttexdata)
{
  assert(state);

  TexInfo * tex = new TexInfo;
  tex->unusedcount = 0;
  tex->clienttexdata = clienttexdata;

  sc_get_context_texhash(state)->put(key, tex);
  return tex;
}

static void sc_texture_hash_inc_unused(const unsigned int & key, TexInfo * const & val, void * closure);


// FIXME: the idea of making this the client code's responsibility
// seems silly -- unused textures could be handled and destructed
// automatically, methinks. 20040512 mortene.
void
sc_delete_unused_textures(RenderState * state)
{
  assert(state);

  // FIXME: when called outside of a GL context, should just tag
  // textures for destruction. 20040602 mortene.
  if (!PRIVATE(state)->glcontextidset) { return; }

  // FIXME: only the textures in the currently active context will be
  // destructed as it is now. Should in addition place all non-current
  // textures in a list for later destruction (when the correct
  // context is made current again). 20040602 mortene.

  SbList<unsigned int> keylist;
  SbHash<TexInfo *, unsigned int> * texhash = sc_get_context_texhash(state);

  texhash->makeKeyList(keylist);
  
  for (int i = 0; i < keylist.getLength(); i++) {
    const unsigned int key = keylist[i];
    TexInfo * tex = NULL;
    texhash->get(key, tex);
    assert(tex);

    if (tex->unusedcount > MAX_UNUSED_COUNT) {
      texture_release(state, tex->clienttexdata);
      delete tex;
      texhash->remove(key);
    }
  }
}

void
sc_delete_all_textures(RenderState * state)
{
  assert(state);

  // FIXME: when called outside of a GL context, should just tag
  // textures for destruction. 20040602 mortene.
  if (!PRIVATE(state)->glcontextidset) { return; }


  // FIXME: only the textures in the currently active context will be
  // destructed as it is now. Should in addition place all non-current
  // textures in a list for later destruction (when the correct
  // context is made current again). 20040602 mortene.

  SbList<unsigned int> keylist;
  SbHash<TexInfo *, unsigned int> * texhash = sc_get_context_texhash(state);

  texhash->makeKeyList(keylist);

  for (int i = 0; i < keylist.getLength(); i++ ) {
    const unsigned int key = keylist[i];
    TexInfo * tex = NULL;
    texhash->get(key, tex);
    assert(tex);
    texhash->remove(key);

    texture_release(state, tex->clienttexdata);
    delete tex;
  }
}

void
sc_mark_unused_textures(RenderState * state)
{
  // FIXME: only the textures in the currently active context will
  // have their "unused" counter increased. Must be fixed -- all
  // textures from all contexts should be marked. 20040602 mortene.

  sc_get_context_texhash(state)->apply(sc_texture_hash_inc_unused, NULL);
}

/* ********************************************************************** */

void
sc_texture_hash_inc_unused(const unsigned int & key, TexInfo * const & tex, void * closure)
{
  assert(tex);
  tex->unusedcount++;
}

/* ********************************************************************** */

void
sc_generate_elevation_line_texture(float distance,
                                   float offset,
                                   float thickness,
                                   int emphasis,
                                   unsigned char * buffer,
                                   int components,
                                   int texturesize,
                                   float * texcoordscale,
                                   float * texcoordoffset)
{
  assert(distance > 0.0f);
  assert(thickness > 0.0f);
  assert(components > 0 && components <= 4);
  assert(components == 4);

  int i, j;
  // clear texture to white first
  for ( i = 0; i < texturesize; i++ ) {
    for ( j = 0; j < components; j++ ) {
      buffer[i*components+j] = 255;
    }
  }

  // in case of emphasis, the texture has to include N lines, not just 1
  float lines = 1.0f;
  if ( emphasis > 1 ) lines = (float) emphasis;

  // set elevation lines to black
  float pixelsize = distance / (float(texturesize) / lines);
  float pixelpos = 0.0f;
  int bolds = 0;
  for ( i = 0; i < texturesize; i++ ) {
    float pixelpos = float(i) * pixelsize;
    if ( (fabs(fmod(pixelpos, distance)) <= (thickness * 0.5f)) ||
         ((distance - fabs(fmod(pixelpos, distance))) < (thickness * 0.5f)) ) {
      for ( j = 0; j < components; j++ ) {
        buffer[i*components+j] = 0;
      }
      if ( (components == 2) || (components == 4) ) { // alpha channel
        buffer[i*components+components-1] = 255;
      }
    }
    else if ( lines > 1.0f ) { /* emphasis is on */
      if ( (fabs(fmod(pixelpos, (lines * distance))) <= (thickness * 1.5f)) ||
           (((lines * distance) - fabs(fmod(pixelpos, (lines * distance)))) < (thickness * 1.5f)) ) {
        for ( j = 0; j < components; j++ ) {
          buffer[i*components+j] = 0;
        }
        if ( (components == 2) || (components == 4) ) {
          buffer[i*components+components-1] = 255;
        }
        bolds++;
      }
    }
  }

  *texcoordscale = 1.0f / (lines * distance);
  *texcoordoffset = offset * (*texcoordscale);
}

/* ********************************************************************** */

void
sc_init_debug_info(RenderState * state)
{
  PRIVATE(state)->debuglist.truncate(0);
}

void
sc_display_debug_info(RenderState * state, float * campos, short * vpsize)
{
  assert(state);

  int depthtest = glIsEnabled(GL_DEPTH_TEST);
  if ( depthtest ) glDisable(GL_DEPTH_TEST);

  glPushAttrib(GL_CURRENT_BIT|GL_ENABLE_BIT);
  glDisable(GL_LIGHTING);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(-0.5f, 0.5f, -0.5f, 0.5f, -1.0f, 1.0f);
  glDepthMask(GL_FALSE);
  glColor3f(1.0f, 0.0f, 0.0f);

  int num = PRIVATE(state)->debuglist.getLength() / 4;
  int i;

  float mind = 1.0f;

  for (i = 0; i < num; i++) {
    float x0, x1;
    x0 = PRIVATE(state)->debuglist.operator[](i*4);
    x1 = PRIVATE(state)->debuglist.operator[](i*4+2);

    float d = x1-x0;
    if (d < mind) mind = d;
  }

  float numpix = vpsize[0] * mind;

  float scale = 0.5f;
  if (numpix < 4.0f) {
    scale = 4.0f / numpix;
  }

  for (i = 0; i < num; i++) {

    float x0, x1, y0, y1;
    x0 = PRIVATE(state)->debuglist.operator[](i*4) - 0.5f;
    y0 = PRIVATE(state)->debuglist.operator[](i*4+1) - 0.5f;
    x1 = PRIVATE(state)->debuglist.operator[](i*4+2) - 0.5f;
    y1 = PRIVATE(state)->debuglist.operator[](i*4+3) - 0.5f;

    x0 -= campos[0];
    x1 -= campos[0];
    y0 -= campos[1];
    y1 -= campos[1];

    x0 *= scale;
    x1 *= scale;
    y0 *= scale;
    y1 *= scale;

    glBegin(GL_LINE_LOOP);
    glVertex3f(x0, y0, 0.0f);
    glVertex3f(x1, y0, 0.0f);
    glVertex3f(x1, y1, 0.0f);
    glVertex3f(x0, y1, 0.0f);
    glEnd();
  }

  glPopAttrib();
  if ( depthtest ) glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}

/* ********************************************************************** */
/* culling callbacks */

int
sc_plane_culling_pre_cb(void * closure, const double * bmin, const double * bmax)
{
  assert(closure);
  RenderState * state = (RenderState *) closure;

  // when a block is not culled, it is either rendered, or its
  // children are checked against the cull planes.  therefore - pre()
  // and post() callbacks can be used stack- based and flags can be
  // set to avoid testing against certain planes further down in the
  // recursion when all corners are inside (inside == 8) the plane.

  int mask = 0;
  if ( PRIVATE(state)->cullstate.getLength() > 0 ) {
    // This optimizes culling by telling which planes we do not need
    // to cull against.  It can only be enabled as long as we recurse
    // the quadtree.  If random order is implemented to render from
    // front to back, this will have to be disabled.
    mask = PRIVATE(state)->cullstate.getLast();
  }

  // set up bbox corner points
  SbVec3<float> point[8];
  int i;
  for ( i = 0; i < 8; i++ ) {
    point[i].setValue((float) ((i & 1) ? bmin[0] : bmax[0]),
                      (float) ((i & 2) ? bmin[1] : bmax[1]),
                      (float) ((i & 4) ? bmin[2] : bmax[2]));
  }
  int j, bits = 0;
  int total_inside = 0, total_outside = 0;
  if ( state->numclipplanes > 0 ) {
    assert(state->clipplanes);
    for ( i = 0; i < state->numclipplanes; i++ ) { // foreach plane
      if ( (mask & (1 << i)) != 0 ) {
        total_inside += 8;
        continue; // uncullable plane - all corners will be inside
      }
      SbVec3<float> normal(state->clipplanes[i*4+0], state->clipplanes[i*4+1], state->clipplanes[i*4+2]);
      float distance = state->clipplanes[i*4+3];
      SbPlane<float> plane(normal, distance);
      int outside = 0, inside = 0;
      for ( j = 0; j < 8; j++ ) { // foreach bbox corner point
        if ( !plane.isInHalfSpace(point[j]) ) { outside++; }
        else { inside++; }
      }
      total_inside += inside;
      total_outside += outside;
      if ( inside == 8 ) { // mark this plane as uncullable
        bits = bits | (1 << i);
      }
      if ( outside == 8 ) {
        PRIVATE(state)->cullstate.push(0); // push state since post_cb pops it
        return FALSE; // culled
      }
    }
  }
  PRIVATE(state)->cullstate.push(mask | bits); // push culling state for next iteration

  // Use the GL_HP_occlusion_test extension to check if bounding box will
  // be totally occluded.

  // Some ATI card returned false positives for the occlusion test.
  // When trying to figure out if there was a problem with my code, I
  // thought of the case where the closest faces of the bounding box will
  // be closer to the camera than the near plane, and therefore not
  // rendered.  This could cause the Z-buffer to be unaffected by
  // rendering the bounding box, causing result to be false and the
  // bounding box to be culled.  I've therefore added the restriction
  // that the bounding box must be totally inside the view volume for
  // this occlusion test to be tried at all.
  // PROLOGUE: The occlusion test is still not enabled for ATI cards
  // again though.

  const unsigned int ctxid = PRIVATE(state)->glcontextid;
  const struct sc_GL * GL = GLi(ctxid);

  if ( state->renderpass && GL->USE_OCCLUSIONTEST ) {
    if ( (state->numclipplanes > 0) && (total_inside == (state->numclipplanes * 8)) ) {
      // save GL state
      glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT);
      // disable backface culling
      glDisable(GL_CULL_FACE);
      // disable updates to color and depth buffer
      glDepthMask(GL_FALSE);
      glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
      // enable occlusion test
      glEnable(GL_OCCLUSION_TEST_HP);
      // render bounding geometry
      glBegin(GL_TRIANGLE_FAN);
      glVertex3f((float) bmin[0], (float) bmin[1], (float) bmin[2]); // center
      glVertex3f((float) bmax[0], (float) bmin[1], (float) bmin[2]); // start
      glVertex3f((float) bmax[0], (float) bmax[1], (float) bmin[2]);
      glVertex3f((float) bmin[0], (float) bmax[1], (float) bmin[2]);
      glVertex3f((float) bmin[0], (float) bmax[1], (float) bmax[2]);
      glVertex3f((float) bmin[0], (float) bmin[1], (float) bmax[2]);
      glVertex3f((float) bmax[0], (float) bmin[1], (float) bmax[2]);
      glVertex3f((float) bmax[0], (float) bmin[1], (float) bmin[2]); // finish = start
      glEnd();
      // and the other side
      glBegin(GL_TRIANGLE_FAN);
      glVertex3f((float) bmax[0], (float) bmax[1], (float) bmax[2]); // center
      glVertex3f((float) bmin[0], (float) bmax[1], (float) bmax[2]); // start
      glVertex3f((float) bmin[0], (float) bmin[1], (float) bmax[2]);
      glVertex3f((float) bmax[0], (float) bmin[1], (float) bmax[2]);
      glVertex3f((float) bmax[0], (float) bmin[1], (float) bmin[2]);
      glVertex3f((float) bmax[0], (float) bmax[1], (float) bmin[2]);
      glVertex3f((float) bmin[0], (float) bmax[1], (float) bmin[2]);
      glVertex3f((float) bmin[0], (float) bmax[1], (float) bmax[2]); // finish = start
      glEnd();
      // disable occlusion test
      glDisable(GL_OCCLUSION_TEST_HP);
      // restore state
      glPopAttrib();
      // read occlusion test result
      GLboolean result;
      glGetBooleanv(GL_OCCLUSION_TEST_RESULT_HP, &result);
      if ( !result ) { return FALSE; } // culled
    }
  }
  return TRUE; // not culled
}

void
sc_plane_culling_post_cb(void * closure)
{
  assert(closure);
  RenderState * state = (RenderState *) closure;
  PRIVATE(state)->cullstate.pop();
}

static int
sc_ray_culling_pre_cb(void * closure, const double * bmin, const double * bmax)
{
  assert(closure);
  RenderState * state = (RenderState *) closure;
  
  if ( (bmin[2] == bmax[2]) || (bmin[0] == bmax[0]) || (bmin[1] == bmax[1]) ) {
    return FALSE; // empty volume - culled
  }

  const SbVec3<float> bounds[2] = {
    SbVec3<float>((float) bmin[0], (float) bmin[1], (float) bmin[2]),
    SbVec3<float>((float) bmax[0], (float) bmax[1], (float) bmax[2])
  };
  const SbVec3<float> raypos(state->raypos[0], state->raypos[1], state->raypos[2]);
  const SbVec3<float> raydir(state->raydir[0], state->raydir[1], state->raydir[2]);

  int i, j;
  for ( j = 0; j < 2; j++ ) {
    for ( i = 0; i < 3; i++ ) {
      SbVec3<float> normal(0.0f, 0.0f, 0.0f);
      normal[i] = 1.0f;
      if ( raydir.dot(normal) == 0.0f ) continue; // ray parallel to plane

      const float t = (bounds[j][i] - normal.dot(raypos)) / normal.dot(raydir);
      const SbVec3<float> intersectionpoint = raypos + raydir * t;
      const int dim1 = (i+1) % 3, dim2 = (i+2) % 3;
      if ( (intersectionpoint[dim1] >= bounds[0][dim1]) &&
           (intersectionpoint[dim1] <= bounds[1][dim1]) &&
           (intersectionpoint[dim2] >= bounds[0][dim2]) &&
           (intersectionpoint[dim2] <= bounds[1][dim2]) ) {
        // we have intersection inside one of the bounding box faces
        return TRUE; // not culled
      }
    }
  }
  return FALSE; // culled
}

static void
sc_ray_culling_post_cb(void * closure)
{
  // nothing to do here
  // assert(closure);
  // RenderState * state = (RenderState *) closure;
}

/* ********************************************************************** */

inline void
GL_VERTEX(RenderState * state, const struct sc_GL * GL,
          const int x, const int y, const float elev)
{
  glTexCoord2f(PRIVATE(state)->toffset[0] + float(x) * PRIVATE(state)->invtsizescale[0],
               PRIVATE(state)->toffset[1] + float(y) * PRIVATE(state)->invtsizescale[1]);
  
  if (state->etexscale != 0.0f && GL->glMultiTexCoord2f != NULL) {
    GL->glMultiTexCoord2f(GL_TEXTURE1,
                          0.0f, (state->etexscale * elev) + state->etexoffset);
  }
  glVertex3f((float) (x*state->vspacing[0] + state->voffset[0]),
             (float) (y*state->vspacing[1] + state->voffset[1]),
             elev);
}

inline void
GL_VA_VERTEX(RenderState * state, const int x, const int y, const float elev)
{
#if VA_INTERLEAVED
  const int idx = PRIVATE(state)->vertexcount;
  PRIVATE(state)->texture1[idx*2+0] = PRIVATE(state)->toffset[0] + float(x) * PRIVATE(state)->invtsizescale[0];
  PRIVATE(state)->texture1[idx*2+1] = PRIVATE(state)->toffset[1] + float(y) * PRIVATE(state)->invtsizescale[1];
  PRIVATE(state)->texture2[idx*2+0] = 0.0f;
  PRIVATE(state)->texture2[idx*2+1] = (state->etexscale * elev) + state->etexoffset;
  PRIVATE(state)->vertices[idx*3+0] = (float) (x*state->vspacing[0] + state->voffset[0]);
  PRIVATE(state)->vertices[idx*3+1] = (float) (y*state->vspacing[1] + state->voffset[1]);
  PRIVATE(state)->vertices[idx*3+2] = elev;
  PRIVATE(state)->vertexcount += 1;
#else // !VA_INTERLEAVED
  static float vertex[3][3];
  static float texture1[3][2];
  static float texture2[3][2];

  const int idx = ((PRIVATE(state)->vertexcount == 0) || (GL.glDrawElements != NULL)) ? 0 : 2; // store fan center in idx 0
  if ( idx != 0 ) { // move up old entries
    vertex[1][0] = vertex[2][0];
    vertex[1][1] = vertex[2][1];
    vertex[1][2] = vertex[2][2];
    texture1[1][0] = texture1[2][0];
    texture1[1][1] = texture1[2][1];
    texture2[1][0] = texture2[2][0];
    texture2[1][1] = texture2[2][1];
  }
  texture1[idx][0] = PRIVATE(state)->toffset[0] + float(x) * PRIVATE(state)->invtsizescale[0];
  texture1[idx][1] = PRIVATE(state)->toffset[1] + float(y) * PRIVATE(state)->invtsizescale[1];
  texture2[idx][0] = 0.0f;
  texture2[idx][1] = (state->etexscale * elev) + state->etexoffset;
  vertex[idx][0] = (float) (x*state->vspacing[0] + state->voffset[0]);
  vertex[idx][1] = (float) (y*state->vspacing[1] + state->voffset[1]);
  vertex[idx][2] = elev;

  if ( GL.glDrawElements != NULL ) {
    PRIVATE(state)->vertexarray.append(vertex[0][0]);
    PRIVATE(state)->vertexarray.append(vertex[0][1]);
    PRIVATE(state)->vertexarray.append(vertex[0][2]);
    PRIVATE(state)->texcoord1array.append(texture1[0][0]);
    PRIVATE(state)->texcoord1array.append(texture1[0][1]);
    PRIVATE(state)->texcoord2array.append(texture2[0][0]);
    PRIVATE(state)->texcoord2array.append(texture2[0][1]);
    PRIVATE(state)->vertexcount += 1;
  } else if ( PRIVATE(state)->vertexcount >= 2 ) {
    // render is done as triangles
    for ( int v = 0; v < 3; v++ ) {
      PRIVATE(state)->vertexarray.append(vertex[v][0]);
      PRIVATE(state)->vertexarray.append(vertex[v][1]);
      PRIVATE(state)->vertexarray.append(vertex[v][2]);
      PRIVATE(state)->texcoord1array.append(texture1[v][0]);
      PRIVATE(state)->texcoord1array.append(texture1[v][1]);
      PRIVATE(state)->texcoord2array.append(texture2[v][0]);
      PRIVATE(state)->texcoord2array.append(texture2[v][1]);
    }
    PRIVATE(state)->vertexcount += 1;
  }
#endif // !VA_INTERLEAVED
}

inline void
GL_VERTEX_N(RenderState * state, const struct sc_GL * GL,
            const int x, const int y, const float elev, const signed char * n)
{
  if ( GL->USE_BYTENORMALS ) {
    glNormal3bv((const GLbyte *)n);
  }
  else {
    static const float factor = 1.0f/127.0f;
    glNormal3f(n[0] * factor, n[1] * factor, n[2] * factor);
  }
  glVertex3f((float) (x*state->vspacing[0] + state->voffset[0]),
             (float) (y*state->vspacing[1] + state->voffset[1]),
             elev);
}

inline void
GL_VA_VERTEX_N(RenderState * state, const struct sc_GL * GL,
               const int x, const int y, const float elev, const signed char * n)
{
#if VA_INTERLEAVED
  const int idx = PRIVATE(state)->vertexcount;
  if ( GL->USE_BYTENORMALS ) {
    PRIVATE(state)->normals[idx*3+0] = n[0];
    PRIVATE(state)->normals[idx*3+1] = n[1];
    PRIVATE(state)->normals[idx*3+2] = n[2];
  } else {
    static const float factor = 1.0f / 127.0f;
    PRIVATE(state)->fnormals[idx*3+0] = float(n[0]) * factor;
    PRIVATE(state)->fnormals[idx*3+1] = float(n[1]) * factor;
    PRIVATE(state)->fnormals[idx*3+2] = float(n[2]) * factor;
  }
  PRIVATE(state)->vertices[idx*3+0] = (float) (x*state->vspacing[0] + state->voffset[0]);
  PRIVATE(state)->vertices[idx*3+1] = (float) (y*state->vspacing[1] + state->voffset[1]);
  PRIVATE(state)->vertices[idx*3+2] = elev;
  PRIVATE(state)->vertexcount += 1;
#else // !VA_INTERLEAVED
  static signed char normal[3][3];
  static float vertex[3][3];

  const int idx = ((PRIVATE(state)->vertexcount == 0) || (GL->glDrawElements != NULL)) ? 0 : 2; // store fan center in idx 0
  if ( idx != 0 ) { // move up old entries
    normal[1][0] = normal[2][0];
    normal[1][1] = normal[2][1];
    normal[1][2] = normal[2][2];
    vertex[1][0] = vertex[2][0];
    vertex[1][1] = vertex[2][1];
    vertex[1][2] = vertex[2][2];
  }
  normal[idx][0] = n[0];
  normal[idx][1] = n[1];
  normal[idx][2] = n[2];
  vertex[idx][0] = (float) (x*state->vspacing[0] + state->voffset[0]);
  vertex[idx][1] = (float) (y*state->vspacing[1] + state->voffset[1]);
  vertex[idx][2] = elev;

  if ( GL->glDrawElements != NULL ) {
    PRIVATE(state)->normalarray.append(normal[0][0]);
    PRIVATE(state)->normalarray.append(normal[0][1]);
    PRIVATE(state)->normalarray.append(normal[0][2]);
    PRIVATE(state)->vertexarray.append(vertex[0][0]);
    PRIVATE(state)->vertexarray.append(vertex[0][1]);
    PRIVATE(state)->vertexarray.append(vertex[0][2]);
    PRIVATE(state)->vertexcount += 1;
  } else if ( PRIVATE(state)->vertexcount >= 2 ) {
    for ( int v = 0; v < 3; v++ ) {
      PRIVATE(state)->normalarray.append(normal[v][0]);
      PRIVATE(state)->normalarray.append(normal[v][1]);
      PRIVATE(state)->normalarray.append(normal[v][2]);
      PRIVATE(state)->vertexarray.append(vertex[v][0]);
      PRIVATE(state)->vertexarray.append(vertex[v][1]);
      PRIVATE(state)->vertexarray.append(vertex[v][2]);
    }
    PRIVATE(state)->vertexcount += 1;
  }
#endif // !VA_INTERLEAVED
}

inline void
GL_VERTEX_TN(RenderState * state, const struct sc_GL * GL,
             const int x, const int y, const float elev, const signed char * n)
{
  if ( GL->USE_BYTENORMALS ) { glNormal3bv((const GLbyte *)n); }
  else {
    static const float factor = 1.0f/127.0f;
    glNormal3f(n[0] * factor, n[1] * factor, n[2] * factor);
  }
  glTexCoord2f(PRIVATE(state)->toffset[0] + float(x) * PRIVATE(state)->invtsizescale[0],
               PRIVATE(state)->toffset[1] + float(y) * PRIVATE(state)->invtsizescale[1]);
  if (state->etexscale != 0.0f && GL->glMultiTexCoord2f != NULL) {
    GL->glMultiTexCoord2f(GL_TEXTURE1,
                         0.0f, (state->etexscale * elev) + state->etexoffset);
  }
  glVertex3f((float) (x*state->vspacing[0] + state->voffset[0]),
             (float) (y*state->vspacing[1] + state->voffset[1]),
             elev);
}

inline void
GL_VA_VERTEX_TN(RenderState * state, const struct sc_GL * GL,
                const int x, const int y, const float elev, const signed char * n)
{
#if VA_INTERLEAVED
  const int idx = PRIVATE(state)->vertexcount;
  if ( GL->USE_BYTENORMALS ) {
    PRIVATE(state)->normals[idx*3+0] = n[0];
    PRIVATE(state)->normals[idx*3+1] = n[1];
    PRIVATE(state)->normals[idx*3+2] = n[2];
  } else {
    static const float factor = 1.0f / 127.0f;
    PRIVATE(state)->fnormals[idx*3+0] = float(n[0]) * factor;
    PRIVATE(state)->fnormals[idx*3+1] = float(n[1]) * factor;
    PRIVATE(state)->fnormals[idx*3+2] = float(n[2]) * factor;
  }
  PRIVATE(state)->texture1[idx*2+0] = PRIVATE(state)->toffset[0] + float(x) * PRIVATE(state)->invtsizescale[0];
  PRIVATE(state)->texture1[idx*2+1] = PRIVATE(state)->toffset[1] + float(y) * PRIVATE(state)->invtsizescale[1];
  PRIVATE(state)->texture2[idx*2+0] = 0.0f;
  PRIVATE(state)->texture2[idx*2+1] = (state->etexscale * elev) + state->etexoffset;
  PRIVATE(state)->vertices[idx*3+0] = (float) (x*state->vspacing[0] + state->voffset[0]);
  PRIVATE(state)->vertices[idx*3+1] = (float) (y*state->vspacing[1] + state->voffset[1]);
  PRIVATE(state)->vertices[idx*3+2] = elev;
  PRIVATE(state)->vertexcount += 1;
#else // !VA_INTERLEAVED
  static signed char normal[3][3];
  static float vertex[3][3];
  static float texture1[3][2];
  static float texture2[3][2];

  const int idx = ((PRIVATE(state)->vertexcount == 0) || (GL->glDrawElements != NULL)) ? 0 : 2; // store fan center in idx 0
  if ( idx != 0 ) { // move up old entries
    normal[1][0] = normal[2][0];
    normal[1][1] = normal[2][1];
    normal[1][2] = normal[2][2];
    vertex[1][0] = vertex[2][0];
    vertex[1][1] = vertex[2][1];
    vertex[1][2] = vertex[2][2];
    texture1[1][0] = texture1[2][0];
    texture1[1][1] = texture1[2][1];
    texture2[1][0] = texture2[2][0];
    texture2[1][1] = texture2[2][1];
  }
  normal[idx][0] = n[0];
  normal[idx][1] = n[1];
  normal[idx][2] = n[2];
  texture1[idx][0] = PRIVATE(state)->toffset[0] + float(x) * PRIVATE(state)->invtsizescale[0];
  texture1[idx][1] = PRIVATE(state)->toffset[1] + float(y) * PRIVATE(state)->invtsizescale[1];
  texture2[idx][0] = 0.0f;
  texture2[idx][1] = (state->etexscale * elev) + state->etexoffset;
  vertex[idx][0] = (float) (x*state->vspacing[0] + state->voffset[0]);
  vertex[idx][1] = (float) (y*state->vspacing[1] + state->voffset[1]);
  vertex[idx][2] = elev;

  if ( GL->glDrawElements != NULL ) {
    PRIVATE(state)->texcoord1array.append(texture1[0][0]);
    PRIVATE(state)->texcoord1array.append(texture1[0][1]);
    PRIVATE(state)->texcoord2array.append(texture2[0][0]);
    PRIVATE(state)->texcoord2array.append(texture2[0][1]);
    PRIVATE(state)->normalarray.append(normal[0][0]);
    PRIVATE(state)->normalarray.append(normal[0][1]);
    PRIVATE(state)->normalarray.append(normal[0][2]);
    PRIVATE(state)->vertexarray.append(vertex[0][0]);
    PRIVATE(state)->vertexarray.append(vertex[0][1]);
    PRIVATE(state)->vertexarray.append(vertex[0][2]);
    PRIVATE(state)->vertexcount += 1;
  }
  else if ( PRIVATE(state)->vertexcount >= 2 ) { // we have a new triangle
    for ( int v = 0; v < 3; v++ ) {
      PRIVATE(state)->texcoord1array.append(texture1[v][0]);
      PRIVATE(state)->texcoord1array.append(texture1[v][1]);
      PRIVATE(state)->texcoord2array.append(texture2[v][0]);
      PRIVATE(state)->texcoord2array.append(texture2[v][1]);
      PRIVATE(state)->normalarray.append(normal[v][0]);
      PRIVATE(state)->normalarray.append(normal[v][1]);
      PRIVATE(state)->normalarray.append(normal[v][2]);
      PRIVATE(state)->vertexarray.append(vertex[v][0]);
      PRIVATE(state)->vertexarray.append(vertex[v][1]);
      PRIVATE(state)->vertexarray.append(vertex[v][2]);
    }
    PRIVATE(state)->vertexcount += 1;
  }
#endif // !VA_INTERLEAVED
}

/* ********************************************************************** */

void
sc_render_pre_cb_common(void * closure, ss_render_block_cb_info * info)
{
#ifndef SS_SCENERY_H
  assert(sc_scenery_available());
#endif
  assert(closure);
  RenderState * renderstate = (RenderState *) closure;

  ss_render_get_elevation_measures(info, 
                                   renderstate->voffset,
                                   renderstate->vspacing,
                                   NULL,
                                   &renderstate->elevdata,
                                   &renderstate->normaldata,
                                   NULL);

  // Append data used for debugging
  float ox = (float) (renderstate->voffset[0] / renderstate->bbmax[0]);
  float oy = (float) (renderstate->voffset[1] / renderstate->bbmax[1]);
  float sx = (float) ((renderstate->vspacing[0] * renderstate->blocksize) / renderstate->bbmax[0]);
  float sy = (float) ((renderstate->vspacing[1] * renderstate->blocksize) / renderstate->bbmax[1]);
  PRIVATE(renderstate)->debuglist.append(ox);
  PRIVATE(renderstate)->debuglist.append(oy);
  PRIVATE(renderstate)->debuglist.append(ox+sx);
  PRIVATE(renderstate)->debuglist.append(oy+sy);

  PRIVATE(renderstate)->toffset[0] = 0.0f;
  PRIVATE(renderstate)->toffset[1] = 0.0f;
  PRIVATE(renderstate)->tscale[0] = 1.0f;
  PRIVATE(renderstate)->tscale[1] = 1.0f;
  ss_render_get_texture_measures(info, &PRIVATE(renderstate)->scenerytexid,
                                 PRIVATE(renderstate)->toffset, PRIVATE(renderstate)->tscale);
  // for optimizing texture coordinate calculations
  PRIVATE(renderstate)->invtsizescale[0] = (1.0f / renderstate->blocksize) * PRIVATE(renderstate)->tscale[0];
  PRIVATE(renderstate)->invtsizescale[1] = (1.0f / renderstate->blocksize) * PRIVATE(renderstate)->tscale[1];
}


void
sc_render_pre_cb(void * closure, ss_render_block_cb_info * info)
{
  sc_render_pre_cb_common(closure, info);
  RenderState * renderstate = (RenderState *) closure;

  // set up texture for block
  if (renderstate->dotex && PRIVATE(renderstate)->scenerytexid) {
    if ((PRIVATE(renderstate)->scenerytexid != renderstate->activescenerytexid) ||
        (PRIVATE(renderstate)->activetexturecontext != PRIVATE(renderstate)->glcontextid)) {
      TexInfo * texinfo = sc_find_texture(renderstate, PRIVATE(renderstate)->scenerytexid);
      if ( !texinfo ) {
        unsigned char * texdata;
        int texw, texh, texnc;
        ss_render_get_texture_image(info,
                                    PRIVATE(renderstate)->scenerytexid,
                                    &texdata,
                                    &texw,
                                    &texh,
                                    &texnc);

	// FIXME: if the GL.CLAMP_TO_EDGE value is actually GL_CLAMP
	// (because the driver doesn't support GL_CLAMP_TO_EDGE),
	// rendering artifacts will be the result; there will be
	// clearly visible "seams" inbetween the textures.
	//
	// This is by the way not unlikely to happen, as e.g. the
	// Microsoft OpenGL 1.1 software renderer doesn't support
	// GL_CLAMP_TO_EDGE, and that driver will often be used for
	// offscreen rendering.
	//
	// 20040713 mortene.
        const int clampmode = GLi(PRIVATE(renderstate)->glcontextid)->CLAMP_TO_EDGE;
        assert(texture_construct);
        void * opaquetexstruct = texture_construct(renderstate,
                                                   texdata, texw, texh, texnc,
                                                   clampmode, clampmode, 0.9f);

        texinfo = sc_place_texture_in_hash(renderstate,
                                           PRIVATE(renderstate)->scenerytexid,
                                           opaquetexstruct);
      }
      assert(texture_activate);
      texture_activate(renderstate, texinfo->clienttexdata);
      texinfo->unusedcount = 0;

      renderstate->activescenerytexid = PRIVATE(renderstate)->scenerytexid;
      PRIVATE(renderstate)->activetexturecontext = PRIVATE(renderstate)->glcontextid;
    }

    glEnable(GL_TEXTURE_2D);
  }
  else {
    glDisable(GL_TEXTURE_2D);
  }
}

void 
sc_render_post_cb(void * closure, ss_render_block_cb_info * info)
{
  /* nada - direct rendering */
}

/* ********************************************************************** */

#define ELEVATION(x,y) elev[(y)*W+(x)]    

void 
sc_render_cb(void * closure, const int x, const int y,
             const int len, const unsigned int bitmask)
{
  RenderState * renderstate = (RenderState *) closure;
  const struct sc_GL * GL = GLi(PRIVATE(renderstate)->glcontextid);

  const signed char * normals = renderstate->normaldata;  
  const float * elev = renderstate->elevdata;
  const int W = ((int) renderstate->blocksize) + 1;

  int idx;
  if (normals && PRIVATE(renderstate)->scenerytexid == 0) {

#define SEND_VERTEX(state, GL, x, y) \
  idx = (y)*W + x; \
  GL_VERTEX_N(state, GL, x, y, elev[idx], normals+3*idx);

    glBegin(GL_TRIANGLE_FAN);
    SEND_VERTEX(renderstate, GL, x, y);
    SEND_VERTEX(renderstate, GL, x-len, y-len);
    if (!(bitmask & SS_RENDER_BIT_SOUTH)) {
      SEND_VERTEX(renderstate, GL, x, y-len);
    }
    SEND_VERTEX(renderstate, GL, x+len, y-len);
    if (!(bitmask & SS_RENDER_BIT_EAST)) {
      SEND_VERTEX(renderstate, GL, x+len, y);
    }
    SEND_VERTEX(renderstate, GL, x+len, y+len);
    if (!(bitmask & SS_RENDER_BIT_NORTH)) {
      SEND_VERTEX(renderstate, GL, x, y+len);
    }
    SEND_VERTEX(renderstate, GL, x-len, y+len);
    if (!(bitmask & SS_RENDER_BIT_WEST)) {
      SEND_VERTEX(renderstate, GL, x-len, y);
    }
    SEND_VERTEX(renderstate, GL, x-len, y-len);
    glEnd();
#undef SEND_VERTEX
  }
  else if (normals) {
#define SEND_VERTEX(state, GL, x, y) \
  idx = (y)*W + x; \
  GL_VERTEX_TN(state, GL, x, y, elev[idx], normals+3*idx);

    glBegin(GL_TRIANGLE_FAN);
    SEND_VERTEX(renderstate, GL, x, y);
    SEND_VERTEX(renderstate, GL, x-len, y-len);
    if (!(bitmask & SS_RENDER_BIT_SOUTH)) {
      SEND_VERTEX(renderstate, GL, x, y-len);
    }
    SEND_VERTEX(renderstate, GL, x+len, y-len);
    if (!(bitmask & SS_RENDER_BIT_EAST)) {
      SEND_VERTEX(renderstate, GL, x+len, y);
    }
    SEND_VERTEX(renderstate, GL, x+len, y+len);
    if (!(bitmask & SS_RENDER_BIT_NORTH)) {
      SEND_VERTEX(renderstate, GL, x, y+len);
    }
    SEND_VERTEX(renderstate, GL, x-len, y+len);
    if (!(bitmask & SS_RENDER_BIT_WEST)) {
      SEND_VERTEX(renderstate, GL, x-len, y);
    }
    SEND_VERTEX(renderstate, GL, x-len, y-len);
    glEnd();
#undef SEND_VERTEX
  }
  else {
    glBegin(GL_TRIANGLE_FAN);
    GL_VERTEX(renderstate, GL, x, y, ELEVATION(x, y));
    GL_VERTEX(renderstate, GL, x-len, y-len, ELEVATION(x-len, y-len));
    if (!(bitmask & SS_RENDER_BIT_SOUTH)) {
      GL_VERTEX(renderstate, GL, x, y-len, ELEVATION(x, y-len));
    }
    GL_VERTEX(renderstate, GL, x+len, y-len, ELEVATION(x+len, y-len));
    if (!(bitmask & SS_RENDER_BIT_EAST)) {
      GL_VERTEX(renderstate, GL, x+len, y, ELEVATION(x+len, y));
    }
    GL_VERTEX(renderstate, GL, x+len, y+len, ELEVATION(x+len, y+len));
    if (!(bitmask & SS_RENDER_BIT_NORTH)) {
      GL_VERTEX(renderstate, GL, x, y+len, ELEVATION(x, y+len));
    }
    GL_VERTEX(renderstate, GL, x-len, y+len, ELEVATION(x-len, y+len));
    if (!(bitmask & SS_RENDER_BIT_WEST)) {
      GL_VERTEX(renderstate, GL, x-len, y, ELEVATION(x-len, y));
    }
    GL_VERTEX(renderstate, GL, x-len, y-len, ELEVATION(x-len, y-len));
    glEnd();
  }
}

inline
void
SEND_VA_TRIANGLE_FAN(RenderState * state, const struct sc_GL * GL)
{
  assert(PRIVATE(state)->vertexcount <= 10);
  assert(GL->glDrawElements != NULL);
  static unsigned int indices[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
  if ( GL->glDrawRangeElements != NULL ) {
    GL->glDrawRangeElements(GL_TRIANGLE_FAN, 0, PRIVATE(state)->vertexcount - 1, PRIVATE(state)->vertexcount, GL_UNSIGNED_INT, indices);
  } else {
    GL->glDrawElements(GL_TRIANGLE_FAN, PRIVATE(state)->vertexcount, GL_UNSIGNED_INT, indices);
  }
}

#if VA_INTERLEAVED
#define VA_TRIANGLE_FAN_START() \
  PRIVATE(renderstate)->vertexcount = 0
#define VA_TRIANGLE_FAN_STOP() \
  SEND_VA_TRIANGLE_FAN(renderstate, GL)
#else
#define VA_TRIANGLE_FAN_START() \
  PRIVATE(renderstate)->vertexcount = 0
#define VA_TRIANGLE_FAN_STOP() \
  if ( GL->glDrawRangeElements != NULL ) { \
    PRIVATE(renderstate)->lenarray.append(PRIVATE(renderstate)->vertexcount); \
  }
#endif

void 
sc_va_render_cb(void * closure, const int x, const int y,
                const int len, const unsigned int bitmask)
{
  RenderState * renderstate = (RenderState *) closure;
  const struct sc_GL * GL = GLi(PRIVATE(renderstate)->glcontextid);

  const signed char * normals = renderstate->normaldata;  
  const float * elev = renderstate->elevdata;
  const int W = ((int) renderstate->blocksize) + 1;

  int idx;
  if (normals && PRIVATE(renderstate)->scenerytexid == 0) {

#define SEND_VERTEX(state, GL, x, y) \
  idx = (y)*W + x; \
  GL_VA_VERTEX_N(state, GL, x, y, elev[idx], normals+3*idx);

    VA_TRIANGLE_FAN_START();
    SEND_VERTEX(renderstate, GL, x, y);
    SEND_VERTEX(renderstate, GL, x-len, y-len);
    if (!(bitmask & SS_RENDER_BIT_SOUTH)) {
      SEND_VERTEX(renderstate, GL, x, y-len);
    }
    SEND_VERTEX(renderstate, GL, x+len, y-len);
    if (!(bitmask & SS_RENDER_BIT_EAST)) {
      SEND_VERTEX(renderstate, GL, x+len, y);
    }
    SEND_VERTEX(renderstate, GL, x+len, y+len);
    if (!(bitmask & SS_RENDER_BIT_NORTH)) {
      SEND_VERTEX(renderstate, GL, x, y+len);
    }
    SEND_VERTEX(renderstate, GL, x-len, y+len);
    if (!(bitmask & SS_RENDER_BIT_WEST)) {
      SEND_VERTEX(renderstate, GL, x-len, y);
    }
    SEND_VERTEX(renderstate, GL, x-len, y-len);
#undef SEND_VERTEX
    VA_TRIANGLE_FAN_STOP();
  }
  else if (normals) {
#define SEND_VERTEX(state, GL, x, y) \
  idx = (y)*W + x; \
  GL_VA_VERTEX_TN(state, GL, x, y, elev[idx], normals+3*idx);

    VA_TRIANGLE_FAN_START();
    SEND_VERTEX(renderstate, GL, x, y);
    SEND_VERTEX(renderstate, GL, x-len, y-len);
    if (!(bitmask & SS_RENDER_BIT_SOUTH)) {
      SEND_VERTEX(renderstate, GL, x, y-len);
    }
    SEND_VERTEX(renderstate, GL, x+len, y-len);
    if (!(bitmask & SS_RENDER_BIT_EAST)) {
      SEND_VERTEX(renderstate, GL, x+len, y);
    }
    SEND_VERTEX(renderstate, GL, x+len, y+len);
    if (!(bitmask & SS_RENDER_BIT_NORTH)) {
      SEND_VERTEX(renderstate, GL, x, y+len);
    }
    SEND_VERTEX(renderstate, GL, x-len, y+len);
    if (!(bitmask & SS_RENDER_BIT_WEST)) {
      SEND_VERTEX(renderstate, GL, x-len, y);
    }
    SEND_VERTEX(renderstate, GL, x-len, y-len);
#undef SEND_VERTEX
    VA_TRIANGLE_FAN_STOP();
  }
  else {
    VA_TRIANGLE_FAN_START();
    GL_VA_VERTEX(renderstate, x, y, ELEVATION(x, y));
    GL_VA_VERTEX(renderstate, x-len, y-len, ELEVATION(x-len, y-len));
    if (!(bitmask & SS_RENDER_BIT_SOUTH)) {
      GL_VA_VERTEX(renderstate, x, y-len, ELEVATION(x, y-len));
    }
    GL_VA_VERTEX(renderstate, x+len, y-len, ELEVATION(x+len, y-len));
    if (!(bitmask & SS_RENDER_BIT_EAST)) {
      GL_VA_VERTEX(renderstate, x+len, y, ELEVATION(x+len, y));
    }
    GL_VA_VERTEX(renderstate, x+len, y+len, ELEVATION(x+len, y+len));
    if (!(bitmask & SS_RENDER_BIT_NORTH)) {
      GL_VA_VERTEX(renderstate, x, y+len, ELEVATION(x, y+len));
    }
    GL_VA_VERTEX(renderstate, x-len, y+len, ELEVATION(x-len, y+len));
    if (!(bitmask & SS_RENDER_BIT_WEST)) {
      GL_VA_VERTEX(renderstate, x-len, y, ELEVATION(x-len, y));
    }
    GL_VA_VERTEX(renderstate, x-len, y-len, ELEVATION(x-len, y-len));
    VA_TRIANGLE_FAN_STOP();
  }
}

void 
sc_undefrender_cb(void * closure, const int x, const int y, const int len, 
                  const unsigned int bitmask_org)
{
  RenderState * renderstate = (RenderState *) closure;
  const struct sc_GL * GL = GLi(PRIVATE(renderstate)->glcontextid);

  const signed char * normals = renderstate->normaldata;
  const float * elev = renderstate->elevdata;
  const int W = ((int) renderstate->blocksize) + 1;

  const signed char * ptr = ss_render_get_undef_array(bitmask_org);

  int numv = *ptr++;
  int tx, ty;

  if (normals && PRIVATE(renderstate)->scenerytexid == 0) {
    int idx;
#define SEND_VERTEX(state, GL, x, y) \
    idx = (y)*W + x; \
    GL_VERTEX_N(state, GL, x, y, elev[idx], normals+3*idx);

    while (numv) {
      glBegin(GL_TRIANGLE_FAN);
      while (numv) { 
        tx = x + *ptr++ * len;
        ty = y + *ptr++ * len;
        SEND_VERTEX(renderstate, GL, tx, ty);
        numv--;
      }
      numv = *ptr++;
      glEnd();
    }
#undef SEND_VERTEX
  }
  else if (normals) {
    int idx;
#define SEND_VERTEX(state, GL, x, y) \
    idx = (y)*W + x; \
    GL_VERTEX_TN(state, GL, x, y, elev[idx], normals+3*idx);

    while (numv) {
      glBegin(GL_TRIANGLE_FAN);
      while (numv) { 
        tx = x + *ptr++ * len;
        ty = y + *ptr++ * len;
        SEND_VERTEX(renderstate, GL, tx, ty);
        numv--;
      }
      numv = *ptr++;
      glEnd();
    }
#undef SEND_VERTEX
  }  
  else {    
    while (numv) {
      glBegin(GL_TRIANGLE_FAN);
      while (numv) { 
        tx = x + *ptr++ * len;
        ty = y + *ptr++ * len;
        GL_VERTEX(renderstate, GL, tx, ty, ELEVATION(tx, ty));
        numv--;
      }
      numv = *ptr++;
      glEnd();
    }
  }
}

void 
sc_va_undefrender_cb(void * closure, const int x, const int y, const int len, 
                     const unsigned int bitmask_org)
{
  RenderState * renderstate = (RenderState *) closure;
  const struct sc_GL * GL = GLi(PRIVATE(renderstate)->glcontextid);

  const signed char * normals = renderstate->normaldata;
  const float * elev = renderstate->elevdata;
  const int W = ((int) renderstate->blocksize) + 1;

  const signed char * ptr = ss_render_get_undef_array(bitmask_org);

  int numv = *ptr++;
  int tx, ty;

  if (normals && PRIVATE(renderstate)->scenerytexid == 0) {
    int idx;
#define SEND_VERTEX(state, GL, x, y) \
    idx = (y)*W + x; \
    GL_VA_VERTEX_N(state, GL, x, y, elev[idx], normals+3*idx);

    while (numv) {
      VA_TRIANGLE_FAN_START();
      while (numv) { 
        tx = x + *ptr++ * len;
        ty = y + *ptr++ * len;
        SEND_VERTEX(renderstate, GL, tx, ty);
        numv--;
      }
      VA_TRIANGLE_FAN_STOP();
      numv = *ptr++;
    }
#undef SEND_VERTEX
  }
  else if (normals) {
    int idx;
#define SEND_VERTEX(state, GL, x, y) \
    idx = (y)*W + x; \
    GL_VA_VERTEX_TN(state, GL, x, y, elev[idx], normals+3*idx);

    while (numv) {
      VA_TRIANGLE_FAN_START();
      while (numv) { 
        tx = x + *ptr++ * len;
        ty = y + *ptr++ * len;
        SEND_VERTEX(renderstate, GL, tx, ty);
        numv--;
      }
      VA_TRIANGLE_FAN_STOP();
      numv = *ptr++;
    }
#undef SEND_VERTEX
  }  
  else {    
    while (numv) {
      VA_TRIANGLE_FAN_START();
      while (numv) { 
        tx = x + *ptr++ * len;
        ty = y + *ptr++ * len;
        GL_VA_VERTEX(renderstate, tx, ty, ELEVATION(tx, ty));
        numv--;
      }
      VA_TRIANGLE_FAN_STOP();
      numv = *ptr++;
    }
  }
}

#undef ELEVATION

/* ********************************************************************** */

void
sc_va_render_pre_cb(void * closure, ss_render_block_cb_info * info)
{
#if VA_INTERLEAVED
  assert(closure);
  RenderState * renderstate = (RenderState *) closure;
  const struct sc_GL * GL = GLi(PRIVATE(renderstate)->glcontextid);

  sc_render_pre_cb(closure, info);

  assert(info);
  assert(GL->glEnableClientState != NULL);
  assert(GL->glDisableClientState != NULL);
  assert(GL->glVertexPointer != NULL);
  assert(GL->glNormalPointer != NULL);
  assert(GL->glTexCoordPointer != NULL);
  assert(GL->glDrawElements != NULL);
  assert(GL->glDrawArrays != NULL);

  const signed char * normals = renderstate->normaldata; // used as a flag below
  // elevation data is common for all modes
  GL->glEnableClientState(GL_VERTEX_ARRAY);
  GL->glVertexPointer(3, GL_FLOAT, 0, PRIVATE(renderstate)->vertices);

  if ( normals != NULL && PRIVATE(renderstate)->scenerytexid == 0 ) {
    // elevation and normals
    if ( GL->USE_BYTENORMALS ) {
      GL->glNormalPointer(GL_BYTE, 0, PRIVATE(renderstate)->normals);
    } else {
      GL->glNormalPointer(GL_FLOAT, 0, PRIVATE(renderstate)->fnormals);
    }
    GL->glEnableClientState(GL_NORMAL_ARRAY);
  } else if ( normals ) {
    // elevation, normals and textures
    if ( GL->USE_BYTENORMALS ) {
      GL->glNormalPointer(GL_BYTE, 0, PRIVATE(renderstate)->normals);
    } else {
      GL->glNormalPointer(GL_FLOAT, 0, PRIVATE(renderstate)->fnormals);
    }
    GL->glEnableClientState(GL_NORMAL_ARRAY);

    if ( (renderstate->etexscale != 0.0f) && (GL->glClientActiveTexture != NULL) ) {
      GL->glClientActiveTexture(GL_TEXTURE1);
      GL->glTexCoordPointer(2, GL_FLOAT, 0, PRIVATE(renderstate)->texture2);
      GL->glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      GL->glClientActiveTexture(GL_TEXTURE0);
    }
    GL->glTexCoordPointer(2, GL_FLOAT, 0, PRIVATE(renderstate)->texture1);
    GL->glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  } else {
    // elevation and textures
    if ( (renderstate->etexscale != 0.0f) && (GL->glClientActiveTexture != NULL) ) {
      GL->glClientActiveTexture(GL_TEXTURE1);
      GL->glTexCoordPointer(2, GL_FLOAT, 0, PRIVATE(renderstate)->texture2);
      GL->glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      GL->glClientActiveTexture(GL_TEXTURE0);
    }

    GL->glTexCoordPointer(2, GL_FLOAT, 0, PRIVATE(renderstate)->texture1);
    GL->glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  }
    
#else
  sc_render_pre_cb_common(closure, info);
  RenderState * state = (RenderState *) closure;

  // reset lists
  PRIVATE(state)->vertexarray.truncate(0);
  PRIVATE(state)->normalarray.truncate(0);
  PRIVATE(state)->texcoord1array.truncate(0);
  PRIVATE(state)->texcoord2array.truncate(0);
  PRIVATE(state)->lenarray.truncate(0);
  // PRIVATE(state)->idxarray.truncate(0); // FIXME: necessary to do this?
#endif
}

void
sc_va_render_post_cb(void * closure, ss_render_block_cb_info * info)
{
#if VA_INTERLEAVED
  assert(closure);
  RenderState * state = (RenderState *) closure;
  const struct sc_GL * GL = GLi(PRIVATE(state)->glcontextid);

  sc_render_post_cb(closure, info);

  const signed char * normals = state->normaldata; // used as a flag below

  GL->glDisableClientState(GL_VERTEX_ARRAY);
  if ( normals != NULL && PRIVATE(state)->scenerytexid == 0 ) {
    // elevation and normals
    GL->glDisableClientState(GL_NORMAL_ARRAY);
  } else if ( normals ) {
    // elevation, normals and textures
    GL->glDisableClientState(GL_NORMAL_ARRAY);
    if ( (state->etexscale != 0.0f) && (GL->glClientActiveTexture != NULL) ) {
      GL->glClientActiveTexture(GL_TEXTURE1);
      GL->glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      GL->glClientActiveTexture(GL_TEXTURE0);
    }
    GL->glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  } else {
    // elevation and textures
    if ( (state->etexscale != 0.0f) && (GL->glClientActiveTexture != NULL) ) {
      GL->glClientActiveTexture(GL_TEXTURE1);
      GL->glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      GL->glClientActiveTexture(GL_TEXTURE0);
    }
    GL->glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  }

#else
  assert(closure);
  RenderState * state = (RenderState *) closure;

  if ( PRIVATE(state)->vertexarray.getLength() == 0 ) { return; } // nothing to render in this block
  if ( PRIVATE(state)->idxarray.getLength() < (PRIVATE(state)->vertexarray.getLength() / 3) ) {
    int c;
    const int needed = PRIVATE(state)->vertexarray.getLength() / 3;
    for ( c = PRIVATE(state)->idxarray.getLength(); c < needed; c++ ) {
      PRIVATE(state)->idxarray.append(c);
    }
  }

  // If the number of vertices for a block is small, we might consider adding
  // another block or two before pushing the vertex arrays to the graphics card,
  // but then we would have to have a post-render callback or something to make
  // sure the last blocks are actually rendered.

  assert(info);
  assert(GL->glEnableClientState != NULL);
  assert(GL->glDisableClientState != NULL);
  assert(GL->glVertexPointer != NULL);
  assert(GL->glNormalPointer != NULL);
  assert(GL->glTexCoordPointer != NULL);
  assert(GL->glDrawElements != NULL);
  assert(GL->glDrawArrays != NULL);

  // Set up textures before rendering - we delayed this because some blocks have
  // textures, but will be tessellated to 0 triangles if the block is mostly
  // undefined.
  if (state->dotex && PRIVATE(state)->scenerytexid) {
    if ((PRIVATE(state)->scenerytexid != state->activescenerytexid) ||
        (PRIVATE(state)->activetexturecontext != PRIVATE(state)->glcontextid)) {
      TexInfo * texinfo = sc_find_texture(state, PRIVATE(state)->scenerytexid);
      if ( !texinfo ) {
        unsigned char * texdata;
        int texw, texh, texnc;
        ss_render_get_texture_image(info, PRIVATE(state)->scenerytexid,
                                    &texdata, &texw, &texh, &texnc);

	// FIXME: if the GL.CLAMP_TO_EDGE value is actually GL_CLAMP
	// (because the driver doesn't support GL_CLAMP_TO_EDGE),
	// rendering artifacts will be the result; there will be
	// clearly visible "seams" inbetween the textures.
	//
	// This is by the way not unlikely to happen, as e.g. the
	// Microsoft OpenGL 1.1 software renderer doesn't support
	// GL_CLAMP_TO_EDGE, and that driver will often be used for
	// offscreen rendering.
	//
	// 20040713 mortene.
        const int clampmode = GL->CLAMP_TO_EDGE;
        assert(texture_construct);
        void * opaquetexstruct = texture_construct(texdata, texw, texh, texnc,
                                                   clampmode, clampmode, 0.9f, 0);

        texinfo = sc_place_texture_in_hash(state, PRIVATE(state)->scenerytexid,
                                           opaquetexstruct);
      }
      assert(texture_activate);
      texture_activate(state, texinfo->clienttexdata);
      texinfo->unusedcount = 0;

      state->activescenerytexid = PRIVATE(state)->scenerytexid;
      PRIVATE(state)->activetexturecontext = PRIVATE(state)->glcontextid;
    }

    glEnable(GL_TEXTURE_2D);
  }
  else {
    glDisable(GL_TEXTURE_2D);
  }

  const float * vertexarrayptr = PRIVATE(state)->vertexarray.getArrayPtr();
  const signed char * normalarrayptr = PRIVATE(state)->normalarray.getArrayPtr();
  const float * texcoord1arrayptr = PRIVATE(state)->texcoord1array.getArrayPtr();
  const float * texcoord2arrayptr = PRIVATE(state)->texcoord2array.getArrayPtr();
  const unsigned int * idxarrayptr = PRIVATE(state)->idxarray.getArrayPtr();

  const int vertices = PRIVATE(state)->vertexarray.getLength() / 3;
  const int triangles = vertices / 3;

  const signed char * normals = state->normaldata; // used as a flag below
  // elevation data is common for all modes
  GL->glEnableClientState(GL_VERTEX_ARRAY);
  GL->glVertexPointer(3, GL_FLOAT, 0, vertexarrayptr);

  if ( normals != NULL && PRIVATE(state)->scenerytexid == 0 ) {
    // elevation and normals
    assert(PRIVATE(state)->vertexarray.getLength() == PRIVATE(state)->normalarray.getLength());
    GL->glNormalPointer(GL_BYTE, 0, normalarrayptr);
    GL->glEnableClientState(GL_NORMAL_ARRAY);
  } else if ( normals ) {
    // elevation, normals and textures
    assert(PRIVATE(state)->vertexarray.getLength() == PRIVATE(state)->normalarray.getLength());
    GL->glNormalPointer(GL_BYTE, 0, normalarrayptr);
    GL->glEnableClientState(GL_NORMAL_ARRAY);

    if ( (state->etexscale != 0.0f) && (GL->glClientActiveTexture != NULL) ) {
      assert((PRIVATE(state)->vertexarray.getLength() / 3) == (PRIVATE(state)->texcoord2array.getLength() / 2));
      GL->glClientActiveTexture(GL_TEXTURE1);
      GL->glTexCoordPointer(2, GL_FLOAT, 0, texcoord2arrayptr);
      GL->glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      GL->glClientActiveTexture(GL_TEXTURE0);
    }
    assert((PRIVATE(state)->vertexarray.getLength() / 3) == (PRIVATE(state)->texcoord1array.getLength() / 2));
    GL->glTexCoordPointer(2, GL_FLOAT, 0, texcoord1arrayptr);
    GL->glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  } else {
    // elevation and textures
    if ( (state->etexscale != 0.0f) && (GL->glClientActiveTexture != NULL) ) {
      assert((PRIVATE(state)->vertexarray.getLength() / 3) == (PRIVATE(state)->texcoord2array.getLength() / 2));
      GL->glClientActiveTexture(GL_TEXTURE1);
      GL->glTexCoordPointer(2, GL_FLOAT, 0, texcoord2arrayptr);
      GL->glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      GL->glClientActiveTexture(GL_TEXTURE0);
    }

    assert((PRIVATE(state)->vertexarray.getLength() / 3) == (PRIVATE(state)->texcoord1array.getLength() / 2));
    GL->glTexCoordPointer(2, GL_FLOAT, 0, texcoord1arrayptr);
    GL->glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  }
    
  // Indexed vertex arrays will probably perform better, so if we can create
  // the index table without too much overhead, we're going to try it out...
  // GL->glDrawElements(GL_TRIANGLES,
  //                   indexarray->getLength(), GL_UNSIGNED_INT, indexarrayptr);
  if ( GL->glDrawRangeElements == NULL ) {
    GL->glDrawArrays(GL_TRIANGLES, 0, vertices);
  } else {
    int offset = 0;
    const int numstrips = PRIVATE(state)->lenarray.getLength();
    const int numindices = PRIVATE(state)->idxarray.getLength();
    int i;
    for ( i = 0; i < numstrips; i++ ) {
      int indices[10];
      int j;
      const int len = PRIVATE(state)->lenarray[i];
      // assert(len <= 10);
      for ( j = 0; j < len; j++ ) {
        indices[j] = offset + j;
      }
      // printf("len: %d   offset: %d\n", len, offset);
      GL->glDrawRangeElements(GL_TRIANGLE_FAN, offset, offset + len - 1, len, GL_UNSIGNED_INT, indices);
      offset += len;
    }
    // printf("offset: %d   vertices: %d\n", offset, PRIVATE(state)->vertexarray.getLength() / 3);
  }

  GL->glDisableClientState(GL_VERTEX_ARRAY);
  if ( normals != NULL && PRIVATE(state)->scenerytexid == 0 ) {
    // elevation and normals
    GL->glDisableClientState(GL_NORMAL_ARRAY);
  } else if ( normals ) {
    // elevation, normals and textures
    GL->glDisableClientState(GL_NORMAL_ARRAY);
    if ( (state->etexscale != 0.0f) && (GL->glClientActiveTexture != NULL) ) {
      GL->glClientActiveTexture(GL_TEXTURE1);
      GL->glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      GL->glClientActiveTexture(GL_TEXTURE0);
    }
    GL->glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  } else {
    // elevation and textures
    if ( (state->etexscale != 0.0f) && (GL->glClientActiveTexture != NULL) ) {
      GL->glClientActiveTexture(GL_TEXTURE1);
      GL->glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      GL->glClientActiveTexture(GL_TEXTURE0);
    }
    GL->glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  }

#if 0
  // DEBUG CODE:  Render fences between terrain blocks.
  if ( TRUE ) { // outline blocks
    int tex = glIsEnabled(GL_TEXTURE_2D);
    if ( tex ) { glDisable(GL_TEXTURE_2D); }

    const float height = (state->vspacing[0] + state->vspacing[1]);
    int i;

    // FIXME:  Sort vertices along axis.  Find out why normal doesn't work.
    // Figure out how to detect undef-areas.
    glNormal3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_TRIANGLE_STRIP);
    for ( i = 0; i < vertices; i++ ) {
      if ( vertexarrayptr[i*3] == state->voffset[0] ) {
        glVertex3f(vertexarrayptr[i*3+0], vertexarrayptr[i*3+1], vertexarrayptr[i*3+2]);
        glVertex3f(vertexarrayptr[i*3+0], vertexarrayptr[i*3+1], vertexarrayptr[i*3+2] + height);
      }
    }
    glEnd(); // GL_TRIANGLE_STRIP

    glNormal3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_TRIANGLE_STRIP);
    for ( i = 0; i < vertices; i++ ) {
      if ( vertexarrayptr[i*3+1] == state->voffset[1] ) {
        glVertex3f(vertexarrayptr[i*3+0], vertexarrayptr[i*3+1], vertexarrayptr[i*3+2]);
        glVertex3f(vertexarrayptr[i*3+0], vertexarrayptr[i*3+1], vertexarrayptr[i*3+2] + height);
      }
    }
    glEnd(); // GL_TRIANGLE_STRIP

    if ( tex ) { glEnable(GL_TEXTURE_2D); }
  }
#endif
#endif
}

/* ********************************************************************** */

static
void
intersect(RenderState * state, const SbVec3<float> & a, const SbVec3<float> & b, const SbVec3<float> & c)
{
  const SbPlane<float> plane(a, b, c);
  const SbVec3<float> normal(plane.getNormal());

  const SbVec3<float> raypos(state->raypos[0], state->raypos[1], state->raypos[2]);
  const SbVec3<float> raydir(state->raydir[0], state->raydir[1], state->raydir[2]);
  if ( raydir.dot(normal) == 0.0f ) { return; } // ray parallel to plane
  
  const float dist = plane.getDistanceFromOrigin(); // bounds[j][i]
  const float t = (dist - normal.dot(raypos)) / normal.dot(raydir);
  if ( t < 0.0f ) { return; } // don't pick backwards
  // this is where the ray intersects with the triangle plane
  const SbVec3<float> intersectionpoint = raypos + raydir * t;

  // is intersection point inside the triangle bounding box?
  SbBox3<float> box(a, a);
  box.extendBy(b);
  box.extendBy(c);
  const SbVec3<float> bounds[2] = { box.getMin(), box.getMax() };
  if ( (intersectionpoint[0] >= bounds[0][0]) && (intersectionpoint[0] <= bounds[1][0]) &&
       (intersectionpoint[1] >= bounds[0][1]) && (intersectionpoint[1] <= bounds[1][1]) &&
       (intersectionpoint[2] >= bounds[0][2]) && (intersectionpoint[2] <= bounds[1][2]) ) {
    if ( PRIVATE(state)->intersected ) { // already have an intersection
      const SbVec3<float> newvec = intersectionpoint - raypos;
      const SbVec3<float> oldvec =
        SbVec3<float>(PRIVATE(state)->intersection[0], PRIVATE(state)->intersection[1], PRIVATE(state)->intersection[2]) - raypos;
      if ( oldvec.sqrLength() <= newvec.sqrLength() ) {
        // old pickpoint is closer, whether this is an actual hit or not,
        // and we're only interested in the closest intersection
        return;
      }
    }
    // make sure this pick-point is inside the triangle, not just inside the bbox
    // make a vertex-ordered pyramid of the four points (raypos and triangle), and
    // see if the three planes reports the intersection point as being on the same
    // side of the plane (meaning it must be inside).
    const SbPlane<float> pl0(raypos, a, b);
    const SbPlane<float> pl1(raypos, b, c);
    const SbPlane<float> pl2(raypos, c, a);
    const int inside = pl0.isInHalfSpace(intersectionpoint);
    if ( (inside == pl1.isInHalfSpace(intersectionpoint)) &&
         (inside == pl2.isInHalfSpace(intersectionpoint)) ) {
      PRIVATE(state)->intersected = TRUE;
      PRIVATE(state)->intersection[0] = intersectionpoint[0];
      PRIVATE(state)->intersection[1] = intersectionpoint[1];
      PRIVATE(state)->intersection[2] = intersectionpoint[2];
      if ( PRIVATE(state)->dataset != -2 ) {
        // we want to know which dataset we picked on
        double voffset[2];
        double vspacing[2];
        int dimension[2];
        float * elevation;
        signed char * texture;
        int * datasets;
        assert(PRIVATE(state)->blockinfo);
        ss_render_get_elevation_measures(PRIVATE(state)->blockinfo,
                                         voffset, vspacing,
                                         dimension,
                                         &elevation, &texture,
                                         &datasets);
        int x = (int) ((intersectionpoint[0] - voffset[0]) / vspacing[0]);
        int y = (int) ((intersectionpoint[1] - voffset[1]) / vspacing[1]);
        // printf("x, y   : %g, %g\n", intersectionpoint[0], intersectionpoint[1]);
        // printf("offset : %g, %g\n", voffset[0], voffset[1]);
        // printf("index  : %d, %d = dataset %d\n", x, y, datasets[y*dimension[0]+x]);
        PRIVATE(state)->dataset = datasets[y*dimension[0]+x];
      }
    }
  }
}

static void
sc_raypick_pre_cb(void * closure, ss_render_block_cb_info * info)
{
  assert(closure);
  RenderState * state = (RenderState *) closure;
  ss_render_get_elevation_measures(info,
                                   state->voffset,
                                   state->vspacing,
                                   NULL,
                                   &state->elevdata,
                                   &state->normaldata,
                                   NULL);
  PRIVATE(state)->blockinfo = info;
}

static void
sc_raypick_post_cb(void * closure, ss_render_block_cb_info * info)
{
  assert(closure);
  RenderState * state = (RenderState *) closure;
  PRIVATE(state)->blockinfo = NULL;
}

#define ELEVATION(x, y) elev[(y)*W+(x)]

#define GEN_VERTEX(state, x, y, z) \
  second = third; \
  third.setValue((float) ((x)*state->vspacing[0] + state->voffset[0]), \
                 (float) ((y)*state->vspacing[1] + state->voffset[1]), (z)); \
  if ( ++vertices >= 3 ) { \
    intersect(state, first, second, third); \
  }


static void 
sc_raypick_cb(void * closure, const int x, const int y,
               const int len, const unsigned int bitmask)
{
  assert(closure);
  RenderState * state = (RenderState *) closure;

  const signed char * normals = state->normaldata;
  const float * elev = state->elevdata;
  const int W = (int) state->blocksize + 1;

  int vertices = 1;
  SbVec3<float> second(0.0f, 0.0f, 0.0f), third(0.0f, 0.0f, 0.0f);

  const SbVec3<float> first((float) (x*state->vspacing[0] + state->voffset[0]),
                            (float) (y*state->vspacing[1] + state->voffset[1]),
                            ELEVATION(x, y));

  GEN_VERTEX(state, x-len, y-len, ELEVATION(x-len, y-len));
  if (!(bitmask & SS_RENDER_BIT_SOUTH)) {
    GEN_VERTEX(state, x, y-len, ELEVATION(x, y-len));
  }
  GEN_VERTEX(state, x+len, y-len, ELEVATION(x+len, y-len));
  if (!(bitmask & SS_RENDER_BIT_EAST)) {
    GEN_VERTEX(state, x+len, y, ELEVATION(x+len, y));
  }
  GEN_VERTEX(state, x+len, y+len, ELEVATION(x+len, y+len));
  if (!(bitmask & SS_RENDER_BIT_NORTH)) {
    GEN_VERTEX(state, x, y+len, ELEVATION(x, y+len));
  }
  GEN_VERTEX(state, x-len, y+len, ELEVATION(x-len, y+len));
  if (!(bitmask & SS_RENDER_BIT_WEST)) {
    GEN_VERTEX(state, x-len, y, ELEVATION(x-len, y));
  }
  GEN_VERTEX(state, x-len, y-len, ELEVATION(x-len, y-len));
}

#undef ELEVATION
#undef GEN_VERTEX

#define ELEVATION(x, y) elev[(y)*W+(x)]

#define GEN_VERTEX(state, x, y, z) \
  if ( vertices == 2 ) { first = second; } \
  second = third; \
  third.setValue((float) ((x)*(state)->vspacing[0] + (state)->voffset[0]), \
                 (float) ((y)*(state)->vspacing[1] + (state)->voffset[1]), (z)); \
  if ( ++vertices >= 3 ) { \
    intersect((state), first, second, third); \
  }


static void 
sc_undefraypick_cb(void * closure, const int x, const int y,
                    const int len, const unsigned int bitmask_org)
{
  assert(closure);
  RenderState * state = (RenderState *) closure;

  const signed char * normals = state->normaldata;
  const float * elev = state->elevdata;
  const int W = (int) state->blocksize + 1;

  SbVec3<float> first(0.0f, 0.0f, 0.0f), second(0.0f, 0.0f, 0.0f), third(0.0f, 0.0f, 0.0f);

  const signed char * ptr = ss_render_get_undef_array(bitmask_org);
  int numv = *ptr++;
  int tx, ty;

  while ( numv ) {
    int vertices = 0;
    while ( numv ) {
      tx = x + *ptr++ * len;
      ty = y + *ptr++ * len;
      GEN_VERTEX(state, tx, ty, ELEVATION(tx, ty));
      numv--;
    }
    numv = *ptr++;
  }
}

#undef ELEVATION
#undef GEN_VERTEX

/* ********************************************************************** */
