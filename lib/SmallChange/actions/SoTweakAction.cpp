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

#include <cstdio>
#include <cassert>

#include <Inventor/nodes/SoGroup.h>
#include <Inventor/misc/SoChildList.h>

#include <SmallChange/actions/SoTweakAction.h>

// *************************************************************************

class SoTweakActionP {
public:
  SoTweakActionP(SoTweakAction * action);
  ~SoTweakActionP(void);

  void enterNode(SoNode * node);
  void visit(SoNode * node);

  SbBool clearnodenames;

  SoTweakAction * api;
};

SoTweakActionP::SoTweakActionP(SoTweakAction * action)
{
  this->api = action;
  this->clearnodenames = FALSE;
}

SoTweakActionP::~SoTweakActionP(void)
{
}

void
SoTweakActionP::enterNode(SoNode * node)
{
  if ( this->clearnodenames != FALSE )
    node->setName(SbName(""));
}

void
SoTweakActionP::visit(SoNode * node)
{
  assert(node != NULL && node->getTypeId() != SoType::badType());
  this->enterNode(node);
  if ( node->getTypeId().isDerivedFrom(SoGroup::getClassTypeId()) ) {
    SoGroup * group = (SoGroup *) node;
    if ( group->getChildren()->getLength() > 0 ) {
      // this->pushLevel();
      group->getChildren()->traverse(this->api);
      // this->popLevel();
    }
  }
  // this->exitNode(node);
}

// *************************************************************************

SO_ACTION_SOURCE(SoTweakAction);

#define THIS (this->pimpl)

void
SoTweakAction::initClass(void)
{
  SO_ACTION_INIT_CLASS(SoTweakAction, SoAction);
  SO_ACTION_ADD_METHOD(SoNode, SoTweakAction::visitS);
}

SoTweakAction::SoTweakAction(void)
{
  THIS = new SoTweakActionP(this);
  SO_ACTION_CONSTRUCTOR(SoTweakAction);
}

SoTweakAction::~SoTweakAction(void)
{
  delete THIS;
}

void
SoTweakAction::setClearNodeNames(SbBool clear)
{
  THIS->clearnodenames = clear;
}

SbBool
SoTweakAction::getClearNodeNames(void) const
{
  return THIS->clearnodenames;
}

void
SoTweakAction::beginTraversal(SoNode * node)
{
  assert(this->traversalMethods);
  this->traverse(node);
}

void
SoTweakAction::visitS(SoAction * action, SoNode * node)
{
  assert(action != NULL);
  ((SoTweakAction *) action)->pimpl->visit(node);
}

