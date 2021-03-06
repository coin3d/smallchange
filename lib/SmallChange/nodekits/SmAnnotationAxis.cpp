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

#include "SmAnnotationAxis.h"
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoProjectionMatrixElement.h>
#include <Inventor/elements/SoViewingMatrixElement.h>
#include <Inventor/elements/SoCacheElement.h>
#include <Inventor/elements/SoViewportRegionElement.h>
#include <Inventor/elements/SoViewVolumeElement.h>
#include <Inventor/elements/SoCullElement.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/nodes/SoSwitch.h>
#include <Inventor/nodes/SoInfo.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoLineSet.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoVertexProperty.h>
#include <Inventor/SbTime.h>
#include <Inventor/sensors/SoAlarmSensor.h>
#include <Inventor/sensors/SoOneShotSensor.h>
#include <SmallChange/nodes/SmTextureText2.h>
#include <SmallChange/nodes/SmTextureText2Collector.h>
#include <cstring>

// *************************************************************************

SO_KIT_SOURCE(SmAnnotationAxis);

// *************************************************************************

class SmAnnotationAxisP {

public:
  SmAnnotationAxisP(SmAnnotationAxis * master) {
    this->master = master;
  }

  SmAnnotationAxis * master;
  SbList <int> axisidx;
  SoOneShotSensor * regen_sensor;

  void add_anno_text(const int level,
                     SbList <int> & list,
                     const SbMatrix & projm, 
                     const SbVec2s & vpsize,
		     const float gap,
                     const SbVec3f * pos, int i0, int i1);

  SbTime lastchanged;
  SoAlarmSensor * alarm;

  static void alarmCB(void * closure, SoSensor * s) {
    SmAnnotationAxisP * thisp = (SmAnnotationAxisP*) closure;
    thisp->master->touch();
  }
};

#define PRIVATE(p) ((p)->pimpl)
#define PUBLIC(p) ((p)->master)

// *************************************************************************

