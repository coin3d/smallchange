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
#include <Inventor/sensors/SoFieldSensor.h>
#include <Inventor/system/gl.h>
#include <Inventor/misc/SoState.h>
#include <Inventor/misc/SoGLImage.h>
#include <Inventor/elements/SoCullElement.h>
#include <Inventor/elements/SoGLTextureImageElement.h>
#include <Inventor/elements/SoGLLazyElement.h>
#include <Inventor/elements/SoGLCacheContextElement.h>
#include <Inventor/elements/SoTextureQualityElement.h>
#include <Inventor/elements/SoViewportRegionElement.h>
#include <Inventor/SoInput.h>
#include <Inventor/lists/SbStringList.h>

#include <SmallChange/misc/SceneryGlue.h>
#include <SmallChange/nodes/Scenery.h>

#define MAX_UNUSED_COUNT 200

// FIXME: implement rayPick() method
// FIXME: not thread safe. Need one view per thread (use thread-local-storage).

#define SS_IMPORT_XYZ 1
#define SS_RTTEXTURE2D_TEST 0

SO_NODE_SOURCE(Scenery);

Scenery::Scenery(void)
{
  SO_NODE_CONSTRUCTOR(Scenery);
  
  SO_NODE_ADD_FIELD(filename, (""));
  SO_NODE_ADD_FIELD(renderSequence, (-1));
  SO_NODE_ADD_FIELD(blockRottger, (20.0f));
  SO_NODE_ADD_FIELD(loadRottger, (16.0f));
  SO_NODE_ADD_FIELD(visualDebug, (FALSE));

  SO_NODE_ADD_FIELD(colorTexture, (FALSE));
  SO_NODE_ADD_FIELD(colorMap, (0.0f));

  this->renderSequence.setNum(0);
  this->colorMap.setNum(0);
  this->colormaptexid = -1;

  this->filenamesensor = new SoFieldSensor(filenamesensor_cb, this);
  this->filenamesensor->attach(&this->filename);
  this->filenamesensor->setPriority(0);

  this->blocksensor = new SoFieldSensor(blocksensor_cb, this);
  this->blocksensor->attach(&this->blockRottger);

  this->loadsensor = new SoFieldSensor(loadsensor_cb, this);
  this->loadsensor->attach(&this->loadRottger);

  this->colormapsensor = new SoFieldSensor(colormapsensor_cb, this);
  this->colormapsensor->attach(&this->colorMap);
  this->colortexturesensor = new SoFieldSensor(colormapsensor_cb, this);
  this->colortexturesensor->attach(&this->colorTexture);
  
  // FIXME: view-specific. Move to struct.
  this->pvertex = new SoPrimitiveVertex;
  this->facedetail = new SoFaceDetail;
  this->currhotspot = SbVec3f(0.0f, 0.0f, 0.0f);
  this->system = NULL;
  this->viewid = -1;
  this->texhash = cc_hash_construct(1024, 0.7f);
}

Scenery::~Scenery()
{
  delete this->filenamesensor;
  delete this->blocksensor;
  delete this->loadsensor;
  delete this->colormapsensor;

  if (sc_scenery_available() && this->system) {
    sc_ssglue_view_deallocate(this->system, this->viewid);
    sc_ssglue_system_close(this->system);
  }
  delete this->pvertex;
  delete this->facedetail;
  cc_hash_apply(this->texhash, hash_clear, NULL);
  cc_hash_destruct(this->texhash);
}

