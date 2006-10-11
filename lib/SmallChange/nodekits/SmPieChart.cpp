
#include <Inventor/SbBasic.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoTexture2.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoTranslation.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoIndexedFaceSet.h>
#include <Inventor/nodes/SoIndexedLineSet.h>
#include <Inventor/nodes/SoPointSet.h>
#include <Inventor/nodes/SoPolygonOffset.h>
#include <Inventor/sensors/SoFieldSensor.h>
#include <Inventor/sensors/SoOneShotSensor.h>

#include <SmallChange/nodekits/SmPieChart.h>

/*
 * Ideas:
 * - add polygonoffset lines
 * - add callback to indicate which slice mouse is howering over
 * - add names and auto-tooltips to each slice
 * - reorganize geometry generation to allow creasangle to smooth edge shading
 *
*/

// *************************************************************************

class SmPieChartP {
public:
  SmPieChartP(SmPieChart * pub);
  ~SmPieChartP(void);

  SoOneShotSensor * trigger;
  SoFieldSensor * heightSensor;
  SoFieldSensor * radiusSensor;
  SoFieldSensor * valueTypeSensor;
  SoFieldSensor * valuesSensor;
  SoFieldSensor * colorsSensor;

  static void trigger_update_cb(void * closure, SoSensor * sensor);
  static void update_cb(void * closure, SoSensor * sensor);

  SmPieChart * api;
};

// *************************************************************************

#define PRIVATE(obj) ((obj)->pimpl)

SO_KIT_SOURCE(SmPieChart);

void
SmPieChart::initClass(void)
{
  SO_KIT_INIT_CLASS(SmPieChart, SoBaseKit, "SoBaseKit");
}

SmPieChart::SmPieChart(void)
{
  SO_KIT_CONSTRUCTOR(SmPieChart);

  SO_KIT_DEFINE_ENUM_VALUE(ValueType, ITEM_SIZE);
  SO_KIT_DEFINE_ENUM_VALUE(ValueType, ITEM_BORDER);

  SO_KIT_ADD_FIELD(height, (1.0f));
  SO_KIT_ADD_FIELD(radius, (1.0f));

  SO_KIT_ADD_FIELD(valueType, (ITEM_SIZE));
  SO_KIT_ADD_FIELD(values, (0.0f));
  SO_KIT_ADD_FIELD(colors, (SbColor(0.8f, 0.8f, 0.8f)));
  SO_KIT_ADD_FIELD(retraction, (0.0f));

  SO_KIT_SET_SF_ENUM_TYPE(valueType, ValueType);

  this->values.setNum(0);
  this->values.setDefault(TRUE);
  this->colors.setNum(0);
  this->colors.setDefault(TRUE);
  this->retraction.setNum(0);
  this->retraction.setDefault(TRUE);

  // set up catalog
  SO_KIT_ADD_CATALOG_ENTRY(topSeparator, SoSeparator, FALSE, this, "", FALSE);

  SO_KIT_INIT_INSTANCE();

  PRIVATE(this) = new SmPieChartP(this);

  PRIVATE(this)->radiusSensor =
    new SoFieldSensor(SmPieChartP::trigger_update_cb, PRIVATE(this));
  PRIVATE(this)->radiusSensor->attach(&this->radius);
  PRIVATE(this)->heightSensor =
    new SoFieldSensor(SmPieChartP::trigger_update_cb, PRIVATE(this));
  PRIVATE(this)->heightSensor->attach(&this->height);
  PRIVATE(this)->valueTypeSensor =
    new SoFieldSensor(SmPieChartP::trigger_update_cb, PRIVATE(this));
  PRIVATE(this)->valueTypeSensor->attach(&this->valueType);
  PRIVATE(this)->valuesSensor =
    new SoFieldSensor(SmPieChartP::trigger_update_cb, PRIVATE(this));
  PRIVATE(this)->valuesSensor->attach(&this->values);
  PRIVATE(this)->colorsSensor =
    new SoFieldSensor(SmPieChartP::trigger_update_cb, PRIVATE(this));
  PRIVATE(this)->colorsSensor->attach(&this->colors);
  
  PRIVATE(this)->trigger =
    new SoOneShotSensor(SmPieChartP::update_cb, PRIVATE(this));

  // this->generateGeometry();
}

SmPieChart::~SmPieChart(void)
{
  delete PRIVATE(this);
}

/*!
  Constructs the geometry for the pie chart.
*/

