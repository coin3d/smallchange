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
#include <stdio.h>

#include <Inventor/errors/SoDebugError.h>
#include <Inventor/misc/SoState.h>
#include <Inventor/bundles/SoTextureCoordinateBundle.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoCallbackAction.h>
#include <Inventor/actions/SoRayPickAction.h>
#include <Inventor/actions/SoGetPrimitiveCountAction.h>
#include <Inventor/SoPrimitiveVertex.h>
#include <Inventor/nodes/SoVertexProperty.h>
#include <Inventor/nodes/SoShape.h>
#include <Inventor/bundles/SoMaterialBundle.h>
#include <Inventor/details/SoPointDetail.h>
#include <Inventor/details/SoFaceDetail.h>
#include <Inventor/misc/SoGLImage.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoViewVolumeElement.h>
#include <Inventor/elements/SoCacheElement.h>
#include <Inventor/elements/SoCullElement.h>
#include <Inventor/elements/SoGLTextureImageElement.h>
#include <Inventor/elements/SoGLLazyElement.h>
#include <Inventor/elements/SoGLCacheContextElement.h>
#include <Inventor/elements/SoTextureQualityElement.h>
#include <Inventor/elements/SoViewportRegionElement.h>
#include <Inventor/sensors/SoFieldSensor.h>
#include <Inventor/lists/SbStringList.h>

#include <Inventor/SoInput.h>

#include <Inventor/misc/SoGLImage.h>
#include <Inventor/C/glue/gl.h>

#include <SmallChange/misc/SceneryGlue.h>
#include <SmallChange/nodes/SmScenery.h>
#include <SmallChange/nodes/SceneryGL.h>

#define MAX_UNUSED_COUNT 200

// FIXME: implement rayPick() method
// (is this old, or does it still count for undef-blocks? 20031019 larsa)

// FIXME: not thread safe. Need one view per thread (use thread-local-storage).

#define SS_IMPORT_XYZ 0

/*
  FIXME NOTE
  When using a discrete color texture, the bilinear texture filtering causes
  unwanted artifacts between colors.  It should be turned off.
*/


/* ********************************************************************** */

/*!
  \class SmScenery SmScenery.h SmallChange/nodes/SmScenery.h
  \brief The SmScenery class is a Coin node interface to the SIM Scenery
  library.

*/

/*!
  \var SmSFString SmScenery::filename
  \brief The filename for a SIM Scenery database.
*/

/*!
  \var SmSFFloat SmScenery::blockRottger
  \brief The rottger factor for tessellating the surface of a terrain block.

  If the terrain is textured with a light-based texture, you can keep this
  fairly low (16-32) and still have excellent visual results.  If you on
  the other hand depend on normals for shading the terrain, you may need to
  use a high value (64->)
*/

/*!
  \var SoSFFloat SmScenery::loadRottger
  \brief The rottger formula constant for deciding the LOD level of blocks
  as you remove yourself from the focus points.

  Don't use a high value for this parameter unless you know what you are
  doing.
*/

/*!
  \var SoMFInt32 SmScenery::renderSequence
  \brief The description of how datasets in the SIM Scenery database should be
  merged before being rendered.

  Not really supported yet, but you can currently control which one terrain
  dataset will be rendered with which one texture dataset in the SIM Scnery
  database with it, which is kind of a subset of the functionality we aim to
  implement with this field.
*/

/*!
  \var SoSFEnum SmScenery::colorTexturing
  \brief Decides whether or not the values of colorMap and colorElevation
  (optional) should be used to generate a texture for the terrain.

  The value DISABLED (default) is for not rendering with a color texture.
  The value INTERPOLATED is for rendering with a texture that has
  interpolated colors.  The value DISCRETE will give you uniform
  colors with discrete, hard edges between the color changes.
*/

/*!
  \var SoMFFloat SmScenery::colorMap
  \brief A table of colors to use for coloring the terrain, starting with the
  color of the lowest elevations, going upwards.

  The colorization values should range from 0.0 to 1.0, and are given in
  the red, green, blue, and alpha component order.  The alpha component is
  currently ignored.  To avoid surprises if it is ever turned on in the
  future, you should always use 1.0 for the alpha component.

  Going outside the 0.0 - 1.0 range is possible, and won't lead to anything
  other than that values will be used as is during color interpolation, but
  clamped to a value within the 0.0 - 1.0 range before being used to colorize
  the texture.  You can probably abuse this information to achieve some effect
  or other, although we won't guarantee this behaviour to stay unchanged.
*/

/*!
  \var SoMFFloat SmScenery::colorElevation
  \brief A table of elevation values, which will decide where the colors
  given in colorMap will kick in.

  If empty, colors will be interpolated evenly over the full range of the
  terrain.

  If used, the number of values must correspond to the number of colors in
  the colorMap field, or coloration will be ignored alltogether.
  For TRUE / INTERPOLATED, the number of elevation values will have to
  be equal to the number of colors - elevation values outside the given
  range of elevation values will just use the colosest color value.
  For DISCRETE, the number of elevation values has to be one less than the
  number of colors.
*/

/*!
  \var SoSFBool SmScenery::elevationLines
  \brief Whether or not the elevation lines feature should be used.
*/

/*!
  \var SoSFFloat SmScenery::elevationLineDistance
  \brief The elevation distance between elevation lines.

  The math is not correct for this feature yet.
*/

/*!
  \var SoSFFloat SmScenery::elevationLineOffset
  \brief Offset the elevation lines with this value.

  The math is not correct for this feature yet.
*/

/*!
  \var SoSFShort SmScenery::elevationLineEmphasis
  \brief Emphasize every Nth elevation line.

  Not implemented yet.
*/

/*!
  \var SoSFBool SmScenery::visualDebug
  \brief Show information designed for debugging purposes on the viewport
*/

