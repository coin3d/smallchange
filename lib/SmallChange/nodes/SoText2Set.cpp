/**************************************************************************\
 *
 *  This file is part of the Coin 3D visualization library.
 *  Copyright (C) 1998-2002 by Systems in TCB. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License
 *  version 2.1 as published by the Free Software Foundation. See the
 *  file LICENSE.LGPL at the root directory of the distribution for
 *  more details.
 *
 *  If you want to use Coin for applications not compatible with the
 *  LGPL, please contact SIM to acquire a Professional Edition license.
 *
 *  Systems in TCB, Prof Brochs gate 6, 7030 Trondheim, NORWAY
 *  http://www.sim.no support@sim.no Voice: +47 22114160 Fax: +47 22207097
 *
\**************************************************************************/
 
/*!
  \class SoText2Set SoText2Set.h Inventor/nodes/SoText2Set.h
  \brief The SoText2Set class is a node type for visualizing a set of 2D texttags aligned with the camera plane.
  \ingroup nodes

  SoText2Set text is not scaled according to the distance from the
  camera, and is not influenced by rotation or scaling as 3D
  primitives are. If these are properties that you want the text to
  have, you should instead use an SoText3 or SoAsciiText node.

  Note that even though the size of the 2D text is not influenced by
  the distance from the camera, the text is still subject to the usual
  rules with regard to the depthbuffer, so it \e will be obscured by
  graphics laying in front of it.

  The text will be \e positioned according to the current transformation.
  The x origin of the text is the first pixel of the leftmost character
  of the text. The y origin of the text is the baseline of the first line
  of text (the baseline being the imaginary line on which all upper case
  characters are standing).

  \sa SoFont, SoFontStyle, SoText3, SoText2, SoAsciiText
*/

// FIXME -- FIXME -- FIXME
//
//  * computeBoundingBox() is not implemented properly, rayPick() and
//    generatePrimitives() are just stubs
//  * allocations aren't cleaned out on exit (Display *, XFontStructs,
//    SbDict, OpenGL display lists, ...)
//  * integrate with libfreetype to remove dependency on X11.
//         -- 19990418 mortene.

//  * This code is more or less copied directly from SoText2. I couldn't
//    figure out this strange SoText2P-class, so I removed the 
//    function called getFontSize and inserted a constant instead. 
//     
//         -- 06182002 torbjorv



#include "SoText2Set.h"

#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/elements/SoGLCoordinateElement.h>
#include <Inventor/SoPrimitiveVertex.h>
#include <Inventor/details/SoLineDetail.h>
#include <Inventor/bundles/SoMaterialBundle.h>
#include <Inventor/elements/SoGLLightModelElement.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoViewportRegionElement.h>
#include <Inventor/elements/SoViewVolumeElement.h>
#include <Inventor/actions/SoRayPickAction.h>
#include <Inventor/SbLine.h>
#include <Inventor/SbPlane.h>
#include <Inventor/SbVec2f.h>
#include <Inventor/SoPickedPoint.h>
#include <Inventor/details/SoTextDetail.h>
#include <Inventor/actions/SoGetPrimitiveCountAction.h>

#if HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#if HAVE_WINDOWS_H
#include <windows.h>
#endif // HAVE_WINDOWS_H
#include <GL/gl.h>

/*!
  \enum SoText2Set::Justification

  Enum contains the various options for how the horizontal text layout
  text should be done.
*/


/*!
  \var SoMFString SoText2Set::strings

  The set of strings to render.  Each string in the multiple value
  field will be according to the coordinates on the state. 

  The default value of the field is a single empty string.
*/

/*!
  \var SoSFEnum SoText2Set::justification

  Decides how the horizontal layout of the text strings is done.
*/
 
/*!
  \var SoSFVec2f SoText2Set::displacement

  Pixelwise displacement of the text after projection of 3D-position.
*/


// *************************************************************************