void
Scenery::initClass(void)
{
  static int first = 1;
  if (first) {
    first = 0;
    if ( sc_scenery_available() ) {
      sc_ssglue_initialize();
    }
    SO_NODE_INIT_CLASS(Scenery, SoShape, "Shape");
  }
}

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
Scenery::render_pre_cb(void * closure, ss_render_pre_cb_info * info)
{
  if ( sc_scenery_available() ) {
  Scenery * thisp = (Scenery*) closure; 
  SoState * state = thisp->currstate;
  
  RenderState & renderstate = thisp->renderstate;
  
  sc_ssglue_render_get_elevation_measures(info, 
                                   renderstate.voffset,
                                   renderstate.vspacing,
                                   &renderstate.elevdata,
                                   &renderstate.normaldata);

  float ox = renderstate.voffset[0] / thisp->bboxmax[0];
  float oy = renderstate.voffset[1] / thisp->bboxmax[1];
  float sx = renderstate.vspacing[0] * renderstate.blocksize;
  float sy = renderstate.vspacing[1] * renderstate.blocksize;

  sx /= thisp->bboxmax[0];
  sy /= thisp->bboxmax[1];

  thisp->debuglist.append(ox);
  thisp->debuglist.append(oy);
  thisp->debuglist.append(ox+sx);
  thisp->debuglist.append(oy+sy);

  sc_ssglue_render_get_texture_measures(info,
                                 &renderstate.texid,
                                 renderstate.toffset,
                                 renderstate.tscale);
  
  if (thisp->dotex && renderstate.texid) {
    if (renderstate.texid != thisp->currtexid) {      
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
        thisp->numnewtextures++;
      }
      image->getGLDisplayList(state)->call(state);
      thisp->currtexid = renderstate.texid;
    }
    else {
      //      fprintf(stderr,"reused tex\n");
    }
    if (!thisp->texisenabled) {
      glEnable(GL_TEXTURE_2D);
      thisp->texisenabled = TRUE;
    }
  }
  else {
    if (thisp->texisenabled) {
      glDisable(GL_TEXTURE_2D);
      thisp->texisenabled = FALSE;
    }
  }
  return 1;
  } else {
    return 0;
  }
}

