/**************************************************************************\
 *
 *  This file is part of the Coin 3D visualization library.
 *  Copyright (C) 1998-2003 by Systems in Motion.  All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  ("GPL") version 2 as published by the Free Software Foundation.
 *  See the file LICENSE.GPL at the root directory of this source
 *  distribution for additional information about the GNU GPL.
 *
 *  For using Coin with software that can not be combined with the GNU
 *  GPL, and for taking advantage of the additional benefits of our
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
  \class SmMarkerSet SmMarkerSet.h SmallChange/nodes/SmMarkerSet.h
  \brief The SmMarkerSet class displays a set of 2D bitmap markers in 3D.
  \ingroup nodes

  This node is identical with the Coin3D node called SoMarkerSet. The
  only difference is the new field called \var maxMarkersToRender
  which specifies how many markers to render based on the distance to
  the camera.

*/

#include "SmMarkerSet.h"
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/elements/SoGLCoordinateElement.h>
#include <Inventor/elements/SoMaterialBindingElement.h>
#include <Inventor/bundles/SoMaterialBundle.h>
#include <Inventor/elements/SoGLTextureEnabledElement.h>
#include <Inventor/elements/SoGLTexture3EnabledElement.h>
#include <Inventor/elements/SoLazyElement.h>
#include <Inventor/elements/SoViewVolumeElement.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoViewingMatrixElement.h>
#include <Inventor/elements/SoProjectionMatrixElement.h>
#include <Inventor/elements/SoViewportRegionElement.h>
#include <Inventor/elements/SoCullElement.h>
#include <Inventor/misc/SoState.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#include <Inventor/system/gl.h>

#if COIN_DEBUG
#include <Inventor/errors/SoDebugError.h>
#endif // COIN_DEBUG

/*!
  \enum SoMarkerSet::MarkerType
  Defines the different standard markers.
*/
// FIXME: should have a png picture in the doc showing the various
// marker graphics. 20010815 mortene.

/*!
  \var SoMFInt32 SoMarkerSet::markerIndex
  Contains the set of index markers to display, defaults to 0 (CROSS_5_5).
  The special value NONE renders nothing for that marker.
*/

/*!
  \var SoSFInt32 SmMarkerSet::maxMarkersToRender

  Specifies how many markers to render based on their closeness to the camera.
*/


// *************************************************************************

SO_NODE_SOURCE(SmMarkerSet);

struct smmarkerset_indexdistance {
  unsigned int index;
  float distance; 
};

static int smmarkerset_sortcompare(const void * element1, const void * element2)
{
  smmarkerset_indexdistance * item1 = (smmarkerset_indexdistance *) element1;
  smmarkerset_indexdistance * item2 = (smmarkerset_indexdistance *) element2;

  if (item1->distance < item2->distance) return -1;
  else if (item1->distance > item2->distance) return 1;
  else return 0;

}


class SmMarkerSetP {
public:
  SmMarkerSetP(SmMarkerSet * master) : master(master) { }
  smmarkerset_indexdistance * pointdistancelist;
  int pointdistancelistlen;

private:
  SmMarkerSet * master;
};

#undef PRIVATE
#define PRIVATE(obj) ((obj)->pimpl)
#undef PUBLIC
#define PUBLIC(obj) ((obj)->master)


/*!
  Constructor.
*/
SmMarkerSet::SmMarkerSet()
{
  PRIVATE(this) = new SmMarkerSetP(this);
  PRIVATE(this)->pointdistancelist = NULL;
  PRIVATE(this)->pointdistancelistlen = 0;

  SO_NODE_CONSTRUCTOR(SmMarkerSet);
  SO_NODE_ADD_FIELD(maxMarkersToRender, (-1));
}

/*!
  Destructor.
*/
SmMarkerSet::~SmMarkerSet()
{
  if (PRIVATE(this)->pointdistancelist != NULL)
    delete PRIVATE(this)->pointdistancelist;
}

// ----------------------------------------------------------------------
typedef struct {
  int width;
  int height;
  int align;
  unsigned char *data;
  SbBool deletedata;
} so_marker;

