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
#include <Inventor/SbLine.h>

#include <Inventor/SoInput.h>

#include <Inventor/misc/SoGLImage.h>
#include <Inventor/C/glue/gl.h>
#include <Inventor/C/tidbits.h>

#include <SmallChange/misc/SceneryGlue.h>
#include <SmallChange/nodes/SmScenery.h>
#include <SmallChange/nodes/SceneryGL.h>

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
  \var SoSFFloat SmScenery::elevationLineThickness
  \brief Make elevation lines this thick.

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
  SoFieldSensor * elevationthicknesssensor;
  SoFieldSensor * elevationemphasissensor;

  SbBool didevaluate;
  SbBool didevaluateonce;

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
  uint8_t * elevationlinesdata;
  int elevationlinestexturesize;

  int usevertexarrays;

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

  // generate primitives / raypick
  void GEN_VERTEX(RenderState * state, const int x, const int y, const float elev);
  static void gen_pre_cb(void * closure, ss_render_block_cb_info * info);
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
  elevationthicknesssensor(NULL), elevationemphasissensor(NULL),
  cbtexcb(NULL), cbtexclosure(NULL),
  system(NULL), blocksize(0), pvertex(NULL), colormaptexid(-1), firstGLRender(TRUE),
  facedetail(NULL), currhotspot(0.0f, 0.0f, 0.0f), curraction(NULL),
  currstate(NULL), viewid(-1), dummyimage(NULL),
  elevationlinesimage(NULL), elevationlinesdata(NULL),
  elevationlinestexturesize(0),
  usevertexarrays(TRUE)
{
  this->renderstate.bbmin[0] = 0.0;
  this->renderstate.bbmin[1] = 0.0;
  this->renderstate.bbmin[2] = 0.0;
  this->renderstate.bbmax[0] = 0.0;
  this->renderstate.bbmax[1] = 0.0;
  this->renderstate.bbmax[2] = 0.0;
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

#define PRIVATE(obj) ((obj)->pimpl)
#define PUBLIC(obj) ((obj)->api)

void
SmScenery::initClass(void)
{
  static SbBool first = TRUE;
  if (!first) { return; }
  first = FALSE;
  if (sc_scenery_available()) {
    sc_ssglue_initialize();
    if ( ok_to_use_bytevalue_normals() ) {
      sc_set_use_bytenormals(TRUE);
    }
    else {
      sc_set_use_bytenormals(FALSE);
    }
  }
  SO_NODE_INIT_CLASS(SmScenery, SoShape, "Shape");
}

SO_NODE_SOURCE(SmScenery);

SmScenery *
SmScenery::createInstance(double * origo, double * spacing, int * elements, float * values, const float undefval)
{
  if (!sc_scenery_available()) { return NULL; }
  ss_system * system = sc_ssglue_system_construct(1, origo, spacing, elements, values, undefval);
  if (!system) { return NULL; }
  return new SmScenery(system);
}

SmScenery *
SmScenery::createInstance(const int cols, const int rows, double * xyzgrid, const float undefz)
{
  if (!sc_scenery_available()) { return NULL; }
  ss_system * system = sc_ssglue_system_construct_rotated(1, cols, rows, xyzgrid, undefz);
  if (!system) { return NULL; }
  return new SmScenery(system);
}

SmScenery *
SmScenery::createInstance(const int points, double * xyzvals, const float reach)
{
  if (!sc_scenery_available()) { return NULL; }
  ss_system * system = sc_ssglue_system_construct_randomized(1, points, xyzvals, reach);
  if (!system) { return NULL; }
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
  SO_NODE_ADD_FIELD(elevationLineThickness, (1.0f));
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

  PRIVATE(this)->didevaluate = FALSE;
  PRIVATE(this)->didevaluateonce = FALSE;
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
  SO_NODE_ADD_FIELD(elevationLineThickness, (1.0f));
  SO_NODE_ADD_FIELD(elevationLineEmphasis, (0));

  SO_NODE_ADD_FIELD(visualDebug, (FALSE));

  // old compat field
  SO_NODE_ADD_FIELD(colorTexture, (FALSE));

  PRIVATE(this)->commonConstructor();

  // FIXME: view-specific. Move to struct.
  PRIVATE(this)->pvertex = new SoPrimitiveVertex;
  PRIVATE(this)->facedetail = new SoFaceDetail;

  PRIVATE(this)->system = system;

  // this happens in the filenamesensor_cb, so must be set up specifically here
  PRIVATE(this)->colormaptexid = 
    sc_ssglue_system_add_runtime_texture2d(PRIVATE(this)->system, 0, SceneryP::invokecolortexturecb, PRIVATE(this));
  sc_ssglue_system_get_object_box(PRIVATE(this)->system,
                                  PRIVATE(this)->renderstate.bbmin,
                                  PRIVATE(this)->renderstate.bbmax); 

  PRIVATE(this)->blocksize =
    sc_ssglue_system_get_blocksize(PRIVATE(this)->system);
  PRIVATE(this)->renderstate.blocksize = (float) (PRIVATE(this)->blocksize-1);
  PRIVATE(this)->viewid = sc_ssglue_view_allocate(PRIVATE(this)->system);
  assert(PRIVATE(this)->viewid >= 0);
  sc_ssglue_view_enable(PRIVATE(this)->system, PRIVATE(this)->viewid);

  // FIXME: factor this piece of logic into separate function
  // setImplicitRenderSequence() or something similar
  const int sequencelen = this->renderSequence.getNum();
  if ((this->colorTexturing.getValue() != SmScenery::DISABLED) && PRIVATE(this)->colormaptexid != -1) {
    int localsequence[2] = { PRIVATE(this)->colormaptexid, 0 };
    sc_ssglue_view_set_render_sequence_a(PRIVATE(this)->system, PRIVATE(this)->viewid, 2, localsequence);
  } else if (sequencelen == 0) {
    sc_ssglue_view_set_render_sequence_a(PRIVATE(this)->system, PRIVATE(this)->viewid, 0, NULL);
  } else {
    this->renderSequence.enableNotify(FALSE);
    int * sequence = this->renderSequence.startEditing();
    sc_ssglue_view_set_render_sequence_a(PRIVATE(this)->system, PRIVATE(this)->viewid, sequencelen, sequence);
    this->renderSequence.finishEditing();
    this->renderSequence.enableNotify(TRUE);
  }
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

  this->elevationthicknesssensor = new SoFieldSensor(SceneryP::elevationlinessensor_cb, PUBLIC(this));
  this->elevationthicknesssensor->attach(&PUBLIC(this)->elevationLineThickness);

  this->elevationemphasissensor = new SoFieldSensor(SceneryP::elevationlinessensor_cb, PUBLIC(this));
  this->elevationemphasissensor->attach(&PUBLIC(this)->elevationLineEmphasis);

  // elevation texture test
  this->renderstate.etexscale = 0.0f;
  this->renderstate.etexoffset = 0.0f;

  this->cbtexcb = SmScenery::colortexture_cb;
  this->cbtexclosure = PUBLIC(this);

  sc_renderstate_construct(&this->renderstate);
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
  delete PRIVATE(this)->elevationthicknesssensor;
  delete PRIVATE(this)->elevationemphasissensor;
  delete PRIVATE(this)->old_colortexturesensor;

  delete PRIVATE(this)->pvertex;
  delete PRIVATE(this)->facedetail;
  if (sc_scenery_available() &&
      (PRIVATE(this)->system != NULL) &&
      (PRIVATE(this)->viewid != -1)) {
    sc_ssglue_view_deallocate(PRIVATE(this)->system, PRIVATE(this)->viewid);
    sc_ssglue_system_close(PRIVATE(this)->system);
    PRIVATE(this)->viewid = -1;
  }
  if (PRIVATE(this)->elevationlinesimage) {
    PRIVATE(this)->elevationlinesimage->unref();
    PRIVATE(this)->elevationlinesimage = NULL;
  }
  if (PRIVATE(this)->elevationlinesdata) {
    free(PRIVATE(this)->elevationlinesdata);
    PRIVATE(this)->elevationlinesdata = NULL;
  }
  sc_renderstate_destruct(&PRIVATE(this)->renderstate);
  delete PRIVATE(this);
}

/* ********************************************************************** */

void 
SmScenery::GLRender(SoGLRenderAction * action)
{
  if (!sc_scenery_available()) { return; }
  if (PRIVATE(this)->system == NULL) { return; }
  if (!this->shouldGLRender(action)) { return; }

  SbBool needpostframe = FALSE;

  if (!PRIVATE(this)->didevaluate) {
    this->preFrame();
    this->evaluate(action);
    needpostframe = TRUE;
  }
  PRIVATE(this)->didevaluate = FALSE;

  // rendersequence start
  const int sequencelen = this->renderSequence.getNum();
  if ((this->colorTexturing.getValue() != SmScenery::DISABLED) && PRIVATE(this)->colormaptexid != -1) {
    // FIXME: add runtime colortexture
    int localsequence[2] = { PRIVATE(this)->colormaptexid, 0 };
    sc_ssglue_view_set_render_sequence_a(PRIVATE(this)->system, PRIVATE(this)->viewid, 2, localsequence);
  } 
  else if (sequencelen == 0) {
    sc_ssglue_view_set_render_sequence_a(PRIVATE(this)->system, PRIVATE(this)->viewid, 0, NULL);
  } 
  else {
    this->renderSequence.enableNotify(FALSE);
    int * sequence = this->renderSequence.startEditing();
    sc_ssglue_view_set_render_sequence_a(PRIVATE(this)->system, PRIVATE(this)->viewid, sequencelen, sequence);
    this->renderSequence.finishEditing();
    this->renderSequence.enableNotify(TRUE);
  }
  // rendersequenceend

  if ( PRIVATE(this)->elevationlinestexturesize == 0 ) {
    GLint texturesize = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texturesize);
    // SoDebugError::postInfo("GLRender", "texture size: %d", texturesize);
    PRIVATE(this)->elevationlinestexturesize = texturesize;
    if ( this->elevationLines.getValue() != SmScenery::DISABLED ) {
      PRIVATE(this)->elevationlinestexchange();
    }
  }

  SoState * state = action->getState();
  const cc_glglue * gl = cc_glglue_instance(SoGLCacheContextElement::get(state));
  assert(gl);

  if (PRIVATE(this)->firstGLRender) {
    // FIXME: this should not really be necessary, and should be
    // considered a work-around for a bug in the scenery SDK. 20031015 mortene.
    if ((this->colorTexturing.getValue() != SmScenery::DISABLED) && (PRIVATE(this)->colormaptexid != -1)) {
      this->refreshTextures(PRIVATE(this)->colormaptexid);
    }

    sc_probe_gl(FALSE);
    // just override defaults when probing - power-users can override later
    if ( !sc_found_multitexturing() ) {
      if ( this->elevationLines.getValue() ) {
        // a better fix would be to have an internal flag on whether
        // multitexturing was possible or not, and to combine both
        // flags on whether to enable multitexturing 
        this->elevationLines.setValue(FALSE);
      }
    }
    if ( !sc_found_vertexarrays() ) {
      PRIVATE(this)->usevertexarrays = FALSE;
    } else {
      PRIVATE(this)->usevertexarrays = sc_suggest_vertexarrays();
      PRIVATE(this)->usevertexarrays = TRUE; // FIXME: hack
    }
    sc_set_use_bytenormals(sc_suggest_bytenormals());

    if ( cc_glglue_has_texture_edge_clamp(gl) ) {
      sc_set_have_clamp_to_edge(TRUE);
    }
    PRIVATE(this)->firstGLRender = FALSE;
  }

  SbVec2s vpsize = SoViewportRegionElement::get(state).getViewportSizePixels();
  state->push();
  
  float quality = SoTextureQualityElement::get(state);
  PRIVATE(this)->renderstate.dotex = quality > 0.0f;
  SbVec3f campos = SoViewVolumeElement::get(state).getProjectionPoint();

  //  transform into local coordinate system
  SbMatrix worldtolocal =
    SoModelMatrixElement::get(state).inverse();
  worldtolocal.multVecMatrix(campos, campos);

  SoCacheElement::invalidate(state);
  SoMaterialBundle mb(action);
  mb.sendFirst();

  SbBool texwasenabled = glIsEnabled(GL_TEXTURE_2D);

  PRIVATE(this)->renderstate.texisenabled = texwasenabled;
  PRIVATE(this)->renderstate.currtexid = 0;

  sc_init_debug_info(&PRIVATE(this)->renderstate);

  PRIVATE(this)->currhotspot = campos;
  PRIVATE(this)->curraction = action;
  PRIVATE(this)->currstate = state;

  const SbViewVolume & vv = SoViewVolumeElement::get(state);
  const SbMatrix & mm = SoModelMatrixElement::get(state);
  SbMatrix imm = mm.inverse();

  SbPlane sbplanes[6];
  vv.getViewVolumePlanes(sbplanes);
  float planes[24];
  assert(planes);
  int i;
  for ( i = 0; i < 6; i++ ) {
    sbplanes[i].transform(imm);
    SbVec3f normal = sbplanes[i].getNormal();
    planes[i*4+0] = normal[0];
    planes[i*4+1] = normal[1];
    planes[i*4+2] = normal[2];
    planes[i*4+3] = sbplanes[i].getDistanceFromOrigin();
  }
  
  PRIVATE(this)->renderstate.numclipplanes = 6;
  PRIVATE(this)->renderstate.clipplanes = planes;

  // set up culling suitable for rendering
  sc_ssglue_view_set_culling_pre_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                          sc_plane_culling_pre_cb, &PRIVATE(this)->renderstate);
  sc_ssglue_view_set_culling_post_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                           sc_plane_culling_post_cb, &PRIVATE(this)->renderstate);

  if ( PRIVATE(this)->usevertexarrays ) {
    sc_ssglue_view_set_render_pre_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                           sc_va_render_pre_cb, &PRIVATE(this)->renderstate);
    sc_ssglue_view_set_render_post_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                            sc_va_render_post_cb, &PRIVATE(this)->renderstate);

    sc_ssglue_view_set_render_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                       sc_va_render_cb, &PRIVATE(this)->renderstate);
    sc_ssglue_view_set_undef_render_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                             sc_va_undefrender_cb, &PRIVATE(this)->renderstate); 
  }
  else {
    sc_ssglue_view_set_render_pre_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                           sc_render_pre_cb, &PRIVATE(this)->renderstate);
    sc_ssglue_view_set_render_post_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                            sc_render_post_cb, &PRIVATE(this)->renderstate);

    sc_ssglue_view_set_render_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                       sc_render_cb, &PRIVATE(this)->renderstate);
    sc_ssglue_view_set_undef_render_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                             sc_undefrender_cb, &PRIVATE(this)->renderstate); 
  }

  double hotspot[3];
  hotspot[0] = campos[0];
  hotspot[1] = campos[1];
  hotspot[2] = campos[2];

  sc_ssglue_view_set_hotspots(PRIVATE(this)->system, PRIVATE(this)->viewid, 1, hotspot); 

  // PRIVATE(this)->debuglist.truncate(0);
  // PRIVATE(this)->numnewtextures = 0;
  PRIVATE(this)->renderstate.newtexcount = 0;


  SbBool didenabletexture1 = FALSE;
  float oldetexscale = PRIVATE(this)->renderstate.etexscale;
  
  if ((quality > 0.0f) &&
      (PRIVATE(this)->renderstate.etexscale != 0.0f) &&
      (PRIVATE(this)->elevationlinesimage != NULL) &&
      cc_glglue_has_multitexture(gl)) {
    cc_glglue_glActiveTexture(gl, GL_TEXTURE1);
    glEnable(GL_TEXTURE_2D);
    PRIVATE(this)->elevationlinesimage->getGLDisplayList(state)->call(state);
    cc_glglue_glActiveTexture(gl, GL_TEXTURE0);
    didenabletexture1 = TRUE;
  }
  else {
    // init to 0.0f so that texture coordinates are not sent for unit 1
    PRIVATE(this)->renderstate.etexscale = 0.0f;
  }
  
  PRIVATE(this)->renderstate.renderpass = TRUE;
  sc_ssglue_view_render(PRIVATE(this)->system, PRIVATE(this)->viewid);
  PRIVATE(this)->renderstate.renderpass = FALSE;

  PRIVATE(this)->renderstate.numclipplanes = 0;
  PRIVATE(this)->renderstate.clipplanes = NULL;

  if (didenabletexture1) {
    cc_glglue_glActiveTexture(gl, GL_TEXTURE1);
    glDisable(GL_TEXTURE_2D);
    cc_glglue_glActiveTexture(gl, GL_TEXTURE0);
  }
  // restore etexscale
  PRIVATE(this)->renderstate.etexscale = oldetexscale;
  
