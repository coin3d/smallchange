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
#include <stdio.h>
#include <math.h> // fmod()

#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif /* HAVE_WINDOWS_H */

#include <GL/gl.h>

#define SS_DLL

#include "SceneryGL.h"

/* this source file is shared between SIM Scenery and SmallChange, hence the
 * strange conditional includes below */
#ifdef SS_MAJOR_VERSION
#include "SbList.h"
#include "SbHash.h"
#include "SbVec3.h"
#include "SbBox3.h"
#include "SbPlane.h"

#include <sim/scenery/scenery.h>
#else
#include "../misc/SbList.h"
#include "../misc/SbHash.h"
#include "../misc/SbVec3.h"
#include "../misc/SbBox3.h"
#include "../misc/SbPlane.h"
#include <SmallChange/misc/SceneryGlue.h>
#endif

/* ********************************************************************** */

// the number of frames a texture can be unused before being recycled
#define MAX_UNUSED_COUNT 200

/* how to interface back to scenery library */
#ifndef SS_SCENERY_H
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

/* ********************************************************************** */
/* GL setup */

typedef void glMultiTexCoord2f_f(GLenum target, GLfloat x, GLfloat y);

typedef void glEnableClientState_f(GLenum cap);
typedef void glDisableClientState_f(GLenum cap);
typedef void glVertexPointer_f(GLint size, GLenum type, GLsizei stride, const GLvoid * ptr);
typedef void glNormalPointer_f(GLenum type, GLsizei stride, const GLvoid * ptr);
typedef void glTexCoordPointer_f(GLint dims, GLenum type, GLsizei stride, const GLvoid * ptr);
typedef void glDrawElements_f(GLenum mode, GLsizei count, GLenum type, const GLvoid * ptr);
typedef void glDrawArrays_f(GLenum mode, GLint first, GLsizei count);
typedef void glClientActiveTexture_f(GLenum texture);

struct sc_GL {
  glMultiTexCoord2f_f * glMultiTexCoord2f;
  glEnableClientState_f * glEnableClientState;
  glDisableClientState_f * glDisableClientState;
  glVertexPointer_f * glVertexPointer;
  glNormalPointer_f * glNormalPointer;
  glTexCoordPointer_f * glTexCoordPointer;
  glDrawElements_f * glDrawElements;
  glDrawArrays_f * glDrawArrays;
  glClientActiveTexture_f * glClientActiveTexture;

  GLenum CLAMP_TO_EDGE;
  int use_byte_normals;
};

static struct sc_GL GL = {
  NULL, // glMultiTexCoord2f
  NULL, // glEnableClientState
  NULL, // glDisableClientState
  NULL, // glVertexPointer
  NULL, // glNormalPointer
  NULL, // glTexCoordPointer
  NULL, // glDrawElements
  NULL, // glDrawArrays
  NULL, // glClientActiveTexture

  GL_CLAMP, // clamp_to_edge
  TRUE  // use_byte_normals
};

void
sc_set_glEnableClientState(void * fptr)
{
  assert(fptr != NULL);
  GL.glEnableClientState = (glEnableClientState_f *) fptr;
}

void
sc_set_glDisableClientState(void * fptr)
{
  assert(fptr != NULL);
  GL.glDisableClientState = (glDisableClientState_f *) fptr;
}

void
sc_set_glVertexPointer(void * fptr)
{
  assert(fptr != NULL);
  GL.glVertexPointer = (glVertexPointer_f *) fptr;
}

void
sc_set_glNormalPointer(void * fptr)
{
  assert(fptr != NULL);
  GL.glNormalPointer = (glNormalPointer_f *) fptr;
}

void
sc_set_glTexCoordPointer(void * fptr)
{
  assert(fptr != NULL);
  GL.glTexCoordPointer = (glTexCoordPointer_f *) fptr;
}

void
sc_set_glDrawElements(void * fptr)
{
  assert(fptr != NULL);
  GL.glDrawElements = (glDrawElements_f *) fptr;
}

void
sc_set_glDrawArrays(void * fptr)
{
  assert(fptr != NULL);
  GL.glDrawArrays = (glDrawArrays_f *) fptr;
}

void
sc_set_glMultiTexCoord2f(void * fptr)
{
  assert(fptr != NULL);
  GL.glMultiTexCoord2f = (glMultiTexCoord2f_f *) fptr;
}

void
sc_set_glClientActiveTexture(void * fptr)
{
  assert(fptr != NULL);
  GL.glClientActiveTexture = (glClientActiveTexture_f *) fptr;
}

void
sc_set_have_clamp_to_edge(int enable)
{
  if ( enable ) {
    GL.CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE;
  } else {
    GL.CLAMP_TO_EDGE = GL_CLAMP;
  }
}