static SbList <so_marker> * markerlist;
static GLubyte * markerimages;
static void convert_bitmaps(void);
// -----------------------------------------------------------------------

// Internal method which translates the current material binding found
// on the state to a material binding for this node.  PER_PART,
// PER_FACE, PER_VERTEX and their indexed counterparts are translated
// to PER_VERTEX binding. OVERALL means overall binding for point set
// also, of course. The default material binding is OVERALL.
SmMarkerSet::Binding
SmMarkerSet::findMaterialBinding(SoState * const state) const
{
  Binding binding = OVERALL;
  if (SoMaterialBindingElement::get(state) !=
      SoMaterialBindingElement::OVERALL) binding = PER_VERTEX;
  return binding;
}

// doc in super
void
SmMarkerSet::initClass(void)
{
  SO_NODE_INIT_CLASS(SmMarkerSet, SoMarkerSet, SoMarkerSet);

  markerimages = new GLubyte[NUM_MARKERS*9*4]; // hardcoded markers, 32x9 bitmaps (9x9 used), dword alignment
  markerlist = new SbList<so_marker>;

  // FIXME: How should this be done as 'coin_atexit' is private? (20040209 handegar)
  //coin_atexit((coin_atexit_f *)free_marker_images, 0);

  convert_bitmaps();
  so_marker temp;
  for (int i = 0; i < NUM_MARKERS; i++) {
    temp.width = 9;
    temp.height = 9;
    temp.align = 4;
    temp.data = markerimages + (i * 36);
    temp.deletedata = FALSE;
    markerlist->append(temp);
  }
  
}


