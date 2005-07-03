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

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <Inventor/SoDB.h>
#include <Inventor/SoInput.h>

#include <Inventor/errors/SoDebugError.h>

#include <Inventor/lists/SbList.h>
#include <Inventor/lists/SbPList.h>
#include <Inventor/misc/SoChildList.h>

#include <Inventor/actions/SoSubAction.h>
#include <Inventor/actions/SoSearchAction.h>

#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoTranslation.h>
#include <Inventor/nodes/SoRotation.h>
#include <Inventor/nodes/SoScale.h>
#include <Inventor/nodes/SoAsciiText.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoTexture2.h>
#include <Inventor/nodes/SoCube.h>

#include <Inventor/nodes/SoCylinder.h>

#include <SmallChange/actions/SoGenerateSceneGraphAction.h>
#include <SmallChange/actions/SoTweakAction.h>

// *************************************************************************

class SoSceneGraphNode {
public:
  SoSceneGraphNode(void);
  ~SoSceneGraphNode(void);

  float full_width;
  float connection_width;
  float connection_offset;

  SbBool calculated;
  void calculate(void);

  SoSceneGraphNode * parent;
  SbList<SoSceneGraphNode *> * children;

  SoNode * node;
  SoNode * sgnode;

  void print(int indent);
  void addChild(SoSceneGraphNode * child);
};

SoSceneGraphNode::SoSceneGraphNode(void)
{
  this->full_width = 0;
  this->connection_width = 0;
  this->connection_offset = 0;
  this->calculated = FALSE;
  this->node = NULL;
  this->parent = NULL;
  this->children = NULL;
}

SoSceneGraphNode::~SoSceneGraphNode(void)
{
  if ( this->children ) {
    for ( int i = 0, num = this->children->getLength(); i < num; i++ )
      delete this->children->operator[](i);
    delete this->children;
  }
}

void
SoSceneGraphNode::addChild(SoSceneGraphNode * child)
{
  if ( this->children == NULL )
    this->children = new SbList<SoSceneGraphNode *>;
  this->children->append(child);
}

void
SoSceneGraphNode::calculate()
{
  if ( this->calculated ) return;
  if ( this->children != NULL ) {
    const int num = this->children->getLength();
    int i;
    for ( i = 0; i < num; i++ )
      this->children->operator[](i)->calculate();
    this->full_width = float(num) - 1.0f;
    for ( i = 0; i < num; i++ )
      this->full_width += this->children->operator[](i)->full_width;
    this->connection_offset =
      this->children->operator[](0)->connection_offset
      + (this->children->operator[](0)->connection_width / 2.0f);
    float connection_right_offset =
      this->children->operator[](num-1)->full_width
      - this->children->operator[](num-1)->connection_offset
      - (this->children->operator[](num-1)->connection_width / 2.0f);
    this->connection_width =
      this->full_width - this->connection_offset - connection_right_offset;
  } else {
    this->full_width = 0;
    this->connection_width = 0;
    this->connection_offset = 0;
  }
  this->calculated = TRUE;
}

void
SoSceneGraphNode::print(int indent)
{
  int c;
  for ( c = 0; c < indent; c++ ) fprintf(stdout, " ");
  fprintf(stdout, "node %s\n", node->getTypeId().getName().getString());
  for ( c = 0; c < indent; c++ ) fprintf(stdout, " ");
  fprintf(stdout, ": f %g w %g o %g\n", this->full_width, this->connection_width, this->connection_offset);
  if ( this->children ) {
    for ( int i = 0, num = this->children->getLength(); i < num; i++ )
      this->children->operator[](i)->print(indent + 2);
  }
}

// *************************************************************************

class SoGenerateSceneGraphActionP {
public:
  SoGenerateSceneGraphActionP(SoGenerateSceneGraphAction * api);
  ~SoGenerateSceneGraphActionP(void);

  SbBool enablenodenames;
  SbBool enablenodetypes;
  SbBool enabledroptypeifname;

  void clear(void);
  void enterNode(SoNode * node);
  void pushLevel(void);
  void popLevel(void);
  void exitNode(SoNode * node);

  void visit(SoNode * node);

  SoSeparator * getGraph(void) const;

  void buildSceneGraph(void);
  SoSeparator * buildSubSceneGraph(SoSceneGraphNode * sgnode);

protected:
  SoSeparator * root;
  SoSceneGraphNode * sgroot, * current;
  SbList<SoSceneGraphNode *> * stack;

private:
  SoSearchAction * searcher;
  SoNode * getSceneGraphItem(SbString itemName);
  SoGenerateSceneGraphAction * api;
};

