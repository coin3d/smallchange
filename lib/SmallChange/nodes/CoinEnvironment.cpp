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
  \class CoinEnvironment CoinEnvironment.h Inventor/nodes/CoinEnvironment.h
  \brief The CoinEnvironment class is a node for specifying global rendering parameters.
  \ingroup nodes

  This node type provides the application programmer with the ability
  to set global parameters influencing lighting and fog. It is a
  temporary replacement of the standard SoEnvironment node, which
  didn't have a fogStart field.

*/

#include "CoinEnvironment.h"

#include <Inventor/actions/SoCallbackAction.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/elements/SoGLEnvironmentElement.h>
#include <Inventor/elements/SoLightAttenuationElement.h>

/*!
  \enum CoinEnvironment::FogType
  Enumeration of available types of fog.
*/
/*!
  \var CoinEnvironment::FogType CoinEnvironment::NONE

  No fog. Visibility will be equal for all objects, independent of
  distance to camera.
*/
/*!
  \var CoinEnvironment::FogType CoinEnvironment::HAZE

  Fog where visibility will decrease linearly with distance to camera.
*/
/*!
  \var CoinEnvironment::FogType CoinEnvironment::FOG

  Fog where visibility will decrease exponentially with distance to
  camera.
*/
/*!
  \var CoinEnvironment::FogType CoinEnvironment::SMOKE

  Fog where visibility will decrease exponentially with the square of
  the distance to camera (simulating really thick fog).
*/


/*!
  \var SoSFFloat CoinEnvironment::ambientIntensity

  A global ambient value for the light intensity for the complete
  scene. This will provide some light even when there are no light
  sources defined for the scene.

  Valid values is from 0.0 (no ambient light) to 1.0 (full ambient
  light intensity). Default value is 0.2.
*/
/*!
  \var SoSFColor CoinEnvironment::ambientColor

  The color of the global ambient light. Defaults to full intensity
  white.
*/
/*!
  \var SoSFVec3f CoinEnvironment::attenuation

  Light attenuation coefficients.
*/
/*!
  \var SoSFEnum CoinEnvironment::fogType

  The fog model. See CoinEnvironment::FoType.
*/
/*!
  \var SoSFColor CoinEnvironment::fogColor

  Color of fog. Defaults to full intensity white.
*/

/*!
  \var SoSFFloat CoinEnvironment::fogStart

  The distance from the near plane from which objects will start to
  be obscured by fog. This field will only be used in linear (HAZE)
  mode.

  Default value is 0.0.
*/

/*!
  \var SoSFFloat CoinEnvironment::fogVisibility

  The "cut-off" distance from the camera where objects will be totally
  obscured by fog. If set to 0.0, the far plane distance will be used
  instead.

  Default value is 0.0.
*/

// *************************************************************************

SO_NODE_SOURCE(CoinEnvironment);

/*!
  Constructor.
*/
CoinEnvironment::CoinEnvironment()
{
  SO_NODE_CONSTRUCTOR(CoinEnvironment);

  SO_NODE_ADD_FIELD(ambientIntensity, (0.2f));
  SO_NODE_ADD_FIELD(ambientColor, (1.0f, 1.0f, 1.0f));
  SO_NODE_ADD_FIELD(attenuation, (0.0f, 0.0f, 1.0f));
  SO_NODE_ADD_FIELD(fogType, (NONE));
  SO_NODE_ADD_FIELD(fogColor, (1.0f, 1.0f, 1.0f));
  SO_NODE_ADD_FIELD(fogVisibility, (0.0f));
  SO_NODE_ADD_FIELD(fogStart, (0.0f));

  SO_NODE_DEFINE_ENUM_VALUE(FogType, NONE);
  SO_NODE_DEFINE_ENUM_VALUE(FogType, HAZE);
  SO_NODE_DEFINE_ENUM_VALUE(FogType, FOG);
  SO_NODE_DEFINE_ENUM_VALUE(FogType, SMOKE);
  SO_NODE_SET_SF_ENUM_TYPE(fogType, FogType);
}

/*!
  Destructor.
*/
CoinEnvironment::~CoinEnvironment()
{
}

/*!
  Required Coin method.
*/
void
CoinEnvironment::initClass(void)
{
  SO_NODE_INIT_CLASS(CoinEnvironment, SoNode, "Node");
}

/*!
  Coin method. Updates state with new environment values.
*/
void
CoinEnvironment::GLRender(SoGLRenderAction * action)
{
  SoLightAttenuationElement::set(action->getState(), this,
                                 this->attenuation.getValue());
  SoEnvironmentElement::set(action->getState(),
                            this,
                            this->ambientIntensity.getValue(),
                            this->ambientColor.getValue(),
                            this->attenuation.getValue(),
                            (int32_t)fogType.getValue(),
                            this->fogColor.getValue(),
                            this->fogVisibility.getValue(),
                            this->fogStart.getValue());
}

/*!
  Coin method. Updates state with the new environment values.
*/
void
CoinEnvironment::callback(SoCallbackAction *action)
{
  SoLightAttenuationElement::set(action->getState(), this,
                                 this->attenuation.getValue());
  SoEnvironmentElement::set(action->getState(),
                            this,
                            this->ambientIntensity.getValue(),
                            this->ambientColor.getValue(),
                            this->attenuation.getValue(),
                            (int32_t)fogType.getValue(),
                            this->fogColor.getValue(),
                            this->fogVisibility.getValue(),
                            this->fogStart.getValue());
}
