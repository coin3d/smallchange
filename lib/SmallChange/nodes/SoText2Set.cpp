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

/*!
  \class SoText2Set SoText2Set.h SmallChange/nodes/SoText2Set.h
  \brief The SoText2Set class is a node type for visualizing a set of 2D texts aligned with the camera plane.
  \ingroup nodes

  See documentation of Inventor/shapenodes/SoText2

  SoText2Set is identical to the built-in SoText2 node except for:

  - Each string is positioned independently in 3D space
  - Each string is aligned independently
  - Each string is rotated independently (in the camera plane)

  The main purpose of this node is optimization; by collecting all text
  that should be rendered with the same font settings in one SoText2Set
  node, overhead is reduced.

  A secondary purpose is to allow rotated rendering of 2D text.

  Note that if you want to render multi-line text, SoText2 is probably
  a better choice since that node takes care of vertical spacing
  automatically. With SoText2Set each string is positioned directly in
  3D space.

  FIXME:
  rayPick() does not function properly with rotated strings unless they
  are CENTERed.

  \sa SoText2
*/

#include "SoText2Set.h"
#include <Inventor/nodes/SoSubNode.h>

#include <Inventor/errors/SoDebugError.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#ifdef HAVE_GLX
#include <GL/glx.h>
#endif // HAVE_GLX

#ifdef __COIN__
#include <Inventor/system/gl.h>
#else // SGI/TGS Inventor
#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif // HAVE_WINDOWS_H
#include <GL/gl.h>
#endif // SGI/TGS Inventor

#include <Inventor/elements/SoViewVolumeElement.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoViewportRegionElement.h>
#include <Inventor/elements/SoFontNameElement.h>
#include <Inventor/elements/SoFontSizeElement.h>
#include <Inventor/elements/SoLazyElement.h>
#include <Inventor/elements/SoCullElement.h>
#include <Inventor/elements/SoGLCacheContextElement.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoGLLazyElement.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoRayPickAction.h>
#include <Inventor/bundles/SoMaterialBundle.h>
#include <Inventor/actions/SoGetPrimitiveCountAction.h>
#include <Inventor/details/SoTextDetail.h>
#include <Inventor/SoPickedPoint.h>
#include <Inventor/SbPlane.h>
#include <Inventor/SbLine.h>
#include <Inventor/SbString.h>
#include <Inventor/SbBox2s.h>
#include <Inventor/misc/SoGlyph.h>

#include "../misc/SbList.h"

#include <limits.h>
#include <string.h>
#include <stdio.h>

static const unsigned int NOT_AVAILABLE = UINT_MAX;

/*!
  \enum SoText2Set::Justification

  Enum contains the various options for how the horizontal text layout
  text should be done. Valid values are LEFT, RIGHT and CENTER.
*/
/*!
  \var SoMFString SoText2Set::string

  The set of strings to render. string[i] is rendered at position[i],
  justified according to justification[i] and rotated according
  to rotation[i].

  The default value of the field is a single empty string.
*/
/*!
  \var SoMFVec3f SoText2Set::position

  Position of each string in local coordinate space.
*/
/*!
  \var SoMFFloat SoText2Set::rotation

  Angle in radians between the text and the horisontal, in the camera
  plane. Positive direction is counter clockwise.
*/

/*!
  \var SoMFEnum SoText2Set::justification

  Decides how the horizontal layout of each text string is done.
*/

/*!
  \var SoSFBool SoText2Set::renderOutline

  When TRUE, text will be rendered as white text with a black border.
  Default value is FALSE.
*/

class SoText2SetP {
public:
  SoText2SetP(SoText2Set * master) : master(master) { }

  SoGlyph *** glyphs;
  SbVec2s ** positions;
  int * stringwidth;
  int * stringheight;
  SbList <SbBox2s> bboxes;
  SbVec2s ** charbboxes;
  int linecnt;
  int validarraydims;
  SbName prevfontname;
  float prevfontsize;
  SbBool hasbuiltglyphcache;
  SbBool dirty;

  SbBox3f stringBBox(SoState * s, unsigned int stringidx);
  void getQuad(SoState * state, SbVec3f & v0, SbVec3f & v1,
               SbVec3f & v2, SbVec3f & v3, unsigned int stringidx);

