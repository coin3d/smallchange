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
  \class SmColorGradient SmColorGradient.h
  \brief The SmColorGradient class is a node used to control the GL depth buffer. 
  \ingroup nodes
*/

#include <SmallChange/nodes/SmColorGradient.h>
#include <SmallChange/elements/SmColorGradientElement.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/sensors/SoFieldSensor.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/errors/SoReadError.h>
#include <Inventor/lists/SbList.h>
#include <stdio.h>

SO_NODE_SOURCE(SmColorGradient);

/*!
  Constructor.
*/
SmColorGradient::SmColorGradient(void)
{
  SO_NODE_CONSTRUCTOR(SmColorGradient);

  SO_NODE_ADD_FIELD(mapping, (SmColorGradient::RELATIVE));
  SO_NODE_ADD_FIELD(color, (1.0f, 1.0f, 1.0f));
  SO_NODE_ADD_FIELD(parameter, (0.0f));
  
  SO_NODE_DEFINE_ENUM_VALUE(Mapping, RELATIVE);
  SO_NODE_DEFINE_ENUM_VALUE(Mapping, ABSOLUTE);
  SO_NODE_SET_SF_ENUM_TYPE(mapping, Mapping);

  // use field sensor for filename since we will load an image if
  // filename changes. This is a time-consuming task which should
  // not be done in notify().
  this->filenamesensor = new SoFieldSensor(filenameSensorCB, this);
  this->filenamesensor->setPriority(0);
  this->filenamesensor->attach(&this->filename);
}

/*!
  Destructor.
*/
SmColorGradient::~SmColorGradient()
{
  delete this->filenamesensor;
}

/*!
  Required Coin method.
*/
void
SmColorGradient::initClass(void)
{
  static int first = 1;
  if (first) {
    first = 0;
    SmColorGradientElement::initClass();
    SO_NODE_INIT_CLASS(SmColorGradient, SoNode, "Node");
    SO_ENABLE(SoGLRenderAction, SmColorGradientElement);
  }
}

/*!
  Coin method.
*/
void
SmColorGradient::GLRender(SoGLRenderAction * action)
{
  SoState * state = action->getState();
  SmColorGradientElement::set(state, this,
                              (SmColorGradientElement::Mapping) this->mapping.getValue(),
                              this->parameter.getNum(),
                              this->parameter.getValues(0),
                              this->color.getValues(0));
}

void
SmColorGradient::notify(SoNotList * l)
{
  SoField * f = l->getLastField();
  if (f == &this->color || f == &this->parameter) {
    // write parameters, not filename
    this->filename.setDefault(TRUE);
    this->color.setDefault(FALSE);
    this->parameter.setDefault(FALSE);
  }
  inherited::notify(l);
}

//
// called when filename changes
//
void
SmColorGradient::filenameSensorCB(void * data, SoSensor *)
{
  SmColorGradient * thisp = (SmColorGradient*) data;

  if (thisp->filename.getValue().getLength() &&
      !thisp->loadFilename()) {
    SoDebugError::postWarning("SmColorGradient::filenameSensorCB",
                              "Gradient file '%s' could not be read",
                              thisp->filename.getValue().getString());
  }
}

SbBool
SmColorGradient::readInstance(SoInput * in, unsigned short flags)
{
  this->filenamesensor->detach();
  SbBool readOK = inherited::readInstance(in, flags);
  if (readOK && !this->filename.isDefault() && this->filename.getValue() != "") {
    if (!this->loadFilename()) {
      SoReadError::post(in, "Could not read gradient file '%s'",
                        filename.getValue().getString());
    }
  }
  this->filenamesensor->attach(&this->filename);
  return readOK;
}

SbBool 
SmColorGradient::loadFilename(void)
{
  const SbString & s = this->filename.getValue();
  if (!s.getLength()) return FALSE;
 
  FILE * fp = fopen(s.getString(), "r");
  if (!fp) return FALSE;

  int num;
  if (fscanf(fp, "%d", &num) != 1) {
    fclose(fp);
    return FALSE;
  }

  SbList <float> params;
  while (num) {
    float p;
    if (fscanf(fp,"%f", &p) != 1) {
      fclose(fp);
      return FALSE;
    }
    params.append(p);
    num--;
  }
  
  if (fscanf(fp,"%d", &num) != 1) {
    fclose(fp);
    return FALSE;
  }

  SbList <SbColor> cols;
  SbColor c;
  float t;
  while (num) {
    unsigned int v;
    if (fscanf(fp, "%u", &v) != 1) {
      fclose(fp);
      return FALSE;
    }
    c.setPackedValue(v, t);
    cols.append(c);
    num--;
  }
  fclose(fp);
  
  this->color.setNum(0);
  this->parameter.setNum(0);
  this->color.setValues(0, cols.getLength(), cols.getArrayPtr());
  this->parameter.setValues(0, params.getLength(), params.getArrayPtr());

  this->color.setDefault(TRUE);
  this->parameter.setDefault(TRUE);

  return TRUE;
}
