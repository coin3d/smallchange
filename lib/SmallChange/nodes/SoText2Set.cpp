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

class SoText2SetP {
public:
  SoText2SetP(SoText2Set * textnode) : textnode(textnode) {}

  SoText2Set * textnode;
  
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
  SbBool useglyphcache;
  SbBool hasbuiltglyphcache;
  SbBool dirty;
  
public:
  void getQuad(SoState * state, SbVec3f & v0, SbVec3f & v1,
               SbVec3f & v2, SbVec3f & v3, int stringidx);
  void flushGlyphCache(const SbBool unrefglyphs);
  int buildGlyphCache(SoState * state);
  SbBool shouldBuildGlyphCache(SoState * state);
  void dumpGlyphCache();
  void dumpBuffer(unsigned char * buffer, SbVec2s size, SbVec2s pos);
  SbVec2s findBitmapBBox(unsigned char * buf, SbVec2s & size);
};

// *************************************************************************

#undef THIS
#define THIS this->pimpl

SO_NODE_SOURCE(SoText2Set);

/*!
  Constructor.
*/
SoText2Set::SoText2Set(void)
{
  THIS = new SoText2SetP(this);
  THIS->glyphs = NULL;
  THIS->positions = NULL;
  THIS->charbboxes = NULL;
  THIS->linecnt = 0;
  THIS->validarraydims = 0;
  THIS->stringwidth = NULL;
  THIS->stringheight = NULL;
  THIS->bboxes.truncate(0);
  THIS->prevfontname = SbName("");
  THIS->prevfontsize = 0.0;
  THIS->useglyphcache = TRUE;
  THIS->hasbuiltglyphcache = FALSE;
  THIS->dirty = TRUE;
  
  SO_NODE_CONSTRUCTOR(SoText2Set);
  
  SO_NODE_ADD_FIELD(position, (SbVec3f(0.0f, 0.0f, 0.0f)));
  SO_NODE_ADD_FIELD(rotation, (0.0f));
  SO_NODE_ADD_FIELD(justification, (SoText2Set::LEFT));
  SO_NODE_ADD_FIELD(string, (""));
  
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
  THIS->flushGlyphCache(TRUE);
  delete THIS;
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
  
  // Render using SoGlyphs
  if (THIS->buildGlyphCache(state) == 0) {
    SbBox3f box;
    SbVec3f center;
    // FIXME: cull per string, not for the entire node. preng 2003-03-27.
    this->computeBBox(action, box, center);
    if (!SoCullElement::cullTest(state, box, SbBool(TRUE))) {
      SoMaterialBundle mb(action);
      mb.sendFirst();
      SbVec3f nilpoint;
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
      
      float xpos;
      float ypos;
      float fx, fy, rasterx, rastery, rpx, rpy, offsetx, offsety;
      int ix, iy, charcnt, offvp;
      SbVec2s thispos;
      SbVec2s position;
      SbVec2s thissize;
      GLboolean gldbg;
      unsigned char * buffer;
      // Find number of strings to render
      int stringcnt = this->string.getNum();
      stringcnt = SbMin(stringcnt, this->position.getNum());
      stringcnt = SbMin(stringcnt, this->rotation.getNum());
      stringcnt = SbMin(stringcnt, this->justification.getNum());
      for (int i = 0; i < stringcnt; i++) {
        // Find nilpoint for this string
        nilpoint = this->position[i];
        mat.multVecMatrix(nilpoint, nilpoint);
        vv.projectToScreen(nilpoint, nilpoint);
        nilpoint[2] *= 2.0f;
        nilpoint[2] -= 1.0f;
        nilpoint[0] = nilpoint[0] * float(vpsize[0]);
        nilpoint[1] = nilpoint[1] * float(vpsize[1]);
        xpos = nilpoint[0];
        ypos = nilpoint[1];
        
        charcnt = this->string[i].getLength();
        switch (this->justification[i]) {
        case SoText2Set::LEFT:
          // No action
          break;
        case SoText2Set::RIGHT:
          xpos -= THIS->stringwidth[i];
          ypos -= THIS->positions[i][charcnt-1][1];
          break;
        case SoText2Set::CENTER:
          xpos -= THIS->stringwidth[i]/2.0f;
          ypos -= THIS->stringheight[i]/2.0f;
          break;
        }
        for (int i2 = 0; i2 < charcnt; i2++) {
          buffer = THIS->glyphs[i][i2]->getBitmap(thissize, thispos, SbBool(FALSE));
          ix = thissize[0];
          iy = thissize[1];
          position = THIS->positions[i][i2];
          fx = (float)position[0];
          fy = (float)position[1];
          
          rasterx = xpos + fx;
          rpx = rasterx >= 0 ? rasterx : 0;
          offvp = rasterx < 0 ? 1 : 0;
          offsetx = rasterx >= 0 ? 0 : rasterx;
          
          rastery = ypos + fy;
          rpy = rastery >= 0 ? rastery : 0;
          offvp = offvp || rastery < 0 ? 1 : 0;
          offsety = rastery >= 0 ? 0 : rastery;
         
          glRasterPos3f(rpx, rpy, -nilpoint[2]);
          if (offvp)
            glBitmap(0,0,0,0,offsetx,offsety,NULL);
          glBitmap(ix,iy,0,0,0,0,(const GLubyte *)buffer);
        }
      }
      
      glPixelStorei(GL_UNPACK_ALIGNMENT,4);
      // Pop old GL state.
      glMatrixMode(GL_PROJECTION);
      glPopMatrix();
      glMatrixMode(GL_MODELVIEW);
      glPopMatrix();
    }
  }
  
  state->pop();

  // don't auto cache SoText2Set nodes.
  SoGLCacheContextElement::shouldAutoCache(action->getState(),
                                           SoGLCacheContextElement::DONT_AUTO_CACHE);
}