void
sc_set_use_byte_normals(int enable)
{
  if ( enable ) {
    GL.use_byte_normals = TRUE;
  } else {
    GL.use_byte_normals = FALSE;
  }
}

/* ********************************************************************** */
/* texture management */

static sc_texture_construct_f * texture_construct = NULL;
static sc_texture_activate_f * texture_activate = NULL;
static sc_texture_release_f * texture_release = NULL;

void
sc_set_texture_functions(sc_texture_construct_f * construct, sc_texture_activate_f * activate, sc_texture_release_f * release)
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
    this->image = NULL;
  }
  unsigned int texid;
  void * image;
  int unusedcount;
};

void
sc_renderstate_construct(RenderState * state)
{
  // construct lists
  state->clipplanes = NULL;
  state->numclipplanes = 0;
  state->reusetexlist = (void *) new SbList<TexInfo *>;
  state->tmplist = (void *) new SbList<unsigned int>;
  state->debuglist = (void *) new SbList<float>;
  state->texhash = new SbHash<TexInfo *, unsigned int>(1024, 0.7f);
  state->state = NULL;
  state->action = NULL;

  state->vertexcount = 0;
  state->varray = NULL;
  state->narray = NULL;
  state->t1array = NULL;
  state->t2array = NULL;
  state->idxarray = NULL;
}

static void sc_texture_hash_clear(unsigned int key, TexInfo * val, void * closure);

void
sc_renderstate_destruct(RenderState * state)
{
  // delete lists
  if ( state->reusetexlist ) {
    SbList<TexInfo *> * list = (SbList<TexInfo *> *) state->reusetexlist;
    if ( list->getLength() ) {
      // printf("scenery: reusetexlist not empty (%d)\n", list->getLength());
      int i;
      assert(texture_release);
      for ( i = 0; i < list->getLength(); i++ ) {
        TexInfo * tex = (*list)[i];
        texture_release(tex->image);
        delete tex;
      }
    }
    delete list;
    state->reusetexlist = NULL;
  }
  SbHash<TexInfo *, unsigned int> * texhash = (SbHash<TexInfo *, unsigned int> *) state->texhash;
  texhash->apply(sc_texture_hash_clear, NULL);
  delete texhash;
  state->texhash = NULL;
  // clipplanes are not owned by RenderState - managed elsewhere
#define DELETE_LIST(list, type) \
  if ( state->list ) { \
    SbList<type> * instance = (SbList<type> *) state->list; \
    delete instance; \
    state->list = NULL; \
  }
  DELETE_LIST(debuglist, float);
  DELETE_LIST(tmplist, unsigned int);
  DELETE_LIST(varray, float);
  DELETE_LIST(narray, signed char);
  DELETE_LIST(t1array, float);
  DELETE_LIST(t2array, float);
  DELETE_LIST(idxarray, unsigned int);
#undef DELETE_LIST
}

/* ********************************************************************** */

void *
sc_find_reuse_texture(RenderState * state)
{
  TexInfo * tex = NULL;
  SbHash<TexInfo *, unsigned int> * texhash = (SbHash<TexInfo *, unsigned int> *) state->texhash;
  if (texhash->get(state->texid, tex)) {
    assert(tex->image);
    tex->unusedcount = 0;
    return tex->image;
  }
  return NULL;
}

void *
sc_create_texture(RenderState * state, void * image)
{
  assert(state);
  assert(state->texhash);
  assert(state->reusetexlist);
  TexInfo * tex = NULL;
  SbList<TexInfo *> * reusetexlist = (SbList<TexInfo *> *) state->reusetexlist;
  if ( reusetexlist->getLength() ) {
    tex = reusetexlist->pop();
    // FIXME: optimize this
    assert(texture_release);
    texture_release(tex->image);
    tex->image = NULL;
  }
  else {
    tex = new TexInfo;
  }
  tex->image = image;
  tex->texid = state->currtexid;
  tex->unusedcount = 0;

  SbHash<TexInfo *, unsigned int> * texhash = (SbHash<TexInfo *, unsigned int> *) state->texhash;
  texhash->put(state->texid, tex);
  return tex->image;
}

static void sc_texture_hash_check_unused(unsigned int key, TexInfo * val, void * closure);
static void sc_texture_hash_inc_unused(unsigned int key, TexInfo * val, void * closure);
static void sc_texture_hash_add_all(unsigned int key, TexInfo * val, void * closure);