SoGenerateSceneGraphActionP::SoGenerateSceneGraphActionP(SoGenerateSceneGraphAction * api)
{
  this->api = api;

  this->enablenodenames = TRUE;
  this->enablenodetypes = TRUE;
  this->enabledroptypeifname = FALSE;

  this->root = NULL;
  this->sgroot = NULL;
  this->stack = NULL;
  this->searcher = new SoSearchAction;
}

SoGenerateSceneGraphActionP::~SoGenerateSceneGraphActionP(void)
{
  if ( this->root ) this->root->unref();
  delete this->stack;
  delete this->sgroot;
  delete this->searcher;
}

SoNode *
SoGenerateSceneGraphActionP::getSceneGraphItem(SbString itemName)
{
  static SoSeparator * sgitems = NULL; // how do you unref/delete this on exit?
  if ( sgitems == NULL ) {
    SoInput in;
    if ( !in.openFile("SceneGraphItems.iv") ) {
      fprintf(stderr, "unable to open SceneGraphItems.iv\n");
      return NULL;
    } else {
      sgitems = SoDB::readAll(&in);
      if ( sgitems == NULL ) {
        fprintf(stderr, "no scene graph items\n");
        return NULL;
      } else {
        sgitems->ref();
      }
    }
  }
  SoPath * path = NULL;

  this->searcher->reset();
  this->searcher->setName(itemName);
  this->searcher->setInterest(SoSearchAction::FIRST);
  this->searcher->apply(sgitems);
  path = this->searcher->getPath();

  SoType type = SoType::fromName(itemName);
  while ( path == NULL && !type.isBad() ) {
    SbString name;
    name.sprintf("%sNode", type.getName().getString());
    this->searcher->reset();
    this->searcher->setName(name);
    this->searcher->setInterest(SoSearchAction::FIRST);
    this->searcher->apply(sgitems);
    path = this->searcher->getPath();
    type = type.getParent();
  }
  return (path != NULL) ? path->getTail()->copy() : NULL;
}

// *************************************************************************

SoSeparator *
SoGenerateSceneGraphActionP::getGraph(void) const
{
  return this->root;
}

void
SoGenerateSceneGraphActionP::clear(void)
{
  if ( this->root ) {
    this->root->unref();
    this->root = NULL;
  }
  this->sgroot = NULL;
  this->current = NULL;
  delete this->stack;
  this->stack = new SbList<SoSceneGraphNode *>;
}

// *************************************************************************

// Generating a scene graph structure happens in a combination of
// these different operations, happening in a recursive tree-traversal
// sequence.

//                  1*4
//            2      |      3   
//             +-----+-----+    
//             |           |    
//            1*4         1*4   
//         2   |   3   2   |   3
//          +--+--+     +--+--+
//          |     |     |     | 
//         1*4   1*4   1*4   1*4
//
// 1: enterNode()
// 2: pushLevel()
// 3: popLevel()
// 4: exitNode()


// 1: enterNode()
// - insert right-shift translation
// - insert node geometry (sphere)

void
SoGenerateSceneGraphActionP::enterNode(SoNode * node)
{
  this->current = new SoSceneGraphNode;
  const int level = this->stack->getLength();
  if ( level > 0 ) {
    SoSceneGraphNode * parent = (*this->stack)[level-1];
    parent->addChild(this->current);
    this->current->parent = parent;
  }

  this->current->node = node;
}

// 2: pushLevel()
// - create new group for children
// - initialize children-width measure
// - new group is local "root"

void
SoGenerateSceneGraphActionP::pushLevel(void)
{
  assert(this->current != NULL);
  this->stack->push(this->current);
  this->current = NULL;
}

// 3: popLevel()
// - hook up children group to parent
// - adjust leftshift of whole children group

void
SoGenerateSceneGraphActionP::popLevel(void)
{
  assert(this->stack->getLength() > 0);
  this->sgroot = this->stack->pop();
  this->sgroot->calculate();
}

// 4: exitNode()
// - adjust leftshift based on width

void
SoGenerateSceneGraphActionP::exitNode(SoNode * node)
{
  this->current = NULL;
}