// doc in super
void
SmMarkerSet::GLRender(SoGLRenderAction * action)
{
  // FIXME: the marker bitmaps are toggled off when the leftmost pixel
  // is outside the left border, and ditto for the bottommost pixel
  // versus the bottom border. They should be drawn partly until they
  // are wholly outside the canvas instead. 20011218 mortene.

  SoState * state = action->getState();
  if (!this->shouldGLRender(action)) { return; }

  state->push();
  // We just disable lighting and texturing for markers, since we
  // can't see any reason this should ever be enabled.  send an angry
  // email to <pederb@coin3d.org> if you disagree.

  SoLazyElement::setLightModel(state, SoLazyElement::BASE_COLOR);
  SoGLTextureEnabledElement::set(state, this, FALSE);
  SoGLTexture3EnabledElement::set(state, this, FALSE);

  if (this->vertexProperty.getValue()) {
    this->vertexProperty.getValue()->GLRender(action);
  }

  const SoCoordinateElement * tmpcoord;
  const SbVec3f * dummy;
  SbBool needNormals = FALSE;

  SoVertexShape::getVertexData(state, tmpcoord, dummy, needNormals);
  const SoGLCoordinateElement * coords = (SoGLCoordinateElement *)tmpcoord;

  Binding mbind = this->findMaterialBinding(action->getState());
  SoMaterialBundle mb(action);
  mb.sendFirst();

  int matnr = 0;
  int32_t idx = this->startIndex.getValue();
  int32_t numpts = this->numPoints.getValue();
  if (numpts < 0) numpts = coords->getNum() - idx;

  const SbMatrix & mat = SoModelMatrixElement::get(state);
  const SbViewVolume & vv = SoViewVolumeElement::get(state);
  const SbViewportRegion & vp = SoViewportRegionElement::get(state);
  const SbMatrix & projmatrix = (mat * SoViewingMatrixElement::get(state) *
                                   SoProjectionMatrixElement::get(state));
  SbVec2s vpsize = vp.getViewportSizePixels();

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, vpsize[0], 0, vpsize[1], -1.0f, 1.0f);

  // Find the number of closest markers to render  
  if (numpts != PRIVATE(this)->pointdistancelistlen || 
      PRIVATE(this)->pointdistancelist == NULL) {    
    if (PRIVATE(this)->pointdistancelist != NULL)
      delete PRIVATE(this)->pointdistancelist;
    PRIVATE(this)->pointdistancelist = new smmarkerset_indexdistance[numpts];
    PRIVATE(this)->pointdistancelistlen = numpts;
  }  
  if (this->maxMarkersToRender.getValue() != -1) {
    SbVec3f campos = vv.getProjectionPoint();
    // Calculate distance to camera for all markers
    for (int i=0;i<numpts;++i) {        
      SbVec3f pointpos = coords->get3(idx + i);
      mat.multVecMatrix(pointpos, pointpos);        
      PRIVATE(this)->pointdistancelist[i].distance = (pointpos - campos).length();
      PRIVATE(this)->pointdistancelist[i].index = i;
    }
    // qsort array using distance as key
    qsort(PRIVATE(this)->pointdistancelist, numpts, 
          sizeof(smmarkerset_indexdistance), smmarkerset_sortcompare);
  } 
  else {
    // Regular rendering
    for (int i=0;i<numpts;++i) {
      PRIVATE(this)->pointdistancelist[i].index = i;
      PRIVATE(this)->pointdistancelist[i].distance = 0;
    }
  }


  int counter = (this->maxMarkersToRender.getValue() != -1) ? 
    this->maxMarkersToRender.getValue() : numpts;
  if (counter > numpts) counter = numpts; // Failsafe
  
  for (int i = 0; i < counter; i++) {
    
    const int index = PRIVATE(this)->pointdistancelist[i].index;
    int midx = SbMin(index, this->markerIndex.getNum() - 1);

#if COIN_DEBUG
      if (midx < 0 || (this->markerIndex[midx] >= markerlist->getLength())) {
        static SbBool firsterror = TRUE;
        if (firsterror) {
          SoDebugError::postWarning("SmMarkerSet::GLRender",
                                    "markerIndex %d out of bound",
                                    markerIndex[index]);
          firsterror = FALSE;
        }
        // Don't render, jump back to top of for-loop and continue with
        // next index.
        continue;
      }
#endif // COIN_DEBUG
      
    if (mbind == PER_VERTEX) 
      mb.send(index, TRUE);
      
    if (this->markerIndex[midx] == NONE) 
      { continue; }
    
    SbVec3f point = coords->get3(idx + index);
    
    // OpenGL's glBitmap() will not be clipped against anything but
    // the near and far planes. We want markers to also be clipped
    // against other clipping planes, to behave like the SoPointSet
    // superclass.
    const SbBox3f bbox(point, point);
    // FIXME: if there are *heaps* of markers, this next line will
    // probably become a bottleneck. Should really partition marker
    // positions in a oct-tree data structure and cull several at
    // the same time.  20031219 mortene.
    if (SoCullElement::cullTest(state, bbox, TRUE)) { continue; }
   
    projmatrix.multVecMatrix(point, point);
    point[0] = (point[0] + 1.0f) * 0.5f * vpsize[0];
    point[1] = (point[1] + 1.0f) * 0.5f * vpsize[1];      

    // To have the exact center point of the marker drawn at the
    // projected 3D position.  (FIXME: I haven't actually checked that
    // this is what TGS' implementation of the SoMarkerSet node does
    // when rendering, but it seems likely. 20010823 mortene.)
    so_marker * tmp = &(* markerlist)[this->markerIndex[midx]];
    point[0] = point[0] - (tmp->width - 1) / 2;
    point[1] = point[1] - (tmp->height - 1) / 2;

    glPixelStorei(GL_UNPACK_ALIGNMENT, tmp->align);
    glRasterPos3f(point[0], point[1], -point[2]);
    glBitmap(tmp->width, tmp->height, 0, 0, 0, 0, tmp->data);
  }

  // FIXME: this looks wrong, shouldn't we rather reset the alignment
  // value to what it was previously?  20010824 mortene.
  glPixelStorei(GL_UNPACK_ALIGNMENT, 4); // restore default value
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  state->pop(); // we pushed, remember
}


// NB: this array is copied directly from SoMarkerSet.