/* ********************************************************************** */

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
  SoFieldSensor * old_colortexturesensor;
  SoFieldSensor * colorelevationsensor;
  SoFieldSensor * elevationtexsensor;
  SoFieldSensor * elevationdistsensor;
  SoFieldSensor * elevationoffsensor;

  SmSceneryTexture2CB * cbtexcb;
  void * cbtexclosure;

  ss_system * system;
  int blocksize;

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
  SoGLImage * elevationlinesimage;

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

  void colormaptexchange(void);
  void elevationlinestexchange(void);

  static void filenamesensor_cb(void * closure, SoSensor * sensor);
  static void blocksensor_cb(void * closure, SoSensor * sensor);
  static void loadsensor_cb(void * closure, SoSensor * sensor);
  static void colortexsensor_cb(void * closure, SoSensor * sensor);
  static void old_colortexturesensor_cb(void * closure, SoSensor * sensor);
  static void elevationlinessensor_cb(void * closure, SoSensor * sensor);

  // texture caching
  SoGLImage * findReuseTexture(const unsigned int texid);
  SoGLImage * createTexture(const unsigned int texid);
  void deleteUnusedTextures(void);
  static void hash_clear(unsigned long key, void * val, void * closure);
  static void hash_inc_unused(unsigned long key, void * val, void * closure);
  static void hash_add_all(unsigned long key, void * val, void * closure);
  static void hash_check_unused(unsigned long key, void * val, void * closure);

  // rendering
  static int render_pre_cb(void * closure, ss_render_pre_cb_info * info);

  // generate primitives / raypick
  void GEN_VERTEX(RenderState * state, const int x, const int y, const float elev);
  static int gen_pre_cb(void * closure, ss_render_pre_cb_info * info);
  static void gen_cb(void * closure, const int x, const int y,
                     const int len, const unsigned int bitmask);
  static void undefgen_cb(void * closure, const int x, const int y, const int len,
                          const unsigned int bitmask_org);

  // callbacks
  static uint32_t invokecolortexturecb(void * closure, double * pos, float elevation, double * spacing);
};

SceneryP::SceneryP(void)
: api(NULL), filenamesensor(NULL), blocksensor(NULL), loadsensor(NULL),
  colormapsensor(NULL), colortexturesensor(NULL), colorelevationsensor(NULL),
  elevationtexsensor(NULL), elevationdistsensor(NULL), elevationoffsensor(NULL),
  cbtexcb(NULL), cbtexclosure(NULL),
  system(NULL), blocksize(0), pvertex(NULL), colormaptexid(-1), firstGLRender(TRUE),
  facedetail(NULL), currhotspot(0.0f, 0.0f, 0.0f), curraction(NULL),
  currstate(NULL), viewid(-1), dummyimage(NULL),
  elevationlinesimage(NULL),
  dotex(FALSE), texisenabled(FALSE),
  currtexid(0)
{
  this->renderstate.bbmin[0] = 0.0;
  this->renderstate.bbmin[1] = 0.0;
  this->renderstate.bbmin[2] = 0.0;
  this->renderstate.bbmax[0] = 0.0;
  this->renderstate.bbmax[1] = 0.0;
  this->renderstate.bbmax[2] = 0.0;
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
  SO_NODE_ADD_FIELD(blockRottger, (20.0f));
  SO_NODE_ADD_FIELD(loadRottger, (16.0f));

  SO_NODE_ADD_FIELD(renderSequence, (-1));
  this->renderSequence.setNum(0);
  this->renderSequence.setDefault(TRUE);

  SO_NODE_ADD_FIELD(colorTexturing, (SmScenery::DISABLED));
  SO_NODE_DEFINE_ENUM_VALUE(ColorTexturing, DISABLED);
  SO_NODE_DEFINE_ENUM_VALUE(ColorTexturing, INTERPOLATED);
  SO_NODE_DEFINE_ENUM_VALUE(ColorTexturing, DISCRETE);
  SO_NODE_SET_SF_ENUM_TYPE(colorTexturing, ColorTexturing);

  SO_NODE_ADD_FIELD(colorMap, (0.0f));
  this->colorMap.setNum(0);
  this->colorMap.setDefault(TRUE);
  SO_NODE_ADD_FIELD(colorElevation, (0.0f));
  this->colorElevation.setNum(0);
  this->colorElevation.setDefault(TRUE);

  SO_NODE_ADD_FIELD(elevationLines, (FALSE));
  SO_NODE_ADD_FIELD(elevationLineDistance, (100.0f));
  SO_NODE_ADD_FIELD(elevationLineOffset, (0.0f));
  SO_NODE_ADD_FIELD(elevationLineEmphasis, (0));

  SO_NODE_ADD_FIELD(visualDebug, (FALSE));

  // old compat field
  SO_NODE_ADD_FIELD(colorTexture, (FALSE));

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
  SO_NODE_ADD_FIELD(blockRottger, (20.0f));
  SO_NODE_ADD_FIELD(loadRottger, (16.0f));
  SO_NODE_ADD_FIELD(renderSequence, (-1));
  this->renderSequence.setNum(0);
  this->renderSequence.setDefault(TRUE);

  SO_NODE_ADD_FIELD(colorTexturing, (SmScenery::DISABLED));
  SO_NODE_DEFINE_ENUM_VALUE(ColorTexturing, DISABLED);
  SO_NODE_DEFINE_ENUM_VALUE(ColorTexturing, INTERPOLATED);
  SO_NODE_DEFINE_ENUM_VALUE(ColorTexturing, DISCRETE);
  SO_NODE_SET_SF_ENUM_TYPE(colorTexturing, ColorTexturing);
  SO_NODE_ADD_FIELD(colorMap, (0.0f));
  this->colorMap.setNum(0);
  this->colorMap.setDefault(TRUE);
  SO_NODE_ADD_FIELD(colorElevation, (0.0f));
  this->colorElevation.setNum(0);
  this->colorElevation.setDefault(TRUE);

  SO_NODE_ADD_FIELD(elevationLines, (FALSE));
  SO_NODE_ADD_FIELD(elevationLineDistance, (100.0f));
  SO_NODE_ADD_FIELD(elevationLineOffset, (0.0f));
  SO_NODE_ADD_FIELD(elevationLineEmphasis, (0));

  SO_NODE_ADD_FIELD(visualDebug, (FALSE));

  // old compat field
  SO_NODE_ADD_FIELD(colorTexture, (FALSE));

  PRIVATE(this)->commonConstructor();

  // FIXME: view-specific. Move to struct.
  PRIVATE(this)->pvertex = new SoPrimitiveVertex;
  PRIVATE(this)->facedetail = new SoFaceDetail;
  PRIVATE(this)->texhash = cc_hash_construct(1024, 0.7f);

  PRIVATE(this)->system = system;
}

