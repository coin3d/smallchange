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

#include <stdio.h>

#include <Inventor/misc/SoState.h>
#include <Inventor/bundles/SoTextureCoordinateBundle.h>
#include <Inventor/SoPrimitiveVertex.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoCallbackAction.h>
#include <Inventor/actions/SoRayPickAction.h>
#include <Inventor/nodes/SoVertexProperty.h>
#include <Inventor/actions/SoGetPrimitiveCountAction.h>
#include <Inventor/bundles/SoMaterialBundle.h>
#include <Inventor/details/SoPointDetail.h>
#include <Inventor/details/SoFaceDetail.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoViewVolumeElement.h>
#include <Inventor/elements/SoCacheElement.h>
#include <Inventor/SbBox3f.h>
#include <Inventor/misc/SoState.h>
#include <Inventor/elements/SoCullElement.h>
#include <Inventor/system/gl.h>
#include <Inventor/misc/SoState.h>
#include <Inventor/misc/SoGLImage.h>
#include <Inventor/elements/SoCullElement.h>
#include <Inventor/elements/SoGLTextureImageElement.h>
#include <Inventor/elements/SoGLLazyElement.h>
#include <Inventor/elements/SoGLCacheContextElement.h>
#include <Inventor/elements/SoTextureQualityElement.h>
#include <Inventor/elements/SoViewportRegionElement.h>
#include <Inventor/sensors/SoFieldSensor.h>
#include <Inventor/SoInput.h>
#include <Inventor/lists/SbStringList.h>
#include <Inventor/SoPrimitiveVertex.h>
#include <Inventor/nodes/SoShape.h>

#ifdef __COIN__
#include <Inventor/C/tidbits.h> // coin_getenv()
#endif // __COIN__

#include <SmallChange/misc/SceneryGlue.h>
#include <SmallChange/nodes/SmScenery.h>
/* #include <SmallChange/nodes/SceneryGL.h> */

#define MAX_UNUSED_COUNT 200

// FIXME: implement rayPick() method
// FIXME: not thread safe. Need one view per thread (use thread-local-storage).

#define SS_IMPORT_XYZ 1

/* ********************************************************************** */

typedef struct {
  float blocksize;
  double vspacing[2];
  double voffset[2];
  float tscale[2];
  float toffset[2];
  unsigned int texid;
  float * elevdata;
  signed char * normaldata;
  
  // temporary
  unsigned char * texdata;
  int texw, texh, texnc;
} RenderState;

class TexInfo {
public:
  TexInfo() {
    this->image = NULL;
  }
  unsigned int texid;
  SoGLImage * image;
  int unusedcount;
};

/* ********************************************************************** */

class SceneryP {
public:
  SmScenery * api;

  SoFieldSensor * filenamesensor;
  SoFieldSensor * blocksensor;
  SoFieldSensor * loadsensor;
  SoFieldSensor * colormapsensor;
  SoFieldSensor * colortexturesensor;
  SoFieldSensor * colorelevationsensor;

  SmSceneryTexture2CB * cbtexcb;
  void * cbtexclosure;

  ss_system * system;
  int blocksize;

  double bboxmin[3];
  double bboxmax[3];

  SoPrimitiveVertex * pvertex;

  RenderState renderstate;

  int colormaptexid;
  SbBool firstGLRender;

  // the rest of the data should really be stored in tls
  SoFaceDetail * facedetail;
  SbVec3f currhotspot;
  
  SoAction * curraction;
  SoState * currstate;
  int viewid;

  SoGLImage * dummyimage;

  SbBool dotex;
  SbBool texisenabled;
  unsigned int currtexid;

  SbList <TexInfo *> reusetexlist;
  cc_hash * texhash;
  SbList <unsigned int> tmplist;
  SbList <float> debuglist;
  int numnewtextures;


  SceneryP(void);
  void commonConstructor(void);

  static void filenamesensor_cb(void * data, SoSensor * sensor);
  static void blocksensor_cb(void * data, SoSensor * sensor);
  static void loadsensor_cb(void * data, SoSensor * sensor);
  static void colormapsensor_cb(void * data, SoSensor * sensor);

  void GEN_VERTEX(RenderState * state, const int x, const int y, const float elev);
};

SceneryP::SceneryP(void)
: api(NULL), filenamesensor(NULL), blocksensor(NULL), loadsensor(NULL),
  colormapsensor(NULL), colortexturesensor(NULL), colorelevationsensor(NULL),
  cbtexcb(NULL), cbtexclosure(NULL), system(NULL), blocksize(0),
  pvertex(NULL), colormaptexid(-1), firstGLRender(TRUE),
  facedetail(NULL), currhotspot(0.0f, 0.0f, 0.0f), curraction(NULL),
  currstate(NULL), viewid(-1), dummyimage(NULL), dotex(FALSE), texisenabled(FALSE),
  currtexid(0)
{
  this->bboxmin[0] = 0.0;
  this->bboxmin[1] = 0.0;
  this->bboxmin[2] = 0.0;
  this->bboxmax[0] = 0.0;
  this->bboxmax[1] = 0.0;
  this->bboxmax[2] = 0.0;
}

/* ********************************************************************** */

#define PRIVATE(obj) ((obj)->pimpl)
#define PUBLIC(obj) ((obj)->api)

void
SmScenery::initClass(void)
{
  static SbBool first = TRUE;
  if ( !first ) { return; }
  first = FALSE;
  if ( sc_scenery_available() ) {
    sc_ssglue_initialize();
  }
  SO_NODE_INIT_CLASS(SmScenery, SoShape, "Shape");
}

SO_NODE_SOURCE(SmScenery);

SmScenery *
SmScenery::createInstance(double * origo, double * spacing, int * elements, float * values, float undef)
{
  if ( !sc_scenery_available() ) { return NULL; }
  ss_system * system = sc_ssglue_system_construct(1, origo, spacing, elements, values, undef);
  if ( !system ) { return NULL; }
  return new SmScenery(system);
}