SO_NODE_SOURCE(SoText2Set);

// *************************************************************************

class SoText2SetP{
public:
  SoText2SetP(SoText2Set * master) {
    this->master = master;
  }

  static const SbVec2f fontsize;
private:
  SoText2Set * master;
};


#define PRIVATE(p) (p->pimpl)
#define PUBLIC(p) (p->master)

// *************************************************************************


const SbVec2f SoText2SetP::fontsize = SbVec2f(8.0f, 12.0f);

/*!
  Constructor.
*/
SoText2Set::SoText2Set(void)
{
  SO_NODE_CONSTRUCTOR(SoText2Set);

  SO_NODE_ADD_FIELD(strings, (""));
  SO_NODE_ADD_FIELD(justification, (SoText2Set::LEFT));
}//Constructor



/*!
  Destructor.
*/
SoText2Set::~SoText2Set()
{
  delete PRIVATE(this);
}




// Doc from parent class.
void
SoText2Set::initClass(void)
{
  SO_NODE_INIT_CLASS(SoText2Set, SoShape, "Shape");

}


extern unsigned char coin_default2dfont[][12];


// doc in super
void 
SoText2Set::GLRender(SoGLRenderAction *action)
{
  SoState * state = action->getState();

  if (!shouldGLRender(action)) return;

  const SoCoordinateElement *coords;
  coords = SoCoordinateElement::getInstance(state);
  assert(coords);

  SoMaterialBundle mb(action);
  mb.sendFirst();

#if COIN_MAJOR_VERSION >= 2
  SoGLLightModelElement::forceSend(state, SoLightModelElement::BASE_COLOR);
#else // COIN_MAJOR_VERSION >= 2
  SoGLLightModelElement::getInstance(state)->forceSend(SoLightModelElement::BASE_COLOR);
#endif // COIN_MAJOR_VERSION < 2
  const SbMatrix & mat = SoModelMatrixElement::get(state);

  const SbViewVolume & vv = SoViewVolumeElement::get(state);
  const SbViewportRegion & vp = SoViewportRegionElement::get(state);
  SbVec2s vpsize = vp.getViewportSizePixels();

  // Set new state.
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, vpsize[0], 0, vpsize[1], -1.0f, 1.0f);
  glPixelStorei(GL_UNPACK_ALIGNMENT,1);

  int xpos,ypos,width;

  // Draw all strings
  for (int i = 0; i < this->strings.getNum(); i++) {
    SbVec3f nilpoint = coords->get3(i);

    mat.multVecMatrix(nilpoint, nilpoint);
    vv.projectToScreen(nilpoint, nilpoint);
    nilpoint[2] = nilpoint[2]*2 - 1;
    nilpoint[0] = nilpoint[0] * float(vpsize[0]);
    nilpoint[1] = nilpoint[1] * float(vpsize[1]);

    xpos = (int)nilpoint[0];      
    ypos = (int)nilpoint[1];

    const char *s = this->strings[i].getString();
    width = strlen(s);

    switch (this->justification.getValue()) {

    case SoText2Set::LEFT:
      xpos = (int)nilpoint[0];
      break;


    case SoText2Set::RIGHT:
      xpos = (int)nilpoint[0] - (width*8);
      break;


    case SoText2Set::CENTER:
      xpos = (int)nilpoint[0] - (width*8)/2;
      break;
    }//switch

    xpos += displacement.getValue()[0];
    ypos -= displacement.getValue()[1];

    for (int i2 = 0; i2 < strings[i].getLength(); i2++) {
      if (s[i2] >= 32) {
        glRasterPos3f(float(xpos), float(ypos), -nilpoint[2]);
        glBitmap(8, 12, 0, 0, 0, 0, (const GLubyte *)coin_default2dfont + 12 * (s[i2] - 32));
      }// if
      xpos += 8;
    }// for
  }// for


  glPixelStorei(GL_UNPACK_ALIGNMENT,4);


  // Pop old GL state.
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}// GLRender






