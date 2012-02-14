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