SmScenery::SmScenery(void)
{
  PRIVATE(this) = new SceneryP;
  PRIVATE(this)->api = this;

  SO_NODE_CONSTRUCTOR(SmScenery);
  
  SO_NODE_ADD_FIELD(filename, (""));
  SO_NODE_ADD_FIELD(renderSequence, (-1));
  SO_NODE_ADD_FIELD(blockRottger, (20.0f));
  SO_NODE_ADD_FIELD(loadRottger, (16.0f));
  SO_NODE_ADD_FIELD(visualDebug, (FALSE));

  SO_NODE_ADD_FIELD(colorTexture, (FALSE));
  SO_NODE_ADD_FIELD(colorMap, (0.0f));
  SO_NODE_ADD_FIELD(colorElevation, (0.0f));

  // these fields should start out empty by default
  this->renderSequence.setNum(0);
  this->colorMap.setNum(0);
  this->colorElevation.setNum(0);

  PRIVATE(this)->commonConstructor();

  // when constructing scenery at run-time (other constructor), we don't
  // want the filename field enabled.
  PRIVATE(this)->filenamesensor = new SoFieldSensor(SceneryP::filenamesensor_cb, this);
  PRIVATE(this)->filenamesensor->attach(&this->filename);
  PRIVATE(this)->filenamesensor->setPriority(0);

  // FIXME: view-specific. Move to struct.
  PRIVATE(this)->pvertex = new SoPrimitiveVertex;
  PRIVATE(this)->facedetail = new SoFaceDetail;
  PRIVATE(this)->texhash = cc_hash_construct(1024, 0.7f);
}

SmScenery::SmScenery(ss_system * system)
{
  PRIVATE(this) = new SceneryP;
  PRIVATE(this)->api = this;

  SO_NODE_CONSTRUCTOR(SmScenery);
  
  SO_NODE_ADD_FIELD(filename, (""));
  SO_NODE_ADD_FIELD(renderSequence, (-1));
  SO_NODE_ADD_FIELD(blockRottger, (20.0f));
  SO_NODE_ADD_FIELD(loadRottger, (16.0f));
  SO_NODE_ADD_FIELD(visualDebug, (FALSE));

  SO_NODE_ADD_FIELD(colorTexture, (FALSE));
  SO_NODE_ADD_FIELD(colorMap, (0.0f));
  SO_NODE_ADD_FIELD(colorElevation, (0.0f));

  // these fields should start out empty by default
  this->renderSequence.setNum(0);
  this->colorMap.setNum(0);
  this->colorElevation.setNum(0);

  PRIVATE(this)->commonConstructor();

  PRIVATE(this)->system = system;

}

SmScenery::~SmScenery(void)
{
  delete PRIVATE(this)->filenamesensor;
  delete PRIVATE(this)->blocksensor;
  delete PRIVATE(this)->loadsensor;
  delete PRIVATE(this)->colormapsensor;
  delete PRIVATE(this)->colortexturesensor;
  delete PRIVATE(this)->colorelevationsensor;

  if (sc_scenery_available() && PRIVATE(this)->system) {
    sc_ssglue_view_deallocate(PRIVATE(this)->system, PRIVATE(this)->viewid);
    sc_ssglue_system_close(PRIVATE(this)->system);
  }
  delete PRIVATE(this)->pvertex;
  delete PRIVATE(this)->facedetail;
  cc_hash_apply(PRIVATE(this)->texhash, hash_clear, NULL);
  cc_hash_destruct(PRIVATE(this)->texhash);

  delete PRIVATE(this);
}

/* ********************************************************************** */

static int
pre_block_cb(void * closure, const double * bmin, const double * bmax)
{
  SoState * state = (SoState*) closure;
  state->push();
  if (!SoCullElement::completelyInside(state)) {
    SbBox3f box(bmin[0], bmin[1], bmin[2], bmax[0], bmax[1], bmax[2]);
    if (SoCullElement::cullBox(state, box, TRUE)) {

//       fprintf(stderr,"culled box: %g %g %g, %g %g %g\n",
//               bmin[0], bmin[1], bmin[2],
//               bmax[0], bmax[1], bmax[2]);
      return 0;
    }
  }
  return 1;
}

static void
post_block_cb(void * closure)
{
  SoState * state = (SoState*) closure;  
  state->pop();
}
 

static int
raypick_pre_cb(void * closure, const double * bmin, const double * bmax)
{
  SoRayPickAction * action = (SoRayPickAction*) closure;
  SoState * state = action->getState();
  state->push();

  SbBox3f box(bmin[0], bmin[1], bmin[2], bmax[0], bmax[1], bmax[2]); 
  if (box.isEmpty()) return 0;
  action->setObjectSpace();
  return action->intersect(box, TRUE);
}


static void
raypick_post_cb(void * closure)
{
  SoRayPickAction * action = (SoRayPickAction*) closure;
  SoState * state = action->getState();
  state->pop();
}


