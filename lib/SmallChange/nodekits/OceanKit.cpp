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
  \class SmOceanKit SmOceanKit.h
  \brief The SmOceanKit class... 
  \ingroup nodekits

  FIXME: doc
*/


#include "SmOceanKit.h"
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/actions/SoRayPickAction.h>
#include <Inventor/actions/SoHandleEventAction.h>
#include <Inventor/sensors/SoFieldSensor.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/nodes/SoShape.h>
#include <Inventor/nodes/SoShapeHints.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoCallback.h>
#include <Inventor/SoPickedPoint.h>
#include <Inventor/SoFullPath.h>
#include <Inventor/SbPlane.h>
#include <Inventor/SoPrimitiveVertex.h>
#include <Inventor/details/SoFaceDetail.h>
#include <Inventor/details/SoPointDetail.h>
#include <Inventor/bundles/SoMaterialBundle.h>
#include <Inventor/elements/SoViewVolumeElement.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <SmallChange/nodes/UTMPosition.h>
#include <Inventor/system/gl.h>
#include <Inventor/C/base/memalloc.h>

class SmOceanKitP {
public:
  SmOceanKitP(void) 
  { }
};

class ocean_quadnode;

// helper class for actually rendering the ocean
class OceanShape : public SoShape {
  typedef SoShape inherited;

  SO_NODE_HEADER(OceanShape);

public:
  static void initClass(void);
  OceanShape(void);

  SoSFVec2f size;

  virtual void GLRender(SoGLRenderAction * action);
  virtual void getPrimitiveCount(SoGetPrimitiveCountAction * action);
  virtual void rayPick(SoRayPickAction * action);
  virtual void notify(SoNotList * list);

protected:
  virtual ~OceanShape();
  virtual void generatePrimitives(SoAction * action);
  virtual void computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center);
  
private:  

  void updateQuadtree(SoState * state);
  ocean_quadnode * root;
  SbList <ocean_quadnode*> nodelist;

};


class ocean_quadnode {
public:
  enum Neighbor {
    BOTTOM = 0,
    RIGHT  = 1,
    TOP    = 2,
    LEFT =   3
  };

  enum Child {
    BOTTOM_LEFT  = 0,
    BOTTOM_RIGHT = 1,
    TOP_RIGHT    = 2,
    TOP_LEFT     = 3
  };

  ocean_quadnode(ocean_quadnode * parent,
                 const SbVec3f & c0,
                 const SbVec3f & c1,
                 const SbVec3f & c2,
                 const SbVec3f & c3);

  ~ocean_quadnode(void);
  const SbVec3f & getCorner(const int i) const;
  
  void split(const int findneighbors = 0);
  void distanceSplit(const SbVec3f & pos, 
                     const int level,
                     const int minlevel,
                     const int maxlevel);
  int isSplit(void) const;
  int hasChildren(void) const;

  ocean_quadnode * getChild(const int i);
  ocean_quadnode * getNeighbor(const int i);
  void setNeighbor(const int i, ocean_quadnode * node);
  ocean_quadnode * getParent(void);
  int getChildIndex(const ocean_quadnode * n) const;
  void clearNeighbors(void);

  ocean_quadnode * getNeighborChild(const int neighbor, const int child);
  ocean_quadnode * searchNeighbor(const int i);

  const SbVec3f * getCorners() {
    return this->corner;
  }
private:
  void rotate(void);
  friend class quadsphere;

  ocean_quadnode * parent;
  SbVec3f corner[4];
  ocean_quadnode * child[4];
  ocean_quadnode * neighbor[4];

  unsigned char neighbor_rotate_bits;
  unsigned char flags;

private:
  friend class OceanShape;
  void findNeighbors(void);
  void updateChildRotate(void);
  void setNeighborRotate(const int i, const int rot);
  int getNeighborRotate(const int i) const;
  void unsplit(void);
  void deleteHiddenChildren(void);

public:
  float debugcolor[3];
  static cc_memalloc * getAllocator();
  void * operator new(size_t size, cc_memalloc * memhandler);
  void operator delete(void * ptr);
};

