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

#include <GL/gl.h>

#include "../misc/SbList.h"
#include "../misc/SbHash.h"

#include <SmallChange/misc/SceneryGlue.h>
#include <SmallChange/nodes/SceneryGL.h>

/* ********************************************************************** */

// the number of frames a texture can be unused before being recycled
#define MAX_UNUSED_COUNT 200

/* how to interface back to scenery library */
#ifndef SS_SCENERY_H
#define ss_render_get_elevation_measures sc_ssglue_render_get_elevation_measures
#define ss_render_get_texture_measures sc_ssglue_render_get_texture_measures
#define ss_render_get_texture_image sc_ssglue_render_get_texture_image
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

typedef void glMultiTexCoord2f_f(GLint unit, float fx, float fy);

struct sc_GL {
  glMultiTexCoord2f_f * glMultiTexCoord2f;
  GLenum CLAMP_TO_EDGE;
  int use_byte_normals;
};

static struct sc_GL GL = {
  NULL,
  GL_CLAMP,
  TRUE
};

void
sc_set_glMultiTexCoord2f(void * fptr)
{
  GL.glMultiTexCoord2f = (glMultiTexCoord2f_f *) fptr;
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

#if 0
void *
sc_texture_construct(unsigned char * data, int texw, int texh, int nc, int wraps, int wrapt, float q, int hey)
{
  SoGLImage::Wrap WrapS, WrapT;
  switch ( wraps ) {
  case GL_CLAMP:
    WrapS = SoGLImage::CLAMP;
    break;
  case GL_CLAMP_TO_EDGE:
    WrapS = SoGLImage::CLAMP_TO_EDGE;
    break;
  default:
    WrapS = SoGLImage::CLAMP;
    break;
  }
  switch ( wrapt ) {
  case GL_CLAMP:
    WrapT = SoGLImage::CLAMP;
    break;
  case GL_CLAMP_TO_EDGE:
    WrapT = SoGLImage::CLAMP_TO_EDGE;
    break;
  default:
    WrapT = SoGLImage::CLAMP;
    break;
  }
  SoGLImage * image = new SoGLImage;
  image->setFlags(SoGLImage::FORCE_ALPHA_TEST_TRUE|SoGLImage::INVINCIBLE|SoGLImage::USE_QUALITY_VALUE);
  image->setData(data, SbVec2s(texw, texh), nc, WrapS, WrapT, q, hey, NULL);
  return (void *) image;
}

void
sc_texture_activate(RenderState * state, void * imagehandle)
{
  assert(imagehandle);
  SoGLImage * image = (SoGLImage *) imagehandle;
  image->getGLDisplayList(state->state)->call(state->state);
}

void
sc_texture_release(void * imagehandle)
{
  assert(imagehandle);
  SoGLImage * image = (SoGLImage *) imagehandle;
  image->unref(NULL);
}
#endif

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
  state->reusetexlist = (void *) new SbList<TexInfo *>;
  state->tmplist = (void *) new SbList<unsigned int>;
  state->debuglist = (void *) new SbList<float>;
  state->texhash = new SbHash<TexInfo *, unsigned int>(1024, 0.7f);
}

static void sc_texture_hash_clear(unsigned int key, TexInfo * val, void * closure);