// doc in super
void
SoText2Set::generatePrimitives(SoAction * action)
{
  // This is supposed to be empty. There are no primitives.
}



// doc in super
void
SoText2Set::computeBBox(SoAction *action, SbBox3f &box, SbVec3f &center)
{
  SoState * state = action->getState();

  const SoCoordinateElement *coords;
  coords = SoCoordinateElement::getInstance(state);
  assert(coords);

  const SbMatrix & mat = SoModelMatrixElement::get(state);
  SbMatrix inv = mat.inverse();
  SbVec3f screenpoint;
  const SbViewVolume &vv = SoViewVolumeElement::get(state);

  const SbViewportRegion & vp = SoViewportRegionElement::get(state);
  SbVec2s vpsize = vp.getViewportSizePixels();

  // FIXME: this only works for the default font
  SbVec2f fontsize = PRIVATE(this)->fontsize;
  fontsize[0] /= vpsize[0];
  fontsize[1] /= vpsize[1];

  // Check all texttags
  for (int i = 0; i < strings.getNum(); i++) {
    SbVec3f v0, v1, v2, v3;

    // Calculate a quad fencing strings[i]
    SbVec3f nilpoint = coords->get3(i);
    mat.multVecMatrix(nilpoint, nilpoint);
    vv.projectToScreen(nilpoint, screenpoint);

    float w = strings[i].getLength()*fontsize[0];

    float halfw = w * 0.5f;
    SbVec2f n0, n1, n2, n3;

    n0 = SbVec2f(screenpoint[0]-halfw, screenpoint[1]);
    n1 = SbVec2f(screenpoint[0]+halfw, screenpoint[1]);
    n2 = SbVec2f(screenpoint[0]+halfw, screenpoint[1] + fontsize[1]);
    n3 = SbVec2f(screenpoint[0]-halfw, screenpoint[1] + fontsize[1]);

    switch (justification.getValue()) {
    case SoText2Set::LEFT:
      n0[0] += halfw;
      n1[0] += halfw;
      n2[0] += halfw;
      n3[0] += halfw;
      break;
    case SoText2Set::RIGHT:
      n0[0] -= halfw;
      n1[0] -= halfw;
      n2[0] -= halfw;
      n3[0] -= halfw;
      break;
    case SoText2Set::CENTER:
      break;
    }//switch

    // get distance from nilpoint to camera plane
    float dist = -vv.getPlane(0.0f).getDistance(nilpoint);

    // find the four image points in the plane
    v0 = vv.getPlanePoint(dist, n0);
    v1 = vv.getPlanePoint(dist, n1);
    v2 = vv.getPlanePoint(dist, n2);
    v3 = vv.getPlanePoint(dist, n3);

    // transform back to object space
    inv.multVecMatrix(v0, v0);
    inv.multVecMatrix(v1, v1);
    inv.multVecMatrix(v2, v2);
    inv.multVecMatrix(v3, v3);

    box.extendBy(v0);
    box.extendBy(v1);
    box.extendBy(v2);
    box.extendBy(v3);
  }//for
}//computeBBox




// doc in super
void
SoText2Set::getPrimitiveCount(SoGetPrimitiveCountAction *action)
{
  if (!this->shouldPrimitiveCount(action)) return;

  action->addNumText(this->strings.getNum());
}