void 
Scenery::GLRender(SoGLRenderAction * action)
{
  if ( sc_scenery_available() ) {
  if (this->system == NULL) return;
  if (!this->shouldGLRender(action)) return;

  //  sc_ssglue_view_set_evaluate_rottger_parameters(this->system, this->viewid, 16.0f, 400.0f);

  SoState * state = action->getState();
  SbVec2s vpsize = SoViewportRegionElement::get(state).getViewportSizePixels();
  state->push();
  
  float quality = SoTextureQualityElement::get(state);
  this->dotex = quality > 0.0f;
  SbVec3f campos = SoViewVolumeElement::get(state).getProjectionPoint();

  //  transform into local coordinate system
  SbMatrix worldtolocal =
    SoModelMatrixElement::get(state).inverse();
  worldtolocal.multVecMatrix(campos, campos);

  SoCacheElement::invalidate(state);
  SoMaterialBundle mb(action);
  mb.sendFirst();

  SbBool texwasenabled = glIsEnabled(GL_TEXTURE_2D);
  this->texisenabled = texwasenabled;
  this->currtexid = 0;

  this->currhotspot = campos;
  this->curraction = action;
  this->currstate = state;

  sc_ssglue_view_set_render_callback(this->system, this->viewid,
                              render_cb, this);
  sc_ssglue_view_set_undef_render_callback(this->system, this->viewid,
                                    undefrender_cb, this); 
  sc_ssglue_view_set_render_pre_callback(this->system, this->viewid,
                                  render_pre_cb, this);
  sc_ssglue_view_set_culling_pre_callback(this->system, this->viewid,
                                   pre_block_cb, state);
  sc_ssglue_view_set_culling_post_callback(this->system, this->viewid,
                                    post_block_cb, state);
  double hotspot[3];
  hotspot[0] = campos[0];
  hotspot[1] = campos[1];
  hotspot[2] = campos[2];

  sc_ssglue_view_set_hotspots(this->system, this->viewid, 1, hotspot); 
  this->debuglist.truncate(0);
  this->numnewtextures = 0;

  const int sequencelen = this->renderSequence.getNum();
  if ( this->colorTexture.getValue() && this->colormaptexid != -1 ) {
    // FIXME: add runtime colortexture
    int localsequence[2] = { this->colormaptexid, 0 };
    sc_ssglue_view_set_render_sequence_a(this->system, this->viewid, 2, localsequence);
  } else if ( sequencelen == 0 ) {
    sc_ssglue_view_set_render_sequence_a(this->system, this->viewid, 0, NULL);
  } else {
    this->renderSequence.enableNotify(FALSE);
    int * sequence = this->renderSequence.startEditing();
    sc_ssglue_view_set_render_sequence_a(this->system, this->viewid, sequencelen, sequence);
    this->renderSequence.finishEditing();
    this->renderSequence.enableNotify(TRUE);
  }
  sc_ssglue_view_render(this->system, this->viewid);
//   fprintf(stderr,"num boxes: %d, new texs: %d\n",
//           this->debuglist.getLength()/4, this->numnewtextures);
  
  SbBool texisenabled = glIsEnabled(GL_TEXTURE_2D);
  if (texisenabled != texwasenabled) {
    if (texwasenabled) glEnable(GL_TEXTURE_2D);
    else glDisable(GL_TEXTURE_2D);
  } 
  SoGLLazyElement::getInstance(state)->reset(state, SoLazyElement::GLIMAGE_MASK);

  state->pop();
  sc_ssglue_view_set_culling_pre_callback(this->system, this->viewid,
                                   NULL, NULL);
  sc_ssglue_view_set_culling_post_callback(this->system, this->viewid,
                                    NULL, NULL);

  if (this->visualDebug.getValue()) {
    campos[0] /= this->bboxmax[0];
    campos[1] /= this->bboxmax[1];
    
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
    
    int num = this->debuglist.getLength() / 4;
    int i;
    
    float mind = 1.0f;
    
    for (i = 0; i < num; i++) {
      float x0, x1;
      x0 = this->debuglist[i*4];
      x1 = this->debuglist[i*4+2];
      
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
      x0 = this->debuglist[i*4] - 0.5f;
      y0 = this->debuglist[i*4+1] - 0.5f;
      x1 = this->debuglist[i*4+2] - 0.5f;
      y1 = this->debuglist[i*4+3] - 0.5f;
      
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
Scenery::rayPick(SoRayPickAction * action)
{
  if ( sc_scenery_available() ) {
  sc_ssglue_view_set_culling_pre_callback(this->system, this->viewid,
                                   raypick_pre_cb, action);
  sc_ssglue_view_set_culling_post_callback(this->system, this->viewid,
                                    raypick_post_cb, action);

  inherited::rayPick(action); // just generate primitives
  
  sc_ssglue_view_set_culling_pre_callback(this->system, this->viewid,
                                   NULL, NULL);
  sc_ssglue_view_set_culling_post_callback(this->system, this->viewid,
                                    NULL, NULL);
  }
}

void 
Scenery::callback(SoCallbackAction * action)
{
  if ( sc_scenery_available() ) {
  // FIXME: not correct to just evaluate in the callback()
  // method. SoCallbackAction can be executed for a a number of
  // reasons. Consider creating a SoSimlaEvaluateAction or something...
  if (this->system == NULL) return;
  SoState * state = action->getState();

  SbVec3f campos = SoViewVolumeElement::get(state).getProjectionPoint();
  //  transform into local coordinate system
  SbMatrix worldtolocal =
    SoModelMatrixElement::get(state).inverse();
  worldtolocal.multVecMatrix(campos, campos);
  this->currhotspot = campos;

  sc_ssglue_view_set_culling_pre_callback(this->system, this->viewid,
                                     pre_block_cb, state);
  sc_ssglue_view_set_culling_post_callback(this->system, this->viewid,
                                      post_block_cb, state);
  sc_ssglue_view_set_render_pre_callback(this->system, this->viewid,
                                  NULL, this);
  double hotspot[3];
  hotspot[0] = campos[0];
  hotspot[1] = campos[1];
  hotspot[2] = campos[2];
  sc_ssglue_view_set_hotspots(this->system, this->viewid, 1, hotspot); 
  sc_ssglue_view_evaluate(this->system, this->viewid);

  sc_ssglue_view_set_culling_pre_callback(this->system, this->viewid,
                                   NULL, NULL);
  sc_ssglue_view_set_culling_post_callback(this->system, this->viewid,
                                    NULL, NULL);
  }
}

void 
Scenery::generatePrimitives(SoAction * action)
{
  if ( sc_scenery_available() ) {
  if (this->system == NULL) return;
  SoState * state = action->getState();

  sc_ssglue_view_set_render_callback(this->system, this->viewid,
                              gen_cb, this);
  sc_ssglue_view_set_undef_render_callback(this->system, this->viewid,
                                    undefgen_cb, this);

  sc_ssglue_view_set_render_pre_callback(this->system, this->viewid,
                                  gen_pre_cb, this);

  SoPointDetail pointDetail;
  this->pvertex->setDetail(&pointDetail);
  this->curraction = action;
  sc_ssglue_view_render(this->system, this->viewid);
  }
}

void 
Scenery::computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center)
{
  if (this->system == NULL) return;  
  box = SbBox3f(this->bboxmin[0], this->bboxmin[1], this->bboxmin[2],
                this->bboxmax[0], this->bboxmax[1], this->bboxmax[2]);
  center = box.getCenter();
}

void 
Scenery::blocksensor_cb(void * data, SoSensor * sensor)
{
  Scenery * thisp = (Scenery*) data;
  thisp->setBlockRottger(thisp->blockRottger.getValue());
}

void 
Scenery::loadsensor_cb(void * data, SoSensor * sensor)
{
  Scenery * thisp = (Scenery*) data;
  thisp->setLoadRottger(thisp->loadRottger.getValue());
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
Scenery::colortexgen_cb(void * closure, double * pos, float elevation, double * spacing)
{
  Scenery * thisp = (Scenery *) closure;
  float fac = (elevation - thisp->bboxmin[2]) / (thisp->bboxmax[2] - thisp->bboxmin[2]);
  int steps = ((thisp->colorMap.getNum() / 4) - 1); // four components
  uint32_t abgr = 0xffffffff; // no colors means white
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
  return abgr;
}

void
Scenery::colormaptexchange(void)
{
  if ( this->colorTexture.getValue() ) {
    if ( this->colormaptexid != -1 ) {
      this->refreshTextures(this->colormaptexid);
    }
  }
}

void 
Scenery::colormapsensor_cb(void * data, SoSensor * sensor)
{
  Scenery * thisp = (Scenery *) data;
  thisp->colormaptexchange();
}

void 
Scenery::filenamesensor_cb(void * data, SoSensor * sensor)
{
  Scenery * thisp = (Scenery*) data;

  if ( sc_scenery_available() && thisp->system) {
    sc_ssglue_view_deallocate(thisp->system, thisp->viewid);
    sc_ssglue_system_close(thisp->system);
  }

  const SbStringList & pathlist = SoInput::getDirectories();

  thisp->viewid = -1;
  thisp->system = NULL;
  thisp->colormaptexid = -1;

  if ( sc_scenery_available() ) {
  SbString s = thisp->filename.getValue();
  if (s.getLength()) {
#if 0 && SS_IMPORT_XYZ
    if ( s.find(".xyz") == (s.getLength() - 4) ) {
      thisp->system = readxyz(s.getString());
    }
#endif
    if ( !thisp->system ) {
      int i;
      for ( i = 0; (thisp->system == NULL) && (i <= pathlist.getLength()); i++ ) {
        if ( i == pathlist.getLength() ) {
          thisp->system = sc_ssglue_system_open(s.getString(), 1);
        } else {
          SbString path = *(pathlist[i]);
          path += "/";
          path += s;
          thisp->system = sc_ssglue_system_open(path.getString(), 1);
        }
      }
    }
    if (!thisp->system) {
      fprintf(stderr,"Unable to open Scenery system\n");
    }
    else {
#if 0 && SS_RTTEXTURE2D_TEST
      if ( (sc_ssglue_system_get_num_datasets(thisp->system) == 1) &&
           (sc_ssglue_system_get_dataset_type(thisp->system, 0) == SS_ELEVATION_TYPE) ) {
        // only elevation data - fitting dataset to add runtime texture dataset to
        // for testing purposes...
        sc_ssglue_system_add_runtime_texture2d(thisp->system, 0, calctex_cb, thisp->system);
      }
#endif
      if ( (sc_ssglue_system_get_num_datasets(thisp->system) > 0) &&
           (sc_ssglue_system_get_dataset_type(thisp->system, 0) == SS_ELEVATION_TYPE) ) {
        thisp->colormaptexid = sc_ssglue_system_add_runtime_texture2d(thisp->system, 0, colortexgen_cb, thisp);
      }
      sc_ssglue_system_get_object_box(thisp->system, thisp->bboxmin, thisp->bboxmax); 
      thisp->blocksize = sc_ssglue_system_get_blocksize(thisp->system);
      thisp->renderstate.blocksize = (float) (thisp->blocksize-1);
      thisp->viewid = sc_ssglue_view_allocate(thisp->system);
      assert(thisp->viewid >= 0);
      sc_ssglue_view_enable(thisp->system, thisp->viewid);
      //      fprintf(stderr,"system: %p, viewid: %d\n", thisp->system, thisp->viewid);
    }
  }
  }
}

void 
Scenery::preFrame(void)
{
  if ( sc_scenery_available() && this->system ) {
    sc_ssglue_view_pre_frame(this->system, this->viewid);
    cc_hash_apply(this->texhash, hash_inc_unused, NULL);
  }
}

int 
Scenery::postFrame(void)
{
  if (sc_scenery_available() && this->system) {
    this->deleteUnusedTextures();
    return sc_ssglue_view_post_frame(this->system, this->viewid);
  }
  return 0;
}

void 
Scenery::setBlockRottger(const float c)
{
  if (sc_scenery_available() && this->system) {
    sc_ssglue_view_set_evaluate_rottger_parameters(this->system, this->viewid, 16.0f, c);
  }
}

float
Scenery::getBlockRottger(void) const
{
  if (sc_scenery_available() && this->system) {
    float C, c;
    sc_ssglue_view_get_evaluate_rottger_parameters(this->system, this->viewid, &C, &c);
    return c;
  }
  return 0.0f;
}

void 
Scenery::setLoadRottger(const float c)
{
  if (sc_scenery_available() && this->system) {
    sc_ssglue_view_set_load_rottger_parameters(this->system, this->viewid, 16.0f, c);
  }
}

float
Scenery::getLoadRottger(void) const
{
  if (sc_scenery_available() && this->system) {
    float C, c;
    sc_ssglue_view_get_load_rottger_parameters(this->system, this->viewid, &C, &c);
    return c;
  }
  return 0.0f;
}

void 
Scenery::refreshTextures(const int id)
{
  if (sc_scenery_available() && this->system) {
    sc_ssglue_system_refresh_runtime_texture2d(this->system, id);

    this->tmplist.truncate(0);
    cc_hash_apply(this->texhash, hash_add_all, &this->tmplist);

    for (int i = 0; i < this->tmplist.getLength(); i++) {
      void * tmp;
      if (cc_hash_get(this->texhash, this->tmplist[i], &tmp)) {
        TexInfo * tex = (TexInfo*) tmp;
        this->reusetexlist.push(tex);
        (void) cc_hash_remove(this->texhash, this->tmplist[i]);
      }
      else {
        assert(0 && "huh");
      }
    }
  }
}


int 
Scenery::gen_pre_cb(void * closure, ss_render_pre_cb_info * info)
{
  Scenery * thisp = (Scenery*) closure; 

  RenderState & renderstate = thisp->renderstate;
  
  sc_ssglue_render_get_elevation_measures(info, 
                                   renderstate.voffset,
                                   renderstate.vspacing,
                                   &renderstate.elevdata,
                                   &renderstate.normaldata);
  return 1;
}

////////////// render ///////////////////////////////////////////////////////////

inline void 
GL_VERTEX_OLD(Scenery::RenderState * state, const int x, const int y, const float elev)
{
  glVertex3f((float) (x*state->vspacing[0] + state->voffset[0]),
             (float) (y*state->vspacing[1] + state->voffset[1]),
             elev);
}

inline void 
GL_VERTEX(Scenery::RenderState * state, const int x, const int y, const float elev)
{
  glTexCoord2f(state->toffset[0] + (float(x)/state->blocksize) * state->tscale[0],
               state->toffset[1] + (float(y)/state->blocksize) * state->tscale[1]);
  glVertex3f((float) (x*state->vspacing[0] + state->voffset[0]),
             (float)(y*state->vspacing[1] + state->voffset[1]),
             elev);
}


inline void 
GL_VERTEX_N(Scenery::RenderState * state, const int x, const int y, const float elev, const signed char * n)
{
  glNormal3bv((const GLbyte *)n);
  glVertex3f((float) (x*state->vspacing[0] + state->voffset[0]),
             (float) (y*state->vspacing[1] + state->voffset[1]),
             elev);
}

inline void 
GL_VERTEX_TN(Scenery::RenderState * state, const int x, const int y, const float elev, const signed char * n)
{
  glNormal3bv((const GLbyte *)n);
  glTexCoord2f(state->toffset[0] + (float(x)/state->blocksize) * state->tscale[0],
               state->toffset[1] + (float(y)/state->blocksize) * state->tscale[1]);
  glVertex3f((float) (x*state->vspacing[0] + state->voffset[0]),
             (float) (y*state->vspacing[1] + state->voffset[1]),
             elev);
}


#define ELEVATION(x,y) elev[(y)*W+(x)]    


void 
Scenery::undefrender_cb(void * closure, const int x, const int y, const int len, 
                           const unsigned int bitmask_org)
{
  Scenery * thisp = (Scenery*) closure; 

  Scenery::RenderState * renderstate = &thisp->renderstate;
  const signed char * normals = renderstate->normaldata;
  const float * elev = renderstate->elevdata;
  const int W = thisp->blocksize;

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
Scenery::render_cb(void * closure, const int x, const int y,
                      const int len, const unsigned int bitmask)
{
  Scenery * thisp = (Scenery*) closure;
  
  Scenery::RenderState * renderstate = &thisp->renderstate;

  const signed char * normals = renderstate->normaldata;  
  const float * elev = renderstate->elevdata;
  const int W = thisp->blocksize;

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
Scenery::GEN_VERTEX(Scenery::RenderState * state, const int x, const int y, const float elev)
{
  this->pvertex->setPoint(SbVec3f(x*state->vspacing[0] + state->voffset[0],
                                  y*state->vspacing[1] + state->voffset[1],
                                  elev));
  this->shapeVertex(this->pvertex);
}

void 
Scenery::undefgen_cb(void * closure, const int x, const int y, const int len, 
                     const unsigned int bitmask_org)
{
  Scenery * thisp = (Scenery*) closure; 

  Scenery::RenderState * renderstate = &thisp->renderstate;
  const signed char * normals = renderstate->normaldata;
  const float * elev = renderstate->elevdata;
  const int W = thisp->blocksize;

#define ELEVATION(x,y) elev[(y)*W+(x)]

  const signed char * ptr = sc_ssglue_render_get_undef_array(bitmask_org);

  int numv = *ptr++;
  int tx, ty;

  if (normals) {
    int idx;
#define SEND_VERTEX(state, x, y) \
    idx = (y)*W + x; \
    thisp->GEN_VERTEX(state, x, y, elev[idx]/*, normals+3*idx*/);

    while (numv) {
      thisp->beginShape(thisp->curraction, TRIANGLE_FAN, thisp->facedetail);
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
      thisp->beginShape(thisp->curraction, TRIANGLE_FAN, thisp->facedetail);
      while (numv) { 
        tx = x + *ptr++ * len;
        ty = y + *ptr++ * len;
        thisp->GEN_VERTEX(renderstate, tx, ty, ELEVATION(tx, ty));
        numv--;
      }
      numv = *ptr++;
      thisp->endShape();
    }
  }
}

void 
Scenery::gen_cb(void * closure, const int x, const int y,
                   const int len, const unsigned int bitmask)
{
  Scenery * thisp = (Scenery*) closure;

  Scenery::RenderState * renderstate = &thisp->renderstate;
  const signed char * normals = renderstate->normaldata;
  const float * elev = renderstate->elevdata;
  const int W = thisp->blocksize;

#define ELEVATION(x,y) elev[(y)*W+(x)]
  
  thisp->beginShape(thisp->curraction, TRIANGLE_FAN, thisp->facedetail);
  thisp->GEN_VERTEX(renderstate, x, y, ELEVATION(x, y));
  thisp->GEN_VERTEX(renderstate, x-len, y-len, ELEVATION(x-len, y-len));
  if (!(bitmask & SS_RENDER_BIT_SOUTH)) {
    thisp->GEN_VERTEX(renderstate, x, y-len, ELEVATION(x, y-len));
  }
  thisp->GEN_VERTEX(renderstate, x+len, y-len, ELEVATION(x+len, y-len));
  if (!(bitmask & SS_RENDER_BIT_EAST)) {
    thisp->GEN_VERTEX(renderstate, x+len, y, ELEVATION(x+len, y));
  }
  thisp->GEN_VERTEX(renderstate, x+len, y+len, ELEVATION(x+len, y+len));
  if (!(bitmask & SS_RENDER_BIT_NORTH)) {
    thisp->GEN_VERTEX(renderstate, x, y+len, ELEVATION(x, y+len));
  }
  thisp->GEN_VERTEX(renderstate, x-len, y+len, ELEVATION(x-len, y+len));
  if (!(bitmask & SS_RENDER_BIT_WEST)) {
    thisp->GEN_VERTEX(renderstate, x-len, y, ELEVATION(x-len, y));
  }
  thisp->GEN_VERTEX(renderstate, x-len, y-len, ELEVATION(x-len, y-len));
  thisp->endShape();
  
#undef ELEVATION
}

SoGLImage * 
Scenery::findReuseTexture(const unsigned int texid)
{
  void * tmp;
  if (cc_hash_get(this->texhash, texid, &tmp)) {
    TexInfo * tex = (TexInfo*) tmp;
    assert(tex->image);
    tex->unusedcount = 0;
    return tex->image;
  }
  return NULL;
}

void 
Scenery::deleteUnusedTextures(void)
{
  this->tmplist.truncate(0);
  cc_hash_apply(this->texhash, hash_check_unused, &this->tmplist);

  for (int i = 0; i < this->tmplist.getLength(); i++) {
    void * tmp;
    if (cc_hash_get(this->texhash, this->tmplist[i], &tmp)) {
      TexInfo * tex = (TexInfo*) tmp;
      this->reusetexlist.push(tex);
      (void) cc_hash_remove(this->texhash, this->tmplist[i]);
    }
    else {
      assert(0 && "huh");
    }
  }

//   fprintf(stderr,"Scenery now has %d active textures, %d reusable textures (removed %d)\n",
//           cc_hash_get_num_elements(this->texhash), this->reusetexlist.getLength(), this->tmplist.getLength());

  this->tmplist.truncate(0);
}

SoGLImage * 
Scenery::createTexture(const unsigned int texid)
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
  tex->texid = currtexid;
  tex->unusedcount = 0;

  (void) cc_hash_put(this->texhash, texid, tex);
  return tex->image;
}  

void 
Scenery::hash_clear(unsigned long key, void * val, void * closure)
{
  TexInfo * tex = (TexInfo*) val;
  // safe to do this here since we'll never use this list again
  assert(tex->image);
  tex->image->unref(NULL);
  delete tex;
}

void 
Scenery::hash_check_unused(unsigned long key, void * val, void * closure)
{  
  TexInfo * tex = (TexInfo*) val;
  if (tex->unusedcount > MAX_UNUSED_COUNT) {
    SbList <unsigned int> * keylist = (SbList <unsigned int> *) closure;
    keylist->append(key);
  }
}

void 
Scenery::hash_add_all(unsigned long key, void * val, void * closure)
{  
  TexInfo * tex = (TexInfo*) val;
  SbList <unsigned int> * keylist = (SbList <unsigned int> *) closure;
  keylist->append(key);
}

void 
Scenery::hash_inc_unused(unsigned long key, void * val, void * closure)
{
  TexInfo * tex = (TexInfo*) val;
  tex->unusedcount++;
}