int 
SmScenery::render_pre_cb(void * closure, ss_render_pre_cb_info * info)
{
  if ( sc_scenery_available() ) {
  SmScenery * thisp = (SmScenery*) closure; 
  SoState * state = PRIVATE(thisp)->currstate;
  
  RenderState & renderstate = PRIVATE(thisp)->renderstate;
  
  sc_ssglue_render_get_elevation_measures(info, 
                                   renderstate.voffset,
                                   renderstate.vspacing,
                                   &renderstate.elevdata,
                                   &renderstate.normaldata);

  float ox = renderstate.voffset[0] / PRIVATE(thisp)->bboxmax[0];
  float oy = renderstate.voffset[1] / PRIVATE(thisp)->bboxmax[1];
  float sx = renderstate.vspacing[0] * renderstate.blocksize;
  float sy = renderstate.vspacing[1] * renderstate.blocksize;

  sx /= PRIVATE(thisp)->bboxmax[0];
  sy /= PRIVATE(thisp)->bboxmax[1];

  PRIVATE(thisp)->debuglist.append(ox);
  PRIVATE(thisp)->debuglist.append(oy);
  PRIVATE(thisp)->debuglist.append(ox+sx);
  PRIVATE(thisp)->debuglist.append(oy+sy);

  sc_ssglue_render_get_texture_measures(info,
                                 &renderstate.texid,
                                 renderstate.toffset,
                                 renderstate.tscale);
  
  if (PRIVATE(thisp)->dotex && renderstate.texid) {
    if (renderstate.texid != PRIVATE(thisp)->currtexid) {      
      SoGLImage * image = thisp->findReuseTexture(renderstate.texid);
      if (!image) {
        image = thisp->createTexture(renderstate.texid);
        assert(image);      
        sc_ssglue_render_get_texture_image(info, renderstate.texid,
                                    &renderstate.texdata,
                                    &renderstate.texw,
                                    &renderstate.texh,
                                    &renderstate.texnc);
#if 0 // workaround for preng bug in texture
        assert(renderstate.texnc == 4);
        uint32_t * dst = (uint32_t*) renderstate.texdata;
        unsigned char * ptr = renderstate.texdata;
        for (int i = 0; i < renderstate.texw*renderstate.texh; i++) {
          *dst++ = (ptr[3]<<24)|(ptr[2]<<16)|(ptr[1]<<8)|0xff;
        ptr += 4;
        }
#endif
      
        SbVec2s size(renderstate.texw, renderstate.texh);
        image->setData(renderstate.texdata,
                       size, renderstate.texnc, 
                       SoGLImage::CLAMP_TO_EDGE,
                       SoGLImage::CLAMP_TO_EDGE, 0.9, 0, state);
        PRIVATE(thisp)->numnewtextures++;
      }
      image->getGLDisplayList(state)->call(state);
      PRIVATE(thisp)->currtexid = renderstate.texid;
    }
    else {
      //      fprintf(stderr,"reused tex\n");
    }
    if (!PRIVATE(thisp)->texisenabled) {
      glEnable(GL_TEXTURE_2D);
      PRIVATE(thisp)->texisenabled = TRUE;
    }
  }
  else {
    if (PRIVATE(thisp)->texisenabled) {
      glDisable(GL_TEXTURE_2D);
      PRIVATE(thisp)->texisenabled = FALSE;
    }
  }
  return 1;
  } else {
    return 0;
  }
}

void 
SmScenery::GLRender(SoGLRenderAction * action)
{
  if ( sc_scenery_available() ) {
  if (PRIVATE(this)->system == NULL) return;
  if (!this->shouldGLRender(action)) return;

  //  sc_ssglue_view_set_evaluate_rottger_parameters(PRIVATE(this)->system, this->viewid, 16.0f, 400.0f);

  const int sequencelen = this->renderSequence.getNum();
  if ( this->colorTexture.getValue() && PRIVATE(this)->colormaptexid != -1 ) {
    // FIXME: add runtime colortexture
    int localsequence[2] = { PRIVATE(this)->colormaptexid, 0 };
    sc_ssglue_view_set_render_sequence_a(PRIVATE(this)->system, PRIVATE(this)->viewid, 2, localsequence);
  } else if ( sequencelen == 0 ) {
    sc_ssglue_view_set_render_sequence_a(PRIVATE(this)->system, PRIVATE(this)->viewid, 0, NULL);
  } else {
    this->renderSequence.enableNotify(FALSE);
    int * sequence = this->renderSequence.startEditing();
    sc_ssglue_view_set_render_sequence_a(PRIVATE(this)->system, PRIVATE(this)->viewid, sequencelen, sequence);
    this->renderSequence.finishEditing();
    this->renderSequence.enableNotify(TRUE);
  }

  if ( PRIVATE(this)->firstGLRender ) {
    // FIXME: this should not really be necessary, and should be
    // considered a work-around for a bug in the scenery SDK. 20031015 mortene.
    if ( this->colorTexture.getValue() && (PRIVATE(this)->colormaptexid != -1) ) {
      this->refreshTextures(PRIVATE(this)->colormaptexid);
    }
    PRIVATE(this)->firstGLRender = FALSE;
  }

  SoState * state = action->getState();
  SbVec2s vpsize = SoViewportRegionElement::get(state).getViewportSizePixels();
  state->push();
  
  float quality = SoTextureQualityElement::get(state);
  PRIVATE(this)->dotex = quality > 0.0f;
  SbVec3f campos = SoViewVolumeElement::get(state).getProjectionPoint();

  //  transform into local coordinate system
  SbMatrix worldtolocal =
    SoModelMatrixElement::get(state).inverse();
  worldtolocal.multVecMatrix(campos, campos);

  SoCacheElement::invalidate(state);
  SoMaterialBundle mb(action);
  mb.sendFirst();

  SbBool texwasenabled = glIsEnabled(GL_TEXTURE_2D);
  PRIVATE(this)->texisenabled = texwasenabled;
  PRIVATE(this)->currtexid = 0;

  PRIVATE(this)->currhotspot = campos;
  PRIVATE(this)->curraction = action;
  PRIVATE(this)->currstate = state;

  sc_ssglue_view_set_render_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                              render_cb, this);
  sc_ssglue_view_set_undef_render_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                    undefrender_cb, this); 
  sc_ssglue_view_set_render_pre_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                  render_pre_cb, this);
  sc_ssglue_view_set_culling_pre_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                   pre_block_cb, state);
  sc_ssglue_view_set_culling_post_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                    post_block_cb, state);
  double hotspot[3];
  hotspot[0] = campos[0];
  hotspot[1] = campos[1];
  hotspot[2] = campos[2];

  sc_ssglue_view_set_hotspots(PRIVATE(this)->system, PRIVATE(this)->viewid, 1, hotspot); 
  PRIVATE(this)->debuglist.truncate(0);
  PRIVATE(this)->numnewtextures = 0;

   sc_ssglue_view_render(PRIVATE(this)->system, PRIVATE(this)->viewid);