void
sc_delete_unused_textures(RenderState * state)
{
  assert(state);
  assert(state->tmplist);
  assert(state->texhash);
  assert(state->reusetexlist);
  SbList<unsigned int> * tmplist = (SbList<unsigned int> *) state->tmplist;
  assert(tmplist->getLength() == 0);

  SbHash<TexInfo *, unsigned int> * texhash = (SbHash<TexInfo *, unsigned int> *) state->texhash;
  texhash->apply(sc_texture_hash_check_unused, tmplist);
  
  int i;
  for (i = 0; i < tmplist->getLength(); i++) {
    TexInfo * tex = NULL;
    texhash->get((*tmplist)[i], tex);
    ((SbList<TexInfo *> *) state->reusetexlist)->push(tex);
    texhash->remove((*tmplist)[i]);
  }

//   fprintf(stderr,"SmScenery now has %d active textures, %d reusable textures (removed %d)\n",
//           cc_hash_get_num_elements(this->renderstate.texhash), this->reusetexlist.getLength(), this->tmplist.getLength());

  tmplist->truncate(0);
}

void
sc_delete_all_textures(RenderState * state)
{
  assert(state);
  assert(state->tmplist);
  assert(state->texhash);
  assert(state->reusetexlist);
  SbList<unsigned int> * tmplist = (SbList<unsigned int> *) state->tmplist;
  assert(tmplist->getLength() == 0);
  SbList<TexInfo *> * reusetexlist = (SbList<TexInfo *> *) state->reusetexlist;
  SbHash<TexInfo *, unsigned int> * texhash = (SbHash<TexInfo *, unsigned int> *) state->texhash;
  texhash->apply(sc_texture_hash_add_all, tmplist);

  int i;
  for ( i = 0; i < tmplist->getLength(); i++ ) {
    TexInfo * tex = NULL;
    texhash->get((*tmplist)[i], tex);
    assert(tex);
    reusetexlist->push(tex);
    texhash->remove((*tmplist)[i]);
  }
  tmplist->truncate(0);
}

void
sc_mark_unused_textures(RenderState * state)
{
  SbHash<TexInfo *, unsigned int> * texhash = (SbHash<TexInfo *, unsigned int> *) state->texhash;
  texhash->apply(sc_texture_hash_inc_unused, NULL);
}

/* ********************************************************************** */

void
sc_texture_hash_check_unused(unsigned int key, TexInfo * tex, void * closure)
{
  assert(tex);
  if ( tex->unusedcount > MAX_UNUSED_COUNT ) {
    SbList<unsigned int> * keylist = (SbList<unsigned int> *) closure;
    keylist->append(key);
  }
}

void
sc_texture_hash_clear(unsigned int key, TexInfo * tex, void * closure)
{
  // safe to delete the TexInfo objects here since we'll never use this list again
  assert(tex);
  assert(tex->image);
  assert(texture_release);
  texture_release(tex->image);
  delete tex;
}

void
sc_texture_hash_add_all(unsigned int key, TexInfo * tex, void * closure)
{
  assert(tex);
  SbList <unsigned int> * keylist = (SbList <unsigned int> *) closure;
  keylist->append(key);
}

void
sc_texture_hash_inc_unused(unsigned int key, TexInfo * tex, void * closure)
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
  // printf("lines: %g, pixelsize: %g, texturesize: %d\n", lines, pixelsize, texturesize);
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
  SbList<float> * list = (SbList<float> *) state->debuglist;
  list->truncate(0);
}