static char marker_char_bitmaps[] =
{
  // CROSS_5_5
  "         "
  "         "
  "  x   x  "
  "   x x   "
  "    x    "
  "   x x   "
  "  x   x  "
  "         "
  "         "
  // PLUS_5_5
  "         "
  "         "
  "    x    "
  "    x    "
  "  xxxxx  "
  "    x    "
  "    x    "
  "         "
  "         "
  // MINUS_5_5
  "         "
  "         "
  "         "
  "         "
  "  xxxxx  "
  "         "
  "         "
  "         "
  "         "
  // SLASH_5_5
  "         "
  "         "
  "      x  "
  "     x   "
  "    x    "
  "   x     "
  "  x      "
  "         "
  "         "
  // BACKSLASH_5_5
  "         "
  "         "
  "  x      "
  "   x     "
  "    x    "
  "     x   "
  "      x  "
  "         "
  "         "
  // BAR_5_5
  "         "
  "         "
  "    x    "
  "    x    "
  "    x    "
  "    x    "
  "    x    "
  "         "
  "         "
  // STAR_5_5
  "         "
  "         "
  "  x x x  "
  "   xxx   "
  "  xxxxx  "
  "   xxx   "
  "  x x x  "
  "         "
  "         "
  // Y_5_5
  "         "
  "         "
  "  x   x  "
  "   x x   "
  "    x    "
  "    x    "
  "    x    "
  "         "
  "         "
  // LIGHTNING_5_5
  "         "
  "         "
  "    x    "
  "     x   "
  "  xxxxx  "
  "   x     "
  "    x    "
  "         "
  "         "
  // WELL_5_5
  "         "
  "         "
  "    x    "
  "    x    "
  "   x x   "
  "   xxx   "
  "  x   x  "
  "         "
  "         "
  // CIRCLE_LINE_5_5
  "         "
  "         "
  "   xxx   "
  "  x   x  "
  "  x   x  "
  "  x   x  "
  "   xxx   "
  "         "
  "         "
  // SQUARE_LINE_5_5
  "         "
  "         "
  "  xxxxx  "
  "  x   x  "
  "  x   x  "
  "  x   x  "
  "  xxxxx  "
  "         "
  "         "
  // DIAMOND_LINE_5_5
  "         "
  "         "
  "    x    "
  "   x x   "
  "  x   x  "
  "   x x   "
  "    x    "
  "         "
  "         "
  // TRIANGLE_LINE_5_5
  "         "
  "         "
  "    x    "
  "    x    "
  "   x x   "
  "   x x   "
  "  xxxxx  "
  "         "
  "         "
  // RHOMBUS_LINE_5_5
  "         "
  "         "
  "         "
  "   xxxxx "
  "  x   x  "
  " xxxxx   "
  "         "
  "         "
  "         "
  // HOURGLASS_LINE_5_5
  "         "
  "         "
  "  xxxxx  "
  "   x x   "
  "    x    "
  "   x x   "
  "  xxxxx  "
  "         "
  "         "
  // SATELLITE_LINE_5_5
  "         "
  "         "
  "  x   x  "
  "   xxx   "
  "   x x   "
  "   xxx   "
  "  x   x  "
  "         "
  "         "
  // PINE_TREE_LINE_5_5
  "         "
  "         "
  "    x    "
  "   x x   "
  "  xxxxx  "
  "    x    "
  "    x    "
  "         "
  "         "
  // CAUTION_LINE_5_5
  "         "
  "         "
  "  xxxxx  "
  "   x x   "
  "   x x   "
  "    x    "
  "    x    "
  "         "
  "         "
  // SHIP_LINE_5_5
  "         "
  "         "
  "    x    "
  "    x    "
  "  xxxxx  "
  "   x x   "
  "    x    "
  "         "
  "         "
  // CIRCLE_FILLED_5_5
  "         "
  "         "
  "   xxx   "
  "  xxxxx  "
  "  xxxxx  "
  "  xxxxx  "
  "   xxx   "
  "         "
  "         "
  // SQUARE_FILLED_5_5
  "         "
  "         "
  "  xxxxx  "
  "  xxxxx  "
  "  xxxxx  "
  "  xxxxx  "
  "  xxxxx  "
  "         "
  "         "
  // DIAMOND_FILLED_5_5
  "         "
  "         "
  "    x    "
  "   xxx   "
  "  xxxxx  "
  "   xxx   "
  "    x    "
  "         "
  "         "
  // TRIANGLE_FILLED_5_5
  "         "
  "         "
  "    x    "
  "    x    "
  "   xxx   "
  "   xxx   "
  "  xxxxx  "
  "         "
  "         "
  // RHOMBUS_FILLED_5_5
  "         "
  "         "
  "         "
  "   xxxxx "
  "  xxxxx  "
  " xxxxx   "
  "         "
  "         "
  "         "
  // HOURGLASS_FILLED_5_5
  "         "
  "         "
  "  xxxxx  "
  "   xxx   "
  "    x    "
  "   xxx   "
  "  xxxxx  "
  "         "
  "         "
  // SATELLITE_FILLED_5_5
  "         "
  "         "
  "  x   x  "
  "   xxx   "
  "   xxx   "
  "   xxx   "
  "  x   x  "
  "         "
  "         "
  // PINE_TREE_FILLED_5_5
  "         "
  "         "
  "    x    "
  "   xxx   "
  "  xxxxx  "
  "    x    "
  "    x    "
  "         "
  "         "
  // CAUTION_FILLED_5_5
  "         "
  "         "
  "  xxxxx  "
  "   xxx   "
  "   xxx   "
  "    x    "
  "    x    "
  "         "
  "         "
  // SHIP_FILLED_5_5
  "         "
  "         "
  "    x    "
  "    x    "
  "  xxxxx  "
  "   xxx   "
  "    x    "
  "         "
  "         "
  // CROSS_7_7
  "         "
  " x     x "
  "  x   x  "
  "   x x   "
  "    x    "
  "   x x   "
  "  x   x  "
  " x     x "
  "         "
  // PLUS_7_7
  "         "
  "    x    "
  "    x    "
  "    x    "
  " xxxxxxx "
  "    x    "
  "    x    "
  "    x    "
  "         "
  // MINUS_7_7
  "         "
  "         "
  "         "
  "         "
  " xxxxxxx "
  "         "
  "         "
  "         "
  "         "
  // SLASH_7_7
  "         "
  "       x "
  "      x  "
  "     x   "
  "    x    "
  "   x     "
  "  x      "
  " x       "
  "         "
  // BACKSLASH_7_7
  "         "
  " x       "
  "  x      "
  "   x     "
  "    x    "
  "     x   "
  "      x  "
  "       x "
  "         "
  // BAR_7_7,
  "         "
  "    x    "
  "    x    "
  "    x    "
  "    x    "
  "    x    "
  "    x    "
  "    x    "
  "         "
  // STAR_7_7
  "         "
  "    x    "
  "  x x x  "
  "   xxx   "
  " xxxxxxx "
  "   xxx   "
  "  x x x  "
  "    x    "
  "         "
  // Y_7_7
  "         "
  " x     x "
  "  x   x  "
  "   x x   "
  "    x    "
  "    x    "
  "    x    "
  "    x    "
  "         "
  // LIGHTNING_7_7
  "         "
  "    x    "
  "     x   "
  "      x  "
  " xxxxxxx "
  "  x      "
  "   x     "
  "    x    "
  "         "
  // WELL_7_7
  "         "
  "    x    "
  "   x x   "
  "   xxx   "
  "  xx xx  "
  "  x x x  "
  "  xx xx  "
  " xx   xx "
  "         "
  // CIRCLE_LINE_7_7
  "         "
  "  xxxxx  "
  " x     x "
  " x     x "
  " x     x "
  " x     x "
  " x     x "
  "  xxxxx  "
  "         "
  // SQUARE_LINE_7_7
  "         "
  " xxxxxxx "
  " x     x "
  " x     x "
  " x     x "
  " x     x "
  " x     x "
  " xxxxxxx "
  "         "
  // DIAMOND_LINE_7_7
  "         "
  "    x    "
  "   x x   "
  "  x   x  "
  " x     x "
  "  x   x  "
  "   x x   "
  "    x    "
  "         "
  // TRIANGLE_LINE_7_7
  "         "
  "    x    "
  "    x    "
  "   x x   "
  "   x x   "
  "  x   x  "
  "  x   x  "
  " xxxxxxx "
  "         "
  // RHOMBUS_LINE_7_7
  "         "
  "         "
  "    xxxxx"
  "   x   x "
  "  x   x  "
  " x   x   "
  "xxxx     "
  "         "
  "         "
  // HOURGLASS_LINE_7_7
  "         "
  " xxxxxxx "
  "  x   x  "
  "   x x   "
  "    x    "
  "   x x   "
  "  x   x  "
  " xxxxxxx "
  "         "
  // SATELLITE_LINE_7_7
  "         "
  " x     x "
  "  x x x  "
  "   x x   "
  "  x   x  "
  "   x x   "
  "  x x x  "
  " x     x "
  "         "
  // PINE_TREE_LINE_7_7
  "         "
  "    x    "
  "   x x   "
  "  x   x  "
  " xxxxxxx "
  "    x    "
  "    x    "
  "    x    "
  "         "
  // CAUTION_LINE_7_7
  "         "
  " xxxxxxx "
  "  x   x  "
  "  x   x  "
  "   x x   "
  "   x x   "
  "    x    "
  "    x    "
  "         "
  // SHIP_LINE_7_7
  "         "
  "    x    "
  "    x    "
  "    x    "
  " xxxxxxx "
  "  x   x  "
  "   x x   "
  "    x    "
  "         "
  // CIRCLE_FILLED_7_7
  "         "
  "   xxx   "
  "  xxxxx  "
  " xxxxxxx "
  " xxxxxxx "
  " xxxxxxx "
  "  xxxxx  "
  "   xxx   "
  "         "
  // SQUARE_FILLED_7_7
  "         "
  " xxxxxxx "
  " xxxxxxx "
  " xxxxxxx "
  " xxxxxxx "
  " xxxxxxx "
  " xxxxxxx "
  " xxxxxxx "
  "         "
  // DIAMOND_FILLED_7_7
  "         "
  "    x    "
  "   xxx   "
  "  xxxxx  "
  " xxxxxxx "
  "  xxxxx  "
  "   xxx   "
  "    x    "
  "         "
  // TRIANGLE_FILLED_7_7
  "         "
  "    x    "
  "    x    "
  "   xxx   "
  "   xxx   "
  "  xxxxx  "
  "  xxxxx  "
  " xxxxxxx "
  "         "
  // RHOMBUS_FILLED_7_7
  "         "
  "         "
  "    xxxxx"
  "   xxxxx "
  "  xxxxx  "
  " xxxxx   "
  "xxxxx    "
  "         "
  "         "
  // HOURGLASS_FILLED_7_7
  "         "
  " xxxxxxx "
  "  xxxxx  "
  "   xxx   "
  "    x    "
  "   xxx   "
  "  xxxxx  "
  " xxxxxxx "
  "         "
  // SATELLITE_FILLED_7_7
  "         "
  " x     x "
  "  x x x  "
  "   xxx   "
  "  xxxxx  "
  "   xxx   "
  "  x x x  "
  " x     x "
  "         "
  // PINE_TREE_FILLED_7_7
  "         "
  "    x    "
  "   xxx   "
  "  xxxxx  "
  " xxxxxxx "
  "    x    "
  "    x    "
  "    x    "
  "         "
  // CAUTION_FILLED_7_7
  "         "
  " xxxxxxx "
  "  xxxxx  "
  "  xxxxx  "
  "   xxx   "
  "   xxx   "
  "    x    "
  "    x    "
  "         "
  // SHIP_FILLED_7_7
  "         "
  "    x    "
  "    x    "
  "    x    "
  " xxxxxxx "
  "  xxxxx  "
  "   xxx   "
  "    x    "
  "         "
  // CROSS_9_9
  "x       x"
  " x     x "
  "  x   x  "
  "   x x   "
  "    x    "
  "   x x   "
  "  x   x  "
  " x     x "
  "x       x"
  // PLUS_9_9
  "    x    "
  "    x    "
  "    x    "
  "    x    "
  "xxxxxxxxx"
  "    x    "
  "    x    "
  "    x    "
  "    x    "
  // MINUS_9_9
  "         "
  "         "
  "         "
  "         "
  "xxxxxxxxx"
  "         "
  "         "
  "         "
  "         "
  // SLASH_9_9
  "        x"
  "       x "
  "      x  "
  "     x   "
  "    x    "
  "   x     "
  "  x      "
  " x       "
  "x        "
  // BACKSLASH_9_9
  "x        "
  " x       "
  "  x      "
  "   x     "
  "    x    "
  "     x   "
  "      x  "
  "       x "
  "        x"
  // BAR_9_9,
  "    x    "
  "    x    "
  "    x    "
  "    x    "
  "    x    "
  "    x    "
  "    x    "
  "    x    "
  "    x    "
  // STAR_9_9
  "    x    "
  " x  x  x "
  "  x x x  "
  "   xxx   "
  "xxxxxxxxx"
  "   xxx   "
  "  x x x  "
  " x  x  x "
  "    x    "
  // Y_9_9
  "x       x"
  " x     x "
  "  x   x  "
  "   x x   "
  "    x    "
  "    x    "
  "    x    "
  "    x    "
  "    x    "
  // LIGHTNING_9_9
  "    x    "
  "     x   "
  "      x  "
  "       x "
  "xxxxxxxxx"
  " x       "
  "  x      "
  "   x     "
  "    x    "
  // WELL_9_9
  "    x    "
  "   xxx   "
  "   x x   "
  "   x x   "
  "  x x x  "
  "  xxxxx  "
  "  x x x  "
  " x x x x "
  "xxx   xxx"
  // CIRCLE_LINE_9_9
  "   xxx   "
  " xx   xx "
  " x     x "
  "x       x"
  "x       x"
  "x       x"
  " x     x "
  " xx   xx "
  "   xxx   "
  // SQUARE_LINE_9_9
  "xxxxxxxxx"
  "x       x"
  "x       x"
  "x       x"
  "x       x"
  "x       x"
  "x       x"
  "x       x"
  "xxxxxxxxx"
  // DIAMOND_LINE_9_9
  "    x    "
  "   x x   "
  "  x   x  "
  " x     x "
  "x       x"
  " x     x "
  "  x   x  "
  "   x x   "
  "    x    "
  // TRIANGLE_LINE_9_9
  "    x    "
  "    x    "
  "   x x   "
  "   x x   "
  "  x   x  "
  "  x   x  "
  " x     x "
  " x     x "
  "xxxxxxxxx"
  // RHOMBUS_LINE_9_9
  "         "
  "     xxxx"
  "    x   x"
  "   x   x "
  "  x   x  "
  " x   x   "
  "x   x    "
  "xxxx     "
  "         "
  // HOURGLASS_LINE_9_9
  "xxxxxxxxx"
  " x     x "
  "  x   x  "
  "   x x   "
  "    x    "
  "   x x   "
  "  x   x  "
  " x     x "
  "xxxxxxxxx"
  // SATELLITE_LINE_9_9
  "x       x"
  " x xxx x "
  "  x   x  "
  " x     x "
  " x     x "
  " x     x "
  "  x   x  "
  " x xxx x "
  "x       x"
  // PINE_TREE_LINE_9_9
  "    x    "
  "   x x   "
  "  x   x  "
  " x     x "
  "xxxxxxxxx"
  "    x    "
  "    x    "
  "    x    "
  "    x    "
  // CAUTION_LINE_9_9
  "xxxxxxxxx"
  " x     x "
  " x     x "
  "  x   x  "
  "  x   x  "
  "   x x   "
  "   x x   "
  "    x    "
  "    x    "
  // SHIP_LINE_9_9
  "    x    "
  "    x    "
  "    x    "
  "    x    "
  "xxxxxxxxx"
  " x     x "
  "  x   x  "
  "   x x   "
  "    x    "
  // CIRCLE_FILLED_9_9
  "   xxx   "
  " xxxxxxx "
  " xxxxxxx "
  "xxxxxxxxx"
  "xxxxxxxxx"
  "xxxxxxxxx"
  " xxxxxxx "
  " xxxxxxx "
  "   xxx   "
  // SQUARE_FILLED_9_9
  "xxxxxxxxx"
  "xxxxxxxxx"
  "xxxxxxxxx"
  "xxxxxxxxx"
  "xxxxxxxxx"
  "xxxxxxxxx"
  "xxxxxxxxx"
  "xxxxxxxxx"
  "xxxxxxxxx"
  // DIAMOND_FILLED_9_9
  "    x    "
  "   xxx   "
  "  xxxxx  "
  " xxxxxxx "
  "xxxxxxxxx"
  " xxxxxxx "
  "  xxxxx  "
  "   xxx   "
  "    x    "
  // TRIANGLE_FILLED_9_9
  "    x    "
  "    x    "
  "   xxx   "
  "   xxx   "
  "  xxxxx  "
  "  xxxxx  "
  " xxxxxxx "
  " xxxxxxx "
  "xxxxxxxxx"
  // RHOMBUS_FILLED_9_9
  "         "
  "     xxxx"
  "    xxxxx"
  "   xxxxx "
  "  xxxxx  "
  " xxxxx   "
  "xxxxx    "
  "xxxx     "
  "         "
  // HOURGLASS_FILLED_9_9
  "xxxxxxxxx"
  " xxxxxxx "
  "  xxxxx  "
  "   xxx   "
  "    x    "
  "   xxx   "
  "  xxxxx  "
  " xxxxxxx "
  "xxxxxxxxx"
  // SATELLITE_FILLED_9_9
  "x       x"
  " x xxx x "
  "  xxxxx  "
  " xxxxxxx "
  " xxxxxxx "
  " xxxxxxx "
  "  xxxxx  "
  " x xxx x "
  "x       x"
  // PINE_TREE_FILLED_9_9
  "    x    "
  "   xxx   "
  "  xxxxx  "
  " xxxxxxx "
  "xxxxxxxxx"
  "    x    "
  "    x    "
  "    x    "
  "    x    "
  // CAUTION_FILLED_9_9
  "xxxxxxxxx"
  " xxxxxxx "
  " xxxxxxx "
  "  xxxxx  "
  "  xxxxx  "
  "   xxx   "
  "   xxx   "
  "    x    "
  "    x    "
  // SHIP_FILLED_9_9
  "    x    "
  "    x    "
  "    x    "
  "    x    "
  "xxxxxxxxx"
  " xxxxxxx "
  "  xxxxx  "
  "   xxx   "
  "    x    "
  //"#"
};