//  fprintf(stderr,"num boxes: %d, new texs: %d\n",
//          this->debuglist.getLength()/4, this->numnewtextures);
  
  // texturing state could be changed during scenery rendering
  SbBool texisenabled = glIsEnabled(GL_TEXTURE_2D);
  if (texisenabled != texwasenabled) {
    if (texwasenabled) glEnable(GL_TEXTURE_2D);
    else glDisable(GL_TEXTURE_2D);
  } 
  state->pop();

  sc_ssglue_view_set_culling_pre_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                          NULL, NULL);
  sc_ssglue_view_set_culling_post_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                           NULL, NULL);

  if (this->visualDebug.getValue()) {
    campos[0] = (float) ((campos[0] / PRIVATE(this)->renderstate.bbmax[0]) - 0.5f);
    campos[1] = (float) ((campos[1] / PRIVATE(this)->renderstate.bbmax[1]) - 0.5f);
    state->push();
    sc_display_debug_info(&PRIVATE(this)->renderstate, &campos[0], &vpsize[0]);
    state->pop();
  }
  
  SoGLLazyElement::getInstance(state)->reset(state, SoLazyElement::DIFFUSE_MASK|SoLazyElement::GLIMAGE_MASK);

  if (needpostframe) {
    if (this->postFrame()) {
      action->getCurPath()->getHead()->touch();
    }
  }
}