void
SoGenerateSceneGraphActionP::visit(SoNode * node)
{
  assert(node != NULL && node->getTypeId() != SoType::badType());
  this->enterNode(node);
  if ( node->getTypeId().isDerivedFrom(SoGroup::getClassTypeId()) ) {
    SoGroup * group = (SoGroup *) node;
    if ( group->getChildren()->getLength() > 0 ) {
      this->pushLevel();
      group->getChildren()->traverse(this->api);
      this->popLevel();
    }
  }
  this->exitNode(node);
}

// *************************************************************************

void
SoGenerateSceneGraphActionP::buildSceneGraph(void)
{
  if ( !this->sgroot ) return;
  this->root = new SoSeparator;
  this->root->ref();
  this->root->addChild(this->getSceneGraphItem("SceneGraphHeader"));
  this->root->addChild(this->buildSubSceneGraph(this->sgroot));
  SoTweakAction tweaker;
  tweaker.setClearNodeNames(TRUE);
  tweaker.apply(this->root);
}

static
void
addConnectorStalk(SoSeparator * parent, float offset, float width)
{
  SoSeparator * stalksep = new SoSeparator;
  SoTranslation * translatedown = new SoTranslation;
  SoTexture2 * texture2 = new SoTexture2;
  SoMaterial * material = new SoMaterial;
  SoCube * cube = new SoCube;
  // SoCylinder * cylinder = new SoCylinder;
  // translatedown->translation.setValue(SbVec3f(0.0f, -0.25f, 0.0f));
  translatedown->translation.setValue(SbVec3f(0.0f, -0.35f, 0.0f));
  // cylinder->height.setValue(0.5);
  // cylinder->radius.setValue(0.02);
  material->diffuseColor.setValue(SbColor(0.0f, 0.0f, 0.0f));
  material->emissiveColor.setValue(SbColor(0.8f, 0.8f, 0.0f));
  cube->width.setValue(0.04f);
  cube->depth.setValue(0.04f);
  cube->height.setValue(0.3f);
  stalksep->addChild(translatedown);
  stalksep->addChild(texture2);
  stalksep->addChild(material);
  stalksep->addChild(cube);
  parent->addChild(stalksep);
}

static
void
addConnector(SoSeparator * root, float offset, float width)
{
  SoSeparator * stalksep = new SoSeparator;
  SoTranslation * translateup = new SoTranslation;
  SoRotation * rotate = new SoRotation;
  SoTexture2 * texture2 = new SoTexture2;
  SoMaterial * material = new SoMaterial;
  // SoCylinder * cylinder = new SoCylinder;
  SoCube * cube = new SoCube;
  translateup->translation.setValue(SbVec3f((width / 2.0f), 0.5f, 0.0f));
  // rotate->rotation.setValue(SbRotation(SbVec3f(0.0f, 0.0f, 1.0f), 3.1415926f/2.0f));
  // cylinder->height.setValue(width);
  // cylinder->radius.setValue(0.02);
  material->diffuseColor.setValue(SbColor(0.0f, 0.0f, 0.0f));
  material->emissiveColor.setValue(SbColor(0.8f, 0.8f, 0.0f));
  cube->width.setValue(width + 0.04f);
  cube->height.setValue(0.04f);
  cube->depth.setValue(0.04f);
  stalksep->addChild(translateup);
  stalksep->addChild(texture2);
  stalksep->addChild(material);
  stalksep->addChild(cube);
  root->addChild(stalksep);
}