SmAnnotationAxis::SmAnnotationAxis()
{
  PRIVATE(this) = new SmAnnotationAxisP(this);
  PRIVATE(this)->regen_sensor = NULL;
  PRIVATE(this)->alarm = new SoAlarmSensor(SmAnnotationAxisP::alarmCB, PRIVATE(this));
  PRIVATE(this)->lastchanged = SbTime::zero();

  SO_KIT_CONSTRUCTOR(SmAnnotationAxis);
  SO_KIT_ADD_CATALOG_ENTRY(topSeparator, SoSeparator, FALSE, this, "", FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(textMaterial, SoMaterial, FALSE, topSeparator, text, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(text, SmTextureText2, FALSE, topSeparator, axisSwitch, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(axisSwitch, SoSwitch, FALSE, topSeparator, "", FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(noAxis, SoInfo, FALSE, axisSwitch, axisSep, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(axisSep, SoSeparator, FALSE, axisSwitch, "", FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(axisMaterial, SoMaterial, FALSE, axisSep, axisLineSet, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(axisLineSet, SoLineSet, FALSE, axisSep, "", FALSE);
  
  SO_KIT_ADD_FIELD(annotation, (""));
  SO_KIT_ADD_FIELD(annotationPos, (0.0f, 0.0f, 0.0f));
  SO_KIT_ADD_FIELD(annotationGap, (30.0f));
  SO_KIT_ADD_FIELD(renderAxis, (FALSE));
  SO_KIT_ADD_FIELD(axisTickSize, (0.0f, 0.0f, 0.0f));
  SO_KIT_ADD_FIELD(annotationOffset, (0.0f, 0.0f, 0.0f));
  SO_KIT_ADD_FIELD(annotationRot, (0.0f));

  this->annotation.setNum(0);
  this->annotationPos.setNum(0);
  this->annotation.setDefault(TRUE);
  this->annotationPos.setDefault(TRUE);

  SO_KIT_INIT_INSTANCE();

  SoSwitch * sw = static_cast<SoSwitch*>(this->getAnyPart("axisSwitch", TRUE));
  sw->whichChild.connectFrom(&this->renderAxis);

  SmTextureText2 * t = static_cast<SmTextureText2*>(this->getAnyPart("text", TRUE));
  t->justification = SmTextureText2::CENTER;
  t->rotation.connectFrom(&this->annotationRot);

  PRIVATE(this)->regen_sensor = new SoOneShotSensor(regen_geometry, this);
}

SmAnnotationAxis::~SmAnnotationAxis()
{
  delete PRIVATE(this)->alarm;
  delete PRIVATE(this)->regen_sensor;
  delete PRIVATE(this);
}

void
SmAnnotationAxis::initClass(void)
{
  SO_KIT_INIT_CLASS(SmAnnotationAxis, SoBaseKit, "BaseKit");
}

void 
SmAnnotationAxis::getBoundingBox(SoGetBoundingBoxAction * action)
{
  SoState * state = action->getState();

  // supply an approximate bbox and always invalidate the bbox cache
  SoCacheElement::invalidate(state);
  
  SbBox3f bbox;
  bbox.makeEmpty();
  for (int i = 0; i < this->annotationPos.getNum(); i++) {
    bbox.extendBy(this->annotationPos[i]);
  }
  
  if (!bbox.isEmpty()) {
    action->extendBy(bbox);
    action->setCenter(bbox.getCenter(), TRUE);
  }
  inherited::getBoundingBox(action);
}

void 
SmAnnotationAxis::GLRender(SoGLRenderAction * action)
{
  SoState * state = action->getState();

  SbBool render = TRUE;
  
  // don't create render caches for this node
  SoCacheElement::invalidate(state);
  
  SbMatrix projmatrix;
  projmatrix = (SoModelMatrixElement::get(state) *
                SoViewingMatrixElement::get(state) *
                SoProjectionMatrixElement::get(state));
  
  SbVec2s vpsize = SoViewportRegionElement::get(state).getViewportSizePixels();
  const SbViewVolume & vv = SoViewVolumeElement::get(state);
  float maxsize = SbMax(vpsize[0], vpsize[1]);
  
  SbList <int> l1;
  if (this->annotationPos.getNum() >= 2) {
    l1.truncate(0);
    PRIVATE(this)->add_anno_text(0, l1, projmatrix,
				 vpsize,
                                 this->annotationGap.getValue(),
                                 this->annotationPos.getValues(0), 
                                 0, this->annotationPos.getNum() - 1); 
    
  }
  if (SmTextureText2CollectorElement::isCollecting(state)) {
    if (this->annotation.getNum()) {
      SoMaterial * material = static_cast<SoMaterial*>(this->getAnyPart("textMaterial", TRUE));
      SmTextureText2 * text = static_cast<SmTextureText2*>(this->getAnyPart("text", TRUE));
      SbMatrix modelmatrix = SoModelMatrixElement::get(state);
      SbVec3f pos;
      SbColor4f col(material->diffuseColor[0], 1.0f - material->transparency[0]);
      
      for (int i = 0; i < l1.getLength(); i++) {
        pos = this->annotationPos[l1[i]] + this->annotationOffset.getValue();
        modelmatrix.multVecMatrix(pos, pos);
        
        SmTextureText2CollectorElement::add(state,
                                            this->annotation.getValues(0)[l1[i] % this->annotation.getNum()],
                                            SmTextureFontElement::get(state),
                                            pos,
                                            -1.0,
                                            col,
                                            static_cast<SmTextureText2::Justification>(text->justification.getValue()),
                                            static_cast<SmTextureText2::VerticalJustification>(text->verticalJustification.getValue()));
        
        if (text->string.getNum()) text->string.setNum(0);
      }
    }
  }
  else {  
    if (l1 != PRIVATE(this)->axisidx) {
      // avoid that we update the scene graph and trigger redraws too often
      SbTime curtime = SbTime::getTimeOfDay();
      if ((curtime.getValue() - PRIVATE(this)->lastchanged.getValue()) < 0.5) {
        if (!PRIVATE(this)->alarm->isScheduled()) {
          PRIVATE(this)->alarm->setTimeFromNow(1.0);
          PRIVATE(this)->alarm->schedule();
        }
      }
      else {
        PRIVATE(this)->lastchanged = curtime;
        SmTextureText2 * t = static_cast<SmTextureText2*>(this->getAnyPart("text", TRUE));
        assert(t);
        t->position.setNum(l1.getLength());
        t->string.setNum(l1.getLength());
        SbVec3f * pos = t->position.startEditing();
        SbString * text = t->string.startEditing();
        for (int i = 0; i < l1.getLength(); i++) {
          pos[i] = this->annotationPos[l1[i]] + this->annotationOffset.getValue();
          text[i] = this->annotation.getNum() > 0 ?
            this->annotation.getValues(0)[l1[i]%this->annotation.getNum()] : "";
        }
        t->position.finishEditing();
        t->string.finishEditing();
        
        PRIVATE(this)->axisidx = l1;
      }
    }
  }
  if (render) inherited::GLRender(action);
}

void 
SmAnnotationAxis::notify(SoNotList * list)
{
  if (PRIVATE(this)->regen_sensor) {
    SoField * f = list->getLastField();
    if ((f == &this->annotationPos) ||
        (f == &this->renderAxis) ||
        (f == &this->axisTickSize)) {
      PRIVATE(this)->regen_sensor->schedule();
    }
  }
  inherited::notify(list);
}

void
SmAnnotationAxis::regen_geometry(void * userdata, SoSensor * s)
{
  SmAnnotationAxis * thisp = (SmAnnotationAxis*) userdata;
  if (thisp->renderAxis.getValue()) {
    SoLineSet * ls = static_cast<SoLineSet*>(thisp->getAnyPart("axisLineSet", TRUE));
    SoVertexProperty * vp = static_cast<SoVertexProperty*> (ls->vertexProperty.getValue());
    if (vp == NULL) {
      vp = new SoVertexProperty;
      ls->vertexProperty = vp;
    }
    SbVec3f ticksize = thisp->axisTickSize.getValue();

    const int numanno = thisp->annotationPos.getNum();
    const int numlines = 1 + ((ticksize.length() > 0.0f) ? numanno : 0); 
    const int numcoords = numanno + ((ticksize.length() > 0.0f) ? numanno*2 : 0);
    
    vp->vertex.setNum(numcoords);
    ls->numVertices.setNum(numlines);
    
    const SbVec3f * src = thisp->annotationPos.getValues(0);
    SbVec3f * pts = vp->vertex.startEditing();
    int32_t * v = ls->numVertices.startEditing();

    v[0] = numanno;
    
    int i;
    
    for (i = 0; i < numanno; i++) {
      pts[i] = thisp->annotationPos[i];
    }
    if (ticksize.length() > 0.0f) {
      for (i = 0; i < numanno; i++) {
        v[i+1] = 2;
        pts[i*2+numanno] = src[i];
        pts[i*2+1+numanno] = src[i] + ticksize;
      }
    }
    
    vp->vertex.finishEditing();
    ls->numVertices.finishEditing();
  }
}

// *************************************************************************

void 
SmAnnotationAxisP::add_anno_text(const int level,
                                 SbList <int> & list,
                                 const SbMatrix & projm, 
				 const SbVec2s & vpsize,
                                 const float gap,
                                 const SbVec3f * pos, int i0, int i1)
{
  if (i0 == i1) return;

  int mid = (i0 + i1) / 2;
  SbVec3f p[3];
  p[0] = pos[i0];
  p[1] = pos[mid];
  p[2] = pos[i1];
  
  int i;
  for (i = 0; i < 3; i++) {
    projm.multVecMatrix(p[i], p[i]);
  }
  
  if (level == 0) { // special case to handle the corner points
    if ((p[0][2] < 1.0f) && (p[2][2] < 1.0f)) {
      SbVec3f d = p[2]-p[0];
      d[0] = SbAbs(d[0]) * float(vpsize[0]) * 0.5f;
      d[1] = SbAbs(d[1]) * float(vpsize[1]) * 0.5f;
      d[2] = 0.0f;

      float len = d.length();
      if (len > gap) {
        list.append(i0);
        list.append(i1);
      }
    }
    else if (p[0][2] < 1.0f) {
      list.append(i0);
    }
    else if (p[2][2] < 1.0f) {
      list.append(i1);
    }
  }

  if ((mid == i0) || (mid == i1)) return;

  if (p[1][2] < 1.0f) {
    SbBool add = FALSE;
    float len = 0.0f;
    if (p[0][2] < 1.0f) {
      SbVec3f d = p[1]-p[0];
      d[0] = SbAbs(d[0]) * float(vpsize[0]) * 0.5f;
      d[1] = SbAbs(d[1]) * float(vpsize[1]) * 0.5f;
      d[2] = 0.0f;

      len = d.length();
    }
    else if (p[2][2] < 1.0f) {
      SbVec3f d = p[1]-p[2];
      d[0] = SbAbs(d[0]) * float(vpsize[0]) * 0.5f;
      d[1] = SbAbs(d[1]) * float(vpsize[1]) * 0.5f;
      d[2] = 0.0f;
      len = d.length();
    }
    if (len > gap) {
      list.append(mid);
    }
  }
  add_anno_text(level+1, list, projm, vpsize, gap, pos, i0, mid);
  add_anno_text(level+1, list, projm, vpsize, gap, pos, mid, i1);
}

// *************************************************************************