void 
SmScenery::rayPick(SoRayPickAction * action)
{
  if (!sc_scenery_available()) { return; }

  action->setObjectSpace();
  const SbLine & line = action->getLine();
  const SbVec3f & raydir = line.getDirection();
  const SbVec3f & raypos = line.getPosition();

  PRIVATE(this)->renderstate.raypos[0] = raypos[0];
  PRIVATE(this)->renderstate.raypos[1] = raypos[1];
  PRIVATE(this)->renderstate.raypos[2] = raypos[2];
  PRIVATE(this)->renderstate.raydir[0] = raydir[0];
  PRIVATE(this)->renderstate.raydir[1] = raydir[1];
  PRIVATE(this)->renderstate.raydir[2] = raydir[2];

  sc_ssglue_view_set_culling_pre_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                          sc_ray_culling_pre_cb, &PRIVATE(this)->renderstate);
  sc_ssglue_view_set_culling_post_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                           sc_ray_culling_post_cb, &PRIVATE(this)->renderstate);

  inherited::rayPick(action); // just generate primitives
  
  sc_ssglue_view_set_culling_pre_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                          NULL, NULL);
  sc_ssglue_view_set_culling_post_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                           NULL, NULL);
}

SoCallbackAction::Response 
SmScenery::evaluateS(void * userdata, SoCallbackAction * action, const SoNode * node)
{
  assert(node->isOfType(SmScenery::getClassTypeId()));
  ((SmScenery*)node)->evaluate(action);
  return SoCallbackAction::CONTINUE;
}

