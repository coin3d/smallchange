#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#include <assert.h>

#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoIndexedLineSet.h>
#include <Inventor/sensors/SoFieldSensor.h>
#include <Inventor/actions/SoCallbackAction.h>
#include <Inventor/SoPrimitiveVertex.h>

#include <SmallChange/nodekits/SmNormalsKit.h>

/*
** This nodekit is intended to be used like the examples below.
** It might be a good idea to integrate this nodekit in the SoGui
** viewer components (menu-controllable) sometime in the future.
* 

----8<----- [snip] ----------------8<----- [snip] ------------

#Inventor V2.1 ascii

Separator {
  DEF TheScene Separator {
    Cube {}
    Translation { translation 3 0 0 }
    Sphere {}
    Translation { translation 3 0 0 }
    Cylinder {}
    Translation { translation 3 0 0 }
    Cone {}
  }
  SmNormalsKit {
    scene USE TheScene
    length 0.2
  }
}

----8<----- [snip] ----------------8<----- [snip] ------------

#Inventor V2.1 ascii

Separator {
  DEF TheScene Separator {
    Text3 {
      string "Hei"
      parts ALL
    }
  }
  SmNormalsKit {
    scene USE TheScene
    length 0.2
  }
}

----8<----- [snip] ----------------8<----- [snip] ------------

 *
** Ideas for future improvements:
** + SoSFColor color field for setting the material (or give access to the
**   material node in the nodekit directly instead?
** + SoSFEnum which { VERTEX_NORMALS, TRIANGLE_NORMALS, FACE_NORMALS }
**   field to control what you get normals for - not just normals for
**   vertices.
** + optimize how coordinates are accumulated in the scene traversal (lots
**   of array expansions - probably slow for large scenes).
**
** Problems:
** + SoComplexity node settings might change tessellation dynamically without
**   triggering any form of notification - normals will stay static, while
**   the geometry will change.
*/

class NormalsKitP {
public:
  SmNormalsKit * api;
  SoMaterial * material;
  SoCoordinate3 * coords;
  SoIndexedLineSet * lines;

  SoCallbackAction * cbaction;

  SoFieldSensor * scenesensor;
  static void scenesensor_cb(void * closure, SoSensor * sensor);

  static void pointCB(void * userdata, SoCallbackAction * action, const SoPrimitiveVertex * v); 
  static void triangleCB(void * userdata, SoCallbackAction * action, const SoPrimitiveVertex * v1, const SoPrimitiveVertex * v2, const SoPrimitiveVertex * v3);

  NormalsKitP(void)
    : api(NULL), material(NULL), coords(NULL), lines(NULL), cbaction(NULL),
      scenesensor(NULL) { }
};

// *************************************************************************
//
#define PRIVATE(obj) ((obj)->pimpl)

SO_KIT_SOURCE(SmNormalsKit);

void
SmNormalsKit::initClass(void)
{
  static SbBool initialized = FALSE;
  if ( initialized ) return;
  initialized = TRUE;
  SO_KIT_INIT_CLASS(SmNormalsKit, SoBaseKit, "BaseKit");
}

SmNormalsKit::SmNormalsKit(void)
{
  PRIVATE(this) = new NormalsKitP;
  PRIVATE(this)->api = this;

  SO_KIT_CONSTRUCTOR(SmNormalsKit);
  SO_KIT_ADD_FIELD(scene, (NULL));
  SO_KIT_ADD_FIELD(length, (1.0f));
  SO_KIT_ADD_CATALOG_ENTRY(topSeparator, SoSeparator, FALSE, this, "", FALSE);
  SO_KIT_INIT_INSTANCE();

  SoSeparator * root = new SoSeparator;
  root->addChild((PRIVATE(this)->material = new SoMaterial));
  root->addChild((PRIVATE(this)->coords = new SoCoordinate3));
  root->addChild((PRIVATE(this)->lines = new SoIndexedLineSet));

  this->setAnyPart("topSeparator", root);

  PRIVATE(this)->material->ref();
  PRIVATE(this)->material->diffuseColor.setValue(1.0f, 0.0f, 0.0f);
  PRIVATE(this)->coords->ref();
  PRIVATE(this)->lines->ref();

  PRIVATE(this)->scenesensor = new SoFieldSensor(NormalsKitP::scenesensor_cb, PRIVATE(this));
  PRIVATE(this)->scenesensor->attach(&this->scene);
}

SmNormalsKit::~SmNormalsKit(void)
{
  PRIVATE(this)->material->unref();
  PRIVATE(this)->coords->unref();
  PRIVATE(this)->lines->unref();
  delete PRIVATE(this)->scenesensor;
  if ( PRIVATE(this)->cbaction != NULL ) {
    delete PRIVATE(this)->cbaction;
  }
  delete PRIVATE(this);
}