  void flushGlyphCache(const SbBool unrefglyphs);
  void buildGlyphCache(SoState * state);
  SbBool shouldBuildGlyphCache(SoState * state);
  void dumpGlyphCache();
  void dumpBuffer(unsigned char * buffer, SbVec2s size, SbVec2s pos);

  SoText2Set::Justification getJustification(int idx) const;
  float getRotation(int idx) const;

private:
  SoText2Set * master;
};

// *************************************************************************

#undef PRIVATE
#define PRIVATE(obj) ((obj)->pimpl)
#undef PUBLIC
#define PUBLIC(obj) ((obj)->master)

SO_NODE_SOURCE(SoText2Set);

/*!
  Constructor.
*/
SoText2Set::SoText2Set(void)
{
  PRIVATE(this) = new SoText2SetP(this);
  PRIVATE(this)->glyphs = NULL;
  PRIVATE(this)->positions = NULL;
  PRIVATE(this)->charbboxes = NULL;
  PRIVATE(this)->linecnt = 0;
  PRIVATE(this)->validarraydims = 0;
  PRIVATE(this)->stringwidth = NULL;
  PRIVATE(this)->stringheight = NULL;
  PRIVATE(this)->bboxes.truncate(0);
  PRIVATE(this)->prevfontname = SbName("");
  PRIVATE(this)->prevfontsize = 0.0;
  PRIVATE(this)->hasbuiltglyphcache = FALSE;
  PRIVATE(this)->dirty = TRUE;

  SO_NODE_CONSTRUCTOR(SoText2Set);

  SO_NODE_ADD_FIELD(position, (SbVec3f(0.0f, 0.0f, 0.0f)));
  SO_NODE_ADD_FIELD(rotation, (0.0f));
  SO_NODE_ADD_FIELD(justification, (SoText2Set::LEFT));
  SO_NODE_ADD_FIELD(string, (""));
  SO_NODE_ADD_FIELD(renderOutline, (FALSE));

  SO_NODE_DEFINE_ENUM_VALUE(Justification, LEFT);
  SO_NODE_DEFINE_ENUM_VALUE(Justification, RIGHT);
  SO_NODE_DEFINE_ENUM_VALUE(Justification, CENTER);
  SO_NODE_SET_MF_ENUM_TYPE(justification, Justification);
}

/*!
  Destructor.
*/
SoText2Set::~SoText2Set()
{
  PRIVATE(this)->flushGlyphCache(TRUE);
  delete PRIVATE(this);
}

// doc in super
void
SoText2Set::initClass(void)
{
  SO_NODE_INIT_CLASS(SoText2Set, SoShape, SoShape);
}


// **************************************************************************


