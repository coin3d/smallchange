
#include <SmallChange/nodekits/SmTrackPointKit.h>

#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoDrawStyle.h>
#include <Inventor/nodekits/SoAppearanceKit.h>
#include <Inventor/nodes/SoPointSet.h>
#include <SmallChange/nodes/UTMPosition.h>
#include <SmallChange/nodes/SmTrack.h>

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
  SO_KIT_ADD_CATALOG_ENTRY(utmPosition, UTMPosition, FALSE, topSeparator, appearanceKit, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(appearanceKit, SoAppearanceKit, FALSE, topSeparator, track, FALSE); 
  SO_KIT_ADD_CATALOG_ENTRY(track, SmTrack, FALSE, topSeparator, "", FALSE);
  
  SO_KIT_ADD_FIELD(trackLength, (22.0f));
  SO_KIT_INIT_INSTANCE();
  
  SmTrack * track = (SmTrack *)this->getAnyPart("track", TRUE);
  track->trackLength.connectFrom(&this->trackLength);

  this->set("appearanceKit.drawStyle { pointSize 3 }");
}

SmTrackPointKit::~SmTrackPointKit()
{
  
}

void
SmTrackPointKit::addTrackPoint(const SbVec3d & pos, 
                               const SbTime & timestamp)
{
  SmTrack * track = (SmTrack *)this->getAnyPart("track", TRUE);
  track->append(pos, timestamp);
}