//   fprintf(stderr,"num boxes: %d, new texs: %d\n",
//           this->debuglist.getLength()/4, this->numnewtextures);
  
  SbBool texisenabled = glIsEnabled(GL_TEXTURE_2D);
  if (texisenabled != texwasenabled) {
    if (texwasenabled) glEnable(GL_TEXTURE_2D);
    else glDisable(GL_TEXTURE_2D);
  } 
  SoGLLazyElement::getInstance(state)->reset(state, SoLazyElement::GLIMAGE_MASK);

  state->pop();
  sc_ssglue_view_set_culling_pre_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                   NULL, NULL);
  sc_ssglue_view_set_culling_post_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                    NULL, NULL);

  if (this->visualDebug.getValue()) {
    campos[0] /= PRIVATE(this)->bboxmax[0];
    campos[1] /= PRIVATE(this)->bboxmax[1];
    
    campos[0] -= 0.5f;
    campos[1] -= 0.5f;
    
    state->push();
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(-0.5f, 0.5f, -0.5f, 0.5f, -1.0f, 1.0f);
    glDepthMask(GL_FALSE);
    glColor3f(1.0f, 0.0f, 0.0f);
    
    int num = PRIVATE(this)->debuglist.getLength() / 4;
    int i;
    
    float mind = 1.0f;
    
    for (i = 0; i < num; i++) {
      float x0, x1;
      x0 = PRIVATE(this)->debuglist[i*4];
      x1 = PRIVATE(this)->debuglist[i*4+2];
      
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
      x0 = PRIVATE(this)->debuglist[i*4] - 0.5f;
      y0 = PRIVATE(this)->debuglist[i*4+1] - 0.5f;
      x1 = PRIVATE(this)->debuglist[i*4+2] - 0.5f;
      y1 = PRIVATE(this)->debuglist[i*4+3] - 0.5f;
      
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
    
    glDepthMask(GL_TRUE);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    state->pop();
    SoGLLazyElement::getInstance(state)->reset(state, SoLazyElement::DIFFUSE_MASK);
  }
  }
}

void 
SmScenery::rayPick(SoRayPickAction * action)
{
  if ( sc_scenery_available() ) {
  sc_ssglue_view_set_culling_pre_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                   raypick_pre_cb, action);
  sc_ssglue_view_set_culling_post_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                    raypick_post_cb, action);

  inherited::rayPick(action); // just generate primitives
  
  sc_ssglue_view_set_culling_pre_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                   NULL, NULL);
  sc_ssglue_view_set_culling_post_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                    NULL, NULL);
  }
}

void 
SmScenery::callback(SoCallbackAction * action)
{
  if ( !sc_scenery_available() ) { return; }

  // FIXME: not correct to just evaluate in the callback()
  // method. SoCallbackAction can be executed for a a number of
  // reasons. Consider creating a SoSimlaEvaluateAction or something...
  if (PRIVATE(this)->system == NULL) return;
  SoState * state = action->getState();

  SbVec3f campos = SoViewVolumeElement::get(state).getProjectionPoint();
  //  transform into local coordinate system
  SbMatrix worldtolocal =
    SoModelMatrixElement::get(state).inverse();
  worldtolocal.multVecMatrix(campos, campos);
  PRIVATE(this)->currhotspot = campos;

  sc_ssglue_view_set_culling_pre_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                     pre_block_cb, state);
  sc_ssglue_view_set_culling_post_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                      post_block_cb, state);
  sc_ssglue_view_set_render_pre_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                  NULL, this);
  double hotspot[3];
  hotspot[0] = campos[0];
  hotspot[1] = campos[1];
  hotspot[2] = campos[2];
  sc_ssglue_view_set_hotspots(PRIVATE(this)->system, PRIVATE(this)->viewid, 1, hotspot); 
  sc_ssglue_view_evaluate(PRIVATE(this)->system, PRIVATE(this)->viewid);

  sc_ssglue_view_set_culling_pre_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                   NULL, NULL);
  sc_ssglue_view_set_culling_post_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                    NULL, NULL);
}

void 
SmScenery::generatePrimitives(SoAction * action)
{
  if ( sc_scenery_available() ) {
  if (PRIVATE(this)->system == NULL) return;
  SoState * state = action->getState();

  sc_ssglue_view_set_render_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                              gen_cb, this);
  sc_ssglue_view_set_undef_render_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                    undefgen_cb, this);

  sc_ssglue_view_set_render_pre_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                  gen_pre_cb, this);

  SoPointDetail pointDetail;
  PRIVATE(this)->pvertex->setDetail(&pointDetail);
  PRIVATE(this)->curraction = action;
  sc_ssglue_view_render(PRIVATE(this)->system, PRIVATE(this)->viewid);
  }
}

void 
SmScenery::computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center)
{
  if (PRIVATE(this)->system == NULL) return;  
  box = SbBox3f(PRIVATE(this)->bboxmin[0], PRIVATE(this)->bboxmin[1], PRIVATE(this)->bboxmin[2],
                PRIVATE(this)->bboxmax[0], PRIVATE(this)->bboxmax[1], PRIVATE(this)->bboxmax[2]);
  center = box.getCenter();
}

static
ss_system *
readxyz(const char * filename)
{
  FILE * fp = fopen(filename, "rb");
  if ( !fp ) return NULL;
  int points = 0;
  int loop = TRUE;
  while ( loop ) {
    float fx, fy, fz;
    if ( fscanf(fp, "%f %f %f", &fx, &fy, &fz) == 3 ) {
      points++;
    } else {
      if ( !feof(fp) ) points = 0; // something fishy
      fclose(fp);
      loop = FALSE;
    }
  }
  if ( points < 2 ) return NULL;
  fp = fopen(filename, "rb");
  if ( !fp ) return NULL;
  float * values = (float *) malloc(points * 3 * sizeof(float));
  assert(values);
  loop = TRUE;
  points = 0;
  while ( loop ) {
    float fx, fy, fz;
    if ( fscanf(fp, "%f %f %f", &fx, &fy, &fz) == 3 ) {
      values[points*3+0] = fx;
      values[points*3+1] = fy;
      values[points*3+2] = fz;
      points++;
    } else {
      if ( !feof(fp) ) points = 0; // something fishy
      fclose(fp);
      loop = FALSE;
    }
  }
  if ( points < 2 ) {
    free(values);
    return NULL;
  }
  int dimension[2] = { 0, 0 };
  float deltax[2] = { 0.0f, 0.0f };
  float deltay[2] = { 0.0f, 0.0f };
  float * elevations = (float *) malloc(points * sizeof(float));
  assert(elevations);
  assert(points > 2);
  deltax[0] = values[3] - values[0];
  deltax[1] = values[4] - values[1];

  double origo[2] = { values[0], values[1] };

  int i;
  for ( i = 0; i < points; i++ ) {
    elevations[i] = values[3*i+2];
    if ( deltay[0] == 0.0f && deltay[1] == 0.0f && i < (points - 1) ) {
      float delta[2] = { values[3*(i+1)] - values[3*i], values[3*(i+1)+1] - values[3*i+1] };
      float dot = deltax[0] * delta[0] + deltax[1] * delta[1];
      if ( dot < 0.0f ) {
        deltay[0] = values[3*(i+1)] - values[0];
        deltay[1] = values[3*(i+1)+1] - values[1];
        dimension[0] = i + 1;
        dimension[1] = points / dimension[0];
      }
    }
  }
  free(values);
  double spacing[2] = {
    sqrt(deltax[0] * deltax[0] + deltax[1] * deltax[1]),
    sqrt(deltay[0] * deltay[0] + deltay[1] * deltay[1])
  };
  assert(spacing[0] > 0.0);
  assert(spacing[1] > 0.0);

  fprintf(stderr, "xyz import - cols: %d rows: %d (%d points - %d)\n", dimension[0], dimension[1], points, (dimension[0]*dimension[1]));
  ss_system * system = sc_ssglue_system_construct(1, origo, spacing, dimension, elevations, 999999.0f);
  free(elevations);
  return system;
}