SoSeparator *
SoGenerateSceneGraphActionP::buildSubSceneGraph(SoSceneGraphNode * sgnode)
{
  // add node sphere
  SoSeparator * root = new SoSeparator;
  assert(sgnode->node != NULL);
  SoNode * geometry = this->getSceneGraphItem(sgnode->node->getTypeId().getName().getString());
  root->addChild(geometry);
  if ( this->enablenodetypes || this->enablenodenames ) {
    SoSeparator * nodetype = new SoSeparator;
    SoTexture2 * texture2 = new SoTexture2;
    SoTranslation * translation = new SoTranslation;
    SoScale * scale = new SoScale;
    SoMaterial * material = new SoMaterial;
    SoAsciiText * text = new SoAsciiText;
    translation->translation.setValue(SbVec3f(0.25f, 0.07f, 0.0f));
    scale->scaleFactor.setValue(SbVec3f(0.01f, 0.01f, 0.01f));
    material->emissiveColor.setValue(SbVec3f(0.0f, 0.5f, 1.0f));
    SbBool addtype = TRUE;
    if ( this->enablenodenames && (strcmp(sgnode->node->getName().getString(), "") != 0) ) {
      if ( this->enablenodetypes && !this->enabledroptypeifname ) {
        translation->translation.setValue(SbVec3f(0.25f, 0.02f, 0.0f));
      } else {
        addtype = FALSE;
        translation->translation.setValue(SbVec3f(0.25f, -0.03f, 0.0f));
      }
      SbString name;
      name.sprintf("%s", sgnode->node->getName().getString());
      text->string.set1Value(0, name);
    }
    if ( this->enablenodetypes && addtype ) {
      SbString name;
      name.sprintf("%s", sgnode->node->getTypeId().getName().getString());
      text->string.set1Value(1, name);
    }
    nodetype->addChild(translation);
    nodetype->addChild(scale);
    nodetype->addChild(texture2);
    nodetype->addChild(material);
    nodetype->addChild(text);
    root->addChild(nodetype);
  }
  if ( sgnode->children != NULL ) {
    addConnectorStalk(root, sgnode->connection_offset, sgnode->connection_width);
    SoTranslation * translation = new SoTranslation;
    translation->translation.setValue(SbVec3f(-(sgnode->connection_width/2.0f), -1.0f, 0.0f));
    root->addChild(translation);
    addConnector(root, sgnode->connection_offset, sgnode->connection_width);
    for ( int i = 0, num = sgnode->children->getLength(); i < num; i++ ) {
      SoSceneGraphNode * child = sgnode->children->operator[](i);
      SoSeparator * subgraph = this->buildSubSceneGraph(child);
      root->addChild(subgraph);
      root->addChild(this->getSceneGraphItem("NodeStalk"));
      // addStalk(root);
      if ( i != (num - 1) ) {
        SoSceneGraphNode * next = sgnode->children->operator[](i+1);
        SoTranslation * shift = new SoTranslation;
        float leftshift = child->full_width - child->connection_offset - (child->connection_width / 2.0f);
        leftshift += 1.0f;
        leftshift += next->connection_offset + (next->connection_width / 2.0f);
        shift->translation.setValue(SbVec3f(leftshift, 0.0f, 0.0f));
        root->addChild(shift);
      }
    }
  }
  return root;
}

// *************************************************************************

#define THIS this->pimpl

SO_ACTION_SOURCE(SoGenerateSceneGraphAction);

void
SoGenerateSceneGraphAction::initClass(void)
{
  SO_ACTION_INIT_CLASS(SoGenerateSceneGraphAction, SoAction);
  SO_ACTION_ADD_METHOD(SoNode, SoGenerateSceneGraphAction::visitS);
}

SoGenerateSceneGraphAction::SoGenerateSceneGraphAction(void)
{
  THIS = new SoGenerateSceneGraphActionP(this);
  SO_ACTION_CONSTRUCTOR(SoGenerateSceneGraphAction);
}

SoGenerateSceneGraphAction::~SoGenerateSceneGraphAction(void)
{
  delete THIS;
}

void
SoGenerateSceneGraphAction::setNodeNamesEnabled(SbBool enabled)
{
  THIS->enablenodenames = enabled;
}

SbBool
SoGenerateSceneGraphAction::isNodeNamesEnabled(void) const
{
  return THIS->enablenodenames;
}

void
SoGenerateSceneGraphAction::setNodeTypesEnabled(SbBool enabled)
{
  THIS->enablenodetypes = enabled;
}

SbBool
SoGenerateSceneGraphAction::isNodeTypesEnabled(void) const
{
  return THIS->enablenodetypes;
}

void
SoGenerateSceneGraphAction::setDropTypeIfNameEnabled(SbBool enabled)
{
  THIS->enabledroptypeifname = enabled;
}

SbBool
SoGenerateSceneGraphAction::isDropTypeIfNameEnabled(void) const
{
  return THIS->enabledroptypeifname;
}

SoSeparator *
SoGenerateSceneGraphAction::getGraph(void) const
{
  return THIS->getGraph();
}

void
SoGenerateSceneGraphAction::beginTraversal(SoNode * node)
{
  assert(this->traversalMethods);
  THIS->clear();
  this->traverse(node);
  THIS->buildSceneGraph();
}

void
SoGenerateSceneGraphAction::visitS(SoAction * action, SoNode * node)
{
  assert(action && action->getTypeId().isDerivedFrom(SoGenerateSceneGraphAction::getClassTypeId()));
  ((SoGenerateSceneGraphAction *) action)->pimpl->visit(node);
}

// *************************************************************************