void 
SmScenery::evaluate(SoAction * action)
{
  if (!sc_scenery_available()) { return; }

  PRIVATE(this)->didevaluate = TRUE;
  PRIVATE(this)->didevaluateonce = TRUE;

  // rendersequence start
  const int sequencelen = this->renderSequence.getNum();
  if ((this->colorTexturing.getValue() != SmScenery::DISABLED) && PRIVATE(this)->colormaptexid != -1) {
    // FIXME: add runtime colortexture
    int localsequence[2] = { PRIVATE(this)->colormaptexid, 0 };
    sc_ssglue_view_set_render_sequence_a(PRIVATE(this)->system, PRIVATE(this)->viewid, 2, localsequence);
  } else if (sequencelen == 0) {
    sc_ssglue_view_set_render_sequence_a(PRIVATE(this)->system, PRIVATE(this)->viewid, 0, NULL);
  } else {
    this->renderSequence.enableNotify(FALSE);
    int * sequence = this->renderSequence.startEditing();
    sc_ssglue_view_set_render_sequence_a(PRIVATE(this)->system, PRIVATE(this)->viewid, sequencelen, sequence);
    this->renderSequence.finishEditing();
    this->renderSequence.enableNotify(TRUE);
  }
  // rendersequenceend

  if (PRIVATE(this)->firstGLRender) {
    // FIXME: this should not really be necessary, and should be
    // considered a work-around for a bug in the scenery SDK. 20031015 mortene.
    if ((this->colorTexturing.getValue() != SmScenery::DISABLED) && (PRIVATE(this)->colormaptexid != -1)) {
      this->refreshTextures(PRIVATE(this)->colormaptexid);
    }
  }

  if (PRIVATE(this)->system == NULL) return;
  SoState * state = action->getState();

  SbVec3f campos = SoViewVolumeElement::get(state).getProjectionPoint();
  //  transform into local coordinate system
  SbMatrix worldtolocal = SoModelMatrixElement::get(state).inverse();
  worldtolocal.multVecMatrix(campos, campos);
  PRIVATE(this)->currhotspot = campos;


 const SbViewVolume & vv = SoViewVolumeElement::get(state);
 const SbMatrix & mm = SoModelMatrixElement::get(state);
  SbMatrix imm = mm.inverse();
          
  SbPlane sbplanes[6];
  vv.getViewVolumePlanes(sbplanes);

  float planes[24];
  int i;
  for ( i = 0; i < 6; i++ ) {
    sbplanes[i].transform(imm);
    SbVec3f normal = sbplanes[i].getNormal();
    planes[i*4+0] = normal[0];
    planes[i*4+1] = normal[1];
    planes[i*4+2] = normal[2];
    planes[i*4+3] = sbplanes[i].getDistanceFromOrigin();
  } 
                      
  PRIVATE(this)->renderstate.numclipplanes = 6;
  PRIVATE(this)->renderstate.clipplanes = planes;

  sc_ssglue_view_set_culling_pre_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                          sc_plane_culling_pre_cb, &PRIVATE(this)->renderstate);
  sc_ssglue_view_set_culling_post_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                           sc_plane_culling_post_cb, &PRIVATE(this)->renderstate);
  sc_ssglue_view_set_render_pre_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                         NULL, NULL);
  sc_ssglue_view_set_render_post_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                          NULL, NULL);
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
  PRIVATE(this)->renderstate.clipplanes = NULL;
  PRIVATE(this)->renderstate.numclipplanes = 0;
}