static cc_memalloc * node_memalloc = NULL;
static int numnodes = 0;

#define PRIVATE(obj) (obj)->pimpl

SO_KIT_SOURCE(SmOceanKit);

/*!
  Constructor. 
*/
SmOceanKit::SmOceanKit(void)
{
  PRIVATE(this) = new SmOceanKitP;

  SO_KIT_CONSTRUCTOR(SmOceanKit);
  
  SO_KIT_ADD_FIELD(size, (10000.0f, 10000.0f));

  SO_KIT_ADD_CATALOG_ENTRY(topSeparator, SoSeparator, FALSE, this, "", FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(utmposition, UTMPosition, FALSE, topSeparator, material, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(material, SoMaterial, FALSE, topSeparator, shapeHints, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(shapeHints, SoShapeHints, FALSE, topSeparator, oceanShape, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(oceanShape, OceanShape, FALSE, topSeparator, "", FALSE);

  SO_KIT_INIT_INSTANCE();

  OceanShape * shape = (OceanShape*) this->getAnyPart("oceanShape", TRUE);
  shape->size.connectFrom(&this->size);
}

/*!
  Destructor.
*/
SmOceanKit::~SmOceanKit(void)
{
  delete PRIVATE(this);
}

// Documented in superclass
void
SmOceanKit::initClass(void)
{
  static int first = 1;
  if (first) {
    first = 0;
    SO_KIT_INIT_CLASS(SmOceanKit, SoBaseKit, "BaseKit");
    OceanShape::initClass();
  }
}

void 
SmOceanKit::setDefaultOnNonWritingFields(void)
{
  this->oceanShape.setDefault(TRUE);
}

//********************************************************************************************

SO_NODE_SOURCE(OceanShape);

OceanShape::OceanShape()
{
  SO_NODE_CONSTRUCTOR(OceanShape);
  SO_NODE_ADD_FIELD(size, (10000.0f, 10000.0f));

  this->root = NULL;
  if (node_memalloc == NULL) {
    node_memalloc = cc_memalloc_construct(sizeof(ocean_quadnode));
    numnodes++;
  }
}

OceanShape::~OceanShape()
{
  numnodes--;
  if (numnodes == 0) {
    cc_memalloc_destruct(node_memalloc);
    node_memalloc = NULL;
  }
}

void
OceanShape::initClass()
{
  static int first = 1;
  if (first) {
    first = 0;
    SO_NODE_INIT_CLASS(OceanShape, SoShape, "Shape");
  }
}

static void add_node_rec(SbList <ocean_quadnode*> & list, ocean_quadnode * node)
{
  if (!node->isSplit()) list.append(node);
  else {
    for (int i = 0; i < 4; i++) {
      ocean_quadnode * child = node->getChild(i);
      if (child->isSplit()) add_node_rec(list, child);
      else list.append(child);
    }
  }
}

void
OceanShape::updateQuadtree(SoState * state)
{
  SbVec2f s = this->size.getValue();
  int minlevel = 4, maxlevel = 12;

  const SbMatrix & mat = SoModelMatrixElement::get(state);
  const SbViewVolume & vv = SoViewVolumeElement::get(state);

  SbVec3f pos = vv.getProjectionPoint();
  // move camera position to object space
  mat.inverse().multVecMatrix(pos, pos);

  if (this->root) {
    this->root->clearNeighbors();
    this->root->unsplit();
    this->root->distanceSplit(pos, 1, minlevel, maxlevel);
    this->root->deleteHiddenChildren();
  }
  else {
    this->root = new (node_memalloc) ocean_quadnode(NULL,
                                                    SbVec3f(0.0, 0.0, 0.0),
                                                    SbVec3f(s[0], 0.0, 0.0),
                                                    SbVec3f(s[0], s[1], 0.0),
                                                    SbVec3f(0.0, s[1], 0.0));
    this->root->distanceSplit(pos, 1, minlevel, maxlevel); 
  }

}


void
OceanShape::GLRender(SoGLRenderAction * action)
{
  if (!this->shouldGLRender(action)) return;

  SoState * state = action->getState();

  SoMaterialBundle mb(action);
  mb.sendFirst();

  this->updateQuadtree(state);
  this->nodelist.truncate(0);
  add_node_rec(this->nodelist, this->root);

  for (int i = 0; i < nodelist.getLength(); i++) {
    ocean_quadnode * node = this->nodelist[i];
    const SbVec3f * corners = node->getCorners();
    SbVec3f c = (corners[0] + corners[2]) * 0.5f;

    glNormal3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3fv(c.getValue());
    for (int j = 0; j < 4; j++) {
      glVertex3fv(corners[j].getValue());
      ocean_quadnode * tmp = node->searchNeighbor(j);
      if (tmp && tmp->isSplit()) {
        SbVec3f e = (corners[j]+corners[(j+1)%4]) * 0.5f;
        glVertex3fv(e.getValue());
      }
    }
    glVertex3fv(corners[0].getValue());
    glEnd();
  }
}

/*!
*/
void
OceanShape::generatePrimitives(SoAction * action)
{
  SbVec2f s = this->size.getValue();
  SoPrimitiveVertex vertex;
  SoFaceDetail faceDetail;
  SoPointDetail pointDetail;

  vertex.setDetail(&pointDetail);
  vertex.setNormal(SbVec3f(0.0f, 0.0f, 1.0f));

  this->beginShape(action, QUADS, &faceDetail);
  vertex.setPoint(SbVec3f(0.0f, 0.0f, 0.0f));
  this->shapeVertex(&vertex);

  vertex.setPoint(SbVec3f(s[0], 0.0f, 0.0f));
  this->shapeVertex(&vertex);

  vertex.setPoint(SbVec3f(s[0], s[1], 0.0f));
  this->shapeVertex(&vertex);

  vertex.setPoint(SbVec3f(0.0f, s[1], 0.0f));
  this->shapeVertex(&vertex);
  this->endShape();
}

void
OceanShape::computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center)
{
  SbVec2f s = this->size.getValue();
  box.setBounds(0.0f, 0.0f, 0.0f, s[0], s[1], 0.0f);
  center.setValue(s[0]*0.5f, s[1]*0.5, 0.0f);
}

void
OceanShape::getPrimitiveCount(SoGetPrimitiveCountAction * action)
{
}

void 
OceanShape::rayPick(SoRayPickAction * action)
{
  if (!this->shouldRayPick(action)) return;
  action->setObjectSpace();

  SbVec2f s = this->size.getValue();
  SbPlane p(SbVec3f(0.0f, 0.0, 1.0f), 0.0f);
  SbVec3f isect;
  if (p.intersect(action->getLine(), isect)) {
    if ((isect[0] >= 0.0f) && (isect[1] >= 0.0f) &&
        (isect[0] <= s[0]) && (isect[1] <= s[1]) &&
        action->isBetweenPlanes(isect)) {
      // any details needed?
      (void) action->addIntersection(isect);
    }
  }
}

void 
OceanShape::notify(SoNotList * list)
{
  SoField * f = list->getLastField();
  if (f == &this->size) {
    delete this->root;
    this->root = NULL;
  }
}


//*****************************************************************************************

#define FLAG_ISSPLIT 0x1

cc_memalloc * 
ocean_quadnode::getAllocator()
{
  assert(node_memalloc);
  return node_memalloc;
}

ocean_quadnode::ocean_quadnode(ocean_quadnode * parent,
                 const SbVec3f & c0,
                 const SbVec3f & c1,
                 const SbVec3f & c2,
                 const SbVec3f & c3)
{
  this->parent = parent;

  this->corner[0] = c0;
  this->corner[1] = c1;
  this->corner[2] = c2;
  this->corner[3] = c3;
  
  this->child[0] = NULL;
  this->child[1] = NULL;
  this->child[2] = NULL;
  this->child[3] = NULL;

  this->neighbor[0] = NULL;
  this->neighbor[1] = NULL;
  this->neighbor[2] = NULL;
  this->neighbor[3] = NULL;

  this->neighbor_rotate_bits = 0;
  this->flags = 0;

  if (parent && 0) {
    this->debugcolor[0] = parent->debugcolor[0];
    this->debugcolor[1] = parent->debugcolor[1];
    this->debugcolor[2] = parent->debugcolor[2];
  }
  else {
    for (int i = 0; i < 3; i++) {
      float val = (float) rand();
      val /= float(RAND_MAX);
      val += 1.0;
      val *= 0.5f;
      this->debugcolor[i] = val;
    }
  }
}

ocean_quadnode::~ocean_quadnode(void)
{
  for (int i = 0; i < 4; i++) {
    delete this->child[i];
  }
}

void 
ocean_quadnode::distanceSplit(const SbVec3f & pos,
                              const int level,
                              const int minlevel,
                              const int maxlevel)
{
  if (level == maxlevel) return;
  
  SbVec3f c = (this->corner[0] + this->corner[2]) * 0.5;
  SbVec3f s = c - this->corner[0];
  double len = s.sqrLength();
  double dist = (pos-c).sqrLength();

  // fprintf(stderr,"test: %g %g\n", len, dist);
  if ((dist < len*64.0) || (level < minlevel)) {
    if (!this->isSplit()) this->split(TRUE);
    else this->findNeighbors();
    for (int i = 0; i < 4; i++) {
      this->child[i]->distanceSplit(pos, level+1, minlevel, maxlevel);
    }
  }
  else {
    this->unsplit();
  }
}

void
ocean_quadnode::unsplit(void)
{
  if (this->isSplit()) { 
    this->flags &= ~FLAG_ISSPLIT;
    for (int i = 0; i < 4; i++) {
      this->child[i]->unsplit();
    }
  }
}

void 
ocean_quadnode::deleteHiddenChildren(void)
{
  if (!this->isSplit() && this->hasChildren()) {
    for (int i = 0; i < 4; i++) {
      this->child[i]->deleteHiddenChildren();
      delete this->child[i];
      this->child[i] = NULL;
    }
  }
}

void 
ocean_quadnode::clearNeighbors(void)
{
  int i;
  for (i = 0; i < 4; i++) {
    this->neighbor[i] = NULL;
  }
  if (this->hasChildren()) {
    for (i = 0; i < 4; i++) {
      this->child[i]->clearNeighbors();
    }
  }
}

void 
ocean_quadnode::split(const int findneighbors)
{
  if (this->isSplit()) {
    this->child[0]->split(1);
    this->child[1]->split(1);
    this->child[2]->split(1);
    this->child[3]->split(1);
  }
  else {
    if (!this->hasChildren()) {
      SbVec3f c = (this->corner[0] + this->corner[2]) * 0.5;
      SbVec3f e0 = (this->corner[0] + this->corner[1]) * 0.5;
      SbVec3f e1 = (this->corner[1] + this->corner[2]) * 0.5;
      SbVec3f e2 = (this->corner[2] + this->corner[3]) * 0.5;
      SbVec3f e3 = (this->corner[3] + this->corner[0]) * 0.5;
      
      this->child[BOTTOM_LEFT] = new (node_memalloc) ocean_quadnode(this, this->corner[0], e0, c, e3);
      this->child[BOTTOM_RIGHT] = new (node_memalloc)ocean_quadnode(this, e0, this->corner[1], e1, c);
      this->child[TOP_RIGHT] = new (node_memalloc)   ocean_quadnode(this, c, e1, this->corner[2], e2);
      this->child[TOP_LEFT] = new (node_memalloc)    ocean_quadnode(this, e3, c, e2, this->corner[3]);
      
      this->updateChildRotate();
    }
    this->flags |= FLAG_ISSPLIT;
    if (this->parent) this->parent->findNeighbors();
    
    if (findneighbors) {
      this->findNeighbors();
    }
  }
}


const SbVec3f & 
ocean_quadnode::getCorner(const int i) const
{
  return this->corner[i];
}

int 
ocean_quadnode::isSplit(void) const
{
  return (this->flags & FLAG_ISSPLIT) != 0;
}

int 
ocean_quadnode::hasChildren(void) const
{
  return this->child[0] != NULL;
}

ocean_quadnode * 
ocean_quadnode::getChild(const int i)
{
  return this->child[i];
}

void 
ocean_quadnode::rotate(void)
{
  SbVec3f tmp = this->corner[0];
  
  this->corner[0] = this->corner[1];
  this->corner[1] = this->corner[2];
  this->corner[2] = this->corner[3];
  this->corner[3] = tmp;
}

ocean_quadnode * 
ocean_quadnode::getNeighbor(const int i)
{
  return this->neighbor[i];
}

void 
ocean_quadnode::setNeighbor(const int i, ocean_quadnode * node)
{
  this->neighbor[i] = node;
}

ocean_quadnode * 
ocean_quadnode::getParent(void)
{
  return this->parent;
}


void
ocean_quadnode::findNeighbors()
{
  if (!this->parent) return;
  if (this->neighbor[0] &&
      this->neighbor[1] &&
      this->neighbor[2] &&
      this->neighbor[3]) return;
  
  ocean_quadnode *n[4];
  int i;
  int dosplit[4];
  int didsplit[4];

  for (int i = 0; i < 4; i++) {
    n[i] = this->parent->getNeighbor(i);
    dosplit[i] = 0;
    didsplit[i] = 0;
  }
  switch (this->parent->getChildIndex(this)) {
  case BOTTOM_LEFT:
    dosplit[LEFT] = 1;
    dosplit[BOTTOM] = 1;
    break;
  case BOTTOM_RIGHT:
    dosplit[BOTTOM] = 1;
    dosplit[RIGHT] = 1;
    break;
  case TOP_RIGHT:
    dosplit[TOP] = 1;
    dosplit[RIGHT] = 1;
    break;
  case TOP_LEFT:
    dosplit[TOP] = 1;
    dosplit[LEFT] = 1;
    break;
  default:
    assert(0 && "oops");
    break;
  }
  for (i = 0; i < 4; i++) {
    if (dosplit[i]) {
      if (n[i] && !n[i]->isSplit()) {
        n[i]->split(0);
        didsplit[i] = 1;
      }
    }
  }
  for (i = 0; i < 4; i++) {
    this->neighbor[i] = this->searchNeighbor(i);
  }
  for (i = 0; i < 4; i++) {
    if (didsplit[i] && n[i]) {
      n[i]->findNeighbors();
    }
  }
}

ocean_quadnode * 
ocean_quadnode::searchNeighbor(const int i)
{
  if (this->neighbor[i]) return this->neighbor[i];
  if (this->parent) {
    switch (this->parent->getChildIndex(this)) {
    case BOTTOM_LEFT:
      switch (i) {
      case LEFT:   return this->parent->getNeighborChild(LEFT, BOTTOM_RIGHT);
      case RIGHT:  return this->parent->getChild(BOTTOM_RIGHT);
      case TOP:    return this->parent->getChild(TOP_LEFT);
      case BOTTOM: return this->parent->getNeighborChild(BOTTOM, TOP_LEFT);
      default: assert(0 && "oops");
      }
      break;
    case BOTTOM_RIGHT:
      switch (i) {
      case RIGHT:  return this->parent->getNeighborChild(RIGHT, BOTTOM_LEFT);
      case LEFT:   return this->parent->getChild(BOTTOM_LEFT);
      case TOP:    return this->parent->getChild(TOP_RIGHT);
      case BOTTOM: return this->parent->getNeighborChild(BOTTOM, TOP_RIGHT);
      default: assert(0 && "oops");
      }
      break;
    case TOP_LEFT:
      switch (i) {
      case LEFT:   return this->parent->getNeighborChild(LEFT, TOP_RIGHT);
      case RIGHT:  return this->parent->getChild(TOP_RIGHT);
      case BOTTOM: return this->parent->getChild(BOTTOM_LEFT);
      case TOP:    return this->parent->getNeighborChild(TOP, BOTTOM_LEFT);
      default: assert(0 && "oops");
      }
      break;
    case TOP_RIGHT:
      switch (i) {
      case RIGHT:  return this->parent->getNeighborChild(RIGHT, TOP_LEFT);
      case LEFT:   return this->parent->getChild(TOP_LEFT);
      case BOTTOM: return this->parent->getChild(BOTTOM_RIGHT);
      case TOP:    return this->parent->getNeighborChild(TOP, BOTTOM_RIGHT);
      default: assert(0 && "oops");
      }
      break;
    default:
      assert(0 && "oops");
      break;
    }
  }
  return NULL;
}

int 
ocean_quadnode::getChildIndex(const ocean_quadnode * n) const
{
  for (int i = 0; i < 4; i++) {
    if (this->child[i] == n) return i;
  }
  return -1;
}

void
ocean_quadnode::updateChildRotate(void)
{
  for (int i = 0; i < 4; i++) {
    ocean_quadnode * c = this->child[i];
    switch (i) {
    case BOTTOM_LEFT:
      c->setNeighborRotate(BOTTOM, this->getNeighborRotate(BOTTOM));
      c->setNeighborRotate(LEFT, this->getNeighborRotate(LEFT));
      break;
    case BOTTOM_RIGHT:
      c->setNeighborRotate(BOTTOM, this->getNeighborRotate(BOTTOM));
      c->setNeighborRotate(RIGHT, this->getNeighborRotate(RIGHT));
      break;
    case TOP_LEFT:
      c->setNeighborRotate(TOP, this->getNeighborRotate(TOP));
      c->setNeighborRotate(LEFT, this->getNeighborRotate(LEFT));
      break;
    case TOP_RIGHT:
      c->setNeighborRotate(TOP, this->getNeighborRotate(TOP));
      c->setNeighborRotate(RIGHT, this->getNeighborRotate(RIGHT));
      break;
    default:
      assert(0 && "oops");
      break;
    }
  }
}

void 
ocean_quadnode::setNeighborRotate(const int i, const int rot)
{
  int shift = i*2;
  unsigned int mask = 0x3<<shift;
  this->neighbor_rotate_bits &= ~mask;
  this->neighbor_rotate_bits |= rot<<shift;
}

int 
ocean_quadnode::getNeighborRotate(const int i) const
{
  return (this->neighbor_rotate_bits>>(i*2))&0x3;
}

ocean_quadnode * 
ocean_quadnode::getNeighborChild(const int neighbor, const int child)
{
  ocean_quadnode * n = this->neighbor[neighbor];
  if (n) {
    int idx = child - this->getNeighborRotate(neighbor);
    return n->getChild((idx+4)%4);
  }
  return NULL;
}

void * 
ocean_quadnode::operator new(size_t size, cc_memalloc *memhandler)
{
  return cc_memalloc_allocate(memhandler);
}

void 
ocean_quadnode::operator delete(void * ptr)
{
  cc_memalloc_deallocate(node_memalloc, ptr);
}

#undef FLAG_ISSPLIT


