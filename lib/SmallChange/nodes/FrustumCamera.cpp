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
  \class FrustumCamera FrustumCamera.h SmallChange/nodes/FrustumCamera.h
  \brief The FrustumCamera class defines a camera with a generic frustum..

  The FrustumCamera class makes it possible to specify a frustum in
  the same manner as the OpenGL glFrustum() function. It has four new
  fields (left, right, top, bottom), and will use
  SoCamera::nearDistance and SoCamera::farDistance for the two last
  glFrustum() parameters.

*/

/*!
  \var SoSFFloat FrustumCamera::left

  The left clipping plane position. Default value is -0.5.
*/

/*!
  \var SoSFFloat FrustumCamera::right

  The right clipping plane position. Default value is 0.5
*/

/*!
  \var SoSFFloat FrustumCamera::bottom

  The bottom clipping plane position. Default value is -0.5.
*/

/*!
  \var SoSFFloat FrustumCamera::top

  The top clipping plane position. Default value is 0.5.
*/

#include "FrustumCamera.h"
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/SbSphere.h>
#include <math.h>

SO_NODE_SOURCE(FrustumCamera);

/*!
  Constructor.
*/
FrustumCamera::FrustumCamera(void)
{
  SO_NODE_CONSTRUCTOR(FrustumCamera);

  SO_NODE_ADD_FIELD(top, (0.5f));
  SO_NODE_ADD_FIELD(bottom, (-0.5f));
  SO_NODE_ADD_FIELD(left, (-0.5f));
  SO_NODE_ADD_FIELD(right, (0.5f));
}

/*!
  Destructor.
*/
FrustumCamera::~FrustumCamera(void)
{  
}

// Doc in superclass.
void
FrustumCamera::initClass(void)
{
  static int first = 1;
  if (first) {
    first = 0;
    SO_NODE_INIT_CLASS(FrustumCamera, SoCamera, "Camera");
  }
}

// Doc in superclass.
void 
FrustumCamera::scaleHeight(float scalefactor)
{
  this->top = this->top.getValue() * scalefactor;
  this->bottom = this->bottom.getValue() * scalefactor;
}

// Doc in superclass.
SbViewVolume 
FrustumCamera::getViewVolume(float useaspectratio) const
{
  SbViewVolume vv;

  if (useaspectratio == 0.0f) { // LEAVE_ALONE viewportMappin
    vv.frustum(this->left.getValue(),
               this->right.getValue(),
               this->bottom.getValue(),
               this->top.getValue(),
               this->nearDistance.getValue(),
               this->farDistance.getValue());
  }
  else {
    // calculate left and right based on height
    float cx = this->left.getValue() + this->right.getValue();
    float h_2 = (this->top.getValue() - this->bottom.getValue()) * 0.5f;

    vv.frustum(cx - h_2 * useaspectratio,
               cx + h_2 * useaspectratio,
               this->bottom.getValue(),
               this->top.getValue(),
               this->nearDistance.getValue(),
               this->farDistance.getValue());    
  }
  vv.rotateCamera(this->orientation.getValue());
  vv.translateCamera(this->position.getValue());
  return vv;
}

// Doc in superclass.
void 
FrustumCamera::viewBoundingBox(const SbBox3f & box, float aspect, float slack)
{

#if COIN_DEBUG
  // Only check for "flagged" emptiness, and don't use
  // SbBox3f::hasVolume(), as we *can* handle flat boxes.
  if (box.isEmpty()) {
    SoDebugError::postWarning("Frustum::viewBoundingBox",
                              "bounding box is empty");
    return;
  }
#endif // COIN_DEBUG

  // First, we want to move the camera in such a way that it is
  // pointing straight at the center of the scene bounding box -- but
  // without modifiying the rotation value (so we can't use
  // SoCamera::pointAt()).
  SbVec3f cameradirection;
  this->orientation.getValue().multVec(SbVec3f(0, 0, -1), cameradirection);
  this->position.setValue(box.getCenter() + -cameradirection);

  // Get the radius of the bounding sphere.
  SbSphere bs;
  bs.circumscribe(box);
  float radius = bs.getRadius();

  // Make sure that everything will still be inside the viewing volume
  // even if the aspect ratio "favorizes" width over height.
  float aspectradius = radius / (aspect < 1.0f ? aspect : 1.0f);

  // just calculate a heightangle so that we can reuse code from
  // SoPerspectiveCamera
  float nearv = this->nearDistance.getValue();
  SbVec3f tvec = SbVec3f(0.0f, this->top.getValue(), nearv);
  SbVec3f bvec = SbVec3f(0.0f, this->bottom.getValue(), nearv);

  (void) tvec.normalize();
  (void) bvec.normalize();
  
  float heightangle = (float) acos((double)SbClamp(tvec.dot(bvec), 0.0f, 1.0f));
  
  // Move the camera to the edge of the bounding sphere, while still
  // pointing at the scene.
  SbVec3f direction = this->position.getValue() - box.getCenter();
  direction.normalize();
  float movelength =
    aspectradius + (aspectradius/float(atan(heightangle)));
  this->position.setValue(box.getCenter() + direction * movelength);

  // Set up the far clipping plane according to the slack value (a
  // value of 1.0 will yield a far clipping plane that is tangent to
  // the bounding sphere of the scene).
  float distance_to_midpoint =
    (this->position.getValue() - box.getCenter()).length();
  this->farDistance = distance_to_midpoint + radius * slack;

  // The focal distance is simply the distance from the camera to the
  // scene midpoint. This field is not used in rendering, its just
  // provided to make it easier for the user to do calculations based
  // on the distance between the camera and the scene.
  this->focalDistance = distance_to_midpoint;
}