// doc in super
void
SoText2Set::GLRender(SoGLRenderAction * action)
{
  if (!this->shouldGLRender(action)) return;

  SoState * state = action->getState();

  state->push();
  SoLazyElement::setLightModel(state, SoLazyElement::BASE_COLOR);

  SbBool outline = this->renderOutline.getValue();
  if (outline) {
    glPushAttrib(GL_DEPTH_BUFFER_BIT);
    glDepthFunc(GL_LEQUAL);
  }

  // Render using SoGlyphs
  PRIVATE(this)->buildGlyphCache(state);

  // Try to cull entire node first, each string will also be attempted
  // culled away later on.
  SbBox3f box;
  SbVec3f center;
  this->computeBBox(action, box, center);

  if (!SoCullElement::cullTest(state, box, SbBool(TRUE))) {
    SoMaterialBundle mb(action);
    mb.sendFirst();
    const SbMatrix & mat = SoModelMatrixElement::get(state);
    const SbViewVolume & vv = SoViewVolumeElement::get(state);
    const SbViewportRegion & vp = SoViewportRegionElement::get(state);
    const SbVec2s vpsize = vp.getViewportSizePixels();
    // Set new state.
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, vpsize[0], 0, vpsize[1], -1.0f, 1.0f);
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);

    // Find number of strings to render
    const unsigned int stringcnt = this->string.getNum();
   
    // FIXME: Is this warning enough? Or should there be a warning at
    // all? (20040206 handegar)
    if (stringcnt > (unsigned int)this->position.getNum())
      SoDebugError::postWarning("SoText2Set::GLRender", "Position not specfied for all the strings.");


    for (unsigned int i = 0; i < stringcnt; i++) {

      SbVec3f nilpoint;
      if (i < (unsigned int)this->position.getNum())
        nilpoint = this->position[i];
      else
        nilpoint = SbVec3f(0 ,0, 0); // Default position

      // transform to world coordinate system
      mat.multVecMatrix(nilpoint, nilpoint);

      // FIXME: should make this selection available in public API?
      //
      // Note that the View'EM application currently depends on the
      // point-culling to be the default behavior.
      //
      // Note also that point-culling nullifies the implemented
      // feature of having strings partially disappear on the
      // left-side and top borders of the rendering canvas.
      //
      // 20031222 mortene.
      
#if 1
      // Frustum cull each string, checking just its position point.
      const SbBox3f stringbbox(nilpoint, nilpoint);
      // FIXME: there should be a
      // SoCullElement::cullTest(..,SbVec3f,...) method. 20031222 mortene.
      if (SoCullElement::cullTest(state, stringbbox, TRUE)) { continue; }
#else
      // This culls versus the whole string, i.e. if just a single
      // piece of the string is within the view volume (+ other
      // clipping planes), the (full) string will be shown.
      const SbBox3f stringbbox = PRIVATE(this)->stringBBox(state, i);
      if (SoCullElement::cullTest(state, stringbbox, TRUE)) { continue; }
#endif

      vv.projectToScreen(nilpoint, nilpoint);
      nilpoint[2] *= 2.0f;
      nilpoint[2] -= 1.0f;
      nilpoint[0] = nilpoint[0] * float(vpsize[0]);
      nilpoint[1] = nilpoint[1] * float(vpsize[1]);
      float xpos = nilpoint[0];
      float ypos = nilpoint[1];

      const unsigned int charcnt = this->string[i].getLength();
      switch (PRIVATE(this)->getJustification(i)) {
      case SoText2Set::LEFT:
        // No action
        break;
      case SoText2Set::RIGHT:
        xpos -= PRIVATE(this)->stringwidth[i];
        ypos -= PRIVATE(this)->positions[i][charcnt-1][1];
        break;
      case SoText2Set::CENTER:
        xpos -= PRIVATE(this)->stringwidth[i]/2.0f;
        ypos -= PRIVATE(this)->stringheight[i]/2.0f;
        break;
      }

      for (unsigned int i2 = 0; i2 < charcnt; i2++) {
        SbVec2s thispos;
        SbVec2s thissize;
        unsigned char * buffer = PRIVATE(this)->glyphs[i][i2]->getBitmap(thissize, thispos, SbBool(FALSE));

        int ix = thissize[0];
        int iy = thissize[1];
        SbVec2s position = PRIVATE(this)->positions[i][i2];
        float fx = (float)position[0];
        float fy = (float)position[1];

#define RENDER_TEXT(offx, offy) \
        do { \
          const float rasterx = xpos + fx + float(offx); \
          const float rpx = rasterx >= 0 ? rasterx : 0; \
          unsigned int offvp = rasterx < 0 ? 1 : 0; \
          const float offsetx = rasterx >= 0 ? 0 : rasterx; \
          const float rastery = ypos + fy + float(offy); \
          const float rpy = rastery >= 0 ? rastery : 0; \
          offvp = offvp || rastery < 0 ? 1 : 0; \
          const float offsety = rastery >= 0 ? 0 : rastery; \
          glRasterPos3f(rpx, rpy, -nilpoint[2]); \
          if (offvp) glBitmap(0,0,0,0,offsetx,offsety,NULL); \
          if (buffer) glBitmap(ix,iy,0,0,0,0,(const GLubyte *)buffer); \
        } while (0);

        if (outline) {
          // FIXME: should it be possible to specify colors for
          // outline and base color? pederb, 2003-12-12
          glColor3f(0.0f, 0.0f, 0.0f);
          RENDER_TEXT(-1,-1);
          RENDER_TEXT(0,-1);
          RENDER_TEXT(1,-1);
          RENDER_TEXT(1,0);
          RENDER_TEXT(1,1);
          RENDER_TEXT(0,1);
          RENDER_TEXT(-1,1);
          RENDER_TEXT(-1,0);
          glColor3f(1.0f, 1.0f, 1.0f);
          RENDER_TEXT(0,0);
        }
        else {
          RENDER_TEXT(0,0);
        }
      }
    }