void
sc_display_debug_info(RenderState * state, float * campos, short * vpsize)
{
  assert(state);
  SbList<float> * list = (SbList<float> *) state->debuglist;
  if ( !list ) return;

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

  // when a block is not culled, it is either rendered, or its children are checked
  // against the cull planes.  therefore - pre() and post() callbacks can be used stack-
  // based and flags can be set to avoid testing against certain planes further down in
  // the recursion when all corners are inside (inside == 8) the plane.
  SbList<int> * cullstate = (SbList<int> *) state->tmplist;
  assert(cullstate);

  int mask = 0;
  if ( cullstate->getLength() > 0 ) {
    // FIXME: the masking does not seem to work - disables a lot of culling
    // mask = cullstate->getLast();
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
  for ( i = 0; i < state->numclipplanes; i++ ) { // foreach plane
    if ( (mask & (1 << i)) != 0 ) continue; // uncullable plane - all corners will be inside
    SbPlane<float> plane(SbVec3<float>(state->clipplanes[i*4+0], state->clipplanes[i*4+1], state->clipplanes[i*4+2]),
                         state->clipplanes[i*4+3]);
    int outside = 0, inside = 0;
    for ( j = 0; j < 8; j++ ) { // foreach bbox corner point
      if ( !plane.isInHalfSpace(point[j]) ) { outside++; }
      else { inside++; }
    }
    if ( inside == 8 ) { // mark this plane as uncullable
      bits = bits | (1 << i);
    }
    if ( outside == 8 ) {
      cullstate->push(0); // push state since post_cb pops it
      return FALSE; // culled
    }
  }
  cullstate->push(mask | bits); // push culling state for next iteration
  return TRUE; // not culled
}

void
sc_plane_culling_post_cb(void * closure)
{
  assert(closure);
  RenderState * state = (RenderState *) closure;
  SbList<int> * cullstate = (SbList<int> *) state->tmplist;
  assert(cullstate);
  cullstate->pop();
}

int
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

void
sc_ray_culling_post_cb(void * closure)
{
  // nothing to do here
  // assert(closure);
  // RenderState * state = (RenderState *) closure;
}

/* ********************************************************************** */

inline void
GL_VERTEX(RenderState * state, const int x, const int y, const float elev)
{
  glTexCoord2f(state->toffset[0] + float(x) * state->invtsizescale[0],
               state->toffset[1] + float(y) * state->invtsizescale[1]);
  
  if (state->etexscale != 0.0f && GL.glMultiTexCoord2f != NULL) {
    GL.glMultiTexCoord2f(GL_TEXTURE1,
                         0.0f, (state->etexscale * elev) + state->etexoffset);
  }
  glVertex3f((float) (x*state->vspacing[0] + state->voffset[0]),
             (float) (y*state->vspacing[1] + state->voffset[1]),
             elev);
}

inline void
GL_VA_VERTEX(RenderState * state, const int x, const int y, const float elev)
{
  static float vertex[3][3];
  static float texture1[3][2];
  static float texture2[3][2];

  const int idx = (state->vertexcount == 0) ? 0 : 2; // store fan center in idx 0
  if ( idx != 0 ) { // move up old entries
    vertex[1][0] = vertex[2][0];
    vertex[1][1] = vertex[2][1];
    vertex[1][2] = vertex[2][2];
    texture1[1][0] = texture1[2][0];
    texture1[1][1] = texture1[2][1];
    texture2[1][0] = texture2[2][0];
    texture2[1][1] = texture2[2][1];
  }
  texture1[idx][0] = state->toffset[0] + float(x) * state->invtsizescale[0];
  texture1[idx][1] = state->toffset[1] + float(y) * state->invtsizescale[1];
  texture2[idx][0] = 0.0f;
  texture2[idx][1] = (state->etexscale * elev) + state->etexoffset;
  vertex[idx][0] = (float) (x*state->vspacing[0] + state->voffset[0]);
  vertex[idx][1] = (float) (y*state->vspacing[1] + state->voffset[1]);
  vertex[idx][2] = elev;

  if ( ++state->vertexcount >= 3 ) {
    SbList<float> * vertexarray = (SbList<float> *) state->varray;
    SbList<float> * texcoord1array = (SbList<float> *) state->t1array;
    SbList<float> * texcoord2array = (SbList<float> *) state->t2array;
    for ( int v = 0; v < 3; v++ ) {
      vertexarray->append(vertex[v][0]);
      vertexarray->append(vertex[v][1]);
      vertexarray->append(vertex[v][2]);
      texcoord1array->append(texture1[v][0]);
      texcoord1array->append(texture1[v][1]);
      texcoord2array->append(texture2[v][0]);
      texcoord2array->append(texture2[v][1]);
    }
  }
}

inline void
GL_VERTEX_N(RenderState * state, const int x, const int y, const float elev, const signed char * n)
{
  if ( GL.use_byte_normals ) {
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
GL_VA_VERTEX_N(RenderState * state, const int x, const int y, const float elev, const signed char * n)
{
  static signed char normal[3][3];
  static float vertex[3][3];

  const int idx = (state->vertexcount == 0) ? 0 : 2; // store fan center in idx 0
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

  if ( ++state->vertexcount >= 3 ) {
    SbList<float> * vertexarray = (SbList<float> *) state->varray;
    SbList<signed char> * normalarray = (SbList<signed char> *) state->narray;
    for ( int v = 0; v < 3; v++ ) {
      normalarray->append(normal[v][0]);
      normalarray->append(normal[v][1]);
      normalarray->append(normal[v][2]);
      vertexarray->append(vertex[v][0]);
      vertexarray->append(vertex[v][1]);
      vertexarray->append(vertex[v][2]);
    }
  }
}

inline void
GL_VERTEX_TN(RenderState * state, const int x, const int y, const float elev, const signed char * n)
{
  if ( GL.use_byte_normals ) { glNormal3bv((const GLbyte *)n); }
  else {
    static const float factor = 1.0f/127.0f;
    glNormal3f(n[0] * factor, n[1] * factor, n[2] * factor);
  }
  glTexCoord2f(state->toffset[0] + float(x) * state->invtsizescale[0],
               state->toffset[1] + float(y) * state->invtsizescale[1]);
  if (state->etexscale != 0.0f && GL.glMultiTexCoord2f != NULL) {
    GL.glMultiTexCoord2f(GL_TEXTURE1,
                         0.0f, (state->etexscale * elev) + state->etexoffset);
  }
  glVertex3f((float) (x*state->vspacing[0] + state->voffset[0]),
             (float) (y*state->vspacing[1] + state->voffset[1]),
             elev);
}

inline void
GL_VA_VERTEX_TN(RenderState * state, const int x, const int y, const float elev, const signed char * n)
{
  static signed char normal[3][3];
  static float vertex[3][3];
  static float texture1[3][2];
  static float texture2[3][2];

  const int idx = (state->vertexcount == 0) ? 0 : 2; // store fan center in idx 0
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
  texture1[idx][0] = state->toffset[0] + float(x) * state->invtsizescale[0];
  texture1[idx][1] = state->toffset[1] + float(y) * state->invtsizescale[1];
  texture2[idx][0] = 0.0f;
  texture2[idx][1] = (state->etexscale * elev) + state->etexoffset;
  vertex[idx][0] = (float) (x*state->vspacing[0] + state->voffset[0]);
  vertex[idx][1] = (float) (y*state->vspacing[1] + state->voffset[1]);
  vertex[idx][2] = elev;

  if ( ++state->vertexcount >= 3 ) { // we have a new triangle
    SbList<float> * vertexarray = (SbList<float> *) state->varray;
    SbList<signed char> * normalarray = (SbList<signed char> *) state->narray;
    SbList<float> * texcoord1array = (SbList<float> *) state->t1array;
    SbList<float> * texcoord2array = (SbList<float> *) state->t2array;
    for ( int v = 0; v < 3; v++ ) {
      texcoord1array->append(texture1[v][0]);
      texcoord1array->append(texture1[v][1]);
      texcoord2array->append(texture2[v][0]);
      texcoord2array->append(texture2[v][1]);
      normalarray->append(normal[v][0]);
      normalarray->append(normal[v][1]);
      normalarray->append(normal[v][2]);
      vertexarray->append(vertex[v][0]);
      vertexarray->append(vertex[v][1]);
      vertexarray->append(vertex[v][2]);
    }
  }
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
                                   &renderstate->elevdata,
                                   &renderstate->normaldata);

  if ( renderstate->debuglist ) {
    float ox = (float) (renderstate->voffset[0] / renderstate->bbmax[0]);
    float oy = (float) (renderstate->voffset[1] / renderstate->bbmax[1]);
    float sx = (float) ((renderstate->vspacing[0] * renderstate->blocksize) / renderstate->bbmax[0]);
    float sy = (float) ((renderstate->vspacing[1] * renderstate->blocksize) / renderstate->bbmax[1]);
    ((SbList<float> *) renderstate->debuglist)->append(ox);
    ((SbList<float> *) renderstate->debuglist)->append(oy);
    ((SbList<float> *) renderstate->debuglist)->append(ox+sx);
    ((SbList<float> *) renderstate->debuglist)->append(oy+sy);
  }

  ss_render_get_texture_measures(info, &renderstate->texid,
                                 renderstate->toffset, renderstate->tscale);
  // for optimizing texture coordinate calculations
  renderstate->invtsizescale[0] = (1.0f / renderstate->blocksize) * renderstate->tscale[0];
  renderstate->invtsizescale[1] = (1.0f / renderstate->blocksize) * renderstate->tscale[1];
}


void
sc_render_pre_cb(void * closure, ss_render_block_cb_info * info)
{
  sc_render_pre_cb_common(closure, info);
  RenderState * renderstate = (RenderState *) closure;

  // set up texture for block
  if (renderstate->dotex && renderstate->texid) {
    if (renderstate->texid != renderstate->currtexid) {
      void * imagehandle = sc_find_reuse_texture(renderstate);
      if ( !imagehandle ) {
        ss_render_get_texture_image(info, renderstate->texid,
                                    &renderstate->texdata,
                                    &renderstate->texw,
                                    &renderstate->texh,
                                    &renderstate->texnc);

        int clampmode = GL.CLAMP_TO_EDGE;
        assert(texture_construct);
        imagehandle = texture_construct(renderstate->texdata, renderstate->texw, renderstate->texh, renderstate->texnc, clampmode, clampmode, 0.9f, 0);

        imagehandle = sc_create_texture(renderstate, imagehandle);
        assert(imagehandle);

        renderstate->newtexcount++;
      }
      assert(texture_activate);
      texture_activate(renderstate, imagehandle);

      renderstate->currtexid = renderstate->texid;
    }
    if (!renderstate->texisenabled) {
      glEnable(GL_TEXTURE_2D);
      renderstate->texisenabled = TRUE;
    }
  }
  else {
    if (renderstate->texisenabled) {
      glDisable(GL_TEXTURE_2D);
      renderstate->texisenabled = FALSE;
    }
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
  }
}

void 
sc_va_render_cb(void * closure, const int x, const int y,
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
  GL_VA_VERTEX_N(state, x, y, elev[idx], normals+3*idx);

    renderstate->vertexcount = 0;
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
#undef SEND_VERTEX
  }
  else if (normals) {
#define SEND_VERTEX(state, x, y) \
  idx = (y)*W + x; \
  GL_VA_VERTEX_TN(state, x, y, elev[idx], normals+3*idx);

    renderstate->vertexcount = 0;
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
#undef SEND_VERTEX
  }
  else {
    renderstate->vertexcount = 0;
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
  }
}

void 
sc_undefrender_cb(void * closure, const int x, const int y, const int len, 
                  const unsigned int bitmask_org)
{
  RenderState * renderstate = (RenderState *) closure;

  const signed char * normals = renderstate->normaldata;
  const float * elev = renderstate->elevdata;
  const int W = ((int) renderstate->blocksize) + 1;

  const signed char * ptr = ss_render_get_undef_array(bitmask_org);

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
sc_va_undefrender_cb(void * closure, const int x, const int y, const int len, 
                     const unsigned int bitmask_org)
{
  RenderState * renderstate = (RenderState *) closure;

  const signed char * normals = renderstate->normaldata;
  const float * elev = renderstate->elevdata;
  const int W = ((int) renderstate->blocksize) + 1;

  const signed char * ptr = ss_render_get_undef_array(bitmask_org);

  int numv = *ptr++;
  int tx, ty;

  if (normals && renderstate->texid == 0) {
    int idx;
#define SEND_VERTEX(state, x, y) \
    idx = (y)*W + x; \
    GL_VA_VERTEX_N(state, x, y, elev[idx], normals+3*idx);

    while (numv) {
      renderstate->vertexcount = 0;
      while (numv) { 
        tx = x + *ptr++ * len;
        ty = y + *ptr++ * len;
        SEND_VERTEX(renderstate, tx, ty);
        numv--;
      }
      numv = *ptr++;
    }
#undef SEND_VERTEX
  }
  else if (normals) {
    int idx;
#define SEND_VERTEX(state, x, y) \
    idx = (y)*W + x; \
    GL_VA_VERTEX_TN(state, x, y, elev[idx], normals+3*idx);

    while (numv) {
      renderstate->vertexcount = 0;
      while (numv) { 
        tx = x + *ptr++ * len;
        ty = y + *ptr++ * len;
        SEND_VERTEX(renderstate, tx, ty);
        numv--;
      }
      numv = *ptr++;
    }
#undef SEND_VERTEX
  }  
  else {    
    while (numv) {
      renderstate->vertexcount = 0;
      while (numv) { 
        tx = x + *ptr++ * len;
        ty = y + *ptr++ * len;
        GL_VA_VERTEX(renderstate, tx, ty, ELEVATION(tx, ty));
        numv--;
      }
      numv = *ptr++;
    }
  }
}

#undef ELEVATION

/* ********************************************************************** */

void
sc_va_render_pre_cb(void * closure, ss_render_block_cb_info * info)
{
  sc_render_pre_cb_common(closure, info);
  RenderState * renderstate = (RenderState *) closure;

  if ( (renderstate->varray == NULL) ) {
    renderstate->varray = new SbList<float>(4096);
    renderstate->narray = new SbList<signed char>(4096);
    renderstate->t1array = new SbList<float>(4096);
    renderstate->t2array = new SbList<float>(4096);
    assert(renderstate->varray);
    assert(renderstate->narray);
    assert(renderstate->t1array);
    assert(renderstate->t2array);
  }

  SbList<float> * vertexarray = (SbList<float> *) renderstate->varray;
  SbList<signed char> * normalarray = (SbList<signed char> *) renderstate->narray;
  SbList<float> * texcoord1array = (SbList<float> *) renderstate->t1array;
  SbList<float> * texcoord2array = (SbList<float> *) renderstate->t2array;
  // reset lists
  vertexarray->truncate(0);
  normalarray->truncate(0);
  texcoord1array->truncate(0);
  texcoord2array->truncate(0);
}

void
sc_va_render_post_cb(void * closure, ss_render_block_cb_info * info)
{
  assert(closure);
  RenderState * renderstate = (RenderState *) closure;
  assert(renderstate->varray != NULL);
  SbList<float> * vertexarray = (SbList<float> *) renderstate->varray;
  SbList<signed char> * normalarray = (SbList<signed char> *) renderstate->narray;
  SbList<float> * texcoord1array = (SbList<float> *) renderstate->t1array;
  SbList<float> * texcoord2array = (SbList<float> *) renderstate->t2array;

  if ( vertexarray->getLength() == 0 ) { return; } // nothing to render in this block

  // If the number of vertices for a block is small, we might consider adding
  // another block or two before pushing the vertex arrays to the graphics card,
  // but then we would have to have a post-render callback or something to make
  // sure the last blocks are actually rendered.

  assert(info);
  assert(GL.glClientActiveTexture != NULL);
  assert(GL.glEnableClientState != NULL);
  assert(GL.glDisableClientState != NULL);
  assert(GL.glVertexPointer != NULL);
  assert(GL.glNormalPointer != NULL);
  assert(GL.glTexCoordPointer != NULL);
  assert(GL.glDrawElements != NULL);
  assert(GL.glDrawArrays != NULL);

  // Set up textures before rendering - we delayed this because some blocks have
  // textures, but will be tessellated to 0 triangles if the block is mostly
  // undefined.
  if (renderstate->dotex && renderstate->texid) {
    if (renderstate->texid != renderstate->currtexid) {
      void * imagehandle = sc_find_reuse_texture(renderstate);
      if ( !imagehandle ) {
        ss_render_get_texture_image(info, renderstate->texid,
                                    &renderstate->texdata,
                                    &renderstate->texw,
                                    &renderstate->texh,
                                    &renderstate->texnc);

        int clampmode = GL.CLAMP_TO_EDGE;
        assert(texture_construct);
        imagehandle = texture_construct(renderstate->texdata, renderstate->texw, renderstate->texh, renderstate->texnc, clampmode, clampmode, 0.9f, 0);

        imagehandle = sc_create_texture(renderstate, imagehandle);
        assert(imagehandle);

        renderstate->newtexcount++;
      }
      assert(texture_activate);
      texture_activate(renderstate, imagehandle);

      renderstate->currtexid = renderstate->texid;
    }
    if (!renderstate->texisenabled) {
      glEnable(GL_TEXTURE_2D);
      renderstate->texisenabled = TRUE;
    }
  }
  else {
    if (renderstate->texisenabled) {
      glDisable(GL_TEXTURE_2D);
      renderstate->texisenabled = FALSE;
    }
  }

  const float * vertexarrayptr = vertexarray->getArrayPtr();
  const signed char * normalarrayptr = normalarray->getArrayPtr();
  const float * texcoord1arrayptr = texcoord1array->getArrayPtr();
  const float * texcoord2arrayptr = texcoord2array->getArrayPtr();

  const int vertices = vertexarray->getLength() / 3;
  const int triangles = vertices / 3;

  const signed char * normals = renderstate->normaldata; // used as a flag below

  // elevation data is common for all modes
  GL.glEnableClientState(GL_VERTEX_ARRAY);
  GL.glVertexPointer(3, GL_FLOAT, 0, vertexarrayptr);
  if ( normals != NULL && renderstate->texid == 0 ) {
    // elevation and normals
    assert((vertexarray->getLength() / 3) == (normalarray->getLength() / 3));
    GL.glEnableClientState(GL_NORMAL_ARRAY);
    GL.glNormalPointer(GL_BYTE, 0, normalarrayptr);
  } else if ( normals ) {
    // elevation, normals and textures
    assert((vertexarray->getLength() / 3) == (normalarray->getLength() / 3));
    GL.glEnableClientState(GL_NORMAL_ARRAY);
    GL.glNormalPointer(GL_BYTE, 0, normalarrayptr);
    if ( renderstate->etexscale != 0.0f ) {
      assert((vertexarray->getLength() / 3) == (texcoord2array->getLength() / 2));
      GL.glClientActiveTexture(GL_TEXTURE1);
      GL.glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      GL.glTexCoordPointer(2, GL_FLOAT, 0, texcoord2arrayptr);
    }
    assert((vertexarray->getLength() / 3) == (texcoord1array->getLength() / 2));
    GL.glClientActiveTexture(GL_TEXTURE0);
    GL.glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    GL.glTexCoordPointer(2, GL_FLOAT, 0, texcoord1arrayptr);
  } else {
    // elevation and textures
    if ( renderstate->etexscale != 0.0f ) {
      assert((vertexarray->getLength() / 3) == (texcoord2array->getLength() / 2));
      GL.glClientActiveTexture(GL_TEXTURE1);
      GL.glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      GL.glTexCoordPointer(2, GL_FLOAT, 0, texcoord2arrayptr);
    }
    assert((vertexarray->getLength() / 3) == (texcoord1array->getLength() / 2));
    GL.glClientActiveTexture(GL_TEXTURE0);
    GL.glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    GL.glTexCoordPointer(2, GL_FLOAT, 0, texcoord1arrayptr);
  }
    
  // Indexed vertex arrays will probably perform better, so if we can create
  // the index table without too much overhead, we're going to try it out...
  // GL.glDrawElements(GL_TRIANGLES,
  //                   indexarray->getLength(), GL_UNSIGNED_INT, indexarrayptr);

  GL.glDrawArrays(GL_TRIANGLES, 0, vertices);

  GL.glDisableClientState(GL_VERTEX_ARRAY);
  if ( normals != NULL && renderstate->texid == 0 ) {
    // elevation and normals
    GL.glDisableClientState(GL_NORMAL_ARRAY);
  } else if ( normals ) {
    // elevation, normals and textures
    GL.glDisableClientState(GL_NORMAL_ARRAY);
    if ( renderstate->etexscale != 0.0f ) {
      GL.glClientActiveTexture(GL_TEXTURE1);
      GL.glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
    GL.glClientActiveTexture(GL_TEXTURE0);
    GL.glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  } else {
    // elevation and textures
    if ( renderstate->etexscale != 0.0f ) {
      GL.glClientActiveTexture(GL_TEXTURE1);
      GL.glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
    GL.glClientActiveTexture(GL_TEXTURE0);
    GL.glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  }
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
    if ( state->intersected ) { // already have an intersection
      const SbVec3<float> newvec = intersectionpoint - raypos;
      const SbVec3<float> oldvec =
        SbVec3<float>(state->intersection[0], state->intersection[1], state->intersection[2]) - raypos;
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
      state->intersected = TRUE;
      state->intersection[0] = intersectionpoint[0];
      state->intersection[1] = intersectionpoint[1];
      state->intersection[2] = intersectionpoint[2];
    }
  }
}

/*!
*/

void
sc_raypick_pre_cb(void * closure, ss_render_block_cb_info * info)
{
  assert(closure);
  RenderState * renderstate = (RenderState *) closure;
  ss_render_get_elevation_measures(info,
                                   renderstate->voffset,
                                   renderstate->vspacing,
                                   &renderstate->elevdata,
                                   &renderstate->normaldata);
}

#define ELEVATION(x, y) elev[(y)*W+(x)]

#define GEN_VERTEX(state, x, y, z) \
  second = third; \
  third.setValue((float) ((x)*renderstate->vspacing[0] + renderstate->voffset[0]), \
                 (float) ((y)*renderstate->vspacing[1] + renderstate->voffset[1]), (z)); \
  if ( ++vertices >= 3 ) { \
    intersect(renderstate, first, second, third); \
  }

/*!
*/

void 
sc_raypick_cb(void * closure, const int x, const int y,
               const int len, const unsigned int bitmask)
{
  assert(closure);
  RenderState * renderstate = (RenderState *) closure;

  const signed char * normals = renderstate->normaldata;
  const float * elev = renderstate->elevdata;
  const int W = (int) renderstate->blocksize + 1;

  int vertices = 1;
  SbVec3<float> second(0.0f, 0.0f, 0.0f), third(0.0f, 0.0f, 0.0f);

  const SbVec3<float> first((float) (x*renderstate->vspacing[0] + renderstate->voffset[0]),
                            (float) (y*renderstate->vspacing[1] + renderstate->voffset[1]),
                            ELEVATION(x, y));

  GEN_VERTEX(renderstate, x-len, y-len, ELEVATION(x-len, y-len));
  if (!(bitmask & SS_RENDER_BIT_SOUTH)) {
    GEN_VERTEX(renderstate, x, y-len, ELEVATION(x, y-len));
  }
  GEN_VERTEX(renderstate, x+len, y-len, ELEVATION(x+len, y-len));
  if (!(bitmask & SS_RENDER_BIT_EAST)) {
    GEN_VERTEX(renderstate, x+len, y, ELEVATION(x+len, y));
  }
  GEN_VERTEX(renderstate, x+len, y+len, ELEVATION(x+len, y+len));
  if (!(bitmask & SS_RENDER_BIT_NORTH)) {
    GEN_VERTEX(renderstate, x, y+len, ELEVATION(x, y+len));
  }
  GEN_VERTEX(renderstate, x-len, y+len, ELEVATION(x-len, y+len));
  if (!(bitmask & SS_RENDER_BIT_WEST)) {
    GEN_VERTEX(renderstate, x-len, y, ELEVATION(x-len, y));
  }
  GEN_VERTEX(renderstate, x-len, y-len, ELEVATION(x-len, y-len));
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

/*!
*/

void 
sc_undefraypick_cb(void * closure, const int x, const int y,
                    const int len, const unsigned int bitmask_org)
{
  assert(closure);
  RenderState * renderstate = (RenderState *) closure;

  const signed char * normals = renderstate->normaldata;
  const float * elev = renderstate->elevdata;
  const int W = (int) renderstate->blocksize + 1;

  SbVec3<float> first(0.0f, 0.0f, 0.0f), second(0.0f, 0.0f, 0.0f), third(0.0f, 0.0f, 0.0f);

  const signed char * ptr = ss_render_get_undef_array(bitmask_org);
  int numv = *ptr++;
  int tx, ty;

  while ( numv ) {
    int vertices = 0;
    while ( numv ) {
      tx = x + *ptr++ * len;
      ty = y + *ptr++ * len;
      GEN_VERTEX(renderstate, tx, ty, ELEVATION(tx, ty));
      numv--;
    }
    numv = *ptr++;
  }
}

#undef ELEVATION
#undef GEN_VERTEX

/* ********************************************************************** */