uint32_t
SmScenery::colortexgen_cb(void * closure, double * pos, float elevation, double * spacing)
{
  uint32_t abgr = 0xffffffff; // no colors means white
  SmScenery * thisp = (SmScenery *) closure;
  if ( thisp->colorElevation.getNum() == 0 ) {
    // interpolate color table evenly over elevation range
    float fac = (elevation - PRIVATE(thisp)->bboxmin[2]) / (PRIVATE(thisp)->bboxmax[2] - PRIVATE(thisp)->bboxmin[2]);
    int steps = ((thisp->colorMap.getNum() / 4) - 1); // four components
    if ( steps == 0 ) { // only one color given
      int nr = (int) (SbClamp(thisp->colorMap[0], 0.0f, 1.0f) * 255.0);
      int ng = (int) (SbClamp(thisp->colorMap[1], 0.0f, 1.0f) * 255.0);
      int nb = (int) (SbClamp(thisp->colorMap[2], 0.0f, 1.0f) * 255.0);
      int na = (int) (SbClamp(thisp->colorMap[3], 0.0f, 1.0f) * 255.0);
      abgr = (na << 24) | (nb << 16) | (ng << 8) | nr;
    } else if ( steps > 0 ) { // interpolate color
      int startcolidx = (int) floor(float(steps) * fac);
      float rest = (float(steps) * fac) - float(startcolidx);
      float r = thisp->colorMap[startcolidx * 4 + 0];
      float g = thisp->colorMap[startcolidx * 4 + 1];
      float b = thisp->colorMap[startcolidx * 4 + 2];
      float a = thisp->colorMap[startcolidx * 4 + 3];
      if ( rest > 0.0 ) {
        assert(rest < 1.0);
        float end_r = thisp->colorMap[startcolidx * 4 + 4];
        float end_g = thisp->colorMap[startcolidx * 4 + 5];
        float end_b = thisp->colorMap[startcolidx * 4 + 6];
        float end_a = thisp->colorMap[startcolidx * 4 + 7];
        r = r * (1.0 - rest) + end_r * rest;
        g = g * (1.0 - rest) + end_g * rest;
        b = b * (1.0 - rest) + end_b * rest;
        a = a * (1.0 - rest) + end_a * rest;
      }
      int nr = (int) (SbClamp(r, 0.0f, 1.0f) * 255.0);
      int ng = (int) (SbClamp(g, 0.0f, 1.0f) * 255.0);
      int nb = (int) (SbClamp(b, 0.0f, 1.0f) * 255.0);
      int na = (int) (SbClamp(a, 0.0f, 1.0f) * 255.0);
      abgr = (na << 24) | (nb << 16) | (ng << 8) | nr;
    }
  } else {
    // use elevation values to decide colors
    // FIXME: implement
  }
  return abgr;
}

void
SmScenery::colormaptexchange(void)
{
  // if ( this->colorTexture.getValue() ) {
    if ( PRIVATE(this)->colormaptexid != -1 ) {
      this->refreshTextures(PRIVATE(this)->colormaptexid);
    }
  // }
}

void
SmScenery::setAttributeTextureCB(SmSceneryTexture2CB * callback, void * closure)
{
  PRIVATE(this)->cbtexcb = callback;
  PRIVATE(this)->cbtexclosure = closure;
  // FIXME: invalidate texture if attribute texture is currently enabled
}

void 
SmScenery::preFrame(void)
{
  if ( sc_scenery_available() && PRIVATE(this)->system ) {
    sc_ssglue_view_pre_frame(PRIVATE(this)->system, PRIVATE(this)->viewid);
    cc_hash_apply(PRIVATE(this)->texhash, hash_inc_unused, NULL);
  }
}

int 
SmScenery::postFrame(void)
{
  if (sc_scenery_available() && PRIVATE(this)->system) {
    this->deleteUnusedTextures();
    return sc_ssglue_view_post_frame(PRIVATE(this)->system, PRIVATE(this)->viewid);
  }
  return 0;
}

void 
SmScenery::setBlockRottger(const float c)
{
  if (sc_scenery_available() && PRIVATE(this)->system) {
    sc_ssglue_view_set_evaluate_rottger_parameters(PRIVATE(this)->system, PRIVATE(this)->viewid, 16.0f, c);
  }
}

float
SmScenery::getBlockRottger(void) const
{
  if (sc_scenery_available() && PRIVATE(this)->system) {
    float C, c;
    sc_ssglue_view_get_evaluate_rottger_parameters(PRIVATE(this)->system, PRIVATE(this)->viewid, &C, &c);
    return c;
  }
  return 0.0f;
}

void 
SmScenery::setLoadRottger(const float c)
{
  if (sc_scenery_available() && PRIVATE(this)->system) {
    sc_ssglue_view_set_load_rottger_parameters(PRIVATE(this)->system, PRIVATE(this)->viewid, 16.0f, c);
  }
}

