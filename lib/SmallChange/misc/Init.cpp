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

#include <SmallChange/misc/Init.h>
#include <SmallChange/elements/GLDepthBufferElement.h>
#include <SmallChange/nodes/AutoFile.h>
#include <SmallChange/nodes/Coinboard.h>
#include <SmallChange/nodes/DepthBuffer.h>
#include <SmallChange/nodes/ViewportRegion.h>
#include <SmallChange/nodes/SmSwitchboard.h>
#include <SmallChange/nodes/SmSwitchboardOperator.h>
#include <SmallChange/nodes/SoLODExtrusion.h>
#include <SmallChange/nodes/SoPointCloud.h>
#include <SmallChange/nodes/SoTCBCurve.h>
#include <SmallChange/nodes/SoText2Set.h>
#include <SmallChange/nodes/FrustumCamera.h>
#include <SmallChange/nodes/SmScenery.h>
#include <SmallChange/nodes/SmVertexArrayShape.h>
#include <SmallChange/actions/SoTweakAction.h>
#include <SmallChange/actions/SoGenerateSceneGraphAction.h>
#include <SmallChange/engines/Rot2Heading.h>
#include <SmallChange/engines/SmInverseRotation.h>
#include <SmallChange/nodekits/LegendKit.h>
#include <SmallChange/nodekits/SoFEMKit.h>
#include <SmallChange/nodekits/SmTooltipKit.h>
#include <SmallChange/nodekits/SmCameraControlKit.h>
#include <SmallChange/eventhandlers/SmExaminerEventHandler.h>
#include <SmallChange/eventhandlers/SmHelicopterEventHandler.h>
#include <SmallChange/eventhandlers/SmSphereEventHandler.h>

#include <SmallChange/nodes/CoinEnvironment.h>
#include <SmallChange/nodes/PickCallback.h>
#include <SmallChange/nodes/PickSwitch.h>
#include <SmallChange/nodes/ShapeScale.h>
#include <SmallChange/nodes/SkyDome.h>
#include <SmallChange/nodes/SmTooltip.h>
#include <SmallChange/nodes/SmHQSphere.h>
#include <SmallChange/engines/CubicSplineEngine.h>

#include <SmallChange/elements/UTMElement.h>
#include <SmallChange/nodes/UTMCamera.h>
#include <SmallChange/nodes/UTMPosition.h>
#include <SmallChange/nodes/UTMCoordinate.h>

#include <SmallChange/nodekits/SmWellLogKit.h>
#include <SmallChange/nodekits/SmGeoMarkerKit.h>
#include <SmallChange/nodes/SmBillboardClipPlane.h>
#include <SmallChange/nodekits/SmNormalsKit.h>
#include <SmallChange/nodekits/SmAxisDisplayKit.h>
#include <SmallChange/nodekits/SmAxisKit.h>
#include <SmallChange/nodes/SmHeadlight.h>
#include <SmallChange/draggers/SmRangeTranslate1Dragger.h>

#ifdef HAVE_SOUND

#include <SmallChange/nodes/SoListener.h>
#include <SmallChange/nodes/SoSound.h>
#include <SmallChange/nodes/SoAudioClip.h>
#include <SmallChange/nodes/SoAudioClipStreaming.h>
#include <SmallChange/actions/SoAudioRenderAction.h>

#endif // HAVE_SOUND


void
smallchange_init(void)
{
#ifdef HAVE_SOUND
  // doesn't look like this is working
  //  SoListener::initClass();
  //SoSound::initClass();
  //SoAudioClip::initClass();
  //SoAudioClipStreaming::initClass();
  //SoAudioRenderAction::initClass();
#endif // HAVE_SOUND

  AutoFile::initClass();
  GLDepthBufferElement::initClass();
  Coinboard::initClass();
  DepthBuffer::initClass();
  ViewportRegion::initClass();
  SmSwitchboard::initClass();
  SmSwitchboardOperator::initClass();
  Rot2Heading::initClass();
  SmInverseRotation::initClass();
  LegendKit::initClass();
  SoFEMKit::initClass();
  SmTooltipKit::initClass();
  SoLODExtrusion::initClass();
  SoPointCloud::initClass();
  SoTCBCurve::initClass();
  SoText2Set::initClass();
  SkyDome::initClass();
  CoinEnvironment::initClass();
  PickCallback::initClass();
  PickSwitch::initClass();
  ShapeScale::initClass();
  SoTweakAction::initClass();
  SoGenerateSceneGraphAction::initClass();
  SmTooltip::initClass();
  SmScenery::initClass();

  CubicSplineEngine::initClass();
  FrustumCamera::initClass();

  UTMElement::initClass();
  UTMPosition::initClass();
  UTMCamera::initClass();
  UTMCoordinate::initClass();

  SmCameraControlKit::initClass();
  SmEventHandler::initClass();
  SmExaminerEventHandler::initClass();
  SmSphereEventHandler::initClass();
  SmHelicopterEventHandler::initClass();

  SmWellLogKit::initClass();
  SmHQSphere::initClass();
  SmGeoMarkerKit::initClass();
  SmBillboardClipPlane::initClass();
  SmNormalsKit::initClass();
  SmAxisDisplayKit::initClass();
  SmAxisKit::initClass();
  SmHeadlight::initClass();
  SmRangeTranslate1Dragger::initClass();

  SmVertexArrayShape::initClass();
}