// **************************************************************************


// doc in super
void
SoText2Set::computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center)
{
  SbVec3f v0, v1, v2, v3;
  // this will cause a cache dependency on the view volume,
  // model matrix and viewport.
  box.makeEmpty();
  for (int i=0; i<this->string.getNum(); i++) {
    THIS->getQuad(action->getState(), v0, v1, v2, v3, i);
    box.extendBy(v0);
    box.extendBy(v1);
    box.extendBy(v2);
    box.extendBy(v3);
  }
  center = box.getCenter();
}

// doc in super
void
SoText2Set::rayPick(SoRayPickAction * action)
{
  if (!this->shouldRayPick(action)) return;
  SoState * state = action->getState();
  THIS->buildGlyphCache(state);
  
  state->push();
  action->setObjectSpace();
  SbVec3f v0, v1, v2, v3;

  for (int stringidx=0; stringidx < this->string.getNum(); stringidx++) {
    THIS->getQuad(state, v0, v1, v2, v3, stringidx);

    if (v0 == v1 || v0 == v3) 
      return; // empty
    
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
      THIS->bboxes[stringidx].getBounds(minx, miny, maxx, maxy);
      float bbwidth = (float)(maxx - minx);
      float bbheight = (float)(maxy - miny);
      float charleft, charright, charbottom, chartop;
      SbVec2s thissize, thispos;

      for (int i=0; i<strlength; i++) {
        THIS->glyphs[stringidx][i]->getBitmap(thissize, thispos, SbBool(FALSE));
        charleft = (THIS->positions[stringidx][i][0] - minx) / bbwidth;
        charright = (THIS->positions[stringidx][i][0] + THIS->charbboxes[stringidx][i][0] - minx) / bbwidth;
        
        if (hdist >= charleft && hdist <= charright) {
          chartop = (maxy - THIS->positions[stringidx][i][1] - THIS->charbboxes[stringidx][i][1]) / bbheight;
          charbottom = (maxy - THIS->positions[stringidx][i][1]) / bbheight;
          
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
  THIS->dirty = TRUE;
  inherited::notify(list);
}



// SoText2SetP methods below
#undef THIS

void
SoText2SetP::flushGlyphCache(const SbBool unrefglyphs)
{
  if (this->glyphs && validarraydims > 0) {
    free(this->stringwidth);
    free(this->stringheight);

    for (int i=0; i<this->linecnt; i++) {
      if (validarraydims == 2) {
        if (unrefglyphs) {
          for (int j=0; j<this->textnode->string[i].getLength(); j++) {
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
      fprintf(stderr,"  string[%d]=%s\n", i, this->textnode->string[i].getString());
      if (validarraydims == 2) {
        for (int j = 0; j < (int) strlen(this->textnode->string[i].getString()); j++) {
          fprintf(stderr,"    glyph[%d][%d]=%p\n", i, j, this->glyphs[i][j]);
          fprintf(stderr,"    position[%d][%d]=(%d, %d)\n", i, j, this->positions[i][j][0], this->positions[i][j][1]);
        }
      }
    }
  }
}

// Calculates a quad around the text in 3D.
void
SoText2SetP::getQuad(SoState * state, SbVec3f & v0, SbVec3f & v1,
                  SbVec3f & v2, SbVec3f & v3, int stringidx)
{

  int posindex = stringidx;
  if (posindex > this->textnode->position.getNum())
    posindex = this->textnode->position.getNum() - 1;
    
  this->buildGlyphCache(state);
  
  SbVec3f nilpoint = this->textnode->position[posindex];
  const SbMatrix & mat = SoModelMatrixElement::get(state);
  mat.multVecMatrix(nilpoint, nilpoint);

  const SbViewVolume &vv = SoViewVolumeElement::get(state);

  SbVec3f screenpoint;
  vv.projectToScreen(nilpoint, screenpoint);
  
  const SbViewportRegion & vp = SoViewportRegionElement::get(state);
  SbVec2s vpsize = vp.getViewportSizePixels();
  
  SbVec2f n0, n1, n2, n3, center;
  short xmin, ymin, xmax, ymax;
  float minx, miny, maxx, maxy;
  this->bboxes[stringidx].getBounds(xmin, ymin, xmax, ymax);
  center = SbVec2f( (float)(xmin+xmax)/(float)2.0, (float)(ymin+ymax)/(float)2.0);
  minx = (xmin - center[0]) / vpsize[0];
  miny = (ymin - center[1]) / vpsize[1];
  maxx = (xmax - center[0]) / vpsize[0];
  maxy = (ymax - center[1]) / vpsize[1];
  n0 = SbVec2f(screenpoint[0] + minx, screenpoint[1] + miny);
  n1 = SbVec2f(screenpoint[0] + maxx, screenpoint[1] + miny);
  n2 = SbVec2f(screenpoint[0] + maxx, screenpoint[1] + maxy);
  n3 = SbVec2f(screenpoint[0] + minx, screenpoint[1] + maxy);
  
  float halfw = (maxx - minx) / (float)2.0;
  float halfh = (maxy - miny) / (float)2.0;

  int justificationindex = stringidx;
  if (justificationindex > this->textnode->justification.getNum())
    justificationindex = this->textnode->justification.getNum() - 1;

  switch (this->textnode->justification[justificationindex]) {
  case SoText2Set::LEFT:
    n0[0] += halfw;
    n1[0] += halfw;
    n2[0] += halfw;
    n3[0] += halfw;
    n0[1] += halfh;
    n1[1] += halfh;
    n2[1] += halfh;
    n3[1] += halfh;
    break;
  case SoText2Set::RIGHT:
    n0[0] -= halfw;
    n1[0] -= halfw;
    n2[0] -= halfw;
    n3[0] -= halfw; 
    n0[1] += halfh;
    n1[1] += halfh;
    n2[1] += halfh;
    n3[1] += halfh;
   break;
  case SoText2Set::CENTER:
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
    return SbBool(TRUE);
  if (!this->useglyphcache)
    return SbBool(FALSE);
  if (this->dirty)
    return SbBool(TRUE);

  SbName curfontname = SoFontNameElement::get(state);
  float curfontsize = SoFontSizeElement::get(state);
  SbBool fonthaschanged = (this->prevfontname != curfontname 
                           || this->prevfontsize != curfontsize);
  if (fonthaschanged)
    return fonthaschanged;
  return SbBool(FALSE);
}

SbVec2s
SoText2SetP::findBitmapBBox(unsigned char * buf, SbVec2s & size)
{
  short maxx = 0;
  short maxy = 0;
  int idx;
  int bytewidth = size[0] >> 3;
  unsigned char mask;

  for (int y=0; y<size[1]; y++) {
    for (int byte=0; byte<bytewidth; byte++) {
      for (int bit=0; bit<8; bit++) {
        idx = y * bytewidth + byte;
        mask = 0x80 >> bit;
        if (buf[idx] & mask) {
          if (byte*8 + bit > maxx)
            maxx = byte*8 + bit;
          if (y > maxy)
            maxy = y;
        }
      }
    }
  }

  return SbVec2s(maxx, maxy);
}

int
SoText2SetP::buildGlyphCache(SoState * state)
{

  if (this->shouldBuildGlyphCache(state)) {

    SoText2Set * t = this->textnode;
    const char * s;
    int len, i;
    SbVec2s penpos, advance, kerning, thissize, thispos;
    unsigned int idx;
    SbName curfontname;
    float curfontsize;
    float rotation;
    SbBox2s stringbox;
    unsigned char * bmbuf;
    
    // FIXME: This leads to incorrect support for the use of "<font>:Italic"
    // or "<font>:Bold Italic" etc.(20031008 handegar)
    curfontname = SoFontNameElement::get(state);
    curfontsize = SoFontSizeElement::get(state);

    this->prevfontname = curfontname;
    this->prevfontsize = curfontsize;
    this->flushGlyphCache(FALSE);
    this->hasbuiltglyphcache = SbBool(TRUE);
    this->linecnt = t->string.getNum();
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
    penpos[0] = 0;
    penpos[1] = 0;
    advance = penpos;
    kerning = penpos;

    for (i=0; i<this->linecnt; i++) {

      s = t->string[i].getString();
      stringbox.makeEmpty();
      rotation = t->rotation[i];

      if ((len = strlen(s)) > 0) {

        this->glyphs[i] = (SoGlyph **)malloc(len*sizeof(SoGlyph*));
        this->positions[i] = (SbVec2s *)malloc(len*sizeof(SbVec2s));
        this->charbboxes[i] = (SbVec2s *)malloc(len*sizeof(SbVec2s));
        memset(this->glyphs[i], 0, len*sizeof(SoGlyph*));
        memset(this->positions[i], 0, len*sizeof(SbVec2s));
        memset(this->charbboxes[i], 0, len*sizeof(SbVec2s));
        this->validarraydims = 2;

        for (int j=0; j<len; j++) {
          idx = (unsigned char)s[j];
          this->glyphs[i][j] = (SoGlyph *)(SoGlyph::getGlyph(state, idx, SbVec2s(0,0), rotation));

          if (!this->glyphs[i][j]) {
            this->flushGlyphCache(FALSE);
            this->useglyphcache = FALSE;
            SoDebugError::postWarning("SoText2Set::buildGlyphCache", "unable to build glyph cache for '%s'", s);
            return -1;
          }

          bmbuf = this->glyphs[i][j]->getBitmap(thissize, thispos, SbBool(FALSE));
          this->charbboxes[i][j] = this->findBitmapBBox( bmbuf, thissize);
          
          if (j > 0) {
            kerning = this->glyphs[i][j-1]->getKerning((const SoGlyph &)*this->glyphs[i][j]);
            advance = this->glyphs[i][j-1]->getAdvance();
          } else {
            kerning = SbVec2s(0,0);
            advance = SbVec2s(0,0);
          }

          penpos = penpos + advance + kerning;
          this->positions[i][j] = penpos + thispos + SbVec2s(0, -thissize[1]);
          stringbox.extendBy(this->positions[i][j]);
          stringbox.extendBy(this->positions[i][j] + thissize);
        }

        // FIXME: incorrect for rotated texts, use stringbox width and height instead!
        this->stringwidth[i] = this->positions[i][len-1][0] + thissize[0];
        this->stringheight[i] = this->positions[i][len-1][1] + thissize[1];
        this->bboxes.append(stringbox);
        penpos = SbVec2s(0, 0);

        // FIXME: Incorrect bbox for glyphs like 'g' and 'q'
        // etc. Should use the same techniques as SoText2 instead to
        // solve all these problems. (20031008 handegar)

      }
    }
  }

  this->dirty = FALSE;
  return 0;
}