float
SmScenery::getLoadRottger(void) const
{
  if (sc_scenery_available() && PRIVATE(this)->system) {
    float C, c;
    sc_ssglue_view_get_load_rottger_parameters(PRIVATE(this)->system, PRIVATE(this)->viewid, &C, &c);
    return c;
  }
  return 0.0f;
}

void
SmScenery::getSceneryOffset(double * offset) const
{
  if (sc_scenery_available() && PRIVATE(this)->system) {
    sc_ssglue_system_get_origo_world_position(PRIVATE(this)->system, offset);
  }
}

void 
SmScenery::refreshTextures(const int id)
{
  if (sc_scenery_available() && PRIVATE(this)->system) {
    sc_ssglue_system_refresh_runtime_texture2d(PRIVATE(this)->system, id);

    PRIVATE(this)->tmplist.truncate(0);
    cc_hash_apply(PRIVATE(this)->texhash, hash_add_all, &PRIVATE(this)->tmplist);

    for (int i = 0; i < PRIVATE(this)->tmplist.getLength(); i++) {
      void * tmp;
      if (cc_hash_get(PRIVATE(this)->texhash, PRIVATE(this)->tmplist[i], &tmp)) {
        TexInfo * tex = (TexInfo *) tmp;
        PRIVATE(this)->reusetexlist.push(tex);
        (void) cc_hash_remove(PRIVATE(this)->texhash, PRIVATE(this)->tmplist[i]);
      }
      else {
        assert(0 && "huh");
      }
    }
  }
}


int 
SmScenery::gen_pre_cb(void * closure, ss_render_pre_cb_info * info)
{
  SmScenery * thisp = (SmScenery*) closure; 

  RenderState & renderstate = PRIVATE(thisp)->renderstate;
  
  sc_ssglue_render_get_elevation_measures(info, 
                                   renderstate.voffset,
                                   renderstate.vspacing,
                                   &renderstate.elevdata,
                                   &renderstate.normaldata);
  return 1;
}

////////////// render ///////////////////////////////////////////////////////////

inline void 
GL_VERTEX_OLD(RenderState * state, const int x, const int y, const float elev)
{
  glVertex3f((float) (x*state->vspacing[0] + state->voffset[0]),
             (float) (y*state->vspacing[1] + state->voffset[1]),
             elev);
}

inline void 
GL_VERTEX(RenderState * state, const int x, const int y, const float elev)
{
  glTexCoord2f(state->toffset[0] + (float(x)/state->blocksize) * state->tscale[0],
               state->toffset[1] + (float(y)/state->blocksize) * state->tscale[1]);
  glVertex3f((float) (x*state->vspacing[0] + state->voffset[0]),
             (float)(y*state->vspacing[1] + state->voffset[1]),
             elev);
}

static inline const char *
sm_getenv(const char * s)
{
#ifdef __COIN__
  return coin_getenv(s);
#else // other Inventor
  return getenv(s);
#endif
}