void
SceneryP::commonConstructor(void)
{
  this->blocksensor = new SoFieldSensor(SceneryP::blocksensor_cb, PUBLIC(this));
  this->blocksensor->attach(&PUBLIC(this)->blockRottger);

  this->loadsensor = new SoFieldSensor(SceneryP::loadsensor_cb, PUBLIC(this));
  this->loadsensor->attach(&PUBLIC(this)->loadRottger);

  this->colortexturesensor = new SoFieldSensor(SceneryP::colortexsensor_cb, PUBLIC(this));
  this->colortexturesensor->attach(&PUBLIC(this)->colorTexturing);

  this->colormapsensor = new SoFieldSensor(SceneryP::colortexsensor_cb, PUBLIC(this));
  this->colormapsensor->attach(&PUBLIC(this)->colorMap);

  this->colorelevationsensor = new SoFieldSensor(SceneryP::colortexsensor_cb, PUBLIC(this));
  this->colorelevationsensor->attach(&PUBLIC(this)->colorElevation);

  this->old_colortexturesensor = new SoFieldSensor(SceneryP::old_colortexturesensor_cb, PUBLIC(this));
  this->old_colortexturesensor->attach(&PUBLIC(this)->colorTexture);

  this->elevationtexsensor = new SoFieldSensor(SceneryP::elevationlinessensor_cb, PUBLIC(this));
  this->elevationtexsensor->attach(&PUBLIC(this)->elevationLines);

  this->elevationdistsensor = new SoFieldSensor(SceneryP::elevationlinessensor_cb, PUBLIC(this));
  this->elevationdistsensor->attach(&PUBLIC(this)->elevationLineDistance);

  this->elevationoffsensor = new SoFieldSensor(SceneryP::elevationlinessensor_cb, PUBLIC(this));
  this->elevationoffsensor->attach(&PUBLIC(this)->elevationLineOffset);

  // elevation texture test
  this->renderstate.etexstretch = 0.0f;
  this->renderstate.etexoffset = 0.0f;

  this->cbtexcb = SmScenery::colortexture_cb;
  this->cbtexclosure = PUBLIC(this);
}

SmScenery::~SmScenery(void)
{
  delete PRIVATE(this)->filenamesensor;
  delete PRIVATE(this)->blocksensor;
  delete PRIVATE(this)->loadsensor;
  delete PRIVATE(this)->colormapsensor;
  delete PRIVATE(this)->colortexturesensor;
  delete PRIVATE(this)->colorelevationsensor;
  delete PRIVATE(this)->elevationtexsensor;
  delete PRIVATE(this)->elevationdistsensor;
  delete PRIVATE(this)->elevationoffsensor;

  if (sc_scenery_available() && PRIVATE(this)->system) {
    sc_ssglue_view_deallocate(PRIVATE(this)->system, PRIVATE(this)->viewid);
    sc_ssglue_system_close(PRIVATE(this)->system);
  }
  delete PRIVATE(this)->pvertex;
  delete PRIVATE(this)->facedetail;
  cc_hash_apply(PRIVATE(this)->texhash, SceneryP::hash_clear, NULL);
  cc_hash_destruct(PRIVATE(this)->texhash);
  if ( PRIVATE(this)->elevationlinesimage ) {
    PRIVATE(this)->elevationlinesimage->unref();
    PRIVATE(this)->elevationlinesimage = NULL;
  }
  delete PRIVATE(this);
}

/* ********************************************************************** */

void 
SmScenery::GLRender(SoGLRenderAction * action)
{
  if ( !sc_scenery_available() ) { return; }
  if ( PRIVATE(this)->system == NULL ) { return; }
  if ( !this->shouldGLRender(action) ) { return; }

  // rendersequence start
  const int sequencelen = this->renderSequence.getNum();
  if ( (this->colorTexturing.getValue() != SmScenery::DISABLED) && PRIVATE(this)->colormaptexid != -1 ) {
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
  // rendersequenceend

  if ( PRIVATE(this)->firstGLRender ) {
    // FIXME: this should not really be necessary, and should be
    // considered a work-around for a bug in the scenery SDK. 20031015 mortene.
    if ( (this->colorTexturing.getValue() != SmScenery::DISABLED) && (PRIVATE(this)->colormaptexid != -1) ) {
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

  SbBool texwasenabled = sc_is_texturing_enabled();

  PRIVATE(this)->texisenabled = texwasenabled;
  PRIVATE(this)->currtexid = 0;

  PRIVATE(this)->currhotspot = campos;
  PRIVATE(this)->curraction = action;
  PRIVATE(this)->currstate = state;

  // set up culling suitable for rendering
  sc_ssglue_view_set_culling_pre_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                          SmScenery::box_culling_pre_cb, action);
  sc_ssglue_view_set_culling_post_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                           SmScenery::box_culling_post_cb, action);

  // callback to initialize for each block
  sc_ssglue_view_set_render_pre_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                         SceneryP::render_pre_cb, this);

  // set up rendering callbacks
  sc_ssglue_view_set_render_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                     sc_render_cb, &PRIVATE(this)->renderstate);
  sc_ssglue_view_set_undef_render_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                           sc_undefrender_cb, &PRIVATE(this)->renderstate); 

  double hotspot[3];
  hotspot[0] = campos[0];
  hotspot[1] = campos[1];
  hotspot[2] = campos[2];

  sc_ssglue_view_set_hotspots(PRIVATE(this)->system, PRIVATE(this)->viewid, 1, hotspot); 

  PRIVATE(this)->debuglist.truncate(0);
  PRIVATE(this)->numnewtextures = 0;

  int context = SoGLCacheContextElement::get(state);
  const cc_glglue * gl = cc_glglue_instance(context);
  assert(gl);
  sc_set_glglue_instance(gl);

  if ( (PRIVATE(this)->renderstate.etexstretch != 0.0f) &&
       (PRIVATE(this)->elevationlinesimage != NULL) ) {
    cc_glglue_glActiveTexture(gl, GL_TEXTURE1);
    glEnable(GL_TEXTURE_2D);
    PRIVATE(this)->elevationlinesimage->getGLDisplayList(state)->call(state);
    cc_glglue_glActiveTexture(gl, GL_TEXTURE0);
    PRIVATE(this)->renderstate.etexoffset = this->elevationLineOffset.getValue() / 100.0f;
  }

  sc_ssglue_view_render(PRIVATE(this)->system, PRIVATE(this)->viewid);

  if ( PRIVATE(this)->renderstate.etexstretch != 0.0f ) {
    cc_glglue_glActiveTexture(gl, GL_TEXTURE1);
    SoGLLazyElement::getInstance(state)->reset(state, SoLazyElement::GLIMAGE_MASK);
    glDisable(GL_TEXTURE_2D);
    cc_glglue_glActiveTexture(gl, GL_TEXTURE0);
  }

