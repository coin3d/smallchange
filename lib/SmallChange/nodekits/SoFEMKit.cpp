/**************************************************************************\
 *
 *  Copyright (C) 1998-2000 by Systems in Motion. All rights reserved.
 *
 *  Systems in Motion, Prof Brochs gate 6, 7030 Trondheim, NORWAY
 *  http://www.sim.no support@sim.no Voice: +47 22114160 Fax: +47 22207097
 *
\**************************************************************************/

/*!
  \class SoFEMKit SoFEMKit.h
  \brief The SoFEMKit class is used to visualize finite element meshes.
  \ingroup nodekits
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

#include <string.h>

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

  SbBool needupdate;
  SbBool removehidden;
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
  THIS->needupdate = FALSE;
  THIS->removehidden = TRUE;

  SO_KIT_CONSTRUCTOR(SoFEMKit);

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

  SoNormalBinding * nb = (SoNormalBinding*) this->getAnyPart("nbind", TRUE);
  nb->value = SoNormalBinding::PER_VERTEX_INDEXED;

  SoShapeHints * sh = (SoShapeHints*) this->getAnyPart("shapehints", TRUE);
  sh->vertexOrdering = SoShapeHints::COUNTERCLOCKWISE;
  sh->creaseAngle = 0.0f;
  sh->faceType = SoShapeHints::CONVEX;
  sh->shapeType = SoShapeHints::SOLID;
}

/*!
  Destructor
*/
SoFEMKit::~SoFEMKit()
{
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

  THIS->needupdate = FALSE;
}

/*!
  Add a node (vertex) to the FEM. \a nodeidx should be used
  to refer to this node (e.g. when setting the node color or
  when creating elements). Each node must have an unique 
  nodeidx.
*/
void 
SoFEMKit::addNode(const int nodeidx, const SbVec3f & xyz)
{
  THIS->needupdate = TRUE;

  int n = THIS->nodes.getLength();
  
  while (THIS->nodelookup.getLength() <= nodeidx) {
    THIS->nodelookup.append(-1);
  }
  THIS->nodelookup[nodeidx] = n;

  SoFEMNode node;
  node.coords = xyz;
  node.coloridx = -1;
  THIS->nodes.append(node);
}

/*!
  Add a 3D element to the FEM. \a elementidx should be an unique
  index that is used when setting element color. \a nodes should
  be eight node indices, ordered like this:

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
  THIS->needupdate = TRUE;

  SoFEM3DElement elem;
  elem.active = TRUE;
  elem.layerindex = layerindex;
  elem.coloridx = -1;
  memcpy(elem.nodes, nodes, 8 * sizeof(int32_t));
  
  int n = THIS->elements3d.getLength();
  
  SoFEMLookup lookup;
  lookup.index = -1;

  // append dummy lookup elements
  while (THIS->elementlookup.getLength() <= elementidx) {
    THIS->elementlookup.append(lookup);
  }
  // set the correct lookup element
  lookup.index = n;
  lookup.is3d = TRUE;
  THIS->elementlookup[elementidx] = lookup;  

  THIS->elements3d.append(elem);
}

/*!
  Add a 2D element to the FEM. \a elementidx must be an unique
  index that can be used when setting the element color. \a
  nodes should contain the four node indices in the folowing
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
  THIS->needupdate = TRUE;

  SoFEM2DElement elem;
  elem.active = TRUE;
  elem.layerindex = layerindex;
  elem.coloridx = -1;
  memcpy(elem.nodes, nodes, 4 * sizeof(int32_t));
  
  int n = THIS->elements3d.getLength();
  
  SoFEMLookup lookup;
  lookup.index = -1;

  // append dummy lookup elements
  while (THIS->elementlookup.getLength() <= elementidx) {
    THIS->elementlookup.append(lookup);
  }
  // set the correct lookup element
  lookup.index = n;
  lookup.is3d = TRUE;
  THIS->elementlookup[elementidx] = lookup;  
}

/*!
  Sets the \a nodeidx node color. Node color will override element
  colors if both the element color and a node color is set for any
  given node.
*/
void 
SoFEMKit::setNodeColor(const int nodeidx, const SbColor & color)
{
  THIS->needupdate = TRUE;

  assert(nodeidx >= 0 && nodeidx < THIS->nodelookup.getLength());

  int coloridx = THIS->colors.getLength();
  THIS->colors.append(color.getPackedValue());
  
  THIS->nodes[THIS->nodelookup[nodeidx]].coloridx = coloridx;
}

/*!
  Sets the \elementidx element color.
*/
void 
SoFEMKit::setElementColor(const int elementidx, const SbColor & color)
{
  THIS->needupdate = TRUE;

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
}

/*!
  Enable/disable all elements.
*/
void 
SoFEMKit::enableAllElements(const SbBool onoroff)
{
  THIS->needupdate = TRUE;
  int i;

  for (i = 0; i < THIS->elements3d.getLength(); i++) {
    THIS->elements3d[i].active = onoroff;
  }
  for (i = 0; i < THIS->elements2d.getLength(); i++) {
    THIS->elements2d[i].active = onoroff;
  }
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
  THIS->needupdate = TRUE;

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
}

/*!
  Enable/disable the \a elementidx element.
*/
void 
SoFEMKit::enableElement(const int elementidx, const SbBool onoff)
{
  THIS->needupdate = TRUE;

  assert(elementidx >= 0 && elementidx < THIS->elementlookup.getLength());
  SoFEMLookup lookup = THIS->elementlookup[elementidx];

  if (lookup.is3d) {
    THIS->elements3d[lookup.index].active = onoff;
  }
  else {
    THIS->elements2d[lookup.index].active = onoff;
  }
}

/*!
  Enable/disable elements in the \a leyerindex layer.
*/
void 
SoFEMKit::enableLayer(const int layerindex, const SbBool onoroff)
{
  THIS->needupdate = TRUE;

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
}


// doc in parent
void 
SoFEMKit::getBoundingBox(SoGetBoundingBoxAction * action)
{
  this->updateScene();
  inherited::getBoundingBox(action);
}

// doc in parent
void 
SoFEMKit::GLRender(SoGLRenderAction * action)
{
  this->updateScene();
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
  if (!THIS->needupdate) return;

  SbBSPTree normalbsp;
  int i;

  THIS->needupdate = FALSE;

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
    THIS->needupdate = TRUE;
  }
}