// We provide the environment variable below to work around a bug in
// the 3Dlabs OpenGL driver version 4.10.01.2105-2.16.0866: it doesn't
// properly handle normals given in byte values (i.e. through
// glNormal*b*()).
//
// This driver is fairly old, and 3Dlabs is more or less defunct now,
// so we default to the faster (but bug-triggering) GL call.
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
GL_VERTEX_N(RenderState * state, const int x, const int y, const float elev, const signed char * n)
{
  if (ok_to_use_bytevalue_normals()) {
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
  if (ok_to_use_bytevalue_normals()) {
    glNormal3bv((const GLbyte *)n);
  }
  else {
    static const float factor = 1.0f/127.0f;
    glNormal3f(n[0] * factor, n[1] * factor, n[2] * factor);
  }

  glTexCoord2f(state->toffset[0] + (float(x)/state->blocksize) * state->tscale[0],
               state->toffset[1] + (float(y)/state->blocksize) * state->tscale[1]);
  glVertex3f((float) (x*state->vspacing[0] + state->voffset[0]),
             (float) (y*state->vspacing[1] + state->voffset[1]),
             elev);
}


#define ELEVATION(x,y) elev[(y)*W+(x)]    


void 
SmScenery::undefrender_cb(void * closure, const int x, const int y, const int len, 
                           const unsigned int bitmask_org)
{
  SmScenery * thisp = (SmScenery*) closure; 

  RenderState * renderstate = &PRIVATE(thisp)->renderstate;
  const signed char * normals = renderstate->normaldata;
  const float * elev = renderstate->elevdata;
  const int W = PRIVATE(thisp)->blocksize;

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
SmScenery::render_cb(void * closure, const int x, const int y,
                      const int len, const unsigned int bitmask)
{
  SmScenery * thisp = (SmScenery*) closure;
  
  RenderState * renderstate = &PRIVATE(thisp)->renderstate;

  const signed char * normals = renderstate->normaldata;  
  const float * elev = renderstate->elevdata;
  const int W = PRIVATE(thisp)->blocksize;

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


//////////// generate primitives ///////////////////////////////////////////////

void 
SceneryP::GEN_VERTEX(RenderState * state, const int x, const int y, const float elev)
{
  this->pvertex->setPoint(SbVec3f(x*state->vspacing[0] + state->voffset[0],
                                  y*state->vspacing[1] + state->voffset[1],
                                  elev));
  PUBLIC(this)->shapeVertex(this->pvertex);
}

void 
SmScenery::undefgen_cb(void * closure, const int x, const int y, const int len, 
                     const unsigned int bitmask_org)
{
  SmScenery * thisp = (SmScenery*) closure; 

  RenderState * renderstate = &PRIVATE(thisp)->renderstate;
  const signed char * normals = renderstate->normaldata;
  const float * elev = renderstate->elevdata;
  const int W = PRIVATE(thisp)->blocksize;

#define ELEVATION(x,y) elev[(y)*W+(x)]

  const signed char * ptr = sc_ssglue_render_get_undef_array(bitmask_org);

  int numv = *ptr++;
  int tx, ty;

  if (normals) {
    int idx;
#define SEND_VERTEX(state, x, y) \
    idx = (y)*W + x; \
    PRIVATE(thisp)->GEN_VERTEX(state, x, y, elev[idx]/*, normals+3*idx*/);

    while (numv) {
      thisp->beginShape(PRIVATE(thisp)->curraction, TRIANGLE_FAN, PRIVATE(thisp)->facedetail);
      while (numv) { 
        tx = x + *ptr++ * len;
        ty = y + *ptr++ * len;
        SEND_VERTEX(renderstate, tx, ty);
        numv--;
      }
      numv = *ptr++;
      thisp->endShape();
    }
#undef SEND_VERTEX
  }
  else {    
    while (numv) {
      thisp->beginShape(PRIVATE(thisp)->curraction, TRIANGLE_FAN, PRIVATE(thisp)->facedetail);
      while (numv) { 
        tx = x + *ptr++ * len;
        ty = y + *ptr++ * len;
        PRIVATE(thisp)->GEN_VERTEX(renderstate, tx, ty, ELEVATION(tx, ty));
        numv--;
      }
      numv = *ptr++;
      thisp->endShape();
    }
  }
}

void 
SmScenery::gen_cb(void * closure, const int x, const int y,
                   const int len, const unsigned int bitmask)
{
  SmScenery * thisp = (SmScenery*) closure;

  RenderState * renderstate = &PRIVATE(thisp)->renderstate;
  const signed char * normals = renderstate->normaldata;
  const float * elev = renderstate->elevdata;
  const int W = PRIVATE(thisp)->blocksize;

#define ELEVATION(x,y) elev[(y)*W+(x)]
  
  thisp->beginShape(PRIVATE(thisp)->curraction, TRIANGLE_FAN, PRIVATE(thisp)->facedetail);
  PRIVATE(thisp)->GEN_VERTEX(renderstate, x, y, ELEVATION(x, y));
  PRIVATE(thisp)->GEN_VERTEX(renderstate, x-len, y-len, ELEVATION(x-len, y-len));
  if (!(bitmask & SS_RENDER_BIT_SOUTH)) {
    PRIVATE(thisp)->GEN_VERTEX(renderstate, x, y-len, ELEVATION(x, y-len));
  }
  PRIVATE(thisp)->GEN_VERTEX(renderstate, x+len, y-len, ELEVATION(x+len, y-len));
  if (!(bitmask & SS_RENDER_BIT_EAST)) {
    PRIVATE(thisp)->GEN_VERTEX(renderstate, x+len, y, ELEVATION(x+len, y));
  }
  PRIVATE(thisp)->GEN_VERTEX(renderstate, x+len, y+len, ELEVATION(x+len, y+len));
  if (!(bitmask & SS_RENDER_BIT_NORTH)) {
    PRIVATE(thisp)->GEN_VERTEX(renderstate, x, y+len, ELEVATION(x, y+len));
  }
  PRIVATE(thisp)->GEN_VERTEX(renderstate, x-len, y+len, ELEVATION(x-len, y+len));
  if (!(bitmask & SS_RENDER_BIT_WEST)) {
    PRIVATE(thisp)->GEN_VERTEX(renderstate, x-len, y, ELEVATION(x-len, y));
  }
  PRIVATE(thisp)->GEN_VERTEX(renderstate, x-len, y-len, ELEVATION(x-len, y-len));
  thisp->endShape();
  
#undef ELEVATION
}

SoGLImage * 
SmScenery::findReuseTexture(const unsigned int texid)
{
  void * tmp;
  if (cc_hash_get(PRIVATE(this)->texhash, texid, &tmp)) {
    TexInfo * tex = (TexInfo*) tmp;
    assert(tex->image);
    tex->unusedcount = 0;
    return tex->image;
  }
  return NULL;
}

void 
SmScenery::deleteUnusedTextures(void)
{
  PRIVATE(this)->tmplist.truncate(0);
  cc_hash_apply(PRIVATE(this)->texhash, hash_check_unused, &PRIVATE(this)->tmplist);

  for (int i = 0; i < PRIVATE(this)->tmplist.getLength(); i++) {
    void * tmp;
    if (cc_hash_get(PRIVATE(this)->texhash, PRIVATE(this)->tmplist[i], &tmp)) {
      TexInfo * tex = (TexInfo*) tmp;
      PRIVATE(this)->reusetexlist.push(tex);
      (void) cc_hash_remove(PRIVATE(this)->texhash, PRIVATE(this)->tmplist[i]);
    }
    else {
      assert(0 && "huh");
    }
  }

//   fprintf(stderr,"SmScenery now has %d active textures, %d reusable textures (removed %d)\n",
//           cc_hash_get_num_elements(this->texhash), this->reusetexlist.getLength(), this->tmplist.getLength());

  PRIVATE(this)->tmplist.truncate(0);
}

SoGLImage * 
SmScenery::createTexture(const unsigned int texid)
{
  TexInfo * tex = NULL;
  
  if (PRIVATE(this)->reusetexlist.getLength()) {
    tex = PRIVATE(this)->reusetexlist.pop();
  }
  else {
    tex = new TexInfo;
    tex->image = new SoGLImage;
    tex->image->setFlags(SoGLImage::FORCE_ALPHA_TEST_TRUE|SoGLImage::INVINCIBLE|SoGLImage::USE_QUALITY_VALUE);
  }
  tex->texid = PRIVATE(this)->currtexid;
  tex->unusedcount = 0;

  (void) cc_hash_put(PRIVATE(this)->texhash, texid, tex);
  return tex->image;
}  

void 
SmScenery::hash_clear(unsigned long key, void * val, void * closure)
{
  TexInfo * tex = (TexInfo*) val;
  // safe to do this here since we'll never use this list again
  assert(tex->image);
  tex->image->unref(NULL);
  delete tex;
}

void 
SmScenery::hash_check_unused(unsigned long key, void * val, void * closure)
{  
  TexInfo * tex = (TexInfo*) val;
  if (tex->unusedcount > MAX_UNUSED_COUNT) {
    SbList <unsigned int> * keylist = (SbList <unsigned int> *) closure;
    keylist->append(key);
  }
}

void 
SmScenery::hash_add_all(unsigned long key, void * val, void * closure)
{  
  TexInfo * tex = (TexInfo*) val;
  SbList <unsigned int> * keylist = (SbList <unsigned int> *) closure;
  keylist->append(key);
}

void 
SmScenery::hash_inc_unused(unsigned long key, void * val, void * closure)
{
  TexInfo * tex = (TexInfo*) val;
  tex->unusedcount++;
}

/* ********************************************************************** */

void
SceneryP::commonConstructor(void)
{
  this->blocksensor = new SoFieldSensor(SceneryP::blocksensor_cb, PUBLIC(this));
  this->blocksensor->attach(&PUBLIC(this)->blockRottger);

  this->loadsensor = new SoFieldSensor(SceneryP::loadsensor_cb, PUBLIC(this));
  this->loadsensor->attach(&PUBLIC(this)->loadRottger);

  this->colormapsensor = new SoFieldSensor(SceneryP::colormapsensor_cb, PUBLIC(this));
  this->colormapsensor->attach(&PUBLIC(this)->colorMap);

  this->colortexturesensor = new SoFieldSensor(SceneryP::colormapsensor_cb, PUBLIC(this));
  this->colortexturesensor->attach(&PUBLIC(this)->colorTexture);

  this->colorelevationsensor = new SoFieldSensor(SceneryP::colormapsensor_cb, PUBLIC(this));
  this->colorelevationsensor->attach(&PUBLIC(this)->colorElevation);
}

void 
SceneryP::filenamesensor_cb(void * data, SoSensor * sensor)
{
  if ( !sc_scenery_available() ) { return; }
  SmScenery * thisp = (SmScenery *) data;

  if ( sc_scenery_available() && PRIVATE(thisp)->system) {
    sc_ssglue_view_deallocate(PRIVATE(thisp)->system, PRIVATE(thisp)->viewid);
    sc_ssglue_system_close(PRIVATE(thisp)->system);
  }

  const SbStringList & pathlist = SoInput::getDirectories();

  PRIVATE(thisp)->viewid = -1;
  PRIVATE(thisp)->system = NULL;
  PRIVATE(thisp)->colormaptexid = -1;


  SbString s = thisp->filename.getValue();
  if (s.getLength()) {
#if 0 && SS_IMPORT_XYZ
    if ( s.find(".xyz") == (s.getLength() - 4) ) {
      PRIVATE(thisp)->system = readxyz(s.getString());
    }
#endif
    if ( !PRIVATE(thisp)->system ) {
      int i;
      for ( i = -1; (PRIVATE(thisp)->system == NULL) && (i < pathlist.getLength()); i++ ) {
        if ( i == -1 ) {
          PRIVATE(thisp)->system = sc_ssglue_system_open(s.getString(), 1);
        } else {
          SbString path = *(pathlist[i]);
          path += "/";
          path += s;
          PRIVATE(thisp)->system = sc_ssglue_system_open(path.getString(), 1);
        }
      }
    }
    if (!PRIVATE(thisp)->system) {
      (void)fprintf(stderr, "Unable to open SmScenery system '%s'\n", s.getString());
    }
    else {
      if ( (sc_ssglue_system_get_num_datasets(PRIVATE(thisp)->system) > 0) &&
           (sc_ssglue_system_get_dataset_type(PRIVATE(thisp)->system, 0) == SS_ELEVATION_TYPE) ) {
        PRIVATE(thisp)->colormaptexid = sc_ssglue_system_add_runtime_texture2d(PRIVATE(thisp)->system, 0, SmScenery::colortexgen_cb, thisp);
      }
      sc_ssglue_system_get_object_box(PRIVATE(thisp)->system, PRIVATE(thisp)->bboxmin, PRIVATE(thisp)->bboxmax); 
      PRIVATE(thisp)->blocksize = sc_ssglue_system_get_blocksize(PRIVATE(thisp)->system);
      PRIVATE(thisp)->renderstate.blocksize = (float) (PRIVATE(thisp)->blocksize-1);
      PRIVATE(thisp)->viewid = sc_ssglue_view_allocate(PRIVATE(thisp)->system);
      assert(PRIVATE(thisp)->viewid >= 0);
      sc_ssglue_view_enable(PRIVATE(thisp)->system, PRIVATE(thisp)->viewid);
      //      fprintf(stderr,"system: %p, viewid: %d\n", PRIVATE(thisp)->system, thisp->viewid);

      const int sequencelen = thisp->renderSequence.getNum();
      if ( thisp->colorTexture.getValue() && PRIVATE(thisp)->colormaptexid != -1 ) {
        // FIXME: add runtime colortexture
        int localsequence[2] = { PRIVATE(thisp)->colormaptexid, 0 };
        sc_ssglue_view_set_render_sequence_a(PRIVATE(thisp)->system, PRIVATE(thisp)->viewid, 2, localsequence);
      } else if ( sequencelen == 0 ) {
        sc_ssglue_view_set_render_sequence_a(PRIVATE(thisp)->system, PRIVATE(thisp)->viewid, 0, NULL);
      } else {
        thisp->renderSequence.enableNotify(FALSE);
        int * sequence = thisp->renderSequence.startEditing();
        sc_ssglue_view_set_render_sequence_a(PRIVATE(thisp)->system, PRIVATE(thisp)->viewid, sequencelen, sequence);
        thisp->renderSequence.finishEditing();
        thisp->renderSequence.enableNotify(TRUE);
      }
    }
  }
}

void 
SceneryP::blocksensor_cb(void * data, SoSensor * sensor)
{
  SmScenery * thisp = (SmScenery *) data;
  thisp->setBlockRottger(thisp->blockRottger.getValue());
}

void 
SceneryP::loadsensor_cb(void * data, SoSensor * sensor)
{
  SmScenery * thisp = (SmScenery *) data;
  thisp->setLoadRottger(thisp->loadRottger.getValue());
}

void 
SceneryP::colormapsensor_cb(void * data, SoSensor * sensor)
{
  SmScenery * thisp = (SmScenery *) data;
  thisp->colormaptexchange();
}

/* ********************************************************************** */