static void
convert_bitmaps(void)
{
  int rpos = 0;
  int wpos = 0;
  for (int img = 0; img < SoMarkerSet::NUM_MARKERS; img++) {
    for (int l=8;l>=0;l--) {
      unsigned char v1 = 0;
      unsigned char v2 = 0;
      if (marker_char_bitmaps[(l*9) + rpos] == 'x') v1 += 0x80;
      if (marker_char_bitmaps[(l*9) + rpos + 1] == 'x') v1 += 0x40;
      if (marker_char_bitmaps[(l*9) + rpos + 2] == 'x') v1 += 0x20;
      if (marker_char_bitmaps[(l*9) + rpos + 3] == 'x') v1 += 0x10;
      if (marker_char_bitmaps[(l*9) + rpos + 4] == 'x') v1 += 0x08;
      if (marker_char_bitmaps[(l*9) + rpos + 5] == 'x') v1 += 0x04;
      if (marker_char_bitmaps[(l*9) + rpos + 6] == 'x') v1 += 0x02;
      if (marker_char_bitmaps[(l*9) + rpos + 7] == 'x') v1 += 0x01;
      if (marker_char_bitmaps[(l*9) + rpos + 8] == 'x') v2 += 0x80;
      markerimages[wpos] = v1;
      markerimages[wpos + 1] = v2;
      markerimages[wpos + 2] = 0;
      markerimages[wpos + 3] = 0;
      wpos += 4;
    }
    rpos += (9*9);
  }
}


// ----------------------------------------------------------------------------------------------------