#undef RENDER_TEXT

    glPixelStorei(GL_UNPACK_ALIGNMENT,4);
    // Pop old GL state.
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
  }

  state->pop();

  if (outline) {
    SoGLLazyElement::getInstance(state)->reset(state, SoLazyElement::DIFFUSE_MASK);
    glPopAttrib(); // pop depth func
  }

  // don't auto cache SoText2Set nodes.
  SoGLCacheContextElement::shouldAutoCache(action->getState(),
                                           SoGLCacheContextElement::DONT_AUTO_CACHE);
}

// **************************************************************************


// doc in super
//
// This will cause a cache dependency on the view volume, model matrix
// and viewport.
void
SoText2Set::computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center)
{
  box.makeEmpty();
  for (unsigned int i = 0; i < (unsigned int)this->string.getNum(); i++) {
    box.extendBy(PRIVATE(this)->stringBBox(action->getState(), i));
  }
  center = box.getCenter(); 
}

SbBox3f
SoText2SetP::stringBBox(SoState * s, unsigned int stringidx)
{
  SbVec3f v[4];
  this->getQuad(s, v[0], v[1], v[2], v[3], stringidx);

  SbBox3f box;
  box.makeEmpty();
  for (unsigned int i = 0; i < 4; i++) { box.extendBy(v[i]); }
  return box;
}

// doc in super
void
SoText2Set::rayPick(SoRayPickAction * action)
{
  if (!this->shouldRayPick(action)) return;
  SoState * state = action->getState();
  PRIVATE(this)->buildGlyphCache(state);

  state->push();
  action->setObjectSpace();
  SbVec3f v0, v1, v2, v3;

  for (unsigned int stringidx = 0; stringidx < (unsigned int)this->string.getNum(); stringidx++) {
    PRIVATE(this)->getQuad(state, v0, v1, v2, v3, stringidx);

    if (v0 == v1 || v0 == v3)
      continue; // empty

    SbVec3f isect;
    SbVec3f bary;
    SbBool front;
    SbBool hit = action->intersect(v0, v1, v2, isect, bary, front);

    if (!hit)
      hit = action->intersect(v0, v2, v3, isect, bary, front);

    if (hit && action->isBetweenPlanes(isect)) {
      // FIXME: account for pivot point position in quad. preng 2003-04-01.
      // find normalized 2D hitpoint on quad
      float h = (v3-v0).length();
      float w = (v1-v0).length();
      SbLine horiz(v2,v3);
      SbVec3f ptonline = horiz.getClosestPoint(isect);
      float vdist = (ptonline-isect).length();
      vdist /= h;

      SbLine vert(v0,v3);
      ptonline = vert.getClosestPoint(isect);
      float hdist = (ptonline-isect).length();
      hdist /= w;

      // find the character
      int charidx = -1;
      int strlength = this->string[stringidx].getLength();
      short minx, miny, maxx, maxy;
      PRIVATE(this)->bboxes[stringidx].getBounds(minx, miny, maxx, maxy);
      float bbwidth = (float)(maxx - minx);
      float bbheight = (float)(maxy - miny);
      float charleft, charright, charbottom, chartop;
      SbVec2s thissize, thispos;

      for (int i=0; i<strlength; i++) {
        PRIVATE(this)->glyphs[stringidx][i]->getBitmap(thissize, thispos, SbBool(FALSE));
        charleft = (PRIVATE(this)->positions[stringidx][i][0] - minx) / bbwidth;
        charright = (PRIVATE(this)->positions[stringidx][i][0] + PRIVATE(this)->charbboxes[stringidx][i][0] - minx) / bbwidth;

        if (hdist >= charleft && hdist <= charright) {
          chartop = (maxy - PRIVATE(this)->positions[stringidx][i][1] - PRIVATE(this)->charbboxes[stringidx][i][1]) / bbheight;
          charbottom = (maxy - PRIVATE(this)->positions[stringidx][i][1]) / bbheight;

          if (vdist >= chartop && vdist <= charbottom) {
            charidx = i;
            i = strlength;
          }
        }
      }

      if (charidx >= 0 && charidx < strlength) { // we have a hit!
        SoPickedPoint * pp = action->addIntersection(isect);
        if (pp) {
          SoTextDetail * detail = new SoTextDetail;
          detail->setStringIndex(stringidx);
          detail->setCharacterIndex(charidx);
          pp->setDetail(detail, this);
          pp->setMaterialIndex(0);
          pp->setObjectNormal(SbVec3f(0.0f, 0.0f, 1.0f));
        }
      }
    }
  }
  state->pop();
}

