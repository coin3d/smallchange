
#include <SmallChange/nodekits/SmTrackPointKit.h>

#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoDrawStyle.h>
#include <Inventor/nodes/SoPointSet.h>
#include <SmallChange/nodes/UTMPosition.h>

SO_KIT_SOURCE(SmTrackPointKit);

void
SmTrackPointKit::initClass(void)
{
  SO_KIT_INIT_CLASS(SmTrackPointKit, SoBaseKit, "BaseKit");
}

SmTrackPointKit::SmTrackPointKit(void)
{
  SO_KIT_CONSTRUCTOR(SmTrackPointKit);

  SO_KIT_ADD_CATALOG_ENTRY(topSeparator, SoSeparator, FALSE, this, "", FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(utmPosition, UTMPosition, FALSE, topSeparator, coord3, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(coord3, SoCoordinate3, FALSE, topSeparator, drawStyle, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(drawStyle, SoDrawStyle, FALSE, topSeparator, pointSet, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(pointSet, SoPointSet, FALSE, topSeparator, "", FALSE);

  SO_KIT_ADD_FIELD(trackLength, (22.0f));
  SO_KIT_ADD_FIELD(timeStamps, (SbTime::getTimeOfDay()));
  
  SO_KIT_INIT_INSTANCE();

  this->set("drawStyle { pointSize 3 }");
}

SmTrackPointKit::~SmTrackPointKit()
{
  
}

void
SmTrackPointKit::updateNumPoints(void)
{
  int count = this->timeStamps.getNum();
  if (count < 2) { return; }

  SbTime starttime = this->timeStamps[count-1];
  int numpoints = 0;

  printf("tracklength: %f\n", this->trackLength.getValue());

  for (int i = count-1; i >= 0; i--, numpoints++) {
    SbTime time = this->timeStamps[i];
    SbTime interval(time - starttime);
    if (interval.getValue() > this->trackLength.getValue()) {
      break;
    }
  }
  SoPointSet * pointset = (SoPointSet *) this->getAnyPart("pointSet", TRUE);
  pointset->numPoints.setValue(numpoints);
}

void
SmTrackPointKit::addTrackPoint(const SbVec3f & pos, 
                               const SbTime & timestamp)
{
  SoCoordinate3 * coord3 = (SoCoordinate3 *)this->getAnyPart("coord3", TRUE);
  this->timeStamps.insertSpace(0, 1);
  this->timeStamps.set1Value(0, timestamp);

  coord3->point.insertSpace(0, 1);
  coord3->point.set1Value(0, pos);
  
  this->updateNumPoints();
}