//  fprintf(stderr,"num boxes: %d, new texs: %d\n",
//          this->debuglist.getLength()/4, this->numnewtextures);
  
  // texturing state could be changed during scenery rendering
  SbBool texisenabled = sc_is_texturing_enabled();
  if ( texisenabled != texwasenabled ) {
    if ( texwasenabled ) sc_enable_texturing();
    else sc_disable_texturing();
  } 
  SoGLLazyElement::getInstance(state)->reset(state, SoLazyElement::GLIMAGE_MASK);

  state->pop();

  sc_ssglue_view_set_culling_pre_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                          NULL, NULL);
  sc_ssglue_view_set_culling_post_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                           NULL, NULL);

  if (this->visualDebug.getValue()) {
    campos[0] = (campos[0] / PRIVATE(this)->renderstate.bbmax[0]) - 0.5f;
    campos[1] = (campos[1] / PRIVATE(this)->renderstate.bbmax[1]) - 0.5f;
    state->push();
    sc_display_debug_info(&campos[0], &vpsize[0], &PRIVATE(this)->debuglist);
    state->pop();
    SoGLLazyElement::getInstance(state)->reset(state, SoLazyElement::DIFFUSE_MASK);
  }
}

void 
SmScenery::rayPick(SoRayPickAction * action)
{
  if ( !sc_scenery_available() ) { return; }
  sc_ssglue_view_set_culling_pre_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                          SmScenery::ray_culling_pre_cb, action);
  sc_ssglue_view_set_culling_post_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                           SmScenery::ray_culling_post_cb, action);

  inherited::rayPick(action); // just generate primitives
  
  sc_ssglue_view_set_culling_pre_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                          NULL, NULL);
  sc_ssglue_view_set_culling_post_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                           NULL, NULL);
}

void 
SmScenery::callback(SoCallbackAction * action)
{
  if ( !sc_scenery_available() ) { return; }

  // rendersequence start
  const int sequencelen = this->renderSequence.getNum();
  if ( (this->colorTexturing.getValue() != SmScenery::DISABLED) && PRIVATE(this)->colormaptexid != -1 ) {
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
  // rendersequenceend

  if ( PRIVATE(this)->firstGLRender ) {
    // FIXME: this should not really be necessary, and should be
    // considered a work-around for a bug in the scenery SDK. 20031015 mortene.
    if ( (this->colorTexturing.getValue() != SmScenery::DISABLED) && (PRIVATE(this)->colormaptexid != -1) ) {
      this->refreshTextures(PRIVATE(this)->colormaptexid);
    }
    PRIVATE(this)->firstGLRender = FALSE;
  }

  // FIXME: not correct to just evaluate in the callback()
  // method. SoCallbackAction can be executed for a a number of
  // reasons. Consider creating a SoSimlaEvaluateAction or something...
  if (PRIVATE(this)->system == NULL) return;
  SoState * state = action->getState();

  SbVec3f campos = SoViewVolumeElement::get(state).getProjectionPoint();
  //  transform into local coordinate system
  SbMatrix worldtolocal = SoModelMatrixElement::get(state).inverse();
  worldtolocal.multVecMatrix(campos, campos);
  PRIVATE(this)->currhotspot = campos;

  sc_ssglue_view_set_culling_pre_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                          SmScenery::box_culling_pre_cb, action);
  sc_ssglue_view_set_culling_post_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                           SmScenery::box_culling_post_cb, action);
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
  if ( !sc_scenery_available() ) { return; }
  if (PRIVATE(this)->system == NULL) return;
  SoState * state = action->getState();

  sc_ssglue_view_set_render_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                     SceneryP::gen_cb, this);
  sc_ssglue_view_set_undef_render_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                           SceneryP::undefgen_cb, this);

  sc_ssglue_view_set_render_pre_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                         SceneryP::gen_pre_cb, this);

  SoPointDetail pointDetail;
  PRIVATE(this)->pvertex->setDetail(&pointDetail);
  PRIVATE(this)->curraction = action;
  sc_ssglue_view_render(PRIVATE(this)->system, PRIVATE(this)->viewid);
}

void 
SmScenery::computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center)
{
  if ( PRIVATE(this)->system == NULL ) return;  
  const RenderState & rs = PRIVATE(this)->renderstate;
  box.setBounds(rs.bbmin[0], rs.bbmin[1], rs.bbmin[2],
                rs.bbmax[0], rs.bbmax[1], rs.bbmax[2]);
  center = box.getCenter();
}

#if SS_IMPORT_XYZ

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

#endif // SS_IMPORT_XYZ

void
SmScenery::set2DColorationTextureCB(SmSceneryTexture2CB * callback, void * closure)
{
  PRIVATE(this)->cbtexcb = callback;
  PRIVATE(this)->cbtexclosure = closure;
  // FIXME: invalidate texture if attribute texture is currently enabled
}

void 
SmScenery::preFrame(void)
{
  if ( !sc_scenery_available() || !PRIVATE(this)->system ) { return; }
  sc_ssglue_view_pre_frame(PRIVATE(this)->system, PRIVATE(this)->viewid);
  cc_hash_apply(PRIVATE(this)->texhash, SceneryP::hash_inc_unused, NULL);
}

int 
SmScenery::postFrame(void)
{
  if ( !sc_scenery_available() || !PRIVATE(this)->system ) { return 0; }
  PRIVATE(this)->deleteUnusedTextures();
  return sc_ssglue_view_post_frame(PRIVATE(this)->system, PRIVATE(this)->viewid);
}

