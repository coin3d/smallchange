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

#include <assert.h>
#include <stdlib.h> // atoi()

#include <Inventor/SbBasic.h> // __COIN__

#ifndef __COIN__
#error this part depends on some Coin-specific features
#endif // __COIN__

#include <Inventor/C/tidbits.h> // coin_getenv()
#include <Inventor/system/gl.h>
#include <Inventor/C/glue/gl.h>
#include <Inventor/lists/SbList.h>
#include <Inventor/SbBox3f.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/actions/SoRayPickAction.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoCallbackAction.h>
#include <Inventor/misc/SoState.h>
#include <Inventor/elements/SoCullElement.h>

#include <SmallChange/nodes/SceneryGL.h>
#include <SmallChange/misc/SceneryGlue.h>

/* ********************************************************************** */

static const cc_glglue * glglue = NULL;

void
sc_set_glglue_instance(const cc_glglue * glue)
{
  glglue = glue;
}

int
sc_is_texturing_enabled(void)
{
  return glIsEnabled(GL_TEXTURE_2D);
}

void
sc_enable_texturing(void)
{
  glEnable(GL_TEXTURE_2D);
}

void
sc_disable_texturing(void)
{
  glDisable(GL_TEXTURE_2D);
}

/* ********************************************************************** */

#define R 0
#define G 1
#define B 2
#define A 3

void
sc_generate_elevation_line_texture(float distance,
                                   float offset,
                                   float thickness,
                                   int emphasis,
                                   uint8_t * buffer,
                                   float * texcoordscale,
                                   float * texcoordoffset)
{
  assert(distance > 0.0f);
  assert(thickness > 0.0f);
  assert(SM_SCENERY_ELEVATION_TEXTURE_COMPONENTS == 4);

  int i;
  // clear texture to white first
  for ( i = 0; i < SM_SCENERY_ELEVATION_TEXTURE_SIZE; i++ ) {
    buffer[i*SM_SCENERY_ELEVATION_TEXTURE_COMPONENTS+R] = 255;
    buffer[i*SM_SCENERY_ELEVATION_TEXTURE_COMPONENTS+G] = 255;
    buffer[i*SM_SCENERY_ELEVATION_TEXTURE_COMPONENTS+B] = 255;
    buffer[i*SM_SCENERY_ELEVATION_TEXTURE_COMPONENTS+A] = 255;
  }

  float lines = 1.0f;
  // FIXME:
  if ( emphasis > 1 ) {
    // the texture has to include N lines, not just 1
    lines = (float) emphasis;
  }

  float pixelsize = distance / (1024.0f / lines);

  float pixelpos = 0.0f;
  // set elevation lines to black
  int bolds = 0;
  for ( i = 0; i < SM_SCENERY_ELEVATION_TEXTURE_SIZE; i++ ) {
    float pixelpos = i * pixelsize;
    if ( (fabs(fmod(pixelpos, distance)) <= (thickness * 0.5f)) ||
         ((distance - fabs(fmod(pixelpos, distance))) < (thickness * 0.5f)) ) {
      buffer[i*SM_SCENERY_ELEVATION_TEXTURE_COMPONENTS+R] = 0;
      buffer[i*SM_SCENERY_ELEVATION_TEXTURE_COMPONENTS+G] = 0;
      buffer[i*SM_SCENERY_ELEVATION_TEXTURE_COMPONENTS+B] = 0;
      buffer[i*SM_SCENERY_ELEVATION_TEXTURE_COMPONENTS+A] = 255;
    }
    else if ( lines > 1.0f ) {
      if ( (fabs(fmod(pixelpos, (lines * distance))) <= (thickness * 1.5f)) ||
           (((lines * distance) - fabs(fmod(pixelpos, (lines * distance)))) < (thickness * 1.5f)) ) {
        buffer[i*SM_SCENERY_ELEVATION_TEXTURE_COMPONENTS+R] = 0;
        buffer[i*SM_SCENERY_ELEVATION_TEXTURE_COMPONENTS+G] = 0;
        buffer[i*SM_SCENERY_ELEVATION_TEXTURE_COMPONENTS+B] = 0;
        buffer[i*SM_SCENERY_ELEVATION_TEXTURE_COMPONENTS+A] = 255;
        bolds++;
      }
    }
  }

  *texcoordscale = 1.0f / (lines * distance);
  *texcoordoffset = offset * (*texcoordscale);
}

/* ********************************************************************** */