void
SmPieChart::generateGeometry(void)
{
  int i;

  const float radius = this->radius.getValue();
  const float height = this->height.getValue();

  SoSeparator * root = new SoSeparator;
  root->addChild(new SoTexture2); // disable texturing for this node
  SoMaterial * material = new SoMaterial;
  root->addChild(material);
  SoCoordinate3 * coords = new SoCoordinate3;
  root->addChild(coords);  // all the points
  this->setAnyPart("topSeparator", root);

  if ( this->colors.getNum() != this->values.getNum() ) {
    SoDebugError::postInfo("SmPieChart::generateGeometry",
                           "number of colors and values needs to be the same");
    return;
  }

  if ( this->values.getNum() == 0 ) {
    SoDebugError::postInfo("SmPieChart::generateGeometry",
                           "no values");
    return;
  }

  if ( this->valueType.getValue() == ITEM_BORDER ) {
    float last = 0.0f;
    for ( i = 0; i < this->values.getNum(); i++ ) {
      if ( this->values[i] < last ) {
        SoDebugError::postInfo("SmPieChart::generateGeometry",
                               "in border mode, always increment values");
        return;
      }
      last = this->values[i];
    }
  }

  // calculate all the points
  int numdefaultpoints = 37;
  int numborderpoints = this->values.getNum() - 1;
  
  float * points = new float [numdefaultpoints + numborderpoints];
  int * indexes = new int [numdefaultpoints + numborderpoints];
  for ( i = 0; i < numdefaultpoints; i++ ) {
    points[i] = float(i) / float(numdefaultpoints - 1);
    indexes[i] = -1;
  }

  float sumvalues = 0;
  if ( this->valueType.getValue() == ITEM_SIZE ) {
    for ( i = 0; i < this->values.getNum(); i++ ) {
      sumvalues += this->values[i];
    }
  } else {
    sumvalues = this->values[this->values.getNum() - 1];
  }

  float accumvalues = 0;
  for ( i = 0; i < numborderpoints; i++ ) {
    if ( this->valueType.getValue() == ITEM_SIZE ) {
      accumvalues += this->values[i];
      points[numdefaultpoints + i] = accumvalues / sumvalues;
    } else {
      points[numdefaultpoints + i] = this->values[i] / sumvalues;
    }
    indexes[numdefaultpoints + i] = i + 1;
  }

  int numpoints = numdefaultpoints + numborderpoints;
  // FIXME!
  int bubble = 1;
  while ( bubble ) {
    bubble = 0;
    for ( i = 0; i < (numpoints - 1); i++ ) {
      if ( points[i] > points[i+1] ) {
        bubble = 1;
        float storepoint = points[i];
        points[i] = points[i+1];
        points[i+1] = storepoint;
        int storeindex = indexes[i];
        indexes[i] = indexes[i+1];
        indexes[i+1] = storeindex;
      }
    }
  }

  int idx = 0;
  for ( i = 0; i < numpoints; i++ ) {
    if ( indexes[i] == -1 ) {
      indexes[i] = idx;
    } else {
      idx = indexes[i];
    }
  }

  // collapse equal point values
  for ( i = 0; i < (numpoints - 1); i++ ) {
    if ( points[i] == points[i+1] ) { // epsilon?
      int j;
      for ( j = i; j < (numpoints - 1); j++ ) {
        points[j] = points[j+1];
        indexes[j] = indexes[j+1];
      }
      numpoints--;
    }
  }

  SbVec3f * vecs = new SbVec3f [2 + numpoints * 2];
  vecs[0].setValue(0.0, 0.0, height); // center top
  vecs[1].setValue(0.0, 0.0, 0.0); // center bottom
  
  for ( i = 0; i < numpoints; i++ ) {
    float x = cosf(float(points[i] * 2.0f * M_PI)) * radius;
    float y = cosf(float((points[i] + 0.25f) * 2.0f * M_PI)) * radius;
    vecs[2 + i * 2].setValue(x, y, height);
    vecs[2 + i * 2 + 1].setValue(x, y, 0);
  }

  coords->point.setValues(0, 2 + numpoints * 2, vecs);

  // generate the pie slices

  int sliceindex = 0;
  SoSeparator * current = NULL;
  SoIndexedFaceSet * faceset = NULL;
  SoTranslation * translation = NULL;

  for ( i = 0; i < numpoints; i++ ) {

    // do we need to open a slice?
    if ( (i == 0) || (indexes[i] != indexes[i-1]) ) {
      current = new SoSeparator;
      SoMaterial * mat = new SoMaterial;
      mat->diffuseColor.setValue(this->colors[sliceindex]);
      current->addChild(mat);
      if ( this->retraction.getNum() > indexes[i] &&
           this->retraction[sliceindex] > 0.0f ) {
        translation = new SoTranslation;
        translation->translation.setValue(coords->point[2+i*2+1]);
        current->addChild(translation);
      }
      faceset = new SoIndexedFaceSet;
      current->addChild(faceset);

      int num = faceset->coordIndex.getNum();
      // opening side face
      faceset->coordIndex.set1Value(num++, 0);
      faceset->coordIndex.set1Value(num++, 2+i*2);
      faceset->coordIndex.set1Value(num++, 2+i*2+1);
      faceset->coordIndex.set1Value(num++, 1);
      faceset->coordIndex.set1Value(num++, -1);
    }

    // fill in slice
    if ( i < (numpoints - 1) ) {
      assert(current != NULL && faceset != NULL);
      int num = faceset->coordIndex.getNum();
      // top face
      faceset->coordIndex.set1Value(num++, 0);
      faceset->coordIndex.set1Value(num++, 2+i*2+2);
      faceset->coordIndex.set1Value(num++, 2+i*2);
      faceset->coordIndex.set1Value(num++, -1);
      // bottom face
      faceset->coordIndex.set1Value(num++, 1);
      faceset->coordIndex.set1Value(num++, 2+i*2+1);
      faceset->coordIndex.set1Value(num++, 2+i*2+3);
      faceset->coordIndex.set1Value(num++, -1);
      // side face
      faceset->coordIndex.set1Value(num++, 2+i*2);
      faceset->coordIndex.set1Value(num++, 2+i*2+2);
      faceset->coordIndex.set1Value(num++, 2+i*2+3);
      faceset->coordIndex.set1Value(num++, 2+i*2+1);
      faceset->coordIndex.set1Value(num++, -1);
    }

    // do we need to close current slice?
    if ( i == (numpoints - 1) || (indexes[i] != indexes[i+1]) ) {
      int num = faceset->coordIndex.getNum();
      // closing side face
      if ( i == (numpoints - 1) ) {
        faceset->coordIndex.set1Value(num++, 0);
        faceset->coordIndex.set1Value(num++, 1);
        faceset->coordIndex.set1Value(num++, 3);
        faceset->coordIndex.set1Value(num++, 2);
        faceset->coordIndex.set1Value(num++, -1);
      } else {
        faceset->coordIndex.set1Value(num++, 0);
        faceset->coordIndex.set1Value(num++, 1);
        faceset->coordIndex.set1Value(num++, 2+i*2+3);
        faceset->coordIndex.set1Value(num++, 2+i*2+2);
        faceset->coordIndex.set1Value(num++, -1);
      }

      if ( this->retraction.getNum() > indexes[i] &&
           this->retraction[sliceindex] > 0.0f ) {
        assert(translation != NULL);
        SbVec3f vec = translation->translation.getValue();
        if ( i == (numpoints - 1) ) {
          vec = vec + coords->point[3];
        } else {
          vec = vec + coords->point[2+i*2+3];
        }
        vec.normalize();
        vec *= this->retraction[sliceindex];
        translation->translation.setValue(vec);
      }
      root->addChild(current);
      current = NULL;
      faceset = NULL;
      translation = NULL;
      sliceindex++;
    }
  }

  delete [] vecs;
  delete [] points;
  delete [] indexes;
}

SbBool
SmPieChart::readInstance(SoInput * input, unsigned short flags)
{
  SbBool retval = inherited::readInstance(input, flags);
  this->generateGeometry();
  return retval;
}

// *************************************************************************

SmPieChartP::SmPieChartP(SmPieChart * pub)
{
  this->api = pub;
}

SmPieChartP::~SmPieChartP(void)
{
  delete this->heightSensor;
  delete this->radiusSensor;
  delete this->valueTypeSensor;
  delete this->valuesSensor;
  delete this->colorsSensor;
  delete this->trigger;
}

void
SmPieChartP::trigger_update_cb(void * closure, SoSensor * sensor)
{
  SmPieChartP * obj = (SmPieChartP *) closure;
  obj->trigger->trigger();
}

void
SmPieChartP::update_cb(void * closure, SoSensor * sensor)
{
  SmPieChartP * obj = (SmPieChartP *) closure;
  obj->api->generateGeometry();
}

// *************************************************************************

#undef PRIVATE