void 
SmScenery::generatePrimitives(SoAction * action)
{
  if (!sc_scenery_available()) { return; }
  if (PRIVATE(this)->system == NULL) return;
  if (!PRIVATE(this)->didevaluateonce) return;
  
  sc_ssglue_view_set_render_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                     SceneryP::gen_cb, this);
  sc_ssglue_view_set_undef_render_callback(PRIVATE(this)->system, PRIVATE(this)->viewid,
                                           SceneryP::undefgen_cb, this);

  sc_ssglue_view_set_render_pre_callback(PRIVATE(this)->system,
                                         PRIVATE(this)->viewid,
                                         SceneryP::gen_pre_cb, this);
  sc_ssglue_view_set_render_post_callback(PRIVATE(this)->system,
                                          PRIVATE(this)->viewid,
                                          NULL, NULL);

  SoPointDetail pointDetail;
  PRIVATE(this)->pvertex->setDetail(&pointDetail);
  PRIVATE(this)->curraction = action;
  sc_ssglue_view_render(PRIVATE(this)->system, PRIVATE(this)->viewid);
}

void 
SmScenery::computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center)
{
  if (PRIVATE(this)->system == NULL) return;  
  const RenderState & rs = PRIVATE(this)->renderstate;
  box.setBounds((float) rs.bbmin[0], (float) rs.bbmin[1], (float) rs.bbmin[2],
                (float) rs.bbmax[0], (float) rs.bbmax[1], (float) rs.bbmax[2]);
  center = box.getCenter();
}

#if SS_IMPORT_XYZ