void 
SmScenery::setBlockRottger(const float c)
{
  if ( !sc_scenery_available() || !PRIVATE(this)->system ) { return; }
  sc_ssglue_view_set_evaluate_rottger_parameters(PRIVATE(this)->system, PRIVATE(this)->viewid, 16.0f, c);
}

float
SmScenery::getBlockRottger(void) const
{
  if ( !sc_scenery_available() || !PRIVATE(this)->system ) { return 0.0f; }
  float C, c;
  sc_ssglue_view_get_evaluate_rottger_parameters(PRIVATE(this)->system, PRIVATE(this)->viewid, &C, &c);
  return c;
}

void 
SmScenery::setLoadRottger(const float c)
{
  if ( !sc_scenery_available() || !PRIVATE(this)->system ) { return; }
  sc_ssglue_view_set_load_rottger_parameters(PRIVATE(this)->system, PRIVATE(this)->viewid, 16.0f, c);
}

float
SmScenery::getLoadRottger(void) const
{
  if ( !sc_scenery_available() || !PRIVATE(this)->system ) { return 0.0f; }
  float C, c;
  sc_ssglue_view_get_load_rottger_parameters(PRIVATE(this)->system, PRIVATE(this)->viewid, &C, &c);
  return c;
}

void 
SmScenery::refreshTextures(const int id)
{
  if ( sc_scenery_available() || !PRIVATE(this)->system ) { return; }

  sc_ssglue_system_refresh_runtime_texture2d(PRIVATE(this)->system, id);

  PRIVATE(this)->tmplist.truncate(0);
  cc_hash_apply(PRIVATE(this)->texhash, SceneryP::hash_add_all, &PRIVATE(this)->tmplist);

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

// *************************************************************************

void 
SceneryP::filenamesensor_cb(void * closure, SoSensor * sensor)
{
  assert(closure);
  if ( !sc_scenery_available() ) { return; }

  SmScenery * thisp = (SmScenery *) closure;

  if ( PRIVATE(thisp)->system ) {
    sc_ssglue_view_deallocate(PRIVATE(thisp)->system, PRIVATE(thisp)->viewid);
    sc_ssglue_system_close(PRIVATE(thisp)->system);
  }

  PRIVATE(thisp)->viewid = -1;
  PRIVATE(thisp)->system = NULL;
  PRIVATE(thisp)->colormaptexid = -1;

  const SbStringList & pathlist = SoInput::getDirectories();
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
        PRIVATE(thisp)->colormaptexid = sc_ssglue_system_add_runtime_texture2d(PRIVATE(thisp)->system, 0, SceneryP::invokecolortexturecb, PRIVATE(thisp));
      }
      sc_ssglue_system_get_object_box(PRIVATE(thisp)->system, PRIVATE(thisp)->renderstate.bbmin, PRIVATE(thisp)->renderstate.bbmax); 
      PRIVATE(thisp)->blocksize = sc_ssglue_system_get_blocksize(PRIVATE(thisp)->system);
      PRIVATE(thisp)->renderstate.blocksize = (float) (PRIVATE(thisp)->blocksize-1);
      PRIVATE(thisp)->viewid = sc_ssglue_view_allocate(PRIVATE(thisp)->system);
      assert(PRIVATE(thisp)->viewid >= 0);
      sc_ssglue_view_enable(PRIVATE(thisp)->system, PRIVATE(thisp)->viewid);
      //      fprintf(stderr,"system: %p, viewid: %d\n", PRIVATE(thisp)->system, thisp->viewid);

      const int sequencelen = thisp->renderSequence.getNum();
      if ( (thisp->colorTexturing.getValue() != SmScenery::DISABLED) && PRIVATE(thisp)->colormaptexid != -1 ) {
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
SceneryP::blocksensor_cb(void * closure, SoSensor * sensor)
{
  assert(closure);
  SmScenery * thisp = (SmScenery *) closure;
  thisp->setBlockRottger(thisp->blockRottger.getValue());
}

void 
SceneryP::loadsensor_cb(void * closure, SoSensor * sensor)
{
  assert(closure);
  SmScenery * thisp = (SmScenery *) closure;
  thisp->setLoadRottger(thisp->loadRottger.getValue());
}

void 
SceneryP::colortexsensor_cb(void * closure, SoSensor * sensor)
{
  assert(closure);
  SmScenery * thisp = (SmScenery *) closure;
  PRIVATE(thisp)->colormaptexchange();
}

void 
SceneryP::old_colortexturesensor_cb(void * closure, SoSensor * sensor)
{
  assert(closure);
  SmScenery * thisp = (SmScenery *) closure;
  if ( thisp->colorTexture.getValue() ) {
    thisp->colorTexturing.setValue(SmScenery::INTERPOLATED);
  }
  else {
    thisp->colorTexturing.setValue(SmScenery::DISABLED);
  }
}

void
SceneryP::elevationlinessensor_cb(void * closure, SoSensor * sensor)
{
  assert(closure);
  SmScenery * thisp = (SmScenery *) closure;
  if ( (thisp->elevationLines.getValue() != FALSE) &&
       (thisp->elevationLineDistance.getValue() > 0.0f) ) {
    float fac = (0.01f / 1024.0f) * thisp->elevationLineDistance.getValue();
    PRIVATE(thisp)->renderstate.etexstretch = fac;
    PRIVATE(thisp)->elevationlinestexchange();
  } else {
    PRIVATE(thisp)->renderstate.etexstretch = 0.0f;
  }
}

// *************************************************************************

void
SceneryP::colormaptexchange(void)
{
  if ( this->colormaptexid != -1 ) {
    PUBLIC(this)->refreshTextures(this->colormaptexid);
  }
}

void
SceneryP::elevationlinestexchange(void)
{
  if ( this->elevationlinesimage == NULL ) {
    this->elevationlinesimage = new SoGLImage;
    // this->elevationlinesimage->ref(); ???  unref() but no ref()?
  }
  assert(this->elevationlinesimage);
#define ELTS 1024
#define COMP 4
#define R 0
#define G 1
#define B 2
#define A 3
  uint8_t * bytes = (uint8_t *) malloc(ELTS * COMP);
  int i;
  for ( i = 0; i < ELTS; i++ ) {
    bytes[i*COMP+R] = 255;
    bytes[i*COMP+G] = 255;
    bytes[i*COMP+B] = 255;
    bytes[i*COMP+A] = 255;
  }
  for ( i = 0; i < ELTS; i++ ) {
    if ( ((i % 64) == 0) || (((i+1) % 64) == 0) ) {
      bytes[i*COMP+R] = 0;
      bytes[i*COMP+G] = 0;
      bytes[i*COMP+B] = 0;
      bytes[i*COMP+A] = 255;
    }
  }
  this->elevationlinesimage->setData(bytes, SbVec2s(1, ELTS), COMP);
  // free(bytes);
}

// *************************************************************************

void 
SceneryP::GEN_VERTEX(RenderState * state, const int x, const int y, const float elev)
{
  this->pvertex->setPoint(SbVec3f(x*state->vspacing[0] + state->voffset[0],
                                  y*state->vspacing[1] + state->voffset[1],
                                  elev));
  PUBLIC(this)->shapeVertex(this->pvertex);
}

SoGLImage * 
SceneryP::findReuseTexture(const unsigned int texid)
{
  void * tmp = NULL;
  if (cc_hash_get(this->texhash, texid, &tmp)) {
    TexInfo * tex = (TexInfo *) tmp;
    assert(tex->image);
    tex->unusedcount = 0;
    return tex->image;
  }
  return NULL;
}

SoGLImage * 
SceneryP::createTexture(const unsigned int texid)
{
  TexInfo * tex = NULL;
  if (this->reusetexlist.getLength()) {
    tex = this->reusetexlist.pop();
  }
  else {
    tex = new TexInfo;
    tex->image = new SoGLImage;
    tex->image->setFlags(SoGLImage::FORCE_ALPHA_TEST_TRUE|SoGLImage::INVINCIBLE|SoGLImage::USE_QUALITY_VALUE);
  }
  tex->texid = this->currtexid;
  tex->unusedcount = 0;

  (void) cc_hash_put(this->texhash, texid, tex);
  return tex->image;
}  

void 
SceneryP::deleteUnusedTextures(void)
{
  this->tmplist.truncate(0);
  cc_hash_apply(this->texhash, SceneryP::hash_check_unused, &this->tmplist);

  int i;
  for ( i = 0; i < this->tmplist.getLength(); i++ ) {
    void * tmp;
    if ( cc_hash_get(this->texhash, this->tmplist[i], &tmp) ) {
      TexInfo * tex = (TexInfo *) tmp;
      this->reusetexlist.push(tex);
      (void) cc_hash_remove(this->texhash, this->tmplist[i]);
    }
    else {
      assert(0 && "huh");
    }
  }

//   fprintf(stderr,"SmScenery now has %d active textures, %d reusable textures (removed %d)\n",
//           cc_hash_get_num_elements(this->texhash), this->reusetexlist.getLength(), this->tmplist.getLength());

  this->tmplist.truncate(0);
}

void 
SceneryP::hash_clear(unsigned long key, void * val, void * closure)
{
  TexInfo * tex = (TexInfo *) val;
  // safe to do this here since we'll never use this list again
  assert(tex->image);
  tex->image->unref(NULL);
  delete tex;
}

void 
SceneryP::hash_add_all(unsigned long key, void * val, void * closure)
{  
  TexInfo * tex = (TexInfo *) val;
  SbList <unsigned int> * keylist = (SbList <unsigned int> *) closure;
  keylist->append(key);
}

void 
SceneryP::hash_inc_unused(unsigned long key, void * val, void * closure)
{
  TexInfo * tex = (TexInfo *) val;
  tex->unusedcount++;
}

void 
SceneryP::hash_check_unused(unsigned long key, void * val, void * closure)
{  
  TexInfo * tex = (TexInfo*) val;
  if ( tex->unusedcount > MAX_UNUSED_COUNT ) {
    SbList <unsigned int> * keylist = (SbList <unsigned int> *) closure;
    keylist->append(key);
  }
}

// *************************************************************************
// RENDER

int 
SceneryP::render_pre_cb(void * closure, ss_render_pre_cb_info * info)
{
  if ( !sc_scenery_available() ) { return 0; }

  SmScenery * thisp = (SmScenery *) closure; 

  SoState * state = PRIVATE(thisp)->currstate;
  
  RenderState & renderstate = PRIVATE(thisp)->renderstate;
  
  sc_ssglue_render_get_elevation_measures(info, 
                                          renderstate.voffset,
                                          renderstate.vspacing,
                                          &renderstate.elevdata,
                                          &renderstate.normaldata);

  float ox = renderstate.voffset[0] / PRIVATE(thisp)->renderstate.bbmax[0];
  float oy = renderstate.voffset[1] / PRIVATE(thisp)->renderstate.bbmax[1];
  float sx = renderstate.vspacing[0] * renderstate.blocksize;
  float sy = renderstate.vspacing[1] * renderstate.blocksize;

  sx /= PRIVATE(thisp)->renderstate.bbmax[0];
  sy /= PRIVATE(thisp)->renderstate.bbmax[1];

  PRIVATE(thisp)->debuglist.append(ox);
  PRIVATE(thisp)->debuglist.append(oy);
  PRIVATE(thisp)->debuglist.append(ox+sx);
  PRIVATE(thisp)->debuglist.append(oy+sy);

  sc_ssglue_render_get_texture_measures(info,
                                        &renderstate.texid,
                                        renderstate.toffset,
                                        renderstate.tscale);
  
  if ( PRIVATE(thisp)->dotex && renderstate.texid ) {
    if ( renderstate.texid != PRIVATE(thisp)->currtexid ) {
      SoGLImage * image = PRIVATE(thisp)->findReuseTexture(renderstate.texid);
      if ( !image ) {
        image = PRIVATE(thisp)->createTexture(renderstate.texid);
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
        for ( int i = 0; i < renderstate.texw*renderstate.texh; i++ ) {
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
    if ( !PRIVATE(thisp)->texisenabled ) {
      sc_enable_texturing();
      PRIVATE(thisp)->texisenabled = TRUE;
    }
  }
  else {
    if ( PRIVATE(thisp)->texisenabled ) {
      sc_disable_texturing();
      PRIVATE(thisp)->texisenabled = FALSE;
    }
  }
  return 1;
}

// *************************************************************************
// GENERATE PRIMITIVES

int 
SceneryP::gen_pre_cb(void * closure, ss_render_pre_cb_info * info)
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

void 
SceneryP::undefgen_cb(void * closure, const int x, const int y, const int len, 
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
      thisp->beginShape(PRIVATE(thisp)->curraction, SoShape::TRIANGLE_FAN, PRIVATE(thisp)->facedetail);
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
      thisp->beginShape(PRIVATE(thisp)->curraction, SoShape::TRIANGLE_FAN, PRIVATE(thisp)->facedetail);
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
SceneryP::gen_cb(void * closure, const int x, const int y,
                 const int len, const unsigned int bitmask)
{
  SmScenery * thisp = (SmScenery*) closure;

  RenderState * renderstate = &PRIVATE(thisp)->renderstate;
  const signed char * normals = renderstate->normaldata;
  const float * elev = renderstate->elevdata;
  const int W = PRIVATE(thisp)->blocksize;

#define ELEVATION(x,y) elev[(y)*W+(x)]
  
  thisp->beginShape(PRIVATE(thisp)->curraction, SoShape::TRIANGLE_FAN, PRIVATE(thisp)->facedetail);
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

/* ********************************************************************** */
// CULLING

// view volume box vs terrain block bounding box culling
int
SmScenery::box_culling_pre_cb(void * closure, const double * bmin, const double * bmax)
{
  assert(closure);
  SoAction * action = (SoAction *) closure;
  assert(action->isOfType(SoGLRenderAction::getClassTypeId()) ||
         action->isOfType(SoCallbackAction::getClassTypeId()) );

  SoState * state = action->getState();
  state->push();

  if ( SoCullElement::completelyInside(state) ) { return TRUE; }

  SbBox3f box(bmin[0], bmin[1], bmin[2], bmax[0], bmax[1], bmax[2]);
  if ( !SoCullElement::cullBox(state, box, TRUE) ) { return TRUE; }

  return FALSE;
}

void
SmScenery::box_culling_post_cb(void * closure)
{
  assert(closure);
  SoAction * action = (SoAction *) closure;
  assert(action->isOfType(SoGLRenderAction::getClassTypeId()) ||
         action->isOfType(SoCallbackAction::getClassTypeId()) );

  SoState * state = action->getState();
  state->pop();
}

// ray vs terrain block bounding box culling
int
SmScenery::ray_culling_pre_cb(void * closure, const double * bmin, const double * bmax)
{
  assert(closure);
  SoAction * action = (SoAction *) closure;
  assert(action->isOfType(SoRayPickAction::getClassTypeId()));
  SoRayPickAction * rpaction = (SoRayPickAction *) action;

  SoState * state = rpaction->getState();
  state->push();

  SbBox3f box(bmin[0], bmin[1], bmin[2], bmax[0], bmax[1], bmax[2]); 
  if ( box.isEmpty() ) return FALSE;
  rpaction->setObjectSpace();
  return rpaction->intersect(box, TRUE);
}

void
SmScenery::ray_culling_post_cb(void * closure)
{
  assert(closure);
  SoAction * action = (SoAction *) closure;
  assert(action->isOfType(SoRayPickAction::getClassTypeId()));
  
  SoState * state = action->getState();
  state->pop();
}

/* ********************************************************************** */
// DYNAMIC TEXTURING

uint32_t
SmScenery::colortexture_cb(void * closure, double * pos, float elevation, double * spacing)
{
  assert(closure);
  SmScenery * thisp = (SmScenery *) closure;
  assert(thisp->isOfType(SmScenery::getClassTypeId()));

  const RenderState & rs = PRIVATE(thisp)->renderstate;
  uint32_t abgr = 0xffffffff; // default color to white

  if ( thisp->colorTexturing.getValue() == SmScenery::INTERPOLATED ) {
    if ( thisp->colorElevation.getNum() == 0 ) {
      // interpolate color table evenly over elevation range
      float fac = (elevation - rs.bbmin[2]) / (rs.bbmax[2] - rs.bbmin[2]);
      int steps = ((thisp->colorMap.getNum() / 4) - 1); // four components
      if ( steps == 0 ) { // only one color given
        int nr = (int) (SbClamp(thisp->colorMap[0], 0.0f, 1.0f) * 255.0f);
        int ng = (int) (SbClamp(thisp->colorMap[1], 0.0f, 1.0f) * 255.0f);
        int nb = (int) (SbClamp(thisp->colorMap[2], 0.0f, 1.0f) * 255.0f);
        int na = (int) (SbClamp(thisp->colorMap[3], 0.0f, 1.0f) * 255.0f);
        abgr = (na << 24) | (nb << 16) | (ng << 8) | nr;
      }
      else if ( steps > 0 ) { // interpolate color
        int startcolidx = (int) floor(float(steps) * fac);
        float rest = (float(steps) * fac) - float(startcolidx);
        float r = thisp->colorMap[startcolidx * 4 + 0];
        float g = thisp->colorMap[startcolidx * 4 + 1];
        float b = thisp->colorMap[startcolidx * 4 + 2];
        float a = thisp->colorMap[startcolidx * 4 + 3];
        if ( rest > 0.0f ) {
          assert(rest < 1.0f);
          float end_r = thisp->colorMap[startcolidx * 4 + 4];
          float end_g = thisp->colorMap[startcolidx * 4 + 5];
          float end_b = thisp->colorMap[startcolidx * 4 + 6];
          float end_a = thisp->colorMap[startcolidx * 4 + 7];
          r = r * (1.0f - rest) + end_r * rest;
          g = g * (1.0f - rest) + end_g * rest;
          b = b * (1.0f - rest) + end_b * rest;
          a = a * (1.0f - rest) + end_a * rest;
        }
        int nr = (int) (SbClamp(r, 0.0f, 1.0f) * 255.0);
        int ng = (int) (SbClamp(g, 0.0f, 1.0f) * 255.0);
        int nb = (int) (SbClamp(b, 0.0f, 1.0f) * 255.0);
        int na = (int) (SbClamp(a, 0.0f, 1.0f) * 255.0);
        abgr = (na << 24) | (nb << 16) | (ng << 8) | nr;
      }
    } else {
      if ( (thisp->colorElevation.getNum() * 4) != thisp->colorMap.getNum() ) {
        SoDebugError::postInfo("SmScenery::colortexture_cb", "size of colorElevation does not match size of colorMap");
        thisp->colorTexturing.setValue(SmScenery::DISABLED);
        return abgr;
      }
      // use elevation values to decide colors
      const int max = thisp->colorElevation.getNum();
      int i;
      for ( i = 0; (i < max) && (elevation > thisp->colorElevation[i]); i++ ) { }
      if ( i == 0 ) {
        // first color
        int nr = (int) (SbClamp(thisp->colorMap[0], 0.0f, 1.0f) * 255.0f);
        int ng = (int) (SbClamp(thisp->colorMap[1], 0.0f, 1.0f) * 255.0f);
        int nb = (int) (SbClamp(thisp->colorMap[2], 0.0f, 1.0f) * 255.0f);
        int na = (int) (SbClamp(thisp->colorMap[3], 0.0f, 1.0f) * 255.0f);
        abgr = (na << 24) | (nb << 16) | (ng << 8) | nr;
      }
      else if ( i == max ) {
        // last color
        int nr = (int) (SbClamp(thisp->colorMap[(i-1)*4+0], 0.0f, 1.0f) * 255.0f);
        int ng = (int) (SbClamp(thisp->colorMap[(i-1)*4+1], 0.0f, 1.0f) * 255.0f);
        int nb = (int) (SbClamp(thisp->colorMap[(i-1)*4+2], 0.0f, 1.0f) * 255.0f);
        int na = (int) (SbClamp(thisp->colorMap[(i-1)*4+3], 0.0f, 1.0f) * 255.0f);
        abgr = (na << 24) | (nb << 16) | (ng << 8) | nr;
      }
      else {
        // interpolated color
        float fac =
          (elevation - thisp->colorElevation[i-1]) /
          (thisp->colorElevation[i] - thisp->colorElevation[i-1]);
        float r = thisp->colorMap[(i-1) * 4 + 0];
        float g = thisp->colorMap[(i-1) * 4 + 1];
        float b = thisp->colorMap[(i-1) * 4 + 2];
        float a = thisp->colorMap[(i-1) * 4 + 3];
        float end_r = thisp->colorMap[i * 4 + 0];
        float end_g = thisp->colorMap[i * 4 + 1];
        float end_b = thisp->colorMap[i * 4 + 2];
        float end_a = thisp->colorMap[i * 4 + 3];
        r = r * (1.0f - fac) + end_r * fac;
        g = g * (1.0f - fac) + end_g * fac;
        b = b * (1.0f - fac) + end_b * fac;
        a = a * (1.0f - fac) + end_a * fac;
        int nr = (int) (SbClamp(r, 0.0f, 1.0f) * 255.0);
        int ng = (int) (SbClamp(g, 0.0f, 1.0f) * 255.0);
        int nb = (int) (SbClamp(b, 0.0f, 1.0f) * 255.0);
        int na = (int) (SbClamp(a, 0.0f, 1.0f) * 255.0);
        abgr = (na << 24) | (nb << 16) | (ng << 8) | nr;
      }
    }
  }
  else if ( thisp->colorTexturing.getValue() == SmScenery::DISCRETE ) {
    if ( thisp->colorElevation.getNum() == 0 ) {
      // distribute colors evenly
      int colors = thisp->colorMap.getNum() / 4;
      int color = (int) floor(((elevation - rs.bbmin[2]) / ((rs.bbmax[2] - rs.bbmin[2]) * 1.00001)) * float(colors));
      float r = thisp->colorMap[color * 4 + 0];
      float g = thisp->colorMap[color * 4 + 1];
      float b = thisp->colorMap[color * 4 + 2];
      float a = thisp->colorMap[color * 4 + 3];
      int nr = (int) (SbClamp(r, 0.0f, 1.0f) * 255.0);
      int ng = (int) (SbClamp(g, 0.0f, 1.0f) * 255.0);
      int nb = (int) (SbClamp(b, 0.0f, 1.0f) * 255.0);
      int na = (int) (SbClamp(a, 0.0f, 1.0f) * 255.0);
      abgr = (na << 24) | (nb << 16) | (ng << 8) | nr;
    }
    else {
      // distribute colors based on elevation value table
      const int max = thisp->colorElevation.getNum();
      if ( ((max + 1) * 4) != (thisp->colorMap.getNum())) {
        SoDebugError::postInfo("SmScenery::colortexture_cb", "size of colorElevation does not match size of colorMap");
        thisp->colorTexturing.setValue(SmScenery::DISABLED);
        return abgr;
      }
      // use elevation values to decide colors
      int i;
      for ( i = 1; (i <= max) && (elevation > thisp->colorElevation[i-1]); i++ ) { }
      i = i - 1;
      float r = thisp->colorMap[i * 4 + 0];
      float g = thisp->colorMap[i * 4 + 1];
      float b = thisp->colorMap[i * 4 + 2];
      float a = thisp->colorMap[i * 4 + 3];
      int nr = (int) (SbClamp(r, 0.0f, 1.0f) * 255.0);
      int ng = (int) (SbClamp(g, 0.0f, 1.0f) * 255.0);
      int nb = (int) (SbClamp(b, 0.0f, 1.0f) * 255.0);
      int na = (int) (SbClamp(a, 0.0f, 1.0f) * 255.0);
      abgr = (na << 24) | (nb << 16) | (ng << 8) | nr;
    }
  }
  else {
    // disabled?
  }
  return abgr;
}

uint32_t
SceneryP::invokecolortexturecb(void * closure, double * pos, float elevation, double * spacing)
{
  assert(closure);
  SceneryP * thisp = (SceneryP *) closure;
  if ( thisp->cbtexcb ) {
    return thisp->cbtexcb(thisp->cbtexclosure, pos, elevation, spacing);
  }
  else {
    return 0xffffffff;
  }
}

/* ********************************************************************** */