// *************************************************************************

void
NormalsKitP::scenesensor_cb(void * closure, SoSensor * sensor)
{
  assert(closure);
  NormalsKitP * thisp = (NormalsKitP *) closure;

  SoNode * scene = thisp->api->scene.getValue();
  if ( scene == NULL ) {
    thisp->lines->coordIndex.setNum(0);
    return;
  }

  if ( thisp->cbaction == NULL ) {
    thisp->cbaction = new SoCallbackAction;
    thisp->cbaction->addPointCallback(SoNode::getClassTypeId(), NormalsKitP::pointCB, thisp);
    thisp->cbaction->addTriangleCallback(SoNode::getClassTypeId(), NormalsKitP::triangleCB, thisp);
  }

  thisp->coords->point.enableNotify(FALSE);
  thisp->lines->coordIndex.enableNotify(FALSE);
  thisp->coords->point.setNum(0);
  thisp->lines->coordIndex.setNum(0);
  thisp->cbaction->apply(scene);
  thisp->coords->point.enableNotify(TRUE);
  thisp->lines->coordIndex.enableNotify(TRUE);
  thisp->lines->touch(); // redraw scene
}

void
NormalsKitP::pointCB(void * closure, SoCallbackAction * action, const SoPrimitiveVertex * v)
{
  assert(closure);
  NormalsKitP * thisp = (NormalsKitP *) closure;
  const float len = thisp->api->length.getValue();
  // find points
  SbVec3f p1 = v->getPoint();
  SbVec3f p2 = v->getPoint() + v->getNormal() * len;
  // adjust coordinates to local objectspace
  const SbMatrix & modelmatrix = action->getModelMatrix();
  modelmatrix.multVecMatrix(p1, p1);
  modelmatrix.multVecMatrix(p2, p2);
  // add coordinates
  int idx = thisp->coords->point.getNum();
  thisp->coords->point.set1Value(idx+0, p1);
  thisp->coords->point.set1Value(idx+1, p2);
  // set up line
  int num = thisp->lines->coordIndex.getNum();
  thisp->lines->coordIndex.set1Value(num+0, idx+0);
  thisp->lines->coordIndex.set1Value(num+1, idx+1);
  thisp->lines->coordIndex.set1Value(num+2, -1);
}

void
NormalsKitP::triangleCB(void * closure, SoCallbackAction * action, const SoPrimitiveVertex * v1, const SoPrimitiveVertex * v2, const SoPrimitiveVertex * v3)
{
  assert(closure);
  NormalsKitP * thisp = (NormalsKitP *) closure;
  // find points
  const float len = thisp->api->length.getValue();
  SbVec3f p1 = v1->getPoint();
  SbVec3f p2 = v1->getPoint() + v1->getNormal() * len;
  SbVec3f p3 = v2->getPoint();
  SbVec3f p4 = v2->getPoint() + v2->getNormal() * len;
  SbVec3f p5 = v3->getPoint();
  SbVec3f p6 = v3->getPoint() + v3->getNormal() * len;
  // adjust coordinates to local objectspace
  const SbMatrix & modelmatrix = action->getModelMatrix();
  modelmatrix.multVecMatrix(p1, p1);
  modelmatrix.multVecMatrix(p2, p2);
  modelmatrix.multVecMatrix(p3, p3);
  modelmatrix.multVecMatrix(p4, p4);
  modelmatrix.multVecMatrix(p5, p5);
  modelmatrix.multVecMatrix(p6, p6);
  // add coordinates
  const int idx = thisp->coords->point.getNum();
  thisp->coords->point.setNum(idx + 6); // expand array
  thisp->coords->point.set1Value(idx+0, p1);
  thisp->coords->point.set1Value(idx+1, p2);
  thisp->coords->point.set1Value(idx+2, p3);
  thisp->coords->point.set1Value(idx+3, p4);
  thisp->coords->point.set1Value(idx+4, p5);
  thisp->coords->point.set1Value(idx+5, p6);
  // set up lines
  const int num = thisp->lines->coordIndex.getNum();
  thisp->lines->coordIndex.setNum(num + 9); // expand array
  thisp->lines->coordIndex.set1Value(num+0, idx+0);
  thisp->lines->coordIndex.set1Value(num+1, idx+1);
  thisp->lines->coordIndex.set1Value(num+2, -1);
  thisp->lines->coordIndex.set1Value(num+3, idx+2);
  thisp->lines->coordIndex.set1Value(num+4, idx+3);
  thisp->lines->coordIndex.set1Value(num+5, -1);
  thisp->lines->coordIndex.set1Value(num+6, idx+4);
  thisp->lines->coordIndex.set1Value(num+7, idx+5);
  thisp->lines->coordIndex.set1Value(num+8, -1);
}

// *************************************************************************