void
sc_display_debug_info(float * campos, short * vpsize, void * debuglist)
{
  SbList<float> * list = (SbList<float> *) debuglist;

  int depthtest = glIsEnabled(GL_DEPTH_TEST);
  if ( depthtest ) glDisable(GL_DEPTH_TEST);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(-0.5f, 0.5f, -0.5f, 0.5f, -1.0f, 1.0f);
  glDepthMask(GL_FALSE);
  glColor3f(1.0f, 0.0f, 0.0f);

  int num = list->getLength() / 4;
  int i;

  float mind = 1.0f;

  for (i = 0; i < num; i++) {
    float x0, x1;
    x0 = list->operator[](i*4);
    x1 = list->operator[](i*4+2);

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
    x0 = list->operator[](i*4) - 0.5f;
    y0 = list->operator[](i*4+1) - 0.5f;
    x1 = list->operator[](i*4+2) - 0.5f;
    y1 = list->operator[](i*4+3) - 0.5f;

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

  if ( depthtest ) glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}

/* ********************************************************************** */

// We provide the environment variable below to work around a bug in
// the 3Dlabs OpenGL driver version 4.10.01.2105-2.16.0866: it doesn't
// properly handle normals given in byte values (i.e. through
// glNormal*b*()).
//
// This driver is fairly old, and 3Dlabs is more or less defunct now,
// so we default to the faster (but bug-triggering) GL call.

static inline const char *
sm_getenv(const char * s)
{
#ifdef __COIN__
  return coin_getenv(s);
#else // other Inventor
  return getenv(s);
#endif
}

static inline SbBool
ok_to_use_bytevalue_normals(void)
{
  static int okflag = -1;
  if (okflag == -1) {
    const char * env = sm_getenv("SM_SCENERY_NO_BYTENORMALS");
    okflag = env && (atoi(env) > 0);
  }
  return !okflag;
}

inline void
GL_VERTEX(RenderState * state, const int x, const int y, const float elev)
{
  glTexCoord2f(state->toffset[0] + (float(x)/state->blocksize) * state->tscale[0],
               state->toffset[1] + (float(y)/state->blocksize) * state->tscale[1]);
  
  if (state->etexscale != 0.0f) {
    float val = (state->etexscale * elev) + state->etexoffset;
    if ( val < 0.0f ) val = 0.0f - val;
    cc_glglue_glMultiTexCoord2f(glglue, GL_TEXTURE1, 0.0f, val);
  }
  glVertex3f((float) (x*state->vspacing[0] + state->voffset[0]),
             (float) (y*state->vspacing[1] + state->voffset[1]),
             elev);
}

inline void
GL_VERTEX_N(RenderState * state, const int x, const int y, const float elev, const signed char * n)
{
  if ( ok_to_use_bytevalue_normals()) {
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
GL_VERTEX_TN(RenderState * state, const int x, const int y, const float elev, const signed char * n)
{
  if ( ok_to_use_bytevalue_normals() ) {
    glNormal3bv((const GLbyte *)n);
  }
  else {
    static const float factor = 1.0f/127.0f;
    glNormal3f(n[0] * factor, n[1] * factor, n[2] * factor);
  }
  glTexCoord2f(state->toffset[0] + (float(x)/state->blocksize) * state->tscale[0],
               state->toffset[1] + (float(y)/state->blocksize) * state->tscale[1]);
  if (state->etexscale != 0.0f) {
    float val = (state->etexscale * elev) + state->etexoffset;
    cc_glglue_glMultiTexCoord2f(glglue, GL_TEXTURE1, 0.0f, val);
  }
  glVertex3f((float) (x*state->vspacing[0] + state->voffset[0]),
             (float) (y*state->vspacing[1] + state->voffset[1]),
             elev);
}

/* ********************************************************************** */

#define ELEVATION(x,y) elev[(y)*W+(x)]    

void 
sc_undefrender_cb(void * closure, const int x, const int y, const int len, 
                  const unsigned int bitmask_org)
{
  RenderState * renderstate = (RenderState *) closure;

  const signed char * normals = renderstate->normaldata;
  const float * elev = renderstate->elevdata;
  const int W = ((int) renderstate->blocksize) + 1;

  const signed char * ptr = sc_ssglue_render_get_undef_array(bitmask_org);

  int numv = *ptr++;
  int tx, ty;

  if (normals && renderstate->texid == 0) {
    int idx;
#define SEND_VERTEX(state, x, y) \
    idx = (y)*W + x; \
    GL_VERTEX_N(state, x, y, elev[idx], normals+3*idx);

    while (numv) {
      glBegin(GL_TRIANGLE_FAN);
      while (numv) { 
        tx = x + *ptr++ * len;
        ty = y + *ptr++ * len;
        SEND_VERTEX(renderstate, tx, ty);
        numv--;
      }
      numv = *ptr++;
      glEnd();

    }
#undef SEND_VERTEX
  }
  else if (normals) {
    int idx;
#define SEND_VERTEX(state, x, y) \
    idx = (y)*W + x; \
    GL_VERTEX_TN(state, x, y, elev[idx], normals+3*idx);

    while (numv) {
      glBegin(GL_TRIANGLE_FAN);
      while (numv) { 
        tx = x + *ptr++ * len;
        ty = y + *ptr++ * len;
        SEND_VERTEX(renderstate, tx, ty);
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
        GL_VERTEX(renderstate, tx, ty, ELEVATION(tx, ty));
        numv--;
      }
      numv = *ptr++;
      glEnd();
    }
  }
}

void 
sc_render_cb(void * closure, const int x, const int y,
             const int len, const unsigned int bitmask)
{
  RenderState * renderstate = (RenderState *) closure;

  const signed char * normals = renderstate->normaldata;  
  const float * elev = renderstate->elevdata;
  const int W = ((int) renderstate->blocksize) + 1;

  int idx;
  if (normals && renderstate->texid == 0) {
#define SEND_VERTEX(state, x, y) \
  idx = (y)*W + x; \
  GL_VERTEX_N(state, x, y, elev[idx], normals+3*idx);

    glBegin(GL_TRIANGLE_FAN);
    SEND_VERTEX(renderstate, x, y);
    SEND_VERTEX(renderstate, x-len, y-len);
    if (!(bitmask & SS_RENDER_BIT_SOUTH)) {
      SEND_VERTEX(renderstate, x, y-len);
    }
    SEND_VERTEX(renderstate, x+len, y-len);
    if (!(bitmask & SS_RENDER_BIT_EAST)) {
      SEND_VERTEX(renderstate, x+len, y);
    }
    SEND_VERTEX(renderstate, x+len, y+len);
    if (!(bitmask & SS_RENDER_BIT_NORTH)) {
      SEND_VERTEX(renderstate, x, y+len);
    }
    SEND_VERTEX(renderstate, x-len, y+len);
    if (!(bitmask & SS_RENDER_BIT_WEST)) {
      SEND_VERTEX(renderstate, x-len, y);
    }
    SEND_VERTEX(renderstate, x-len, y-len);
    glEnd();
#undef SEND_VERTEX
  }
  else if (normals) {
#define SEND_VERTEX(state, x, y) \
  idx = (y)*W + x; \
  GL_VERTEX_TN(state, x, y, elev[idx], normals+3*idx);

    glBegin(GL_TRIANGLE_FAN);
    SEND_VERTEX(renderstate, x, y);
    SEND_VERTEX(renderstate, x-len, y-len);
    if (!(bitmask & SS_RENDER_BIT_SOUTH)) {
      SEND_VERTEX(renderstate, x, y-len);
    }
    SEND_VERTEX(renderstate, x+len, y-len);
    if (!(bitmask & SS_RENDER_BIT_EAST)) {
      SEND_VERTEX(renderstate, x+len, y);
    }
    SEND_VERTEX(renderstate, x+len, y+len);
    if (!(bitmask & SS_RENDER_BIT_NORTH)) {
      SEND_VERTEX(renderstate, x, y+len);
    }
    SEND_VERTEX(renderstate, x-len, y+len);
    if (!(bitmask & SS_RENDER_BIT_WEST)) {
      SEND_VERTEX(renderstate, x-len, y);
    }
    SEND_VERTEX(renderstate, x-len, y-len);
    glEnd();
#undef SEND_VERTEX
  }
  else {
    glBegin(GL_TRIANGLE_FAN);
    GL_VERTEX(renderstate, x, y, ELEVATION(x, y));
    GL_VERTEX(renderstate, x-len, y-len, ELEVATION(x-len, y-len));
    if (!(bitmask & SS_RENDER_BIT_SOUTH)) {
      GL_VERTEX(renderstate, x, y-len, ELEVATION(x, y-len));
    }
    GL_VERTEX(renderstate, x+len, y-len, ELEVATION(x+len, y-len));
    if (!(bitmask & SS_RENDER_BIT_EAST)) {
      GL_VERTEX(renderstate, x+len, y, ELEVATION(x+len, y));
    }
    GL_VERTEX(renderstate, x+len, y+len, ELEVATION(x+len, y+len));
    if (!(bitmask & SS_RENDER_BIT_NORTH)) {
      GL_VERTEX(renderstate, x, y+len, ELEVATION(x, y+len));
    }
    GL_VERTEX(renderstate, x-len, y+len, ELEVATION(x-len, y+len));
    if (!(bitmask & SS_RENDER_BIT_WEST)) {
      GL_VERTEX(renderstate, x-len, y, ELEVATION(x-len, y));
    }
    GL_VERTEX(renderstate, x-len, y-len, ELEVATION(x-len, y-len));
    glEnd();
#undef ELEVATION
  }
}

/* ********************************************************************** */