// doc in super
void
SoText2Set::getPrimitiveCount(SoGetPrimitiveCountAction *action)
{
  if (!this->shouldPrimitiveCount(action)) return;

  action->addNumText(this->string.getNum());
}

// doc in super
void
SoText2Set::generatePrimitives(SoAction * action)
{
  // This is supposed to be empty. There are no primitives.
}

/*!
  Overloaded to disable geometry cache.
*/
void
SoText2Set::notify(SoNotList * list)
{
  PRIVATE(this)->dirty = TRUE;
  inherited::notify(list);
}

// SoText2SetP methods below
#undef PRIVATE

// FIXME: kill the local caching -- should be unnecessary, caching is
// done (or at least should be done) at the glyph source. 20031215 mortene.

void
SoText2SetP::flushGlyphCache(const SbBool unrefglyphs)
{
  if (this->glyphs && validarraydims > 0) {
    free(this->stringwidth);
    free(this->stringheight);

    for (int i=0; i<this->linecnt; i++) {
      if (validarraydims == 2) {
        if (unrefglyphs) {
          for (int j=0; j<PUBLIC(this)->string[i].getLength(); j++) {
            if (this->glyphs[i][j])
              this->glyphs[i][j]->unref();
          }
        }
        free(this->positions[i]);
        free(this->charbboxes[i]);
      }
      free(this->glyphs[i]);
    }

    free(this->positions);
    free(this->charbboxes);
    free(this->glyphs);
    this->bboxes.truncate(0);
  }

  this->glyphs = NULL;
  this->positions = NULL;
  this->charbboxes = NULL;
  this->linecnt = 0;
  this->validarraydims = 0;
  this->stringwidth = NULL;
  this->stringheight = NULL;
}

// Debug convenience method.
void
SoText2SetP::dumpGlyphCache()
{
  // FIXME: pure debug method, remove. preng 2003-03-18.
  fprintf(stderr,"dumpGlyphCache: validarraydims=%d\n", validarraydims);
  if (this->glyphs && validarraydims > 0) {
    for (int i=0; i<this->linecnt; i++) {
      fprintf(stderr,"  stringwidth[%d]=%d\n", i, this->stringwidth[i]);
      fprintf(stderr,"  stringheight[%d]=%d\n", i, this->stringheight[i]);
      fprintf(stderr,"  string[%d]=%s\n", i, PUBLIC(this)->string[i].getString());
      if (validarraydims == 2) {
        for (int j = 0; j < (int) strlen(PUBLIC(this)->string[i].getString()); j++) {
          fprintf(stderr,"    glyph[%d][%d]=%p\n", i, j, this->glyphs[i][j]);
          fprintf(stderr,"    position[%d][%d]=(%d, %d)\n", i, j, this->positions[i][j][0], this->positions[i][j][1]);
        }
      }
    }
  }
}

SoText2Set::Justification
SoText2SetP::getJustification(int idx) const
{
  const int justnum = PUBLIC(this)->justification.getNum();
  if (idx >= justnum) { return SoText2Set::LEFT; }
  return (SoText2Set::Justification) PUBLIC(this)->justification[idx];
}

float
SoText2SetP::getRotation(int idx) const
{
  const int num = PUBLIC(this)->rotation.getNum();
  if (idx >= num) { return 0.0f; }
  return PUBLIC(this)->rotation[idx];
}