static
ss_system *
readxyz(const char * filename)
{
  FILE * fp = fopen(filename, "rb");
  if (!fp) return NULL;
  int points = 0;
  int loop = TRUE;
  while (loop) {
    float fx, fy, fz;
    if (fscanf(fp, "%f %f %f", &fx, &fy, &fz) == 3) {
      points++;
    } else {
      if (!feof(fp)) points = 0; // something fishy
      fclose(fp);
      loop = FALSE;
    }
  }
  if (points < 2) return NULL;
  fp = fopen(filename, "rb");
  if (!fp) return NULL;
  float * values = (float *) malloc(points * 3 * sizeof(float));
  assert(values);
  loop = TRUE;
  points = 0;
  while (loop) {
    float fx, fy, fz;
    if (fscanf(fp, "%f %f %f", &fx, &fy, &fz) == 3) {
      values[points*3+0] = fx;
      values[points*3+1] = fy;
      values[points*3+2] = fz;
      points++;
    } else {
      if (!feof(fp)) points = 0; // something fishy
      fclose(fp);
      loop = FALSE;
    }
  }
  if (points < 2) {
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
  for (i = 0; i < points; i++) {
    elevations[i] = values[3*i+2];
    if (deltay[0] == 0.0f && deltay[1] == 0.0f && i < (points - 1)) {
      float delta[2] = { values[3*(i+1)] - values[3*i], values[3*(i+1)+1] - values[3*i+1] };
      float dot = deltax[0] * delta[0] + deltax[1] * delta[1];
      if (dot < 0.0f) {
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

  // fprintf(stderr, "xyz import - cols: %d rows: %d (%d points - %d)\n", dimension[0], dimension[1], points, (dimension[0]*dimension[1]));
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
  this->refreshTextures(PRIVATE(this)->colormaptexid);
}

void 
SmScenery::preFrame(void)
{
  if (!sc_scenery_available() || !PRIVATE(this)->system) { return; }
  sc_ssglue_view_pre_frame(PRIVATE(this)->system, PRIVATE(this)->viewid);
  sc_mark_unused_textures(&PRIVATE(this)->renderstate);
}

int 
SmScenery::postFrame(void)
{
  if (!sc_scenery_available() || !PRIVATE(this)->system) { return 0; }
  sc_delete_unused_textures(&PRIVATE(this)->renderstate);
  return sc_ssglue_view_post_frame(PRIVATE(this)->system, PRIVATE(this)->viewid);
}

void 
SmScenery::setBlockRottger(const float c)
{
  if (!sc_scenery_available() || !PRIVATE(this)->system) { return; }
  sc_ssglue_view_set_evaluate_rottger_parameters(PRIVATE(this)->system, PRIVATE(this)->viewid, 16.0f, c);
}

float
SmScenery::getBlockRottger(void) const
{
  if (!sc_scenery_available() || !PRIVATE(this)->system) { return 0.0f; }
  float C, c;
  sc_ssglue_view_get_evaluate_rottger_parameters(PRIVATE(this)->system, PRIVATE(this)->viewid, &C, &c);
  return c;
}

void 
SmScenery::setLoadRottger(const float c)
{
  if (!sc_scenery_available() || !PRIVATE(this)->system) { return; }
  sc_ssglue_view_set_load_rottger_parameters(PRIVATE(this)->system, PRIVATE(this)->viewid, 16.0f, c);
}

float
SmScenery::getLoadRottger(void) const
{
  if (!sc_scenery_available() || !PRIVATE(this)->system) { return 0.0f; }
  float C, c;
  sc_ssglue_view_get_load_rottger_parameters(PRIVATE(this)->system, PRIVATE(this)->viewid, &C, &c);
  return c;
}

/*!

  Sets whether vertex array rendering should be used (still a bit
  slower, but works on ATI cards). Probably faster on OS X.

*/
void 
SmScenery::setVertexArraysRendering(const SbBool onoff)
{
  PRIVATE(this)->usevertexarrays = (int) onoff;
}

/*!
  Returns whether vertex array rendering is used.
*/
SbBool 
SmScenery::getVertexArraysRendering(void) const
{
  return (SbBool) PRIVATE(this)->usevertexarrays;
}

SbVec3f
SmScenery::getRenderCoordinateOffset(void) const
{
  double origo[3] = { 0.0, 0.0, 0.0 };
  sc_ssglue_system_get_origo_world_position(PRIVATE(this)->system, origo);
  return SbVec3f((float) origo[0], (float) origo[1], (float) origo[2]);
}

SbVec2f
SmScenery::getElevationRange(void) const
{
  return SbVec2f((float) PRIVATE(this)->renderstate.bbmin[2],
                 (float) PRIVATE(this)->renderstate.bbmax[2]);
}

void 
SmScenery::refreshTextures(const int id)
{
  if (!sc_scenery_available() || !PRIVATE(this)->system) { return; }
  sc_ssglue_system_refresh_runtime_texture2d(PRIVATE(this)->system, id);
  sc_delete_all_textures(&PRIVATE(this)->renderstate);
}

// *************************************************************************

void 
SceneryP::filenamesensor_cb(void * closure, SoSensor * sensor)
{
  assert(closure);
  if (!sc_scenery_available()) { return; }

  SmScenery * thisp = (SmScenery *) closure;

  if (PRIVATE(thisp)->system) {
    if (PRIVATE(thisp)->viewid != -1) {
      sc_ssglue_view_deallocate(PRIVATE(thisp)->system, PRIVATE(thisp)->viewid);
    }
    sc_ssglue_system_close(PRIVATE(thisp)->system);
  }

  PRIVATE(thisp)->viewid = -1;
  PRIVATE(thisp)->system = NULL;
  PRIVATE(thisp)->colormaptexid = -1;

  const SbStringList & pathlist = SoInput::getDirectories();
  SbString s = thisp->filename.getValue();
  if (s.getLength()) {
#if 0 && SS_IMPORT_XYZ
    if (s.find(".xyz") == (s.getLength() - 4)) {
      PRIVATE(thisp)->system = readxyz(s.getString());
    }
#endif
    if (!PRIVATE(thisp)->system) {
      int i;
      for (i = -1; (PRIVATE(thisp)->system == NULL) && (i < pathlist.getLength()); i++) {
        if (i == -1) {
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
      if ((sc_ssglue_system_get_num_datasets(PRIVATE(thisp)->system) > 0) &&
           (sc_ssglue_system_get_dataset_type(PRIVATE(thisp)->system, 0) == SS_ELEVATION_TYPE)) {
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
      if ((thisp->colorTexturing.getValue() != SmScenery::DISABLED) && PRIVATE(thisp)->colormaptexid != -1) {
        int localsequence[2] = { PRIVATE(thisp)->colormaptexid, 0 };
        sc_ssglue_view_set_render_sequence_a(PRIVATE(thisp)->system, PRIVATE(thisp)->viewid, 2, localsequence);
      } else if (sequencelen == 0) {
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
  if (thisp->colorTexture.getValue()) {
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
  if ((thisp->elevationLines.getValue() != FALSE) &&
       (thisp->elevationLineDistance.getValue() > 0.0f)) {
    PRIVATE(thisp)->elevationlinestexchange();
  } else {
    PRIVATE(thisp)->renderstate.etexscale = 0.0f;
  }
}

// *************************************************************************

void
SceneryP::colormaptexchange(void)
{
  if (this->colormaptexid != -1) {
    PUBLIC(this)->refreshTextures(this->colormaptexid);
  }
}

void
SceneryP::elevationlinestexchange(void)
{
  if ( this->elevationlinesimage == NULL ) {
    this->elevationlinesimage = new SoGLImage;
    // this->elevationlinesimage->ref(); ???  unref() but no ref()?
    assert(this->elevationlinesimage);
  }
  if ( this->elevationlinestexturesize == 0 ) {
    return;
  }

#define COMPONENTS 4

  if ( this->elevationlinesdata == NULL ) {
    this->elevationlinesdata = (uint8_t *)
      malloc(this->elevationlinestexturesize * COMPONENTS);
    assert(this->elevationlinesdata);
  }

  float dist = PUBLIC(this)->elevationLineDistance.getValue();
  float offset = PUBLIC(this)->elevationLineOffset.getValue();
  float thickness = PUBLIC(this)->elevationLineThickness.getValue();
  int emphasis = PUBLIC(this)->elevationLineEmphasis.getValue();

  sc_generate_elevation_line_texture(dist, offset, thickness, emphasis,
                                     this->elevationlinesdata,
                                     COMPONENTS,
                                     this->elevationlinestexturesize,
                                     &(this->renderstate.etexscale),
                                     &(this->renderstate.etexoffset));

  this->elevationlinesimage->setData(this->elevationlinesdata,
      SbVec2s(1, this->elevationlinestexturesize), COMPONENTS);

#undef COMPONENTS
}

// *************************************************************************
// GENERATE PRIMITIVES

void 
SceneryP::GEN_VERTEX(RenderState * state, const int x, const int y, const float elev)
{
  this->pvertex->setPoint(SbVec3f((float) (x*state->vspacing[0] + state->voffset[0]),
                                  (float) (y*state->vspacing[1] + state->voffset[1]),
                                  elev));
  PUBLIC(this)->shapeVertex(this->pvertex);
}

void
SceneryP::gen_pre_cb(void * closure, ss_render_block_cb_info * info)
{
  SmScenery * thisp = (SmScenery*) closure; 

  RenderState & renderstate = PRIVATE(thisp)->renderstate;
  
  sc_ssglue_render_get_elevation_measures(info, 
                                   renderstate.voffset,
                                   renderstate.vspacing,
                                   NULL,
                                   &renderstate.elevdata,
                                   &renderstate.normaldata,
                                   NULL);
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

#if 0
// view volume box vs terrain block bounding box culling
int
SmScenery::box_culling_pre_cb(void * closure, const double * bmin, const double * bmax)
{
  assert(closure);
  RenderState * renderstate = (RenderState *) closure;
  assert(renderstate->action && renderstate->state);

  SoAction * action = renderstate->action;
  assert(action->isOfType(SoGLRenderAction::getClassTypeId()) ||
         action->isOfType(SoCallbackAction::getClassTypeId()));

  SoState * state = action->getState();
  state->push();

  // just checking existing cull-bits - which is why the state is
  // pushed and popped all the time
  if ( SoCullElement::completelyInside(state) ) { return TRUE; }

  SbBox3f box((float) bmin[0], (float) bmin[1], (float) bmin[2],
              (float) bmax[0], (float) bmax[1], (float) bmax[2]);
  if ( !SoCullElement::cullBox(state, box, TRUE) ) { return TRUE; }

  return FALSE;
}

void
SmScenery::box_culling_post_cb(void * closure)
{
  assert(closure);
  RenderState * renderstate = (RenderState *) closure;

  SoAction * action = renderstate->action;
  assert(action->isOfType(SoGLRenderAction::getClassTypeId()) ||
         action->isOfType(SoCallbackAction::getClassTypeId()));

  SoState * state = action->getState();
  state->pop();
}

// ray vs terrain block bounding box culling
int
SmScenery::ray_culling_pre_cb(void * closure, const double * bmin, const double * bmax)
{
  assert(closure);
  RenderState * renderstate = (RenderState *) closure;

  SoAction * action = renderstate->action;
  assert(action->isOfType(SoRayPickAction::getClassTypeId()));
  SoRayPickAction * rpaction = (SoRayPickAction *) action;

  SoState * state = rpaction->getState();
  state->push();

  SbBox3f box((float) bmin[0], (float) bmin[1], (float) bmin[2],
              (float) bmax[0], (float) bmax[1], (float) bmax[2]); 
  if (box.isEmpty()) return FALSE;
  rpaction->setObjectSpace();
  return rpaction->intersect(box, TRUE);
}

void
SmScenery::ray_culling_post_cb(void * closure)
{
  assert(closure);
  RenderState * renderstate = (RenderState *) closure;

  SoAction * action = renderstate->action;
  assert(action->isOfType(SoRayPickAction::getClassTypeId()));
  
  SoState * state = action->getState();
  state->pop();
}
#endif

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
  if (rs.bbmax[2] == rs.bbmin[2]) return abgr;

  if (thisp->colorTexturing.getValue() == SmScenery::INTERPOLATED) {
    if (thisp->colorElevation.getNum() == 0) {
      // interpolate color table evenly over elevation range
      float fac = (float) ((elevation - rs.bbmin[2]) /
                           (rs.bbmax[2] - rs.bbmin[2]));
      int steps = ((thisp->colorMap.getNum() / 4) - 1); // four components
      if (steps == 0) { // only one color given
        int nr = (int) (SbClamp(thisp->colorMap[0], 0.0f, 1.0f) * 255.0f);
        int ng = (int) (SbClamp(thisp->colorMap[1], 0.0f, 1.0f) * 255.0f);
        int nb = (int) (SbClamp(thisp->colorMap[2], 0.0f, 1.0f) * 255.0f);
        int na = (int) (SbClamp(thisp->colorMap[3], 0.0f, 1.0f) * 255.0f);
        abgr = (na << 24) | (nb << 16) | (ng << 8) | nr;
      }
      else if (steps > 0) { // interpolate color
        int startcolidx = (int) floor(float(steps) * fac);
        float rest = (float(steps) * fac) - float(startcolidx);
        float r = thisp->colorMap[startcolidx * 4 + 0];
        float g = thisp->colorMap[startcolidx * 4 + 1];
        float b = thisp->colorMap[startcolidx * 4 + 2];
        float a = thisp->colorMap[startcolidx * 4 + 3];
        if (rest > 0.0f) {
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
      if ((thisp->colorElevation.getNum() * 4) != thisp->colorMap.getNum()) {
        SoDebugError::postInfo("SmScenery::colortexture_cb", "size of colorElevation does not match size of colorMap");
        thisp->colorTexturing.setValue(SmScenery::DISABLED);
        return abgr;
      }
      // use elevation values to decide colors
      const int max = thisp->colorElevation.getNum();
      int i;
      for (i = 0; (i < max) && (elevation > thisp->colorElevation[i]); i++) { }
      if (i == 0) {
        // first color
        int nr = (int) (SbClamp(thisp->colorMap[0], 0.0f, 1.0f) * 255.0f);
        int ng = (int) (SbClamp(thisp->colorMap[1], 0.0f, 1.0f) * 255.0f);
        int nb = (int) (SbClamp(thisp->colorMap[2], 0.0f, 1.0f) * 255.0f);
        int na = (int) (SbClamp(thisp->colorMap[3], 0.0f, 1.0f) * 255.0f);
        abgr = (na << 24) | (nb << 16) | (ng << 8) | nr;
      }
      else if (i == max) {
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
  else if (thisp->colorTexturing.getValue() == SmScenery::DISCRETE) {
    if (thisp->colorElevation.getNum() == 0) {
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
      if (((max + 1) * 4) != (thisp->colorMap.getNum())) {
        SoDebugError::postInfo("SmScenery::colortexture_cb", "size of colorElevation does not match size of colorMap");
        thisp->colorTexturing.setValue(SmScenery::DISABLED);
        return abgr;
      }
      // use elevation values to decide colors
      int i;
      for (i = 1; (i <= max) && (elevation > thisp->colorElevation[i-1]); i++) { }
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

SbBool 
SmScenery::getElevation(const double tx, const double ty, float & elev)
{
  if (PRIVATE(this)->system) {
    double origo[3];
    sc_ssglue_system_get_origo_world_position(PRIVATE(this)->system, origo);
    double x = tx - origo[0];
    double y = ty - origo[1];
    double bmin[3];
    double bmax[3];
    sc_ssglue_system_get_object_box(PRIVATE(this)->system, bmin, bmax);
    if (x >= bmin[0] && x < bmax[0] && y >= bmin[1] && y < bmax[1]) {
      double pos[3];
      pos[0] = tx;
      pos[1] = ty;
      pos[2] = 0.0;

      float normal[3];
      uint32_t rgba;
      int datasets[1] = { 0 };
      int n = sc_ssglue_system_get_elevation(PRIVATE(this)->system, 1, 
                                             datasets, 1, 
                                             pos, normal, &rgba,
                                             NULL,
                                             SS_USE_RESIDENT_ONLY);
      if (n == 1) {
        elev = (float) pos[2];
        return TRUE;
      }
    }
  }
  return FALSE;
}

uint32_t
SceneryP::invokecolortexturecb(void * closure, double * pos, float elevation, double * spacing)
{
  assert(closure);
  SceneryP * thisp = (SceneryP *) closure;
  if (thisp->cbtexcb) {
    return thisp->cbtexcb(thisp->cbtexclosure, pos, elevation, spacing);
  }
  else {
    return 0xffffffff;
  }
}

/* ********************************************************************** */