void
sc_renderstate_destruct(RenderState * state)
{
  // delete lists
  if ( state->debuglist ) {
    SbList<float> * list = (SbList<float> *) state->debuglist;
    delete list;
    state->debuglist = NULL;
  }
  if ( state->reusetexlist ) {
    SbList<TexInfo *> * list = (SbList<TexInfo *> *) state->reusetexlist;
    if ( list->getLength() ) {
      // FIXME: 
      printf("scenery: reusetexlist not empty\n");
    }
    delete list;
    state->reusetexlist = NULL;
  }
  if ( state->tmplist ) {
    SbList<unsigned int> * list = (SbList<unsigned int> *) state->tmplist;
    delete list;
    state->tmplist = NULL;
  }
  SbHash<TexInfo *, unsigned int> * texhash = (SbHash<TexInfo *, unsigned int> *) state->texhash;
  texhash->apply(sc_texture_hash_clear, NULL);
  delete texhash;
  state->texhash = NULL;
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
  tmplist->truncate(0);

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
  SbList<TexInfo *> * reusetexlist = (SbList<TexInfo *> *) state->reusetexlist;
  tmplist->truncate(0);
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
  // safe to do this here since we'll never use this list again
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

inline void
GL_VERTEX(RenderState * state, const int x, const int y, const float elev)
{
  glTexCoord2f(state->toffset[0] + (float(x)/state->blocksize) * state->tscale[0],
               state->toffset[1] + (float(y)/state->blocksize) * state->tscale[1]);
  
  if (state->etexscale != 0.0f && GL.glMultiTexCoord2f != NULL) {
    float val = (state->etexscale * elev) + state->etexoffset;
    GL.glMultiTexCoord2f(GL_TEXTURE1, 0.0f, val);
  }
  glVertex3f((float) (x*state->vspacing[0] + state->voffset[0]),
             (float) (y*state->vspacing[1] + state->voffset[1]),
             elev);
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
GL_VERTEX_TN(RenderState * state, const int x, const int y, const float elev, const signed char * n)
{
  if ( GL.use_byte_normals ) {
    glNormal3bv((const GLbyte *)n);
  }
  else {
    static const float factor = 1.0f/127.0f;
    glNormal3f(n[0] * factor, n[1] * factor, n[2] * factor);
  }
  glTexCoord2f(state->toffset[0] + (float(x)/state->blocksize) * state->tscale[0],
               state->toffset[1] + (float(y)/state->blocksize) * state->tscale[1]);
  if (state->etexscale != 0.0f && GL.glMultiTexCoord2f != NULL) {
    float val = (state->etexscale * elev) + state->etexoffset;
    GL.glMultiTexCoord2f(GL_TEXTURE1, 0.0f, val);
  }
  glVertex3f((float) (x*state->vspacing[0] + state->voffset[0]),
             (float) (y*state->vspacing[1] + state->voffset[1]),
             elev);
}

/* ********************************************************************** */

int 
sc_render_pre_cb(void * closure, ss_render_pre_cb_info * info)
{
  assert(sc_scenery_available());
  assert(closure);
  RenderState * renderstate = (RenderState *) closure;

  ss_render_get_elevation_measures(info, 
                                   renderstate->voffset,
                                   renderstate->vspacing,
                                   &renderstate->elevdata,
                                   &renderstate->normaldata);

  if ( renderstate->debuglist ) {
    float ox = renderstate->voffset[0] / renderstate->bbmax[0];
    float oy = renderstate->voffset[1] / renderstate->bbmax[1];
    float sx = renderstate->vspacing[0] * renderstate->blocksize;
    float sy = renderstate->vspacing[1] * renderstate->blocksize;

    sx /= renderstate->bbmax[0];
    sy /= renderstate->bbmax[1];

    ((SbList<float> *) renderstate->debuglist)->append(ox);
    ((SbList<float> *) renderstate->debuglist)->append(oy);
    ((SbList<float> *) renderstate->debuglist)->append(ox+sx);
    ((SbList<float> *) renderstate->debuglist)->append(oy+sy);
  }

  ss_render_get_texture_measures(info,
                                 &renderstate->texid,
                                 renderstate->toffset,
                                 renderstate->tscale);
  
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
        imagehandle = texture_construct(renderstate->texdata, renderstate->texw, renderstate->texh, renderstate->texnc, clampmode, clampmode, 0.9, 0);

        imagehandle = sc_create_texture(renderstate, imagehandle);
        assert(imagehandle);

        renderstate->newtexcount++;
      }
      assert(texture_activate);
      texture_activate(renderstate, imagehandle);

      renderstate->currtexid = renderstate->texid;
    }
    else {
      //      fprintf(stderr,"reused tex\n");
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
  return 1;
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

#undef ELEVATION

/* ********************************************************************** */