// Calculates a quad around the text in 3D.
void
SoText2SetP::getQuad(SoState * state, SbVec3f & v0, SbVec3f & v1,
                     SbVec3f & v2, SbVec3f & v3, unsigned int stringidx)
{

  this->buildGlyphCache(state);

  SbVec3f nilpoint;
  if (stringidx >= (unsigned int)PUBLIC(this)->position.getNum())
    nilpoint = SbVec3f(0.0f, 0.0f, 0.0f); // Default position
  else
    nilpoint = PUBLIC(this)->position[stringidx];

  const SbMatrix & mat = SoModelMatrixElement::get(state);
  mat.multVecMatrix(nilpoint, nilpoint);

  const SbViewVolume &vv = SoViewVolumeElement::get(state);

  SbVec3f screenpoint;
  vv.projectToScreen(nilpoint, screenpoint);

  const SbViewportRegion & vp = SoViewportRegionElement::get(state);
  SbVec2s vpsize = vp.getViewportSizePixels();

  SbVec2f n0, n1, n2, n3;
  short xmin, ymin, xmax, ymax;
  this->bboxes[stringidx].getBounds(xmin, ymin, xmax, ymax);
    
  SbVec2f sp((float) screenpoint[0], (float) screenpoint[1]);  
  n0 = SbVec2f(sp[0] + ((float) xmin)/float(vpsize[0]),
               sp[1] + ((float) ymax)/float(vpsize[1]));
  n1 = SbVec2f(sp[0] + ((float) xmax)/float(vpsize[0]), 
               sp[1] + ((float) ymax)/float(vpsize[1]));
  n2 = SbVec2f(sp[0] + ((float) xmax)/float(vpsize[0]), 
               sp[1] + ((float) ymin)/float(vpsize[1]));
  n3 = SbVec2f(sp[0] + ((float) xmin)/float(vpsize[0]), 
               sp[1] + ((float) ymin)/float(vpsize[1]));
  
  float w = n1[0]-n0[0];
  float halfw = w*0.5f;
  switch (this->getJustification(stringidx)) {
  case SoText2Set::LEFT:
    break;
  case SoText2Set::RIGHT:
    n0[0] -= w;
    n1[0] -= w;
    n2[0] -= w;
    n3[0] -= w;
    break;
  case SoText2Set::CENTER:
    n0[0] -= halfw;
    n1[0] -= halfw;
    n2[0] -= halfw;
    n3[0] -= halfw;
    break;
  default:
    assert(0 && "unknown alignment");
    break;
  }
  
  // get distance from nilpoint to camera plane
  float dist = -vv.getPlane(0.0f).getDistance(nilpoint);

  // find the four image points in the plane
  v0 = vv.getPlanePoint(dist, n0);
  v1 = vv.getPlanePoint(dist, n1);
  v2 = vv.getPlanePoint(dist, n2);
  v3 = vv.getPlanePoint(dist, n3);

  // transform back to object space
  SbMatrix inv = mat.inverse();
  inv.multVecMatrix(v0, v0);
  inv.multVecMatrix(v1, v1);
  inv.multVecMatrix(v2, v2);
  inv.multVecMatrix(v3, v3);

}

// Debug convenience method.
void
SoText2SetP::dumpBuffer(unsigned char * buffer, SbVec2s size, SbVec2s pos)
{
  // FIXME: pure debug method, remove. preng 2003-03-18.
  if (!buffer) {
    fprintf(stderr,"bitmap error: buffer pointer NULL.\n");
  } else {
    int rows = size[1];
    int bytes = size[0] >> 3;
    fprintf(stderr,"bitmap dump %d * %d bytes at %d, %d:\n", rows, bytes, pos[0], pos[1]);
    for (int y=rows-1; y>=0; y--) {
      for (int byte=0; byte<bytes; byte++) {
        for (int bit=0; bit<8; bit++)
          fprintf(stderr,"%d", buffer[y*bytes + byte] & 0x80>>bit ? 1 : 0);
      }
      fprintf(stderr,"\n");
    }
  }
}

SbBool
SoText2SetP::shouldBuildGlyphCache(SoState * state)
{
  if (!this->hasbuiltglyphcache)
    return TRUE;
  if (this->dirty)
    return TRUE;

  SbName curfontname = SoFontNameElement::get(state);
  float curfontsize = SoFontSizeElement::get(state);
  SbBool fonthaschanged = (this->prevfontname != curfontname
                           || this->prevfontsize != curfontsize);
  return fonthaschanged;
}


