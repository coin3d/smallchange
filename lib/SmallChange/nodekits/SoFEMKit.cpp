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

/*!
  \class SoFEMKit SoFEMKit.h
  \brief The SoFEMKit class is used to visualize finite element meshes.

  \ingroup nodekits
*/

/*!
  \var SoSFBool SoFEMKit::ccw
  Set to FALSE if you use a left handed coordinate system (or a coordinate system
  where the model matrix determinant is negative).
*/

#include "SoFEMKit.h"

#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoNormal.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoIndexedFaceSet.h>
#include <Inventor/nodes/SoPackedColor.h>
#include <Inventor/nodes/SoShapeHints.h>
#include <Inventor/nodes/SoMaterialBinding.h>
#include <Inventor/nodes/SoNormalBinding.h>
#include <Inventor/lists/SbList.h>
#include <Inventor/SbColor.h>
#include <Inventor/SbPlane.h>
#include <Inventor/SbBSPTree.h>
#include <Inventor/sensors/SoFieldSensor.h>
#include <Inventor/sensors/SoOneShotSensor.h>
#include <Inventor/actions/SoGLRenderAction.h>

#include <cstring>

#ifndef DOXYGEN_SKIP_THIS

typedef struct {
  int coloridx;
  int layerindex;
  int nodes[4];
  SbBool active;
} SoFEM2DElement;

typedef struct {
  int coloridx;
  int layerindex;
  int nodes[8];
  SbBool active;
} SoFEM3DElement;

typedef struct {
  int index;
  SbBool is3d;
} SoFEMLookup;

typedef struct {
  SbVec3f coords;
  int coloridx;
} SoFEMNode;

class SoFEMKitP {
public:
  SbList <uint32_t> colors;
  SbList <SoFEMNode> nodes;
  SbList <SoFEM2DElement> elements2d;
  SbList <SoFEM3DElement> elements3d;
  
  SbList <int> nodelookup;
  SbList <SoFEMLookup> elementlookup;

  SbBool removehidden;

  SoFieldSensor * ccwsensor;
  SoOneShotSensor * updatesensor;
};

#endif // DOXYGEN_SKIP_THIS

// convenience define to access private data
#undef THIS // windoze defines this somewhere *sigh*
#define THIS this->pimpl

SO_KIT_SOURCE(SoFEMKit);

