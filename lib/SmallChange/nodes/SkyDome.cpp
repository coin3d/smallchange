/**************************************************************************\
 *
 *  This file is part of the SmallChange extension library for Coin.
 *  Copyright (C) 1998-2002 by Systems in Motion.  All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  version 2 as published by the Free Software Foundation.  See the
 *  file LICENSE.GPL at the root directory of this source distribution
 *  for more details.
 *
 *  If you desire to use SmallChange with software that is incompatible
 *  licensewise with the GPL, and / or you would like to take
 *  advantage of the additional benefits with regard to our support
 *  services, please contact Systems in Motion about acquiring a Coin
 *  Professional Edition License.  See <URL:http://www.coin3d.org> for
 *  more information.
 *
 *  Systems in Motion, Prof Brochs gate 6, 7030 Trondheim, NORWAY
 *  <URL:http://www.sim.no>, <mailto:support@sim.no>
 *
\**************************************************************************/

/*!
  \class SkyDome SkyDome.h
  \brief The SkyDome class is a shape node for rendering skydomes.
  \ingroup nodes
  
*/

/*!
  \var SoSFFloat SkyDome::startAngle
  Specifies the vertical angle where the (partial) sphere will start. 0 is straight up (+Z).
*/

/*!  
  \var SoSFFloat SkyDome::endAngle;
  Specifies the vertical angle where the (partial) sphere will end. 0 is straight up (+Z).
*/

/*!
  \var SoSFInt32 SkyDome::numStacks;
  Specifies the number of stacks used when rendering.
*/

/*!
  \var SoSFInt32 SkyDome::numSlices;
  Specifies the number of slices used when rendering.
*/

/*!
  \var SoSFFloat SkyDome::height;
  Specifies the height (+Z) of the skydome. 
*/

/*!
  \var SoSFFloat SkyDome::radius;
  Specifies the radius (+X and +Y) if the skydome.
*/

#include "SkyDome.h"
#include <Inventor/misc/SoState.h>
#include <Inventor/actions/SoGetMatrixAction.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoViewVolumeElement.h>
#include <Inventor/elements/SoGLTextureEnabledElement.h>
#include <Inventor/elements/SoLightModelElement.h>
#include <Inventor/elements/SoDiffuseColorElement.h>
#include <Inventor/elements/SoViewVolumeElement.h>
#include <Inventor/elements/SoViewingMatrixElement.h>
#include <Inventor/SbRotation.h>
#include <Inventor/SbMatrix.h>
#include <Inventor/SbPlane.h>
#include <Inventor/lists/SbList.h>
#include <Inventor/bundles/SoMaterialBundle.h>
#include <Inventor/SbXfBox3f.h>
#include <stdlib.h>
#include <assert.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#if HAVE_WINDOWS_H
#include <windows.h>
#endif // HAVE_WINDOWS_H

#include <GL/gl.h>


SO_NODE_SOURCE(SkyDome);

/*!
  Constructor.
*/
SkyDome::SkyDome()
{
  SO_NODE_CONSTRUCTOR(SkyDome);
  SO_NODE_ADD_FIELD(startAngle, (0.0f));
  SO_NODE_ADD_FIELD(endAngle, (float(M_PI)*0.5f));
  SO_NODE_ADD_FIELD(numStacks, (8));
  SO_NODE_ADD_FIELD(numSlices, (32));
  SO_NODE_ADD_FIELD(height, (1.0f));
  SO_NODE_ADD_FIELD(radius, (1.0f));
}

/*!
  Destructor.
*/
SkyDome::~SkyDome()
{
}

/*!
  Required Coin method.
*/
void
SkyDome::initClass()
{
  SO_NODE_INIT_CLASS(SkyDome, SoShape, "Shape");
}