void
SoText2SetP::buildGlyphCache(SoState * state)
{
  if (!this->shouldBuildGlyphCache(state)) { return; }

  const char * s;
  int len;
  SbVec2s thissize, thispos;
  unsigned int idx;
  SbName curfontname;
  float curfontsize;
  float rotation;
  SbBox2s stringbox;

  SbBool outline = PUBLIC(this)->renderOutline.getValue();

  // FIXME: Must add same support for font naming as for
  // SoText2/3. I.e the use of "<font>:Italic" or "<font>:Bold
  // Italic" etc. (20031008 handegar)
  curfontname = SoFontNameElement::get(state);
  curfontsize = SoFontSizeElement::get(state);

  this->prevfontname = curfontname;
  this->prevfontsize = curfontsize;
  this->flushGlyphCache(FALSE);
  this->hasbuiltglyphcache = SbBool(TRUE);
  this->linecnt = PUBLIC(this)->string.getNum();
  this->validarraydims = 0;
  this->glyphs = (SoGlyph ***)malloc(this->linecnt*sizeof(SoGlyph*));
  this->positions = (SbVec2s **)malloc(this->linecnt*sizeof(SbVec2s*));
  this->charbboxes = (SbVec2s **)malloc(this->linecnt*sizeof(SbVec2s*));
  this->stringwidth = (int *)malloc(this->linecnt*sizeof(int));
  this->stringheight = (int *)malloc(this->linecnt*sizeof(int));

  memset(this->glyphs, 0, this->linecnt*sizeof(SoGlyph*));
  memset(this->positions, 0, this->linecnt*sizeof(SbVec2s*));
  memset(this->charbboxes, 0, this->linecnt*sizeof(SbVec2s*));
  memset(this->stringwidth, 0, this->linecnt*sizeof(int));
  memset(this->stringheight, 0, this->linecnt*sizeof(int));

  this->validarraydims = 1;

  for (int i=0; i<this->linecnt; i++) {

    s = PUBLIC(this)->string[i].getString();
    stringbox.makeEmpty();
    rotation = this->getRotation(i);

    if ((len = strlen(s)) > 0) {

      this->glyphs[i] = (SoGlyph **)malloc(len*sizeof(SoGlyph*));
      this->positions[i] = (SbVec2s *)malloc(len*sizeof(SbVec2s));
      this->charbboxes[i] = (SbVec2s *)malloc(len*sizeof(SbVec2s));
      memset(this->glyphs[i], 0, len*sizeof(SoGlyph*));
      memset(this->positions[i], 0, len*sizeof(SbVec2s));
      memset(this->charbboxes[i], 0, len*sizeof(SbVec2s));
      this->validarraydims = 2;

      SbVec2s penpos(0, 0);

      for (int j=0; j<len; j++) {
        idx = (unsigned char)s[j];
        this->glyphs[i][j] = (SoGlyph *)(SoGlyph::getGlyph(state, idx, SbVec2s(0,0), rotation));
        assert(this->glyphs[i][j]);

        this->glyphs[i][j]->getBitmap(thissize, thispos, FALSE);
        SbVec2s advance(this->glyphs[i][j]->getAdvance());
        if (outline) advance[0] += 1;

        SbVec2s kerning;
        if (j > 0) kerning = this->glyphs[i][j]->getKerning((const SoGlyph &)*this->glyphs[i][j-1]);
        else kerning = SbVec2s(0,0);

        this->charbboxes[i][j] = advance + SbVec2s(0, -thissize[1]);

        SbVec2s pos = penpos +
          SbVec2s((short) thispos[0], (short) thispos[1]) +
          SbVec2s(0, (short) -thissize[1]);

        stringbox.extendBy(pos);
        stringbox.extendBy(pos + SbVec2s(advance[0] + kerning[0] + thissize[0], thissize[1]));

        this->positions[i][j] = pos;

        penpos += advance + kerning;
	
      }
	
      this->stringwidth[i] = stringbox.getMax()[0] - stringbox.getMin()[0];
      this->stringheight[i] = stringbox.getMax()[1] - stringbox.getMin()[1];
      this->bboxes.append(stringbox);

      // FIXME: Incorrect bbox for glyphs like 'g' and 'q'
      // etc. Should use the same techniques as SoText2 instead to
      // solve all these problems. (20031008 handegar)

    }
  }

  this->dirty = FALSE;
}

#undef PUBLIC
