/**************************************************************************\
 * Copyright (c) Kongsberg Oil & Gas Technologies AS
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
\**************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <SmallChange/misc/Init.h>
#include <SmallChange/SmallChange.h>
#include <SmallChange/elements/GLDepthBufferElement.h>
#include <SmallChange/nodes/AutoFile.h>
#include <SmallChange/nodes/Coinboard.h>
#include <SmallChange/nodes/SmDepthBuffer.h>
#include <SmallChange/nodes/ViewportRegion.h>
#include <SmallChange/nodes/SmSwitchboard.h>
#include <SmallChange/nodes/SmSwitchboardOperator.h>
#include <SmallChange/nodes/SoLODExtrusion.h>
#include <SmallChange/nodes/SoPointCloud.h>
#include <SmallChange/nodes/SoTCBCurve.h>
#include <SmallChange/nodes/SoText2Set.h>
#include <SmallChange/nodes/SmScenery.h>
#include <SmallChange/nodes/SmVertexArrayShape.h>
#include <SmallChange/actions/SmToVertexArrayShapeAction.h>
#include <SmallChange/actions/SoTweakAction.h>
#include <SmallChange/actions/SoGenerateSceneGraphAction.h>
#include <SmallChange/engines/Rot2Heading.h>
#include <SmallChange/engines/SmInverseRotation.h>
#include <SmallChange/nodekits/LegendKit.h>
#include <SmallChange/nodekits/SoFEMKit.h>
#include <SmallChange/nodekits/SmTooltipKit.h>
#include <SmallChange/nodekits/SmCameraControlKit.h>
#include <SmallChange/nodekits/SmDynamicObjectKit.h>
#include <SmallChange/nodekits/DynamicBaseKit.h>
#include <SmallChange/nodekits/SmOceanKit.h>
#include <SmallChange/eventhandlers/SmExaminerEventHandler.h>
#include <SmallChange/eventhandlers/SmHelicopterEventHandler.h>
#include <SmallChange/eventhandlers/SmSphereEventHandler.h>
#include <SmallChange/eventhandlers/SmPanEventHandler.h>

#include <SmallChange/nodes/CoinEnvironment.h>
#include <SmallChange/nodes/PickCallback.h>
#include <SmallChange/nodes/PickSwitch.h>
#include <SmallChange/nodes/ShapeScale.h>
#include <SmallChange/nodes/SkyDome.h>
#include <SmallChange/nodes/SmTooltip.h>
#include <SmallChange/nodes/SmHQSphere.h>
#include <SmallChange/nodes/SmColorGradient.h>
#include <SmallChange/nodes/InterleavedArraysShape.h>
#include <SmallChange/engines/CubicSplineEngine.h>

#include <SmallChange/elements/UTMElement.h>
#include <SmallChange/elements/SmColorGradientElement.h>
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
#include <SmallChange/nodes/SmMarkerSet.h>
#include <SmallChange/nodes/SmCoordinateSystem.h>
#include <SmallChange/nodes/SmViewpointWrapper.h>
#include <SmallChange/nodekits/SmPopupMenuKit.h>
#include <SmallChange/nodekits/SmTrackPointKit.h>
#include <SmallChange/nodes/SmTrack.h>
#include <SmallChange/nodes/SmLazyFile.h>
#include <SmallChange/nodekits/SmAnnotationWall.h>
#include <SmallChange/nodekits/SmAnnotationAxis.h>

#include <SmallChange/nodekits/SmPieChart.h>
#include <SmallChange/nodes/SmTextureText2.h>
#include <SmallChange/nodes/SmTextureText2Collector.h>
#include <SmallChange/nodes/SmTextureFont.h>
#include <SmallChange/nodes/SmShadowText2.h>

void SmallChange::init(void)
{
  smallchange_init();
}

void SmallChange::cleanup(void)
{
  // FIXME: Implement
}

void
smallchange_init(void)
{
  AutoFile::initClass();
  GLDepthBufferElement::initClass();
  Coinboard::initClass();
  SmDepthBuffer::initClass();
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

  UTMElement::initClass();
  UTMPosition::initClass();
  UTMCamera::initClass();
  UTMCoordinate::initClass();

  SmCameraControlKit::initClass();
  SmEventHandler::initClass();
  SmExaminerEventHandler::initClass();
  SmSphereEventHandler::initClass();
  SmHelicopterEventHandler::initClass();
  SmPanEventHandler::initClass();

  SmWellLogKit::initClass();
  SmHQSphere::initClass();
  SmGeoMarkerKit::initClass();
  SmBillboardClipPlane::initClass();
  SmNormalsKit::initClass();
  SmAxisDisplayKit::initClass();
  SmAxisKit::initClass();
  SmHeadlight::initClass();
  SmRangeTranslate1Dragger::initClass();
  SmMarkerSet::initClass();

  SmVertexArrayShape::initClass();
  SmToVertexArrayShapeAction::initClass();

  SmColorGradientElement::initClass();
  SmColorGradient::initClass();

  SmCoordinateSystem::initClass();
  SmViewpointWrapper::initClass();
  SmPopupMenuKit::initClass();
  SmLazyFile::initClass();

  SmPieChart::initClass();
#if defined(__COIN__) && COIN_MAJOR_VERSION >= 3
  SmDynamicObjectKit::initClass();
  SmTrackPointKit::initClass();
  SmTrack::initClass();
  SmOceanKit::initClass();
#endif // temporary compile fix

  SmTextureFontElement::initClass();
  SmTextureFont::initClass();

  SmTextureText2::initClass();
  SmShadowText2::initClass();

  SmTextureText2CollectorElement::initClass();
  SmTextureText2Collector::initClass();

  SmAnnotationWall::initClass();
  SmAnnotationAxis::initClass();

  DynamicBaseKit::initClass();

  InterleavedArraysShape::initClass();
}