/*!
  Coin method. Renders skydome.
*/
void
SkyDome::GLRender(SoGLRenderAction * action)
{
  if (!this->shouldGLRender(action)) return;

  SoMaterialBundle mb(action);
  mb.sendFirst();

  int stacks = this->numStacks.getValue();
  int slices = this->numSlices.getValue();

  if (stacks < 3) stacks = 3;
  if (slices < 4) slices = 4;

  if (slices > 128) slices = 128;

  // used to cache last stack's data
  SbVec3f coords[129];
  SbVec3f normals[129];
  float S[129];

  int i, j;
  float rho;
  float drho;
  float theta;
  float dtheta;
  float tc, ts;
  SbVec3f tmp;

  float startangle = SbClamp(this->startAngle.getValue(), 0.0f, float(M_PI));
  float endangle = SbClamp(this->endAngle.getValue(), 0.0f, float(M_PI));
  if (endangle <= startangle) return;
  
  float h = this->height.getValue();
  float r = this->radius.getValue();

  float totalangle = endangle - startangle;

  drho = totalangle / float(stacks-1);
  dtheta = 2.0f * float(M_PI) / float(slices);

  float currs = 0.0f;
  float incs = 1.0f / (float)slices;

  rho = drho;
  theta = 0.0f;
  tc = (float) cos(rho);
  ts = - (float) sin(rho);
  tmp.setValue(0.0f,
               ts,
               tc);
  normals[0] = tmp;
  tmp[0] *= r;
  tmp[1] *= r;
  tmp[2] *= h;
  
  coords[0] = tmp;
  S[0] = currs;
  float dT = 1.0f / (float) (stacks-1);
  float T = 1.0f - dT;
  
  glBegin(GL_TRIANGLES);
  
  for (j = 1; j <= slices; j++) {
    if (startangle == 0.0f) {
      glNormal3f(0.0f, 0.0f, 1.0f);
      glTexCoord2f(currs + 0.5f * incs, 1.0f);
      glVertex3f(0.0f, 0.0f, h);
    
      glNormal3fv((const GLfloat*) &normals[j-1]);
      glTexCoord2f(currs, T);
      glVertex3fv((const GLfloat*) &coords[j-1]);
    }
    currs += incs;
    theta += dtheta;
    S[j] = currs;
    tmp.setValue(float(sin(theta))*ts,
                 float(cos(theta))*ts,
                 tc);
    
    normals[j] = tmp;
    tmp[0] *= r;
    tmp[1] *= r;
    tmp[2] *= h;
    coords[j] = tmp;
    
    if (startangle == 0.0f) {
      glNormal3fv((const GLfloat*)&normals[j]);
      glTexCoord2f(currs, T);
      glVertex3fv((const GLfloat*)&coords[j]);
    }
  }
  glEnd(); // GL_TRIANGLES

  rho += drho;

  if (endangle < float(M_PI)) stacks++;
  
  for (i = 2; i < stacks-1; i++) {
    tc = (float)cos(rho);
    ts = - (float) sin(rho);
    glBegin(GL_QUAD_STRIP);
    theta = 0.0f;
    for (j = 0; j <= slices; j++) {
      glTexCoord2f(S[j], T);
      glNormal3fv((const GLfloat*)&normals[j]);
      glVertex3fv((const GLfloat*)&coords[j]);

      glTexCoord2f(S[j], T - dT);
      tmp.setValue(float(sin(theta))*ts,
                   float(cos(theta))*ts,
                   tc);
      normals[j] = tmp;
      glNormal3f(tmp[0], tmp[1], tmp[2]);
      tmp[0] *= r;
      tmp[1] *= r;
      tmp[2] *= h;
      glVertex3f(tmp[0], tmp[1], tmp[2]);
      coords[j] = tmp;
      theta += dtheta;
    }
    glEnd(); // GL_QUAD_STRIP
    rho += drho;
    T -= dT;
  }
  
  if (endangle == float(M_PI)) {
    glBegin(GL_TRIANGLES);
    for (j = 0; j < slices; j++) {
      glTexCoord2f(S[j], T);
      glNormal3fv((const GLfloat*)&normals[j]);
      glVertex3fv((const GLfloat*)&coords[j]);
      
      glTexCoord2f(S[j]+incs*0.5f, 0.0f);
      glNormal3f(0.0f, 0.0f, -1.0f);
      glVertex3f(0.0f, 0.0f, -h);
      
      glTexCoord2f(S[j+1], T);
      glNormal3fv((const GLfloat*)&normals[j+1]);
      glVertex3fv((const GLfloat*)&coords[j+1]);
    }
    glEnd(); // GL_TRIANGLES
  }
}

/*!
  Coin method. Generates skydome primitives. Currently not implemented.
*/
void
SkyDome::generatePrimitives(SoAction * action)
{
}

/*!
  Coin method. Computes bounding box of skydome.
*/
void
SkyDome::computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center)
{
  float r = this->radius.getValue();
  float h = this->height.getValue();
  box.setBounds(-r, -r, -h, r, r, h);
  center = box.getCenter();
}

/*!
  Coin method. Calculates the number of primitives in skydome. Not implemented.
*/ 
void
SkyDome::getPrimitiveCount(SoGetPrimitiveCountAction * action)
{
}