// doc in super
void
SoText2Set::rayPick(SoRayPickAction * action)
{
  if (!this->shouldRayPick(action)) return;
  action->setObjectSpace();

  SoState * state = action->getState();

  const SoCoordinateElement *coords;
  coords = SoCoordinateElement::getInstance(state);
  assert(coords);

  const SbMatrix & mat = SoModelMatrixElement::get(state);
  SbMatrix inv = mat.inverse();
  SbVec3f screenpoint;
  const SbViewVolume &vv = SoViewVolumeElement::get(state);

  const SbViewportRegion & vp = SoViewportRegionElement::get(state);
  SbVec2s vpsize = vp.getViewportSizePixels();

  // FIXME: this only works for the default font
  SbVec2f fontsize = PRIVATE(this)->fontsize;
  fontsize[0] /= vpsize[0];
  fontsize[1] /= vpsize[1];

  // Check all texttags
  for (int i = 0; i < strings.getNum(); i++) {
    SbVec3f v0, v1, v2, v3;



    // Calculate a quad fencing strings[i]
    SbVec3f nilpoint = coords->get3(i);
    mat.multVecMatrix(nilpoint, nilpoint);
    vv.projectToScreen(nilpoint, screenpoint);

    float w = strings[i].getLength()*PRIVATE(this)->fontsize[0];

    float halfw = w * 0.5f;
    SbVec2f n0, n1, n2, n3;

    n0 = SbVec2f(screenpoint[0]-halfw, screenpoint[1]);
    n1 = SbVec2f(screenpoint[0]+halfw, screenpoint[1]);
    n2 = SbVec2f(screenpoint[0]+halfw, screenpoint[1] + fontsize[1]);
    n3 = SbVec2f(screenpoint[0]-halfw, screenpoint[1] + fontsize[1]);

    switch (justification.getValue()) {
    case SoText2Set::LEFT:
      n0[0] += halfw;
      n1[0] += halfw;
      n2[0] += halfw;
      n3[0] += halfw;
      break;
    case SoText2Set::RIGHT:
      n0[0] -= halfw;
      n1[0] -= halfw;
      n2[0] -= halfw;
      n3[0] -= halfw;
      break;
    case SoText2Set::CENTER:
      break;
    }//switch

    // get distance from nilpoint to camera plane
    float dist = -vv.getPlane(0.0f).getDistance(nilpoint);

    // find the four image points in the plane
    v0 = vv.getPlanePoint(dist, n0);
    v1 = vv.getPlanePoint(dist, n1);
    v2 = vv.getPlanePoint(dist, n2);
    v3 = vv.getPlanePoint(dist, n3);

    // transform back to object space
    inv.multVecMatrix(v0, v0);
    inv.multVecMatrix(v1, v1);
    inv.multVecMatrix(v2, v2);
    inv.multVecMatrix(v3, v3);



    if (v0 == v1 || v0 == v3) return; // empty

    SbVec3f isect;
    SbVec3f bary;
    SbBool front;
    SbBool hit = action->intersect(v0, v1, v2, isect, bary, front);
    if (!hit) hit = action->intersect(v0, v2, v3, isect, bary, front);

    if (hit && action->isBetweenPlanes(isect)) {
      // find normalized 2D hitpoint on quad
      float h = (v3 - v0).length();
      float w = (v1 - v0).length();
      SbLine horiz(v2, v3);
      SbVec3f ptonline = horiz.getClosestPoint(isect);
      float vdist = (ptonline-isect).length();
      vdist /= h;

      SbLine vert(v0,v3);
      ptonline = vert.getClosestPoint(isect);
      float hdist = (ptonline-isect).length();
      hdist /= w;

      // find which strings and character was hit
      float fonth =  1.0f;
      int stringidx = i;

      // assumes all characters are equal size...
      float fontw = 1.0f / strings[i].getLength();

      // find the character
      int charidx = -1;
      int strlength = this->strings[stringidx].getLength();
      switch (this->justification.getValue()) {
      case LEFT:
        charidx = int(hdist / fontw);
        break;
      case RIGHT:
        charidx = (strlength-1) - int((1.0f-hdist)/fontw);
        break;
      case CENTER:
        {
          float strstart = 0.5f - fontw*float(strlength)*0.5f;
          charidx = int((hdist-strstart) / fontw);
        }
        break;
      default:
        assert(0 && "unknown justification");
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

          return;
        }
      }
    }

  }//for


}//rayPick