/*!
  Constructor. 
*/
SoFEMKit::SoFEMKit(void) 
{
  THIS = new SoFEMKitP;
  THIS->removehidden = TRUE;

  SO_KIT_CONSTRUCTOR(SoFEMKit);

  SO_KIT_ADD_FIELD(ccw, (TRUE));

  SO_KIT_ADD_CATALOG_ENTRY(topSeparator, SoSeparator, FALSE, this, "", FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(shapehints, SoShapeHints, FALSE, topSeparator, mbind, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(mbind, SoMaterialBinding, FALSE, topSeparator, nbind, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(nbind, SoNormalBinding, FALSE, topSeparator, nodes, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(nodes, SoCoordinate3, FALSE, topSeparator, colors, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(colors, SoPackedColor, FALSE, topSeparator, normals, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(normals, SoNormal, FALSE, topSeparator, faceset, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(faceset, SoIndexedFaceSet, FALSE, topSeparator, "", FALSE);

  SO_KIT_INIT_INSTANCE();

  SoMaterialBinding * mb = (SoMaterialBinding*) this->getAnyPart("mbind", TRUE);
  mb->value = SoMaterialBinding::PER_VERTEX_INDEXED;
  this->mbind.setDefault(TRUE);

  SoNormalBinding * nb = (SoNormalBinding*) this->getAnyPart("nbind", TRUE);
  nb->value = SoNormalBinding::PER_VERTEX_INDEXED;
  this->nbind.setDefault(TRUE);

  THIS->ccwsensor = new SoFieldSensor(ccw_cb, this);
  THIS->ccwsensor->setPriority(0);
  THIS->ccwsensor->attach(&this->ccw);
  
  THIS->updatesensor = new SoOneShotSensor(update_cb, this);
  THIS->updatesensor->setPriority(1); // high priority so that it triggers before rendering

  // set up shape hints
  SoFEMKit::ccw_cb(this, THIS->ccwsensor);
  this->shapehints.setDefault(TRUE);
}

void
SoFEMKit::ccw_cb(void * closure, SoSensor *)
{
  SoFEMKit * thisp = (SoFEMKit*) closure;
  SoShapeHints * sh = (SoShapeHints*) thisp->getAnyPart("shapehints", TRUE);
  sh->vertexOrdering = thisp->ccw.getValue() ? SoShapeHints::COUNTERCLOCKWISE : SoShapeHints::CLOCKWISE;
  sh->creaseAngle = 0.0f;
  sh->faceType = SoShapeHints::CONVEX;
  sh->shapeType = SoShapeHints::SOLID;  
}

/*!
  Destructor
*/
SoFEMKit::~SoFEMKit()
{
  THIS->ccwsensor->detach();
  delete THIS->updatesensor;
  delete THIS->ccwsensor;
  delete THIS;
}

// Documented in superclass
void
SoFEMKit::initClass(void)
{
  static int first = 1;
  if (first) {
    first = 0;
    SO_KIT_INIT_CLASS(SoFEMKit, SoBaseKit, "BaseKit");
  }
}

/*!
  Reset the kit. All elements and nodes will be removed.
*/
void 
SoFEMKit::reset(void)
{
  THIS->nodelookup.truncate(0);
  THIS->elementlookup.truncate(0);
  THIS->nodes.truncate(0);
  THIS->colors.truncate(0);
  THIS->elements3d.truncate(0);
  THIS->elements2d.truncate(0);
  
  // just overwrite with new, empty nodes. The old ones will be deleted
  this->setAnyPart("nodes", new SoCoordinate3);
  this->setAnyPart("colors", new SoPackedColor);
  this->setAnyPart("normals", new SoNormal);
  this->setAnyPart("faceset", new SoIndexedFaceSet);

  if (THIS->updatesensor->isScheduled()) {
    THIS->updatesensor->unschedule();
  }

}

/*!
  Add a node (vertex) to the FEM. \a nodeidx should be used
  to refer to this node (e.g. when setting the node color or
  when creating elements). Each node must have an unique 
  \a nodeidx.
*/
void 
SoFEMKit::addNode(const int nodeidx, const SbVec3f & xyz)
{
  int n = THIS->nodes.getLength();
  
  while (THIS->nodelookup.getLength() <= nodeidx) {
    THIS->nodelookup.append(-1);
  }
  THIS->nodelookup[nodeidx] = n;

  SoFEMNode node;
  node.coords = xyz;
  node.coloridx = -1;
  THIS->nodes.append(node);

  THIS->updatesensor->schedule();
}

/*!
  Add a 3D element to the FEM. \a elementidx should be an unique
  index that is used when setting element color. \a nodes should
  be an array of eight node indices, ordered like this:

     5_______6
     /|     /|
   4/_|____/7|            z   
    | |    | |            |  y
    | |1 _ |_|2           | /
    | /    | /            |/
    |/     |/             ------x
    --------
   0        3

  \a layerindex can be used to specify the layer this element is in.

*/

void 
SoFEMKit::add3DElement(const int elementidx, const int32_t * nodes, const int layerindex)
{
  SoFEM3DElement elem;
  elem.active = TRUE;
  elem.layerindex = layerindex;
  elem.coloridx = -1;
  memcpy(elem.nodes, nodes, 8 * sizeof(int32_t));
  
  int n = THIS->elements3d.getLength();
  
  SoFEMLookup lookup;
  lookup.index = -1;
  lookup.is3d = FALSE;

  // append dummy lookup elements
  while (THIS->elementlookup.getLength() <= elementidx) {
    THIS->elementlookup.append(lookup);
  }
  // set the correct lookup element
  lookup.index = n;
  lookup.is3d = TRUE;
  THIS->elementlookup[elementidx] = lookup;  

  THIS->elements3d.append(elem);

  THIS->updatesensor->schedule();
}

/*!
  Add a 2D element to the FEM. \a elementidx must be an unique
  index that can be used when setting the element color. \a
  nodes should contain the four node indices in the following
  order:


  2______3
  |      |
  |      |      
  |      |           
  |------|
  0      1

  \a layerindex can be used to specify the layer this element is in.
*/

void 
SoFEMKit::add2DElement(const int elementidx, const int32_t * nodes, const int layerindex)
{
  SoFEM2DElement elem;
  elem.active = TRUE;
  elem.layerindex = layerindex;
  elem.coloridx = -1;
  memcpy(elem.nodes, nodes, 4 * sizeof(int32_t));
  
  int n = THIS->elements3d.getLength();
  
  SoFEMLookup lookup;
  lookup.index = -1;
  lookup.is3d = FALSE;

  // append dummy lookup elements
  while (THIS->elementlookup.getLength() <= elementidx) {
    THIS->elementlookup.append(lookup);
  }
  // set the correct lookup element
  lookup.index = n;
  lookup.is3d = TRUE;
  THIS->elementlookup[elementidx] = lookup;  

  THIS->updatesensor->schedule();
}

/*!
  Sets the \a nodeidx node color. Node color will override element
  colors if both the element color and a node color is set for any
  given node.
*/
void 
SoFEMKit::setNodeColor(const int nodeidx, const SbColor & color)
{
  assert(nodeidx >= 0 && nodeidx < THIS->nodelookup.getLength());

  int coloridx = THIS->colors.getLength();
  THIS->colors.append(color.getPackedValue());
  
  THIS->nodes[THIS->nodelookup[nodeidx]].coloridx = coloridx;
  THIS->updatesensor->schedule();
}

/*!
  Sets the \a elementidx element color.
*/
void 
SoFEMKit::setElementColor(const int elementidx, const SbColor & color)
{
  assert(elementidx >= 0 && elementidx < THIS->elementlookup.getLength());
  SoFEMLookup lookup = THIS->elementlookup[elementidx];

  int coloridx = THIS->colors.getLength();
  THIS->colors.append(color.getPackedValue());
  
  if (lookup.is3d) {
    THIS->elements3d[lookup.index].coloridx = coloridx;
  }
  else {
    THIS->elements2d[lookup.index].coloridx = coloridx;
  }
  THIS->updatesensor->schedule();
}

/*!
  Enable/disable all elements.
*/
void 
SoFEMKit::enableAllElements(const SbBool onoroff)
{
  int i;

  for (i = 0; i < THIS->elements3d.getLength(); i++) {
    THIS->elements3d[i].active = onoroff;
  }
  for (i = 0; i < THIS->elements2d.getLength(); i++) {
    THIS->elements2d[i].active = onoroff;
  }
  THIS->updatesensor->schedule();
}

static SbBool
intersect_plane(const SbPlane & p, 
                const int32_t * nodeidx, const int numnodes,
                const SbList <SoFEMNode> & nodes,
                const SbList <int> & nodelookup)
{
  int i;
    
  int numinfront = 0;
  int numbehind = 0;

  for (i = 0; i < numnodes; i++) {
    SbVec3f v = nodes[nodelookup[nodeidx[i]]].coords;
    if (p.isInHalfSpace(v)) numinfront++;
    else numbehind++;

    if (numinfront > 0 && numbehind > 0) return TRUE;
  }
  return FALSE;
}

/*!
  Enable/disable elements intersecting \a plane.
*/
void
SoFEMKit::enableElements(const SbPlane & plane, const SbBool onoroff)
{
  int i;
  for (i = 0; i < THIS->elements3d.getLength(); i++) {
    SbBool isect = intersect_plane(plane, THIS->elements3d[i].nodes, 8, THIS->nodes,
                                   THIS->nodelookup);
    if (isect) {
      THIS->elements3d[i].active = onoroff;
    }
  }
  for (i = 0; i < THIS->elements2d.getLength(); i++) {
    SbBool isect = intersect_plane(plane, THIS->elements2d[i].nodes, 4, 
                                   THIS->nodes, THIS->nodelookup);
    if (isect) {
      THIS->elements2d[i].active = onoroff;
    }
  }
  THIS->updatesensor->schedule();
}

/*!
  Enable/disable the \a elementidx element.
*/
void 
SoFEMKit::enableElement(const int elementidx, const SbBool onoff)
{
  assert(elementidx >= 0 && elementidx < THIS->elementlookup.getLength());
  SoFEMLookup lookup = THIS->elementlookup[elementidx];

  if (lookup.is3d) {
    THIS->elements3d[lookup.index].active = onoff;
  }
  else {
    THIS->elements2d[lookup.index].active = onoff;
  }
  THIS->updatesensor->schedule();
}

/*!
  Enable/disable elements in the \a layerindex layer.
*/
void 
SoFEMKit::enableLayer(const int layerindex, const SbBool onoroff)
{
  int i;
  for (i = 0; i < THIS->elements3d.getLength(); i++) {
    if (THIS->elements3d[i].layerindex == layerindex) {
      THIS->elements3d[i].active = onoroff;
    }
  }
  for (i = 0; i < THIS->elements2d.getLength(); i++) {
    if (THIS->elements2d[i].layerindex == layerindex) {
      THIS->elements2d[i].active = onoroff;
    }
  }
  THIS->updatesensor->schedule();
}


// doc in parent
void 
SoFEMKit::getBoundingBox(SoGetBoundingBoxAction * action)
{
  // SoGetBoundingBoxAction might be applied before the toolkit
  // processes the sensors. Do a manual check and update scene here
  // just in case.
  if (THIS->updatesensor->isScheduled()) {
    THIS->updatesensor->unschedule();
    update_cb(this, THIS->updatesensor);
  }
  inherited::getBoundingBox(action);
}

// doc in parent
void 
SoFEMKit::GLRender(SoGLRenderAction * action)
{
  inherited::GLRender(action);
}

/*!
  \internal
*/
void 
SoFEMKit::create2DIndices(int32_t * idxarray, const int32_t * nodes_org)
{
  int32_t nodes[4];
  for (int i = 0; i < 4; i++) {
    nodes[i] = THIS->nodelookup[nodes_org[i]];
  }

  idxarray[0] = nodes[0];
  idxarray[1] = nodes[1];
  idxarray[2] = nodes[3];
  idxarray[3] = nodes[2];
  idxarray[4] = -1;
}

/*!
  \internal
*/
void 
SoFEMKit::create3DIndices(int32_t * idxarray, const int32_t * nodes_org)
{
  int32_t nodes[8];
  for (int i = 0; i < 8; i++) {
    nodes[i] = THIS->nodelookup[nodes_org[i]];
  }

  idxarray[0] = nodes[0];
  idxarray[1] = nodes[1];
  idxarray[2] = nodes[2];
  idxarray[3] = nodes[3];
  idxarray[4] = -1;

  idxarray[5] = nodes[0];
  idxarray[6] = nodes[3];
  idxarray[7] = nodes[7];
  idxarray[8] = nodes[4];
  idxarray[9] = -1;

  idxarray[10] = nodes[4];
  idxarray[11] = nodes[7];
  idxarray[12] = nodes[6];
  idxarray[13] = nodes[5];
  idxarray[14] = -1;

  idxarray[15] = nodes[3];
  idxarray[16] = nodes[2];
  idxarray[17] = nodes[6];
  idxarray[18] = nodes[7];
  idxarray[19] = -1;

  idxarray[20] = nodes[1];
  idxarray[21] = nodes[0];
  idxarray[22] = nodes[4];
  idxarray[23] = nodes[5];
  idxarray[24] = -1;

  idxarray[25] = nodes[2];
  idxarray[26] = nodes[1];
  idxarray[27] = nodes[5];
  idxarray[28] = nodes[6];
  idxarray[29] = -1;
}

static SbVec3f 
calc_normal(SbVec3f * coords, const int32_t * cidx)
{
  int c0 = cidx[0];
  int c1 = cidx[1];
  int c2 = cidx[2];
  int c3 = cidx[3];

  SbVec3f v0 = coords[c1] - coords[c0];
  SbVec3f v1 = coords[c2] - coords[c0];
  SbVec3f v2 = coords[c3] - coords[c0];
  
  SbVec3f n0 = v0.cross(v1);
  SbVec3f n1 = v1.cross(v2);

  n0.normalize();
  n1.normalize();
  
  SbVec3f n = n0 + n1;
  
  n *= 0.5f;
  return n;
}

static void
count_vertices(SoFEMKit * fem,
               SoFEM3DElement & elem,
               int * vcnt) 
{
  if (!elem.active) return;

  int i;

  int32_t indices[30];
  
  fem->create3DIndices(indices, elem.nodes);
  
  for (i = 0; i < 30; i++) {
    int idx = indices[i];
    if (idx >= 0) vcnt[idx]++;
  }
}

static void
count_vertices(SoFEMKit * fem,
               SoFEM2DElement & elem,
               int * vcnt) 
{
  if (!elem.active) return;

  int i;

  int32_t indices[5];
  
  fem->create2DIndices(indices, elem.nodes);
  
  for (i = 0; i < 4; i++) {
    vcnt[indices[i]]++;
  }
}

static void
add_elem_3d(SoFEMKit * fem,
            SoFEM3DElement & elem,
            SbList <int32_t> & cidx,
            SbList <int32_t> & nidx,
            SbList <int32_t> & midx,
            SbVec3f * coords,
            SbBSPTree & normalbsp,
            const int * vcnt)
{
  if (!elem.active) return;

  int i;

  int32_t indices[30];
  
  fem->create3DIndices(indices, elem.nodes);

  // create normal for all six faces
  int normals[6];
  
  for (i = 0; i < 6; i++) {
    normals[i] = normalbsp.addPoint(calc_normal(coords, indices+i*5));
  }
  
  int coloridx = elem.coloridx >= 0 ? elem.coloridx : 0;
  
  for (int f = 0; f < 6; f++) {
    i = f * 5;
    int stop = i + 4;
    // when vcnt == 24 the node is hidden
    if (!vcnt || 
        (vcnt[indices[i]] < 24 &&
         vcnt[indices[i+1]] < 24 &&
         vcnt[indices[i+2]] < 24 &&
         vcnt[indices[i+3]] < 24)) {
      while (i < stop) {
        cidx.append(indices[i]);
        nidx.append(normals[i/5]);
        midx.append(coloridx);
        i++;
      } 
      cidx.append(-1);
      nidx.append(-1);
      midx.append(-1);
    } 
  }
}

static void
add_elem_2d(SoFEMKit * fem,
            SoFEM2DElement & elem,
            SbList <int32_t> & cidx,
            SbList <int32_t> & nidx,
            SbList <int32_t> & midx,
            SbVec3f * coords,
            SbBSPTree & normalbsp,
            const int * vcnt)
{
  if (!elem.active) return;

  int i;

  int32_t indices[5];
  
  fem->create2DIndices(indices, elem.nodes);

  // create normal for face
  int normalidx = normalbsp.addPoint(calc_normal(coords, indices));
  
  int coloridx = elem.coloridx >= 0 ? elem.coloridx : 0;

  // when vcnt == 24 the node is hidden
  if (!vcnt || 
      (vcnt[indices[0]] < 24 &&
       vcnt[indices[1]] < 24 &&
       vcnt[indices[2]] < 24 &&
       vcnt[indices[3]] < 24)) {
    for (i = 0; i < 4; i++) { 
      cidx.append(indices[i]);
      nidx.append(normalidx);
      midx.append(coloridx);
    } 
    cidx.append(-1);
    nidx.append(-1);
    midx.append(-1);
  }
}

void 
SoFEMKit::updateScene(void)
{
  SbBSPTree normalbsp;
  int i;

  SoNormal * normal = new SoNormal;
  SoCoordinate3 * coords = new SoCoordinate3;
  SoIndexedFaceSet * fs = new SoIndexedFaceSet;
  SoPackedColor * colors = new SoPackedColor;
  coords->point.setNum(THIS->nodes.getLength());
  SbVec3f * dstcoords = coords->point.startEditing();
  for (i = 0; i < THIS->nodes.getLength(); i++) {
    dstcoords[i] = THIS->nodes[i].coords;
  }
  coords->point.finishEditing();

  SbList <int32_t> cidx(1024);
  SbList <int32_t> nidx(1024);
  SbList <int32_t> midx(1024);

  int * vcnt = NULL;
  if (THIS->removehidden) {
    vcnt = new int[THIS->nodes.getLength()];
    memset(vcnt, 0, THIS->nodes.getLength()*sizeof(int));
    for (i = 0; i < THIS->elements3d.getLength(); i++) {
      SoFEM3DElement elem = THIS->elements3d[i];
      count_vertices(this, elem, vcnt);
    }
    for (i = 0; i < THIS->elements2d.getLength(); i++) {
      SoFEM2DElement elem = THIS->elements2d[i];
      count_vertices(this, elem, vcnt);
    }
  }

    
  for (i = 0; i < THIS->elements3d.getLength(); i++) {
    SoFEM3DElement elem = THIS->elements3d[i];
    
    add_elem_3d(this,
                elem,
                cidx, 
                nidx,
                midx, 
                dstcoords, normalbsp, vcnt);
  }
  for (i = 0; i < THIS->elements2d.getLength(); i++) {
    SoFEM2DElement elem = THIS->elements2d[i];
    
    add_elem_2d(this,
                elem,
                cidx, 
                nidx,
                midx, 
                dstcoords, normalbsp, vcnt);
  }
  
  delete[] vcnt;

  normal->vector.setNum(normalbsp.numPoints());
  SbVec3f * ndst = normal->vector.startEditing();
  for (i = 0; i < normalbsp.numPoints(); i++) {
    ndst[i] = normalbsp.getPoint(i);
  }
  normal->vector.finishEditing();

  colors->orderedRGBA.setNum(THIS->colors.getLength());
  uint32_t * dstcol = colors->orderedRGBA.startEditing();
  for (i = 0; i < THIS->colors.getLength(); i++) {
    dstcol[i] = THIS->colors[i];
  }
  colors->orderedRGBA.finishEditing();

  int numidx = cidx.getLength();
  fs->coordIndex.setNum(numidx);
  fs->materialIndex.setNum(numidx);
  fs->normalIndex.setNum(numidx);
  int32_t * cptr = fs->coordIndex.startEditing();
  int32_t * nptr = fs->normalIndex.startEditing();
  int32_t * mptr = fs->materialIndex.startEditing();

  for (i = 0; i < cidx.getLength(); i++) {
    int idx = cidx[i];
    cptr[i] = idx;
    mptr[i] = midx[i];
    nptr[i] = nidx[i];

    if (idx >= 0 && THIS->nodes[idx].coloridx >= 0) {
      mptr[i] = THIS->nodes[idx].coloridx;
    }

  }
  fs->coordIndex.finishEditing();
  fs->materialIndex.finishEditing();
  fs->normalIndex.finishEditing();

  // just overwrite with the new nodes. The old ones will be deleted
  this->setAnyPart("nodes", coords);
  this->setAnyPart("colors", colors);
  this->setAnyPart("normals", normal);
  this->setAnyPart("faceset", fs);
}

/*!
  Turn on/off simple (but effective!) optimization that removes
  all faces that are hidden by other faces. Default is
  to remove hidden faces.
*/
void 
SoFEMKit::removeHiddenFaces(const SbBool onoff)
{
  if (onoff != THIS->removehidden) {
    THIS->removehidden = onoff;
    THIS->updatesensor->schedule();
  }
}

void 
SoFEMKit::update_cb(void * data, SoSensor * sensor)
{
  ((SoFEMKit*)data)->updateScene();
}
