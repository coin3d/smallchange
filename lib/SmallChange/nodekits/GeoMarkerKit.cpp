#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#include <assert.h>

#include <Inventor/errors/SoDebugError.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoSwitch.h>
#include <Inventor/sensors/SoFieldSensor.h>

#include <SmallChange/nodekits/SmGeoMarkerKit.h>
#include <SmallChange/nodes/ShapeScale.h>

class GeoMarkerKitP {
public:
  SmGeoMarkerKit * api;
  SoSeparator * shape;
  SoSwitch * material;
  SoFieldSensor * selectionsensor;
  static const char * scene[];
  static void selectionsensor_cb(void * closure, SoSensor * sensor);

  GeoMarkerKitP(void) : api(NULL), shape(NULL), material(NULL), selectionsensor(NULL) { }
};

#define PRIVATE(obj) ((obj)->pimpl)

SO_KIT_SOURCE(SmGeoMarkerKit);

void
SmGeoMarkerKit::initClass(void)
{
  static SbBool initialized = FALSE;
  if ( initialized ) return;
  initialized = TRUE;
  SO_KIT_INIT_CLASS(SmGeoMarkerKit, SoBaseKit, "BaseKit");
}

SmGeoMarkerKit::SmGeoMarkerKit(void)
{
  PRIVATE(this) = new GeoMarkerKitP;
  PRIVATE(this)->api = this;

  SO_KIT_CONSTRUCTOR(SmGeoMarkerKit);
  SO_KIT_ADD_FIELD(selected, (FALSE));
  SO_KIT_ADD_CATALOG_ENTRY(scene, ShapeScale, FALSE, this, "", FALSE);
  SO_KIT_INIT_INSTANCE();
  
  SoInput in;
  in.setStringArray(GeoMarkerKitP::scene);

  SoNode * root = NULL;
  if ( !SoDB::read(&in, root) ) {
    SoDebugError::post("SmGeoMarkerKit::SmGeoMarkerKit", "error constructing marker");
  } else if ( !root ) {
    SoDebugError::post("SmGeoMarkerKit::SmGeoMarkerKit", "error reading marker description");
  } else if ( !root->isOfType(SoSeparator::getClassTypeId()) ) {
    SoDebugError::post("SmGeoMarkerKit::SmGeoMarkerKit", "invalid marker description");
  } else {
    PRIVATE(this)->shape = (SoSeparator *) root;
    PRIVATE(this)->shape->ref();
    SoNode * materialswitch = PRIVATE(this)->shape->getChild(0);
    if ( !materialswitch || !materialswitch->isOfType(SoSwitch::getClassTypeId()) ) {
      SoDebugError::post("SmGeoMarkerKit::SmGeoMarkerKit", "invalid marker description");
    } else {
      PRIVATE(this)->material = (SoSwitch *) materialswitch;
      PRIVATE(this)->material->ref();
      ShapeScale * shapescale = new ShapeScale;
      if ( !shapescale->setPart("shape", root) ) {
        SoDebugError::post("SmGeoMarkerKit::SmGeoMarkerKit", "ShapeScale shape part problem");
      } else {
        this->setAnyPart("scene", shapescale);
      }
      PRIVATE(this)->selectionsensor = new SoFieldSensor(GeoMarkerKitP::selectionsensor_cb, PRIVATE(this));
      PRIVATE(this)->selectionsensor->setPriority(0);
      PRIVATE(this)->selectionsensor->attach(&this->selected);
    }
  }
}

SmGeoMarkerKit::~SmGeoMarkerKit(void)
{
  if ( PRIVATE(this)->shape != NULL ) {
    PRIVATE(this)->shape->unref();
  }
  if ( PRIVATE(this)->material != NULL ) {
    PRIVATE(this)->material->ref();
  }
  if ( PRIVATE(this)->selectionsensor ) {
    delete PRIVATE(this)->selectionsensor;
  }
  delete PRIVATE(this);
}

// *************************************************************************

#define PUBLIC(obj) ((obj)->api)

const char *
GeoMarkerKitP::scene[] = {
  "#Inventor V2.1 ascii\n",
  "\n",
  "Separator {\n",
  "  DEF materials Switch {\n",
  "    whichChild 0\n",
  "    Material {\n",
  "      diffuseColor 0.8 0.8 0.8\n",
  "    }\n",
  "    Material {\n",
  "      diffuseColor 1.0 1.0 0.0\n",
  "    }\n",
  "  }\n",
  "  SoTexture2 {\n",
  "  }\n",
  "  RotationXYZ {\n",
  "    axis X\n",
  "    angle 1.67\n",
  "  }",
  "  Scale {\n",
  "    scaleFactor 0.2 5 0.2\n",
  "  }\n",
  "  Translation {\n",
  "    translation 0 1 0\n",
  "  }\n",

  // cull away backfacing polygons, to work better with low Z-buffer
  // resolution
  "  ShapeHints {\n",
  "    vertexOrdering COUNTERCLOCKWISE\n",
  "    shapeType SOLID\n",
  "    faceType CONVEX\n",
  "  }\n",

  "  Cylinder { }\n",
  "  Scale { scaleFactor 2.5 0.1 2.5 }\n",
  "  Translation { translation 0 10 0 }\n",
  "  Sphere { radius 1 }\n",
  "}\n",
  NULL
};

void
GeoMarkerKitP::selectionsensor_cb(void * closure, SoSensor * sensor)
{
  assert(closure);
  GeoMarkerKitP * thisp = (GeoMarkerKitP *) closure;
  assert(thisp->material != NULL);
  assert(PUBLIC(thisp) != NULL);
  // notification on SmGeoMarkerKit::enabled will refresh anyways, and this
  // callback triggers with priority 0
  thisp->material->whichChild.enableNotify(FALSE);
  if ( PUBLIC(thisp)->selected.getValue() ) {
    thisp->material->whichChild.setValue(1);
  } else {
    thisp->material->whichChild.setValue(0);
  }
  thisp->material->whichChild.enableNotify(TRUE);
}

// *************************************************************************
