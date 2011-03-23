#include <SmallChange/nodekits/SmTrackPointKit.h>

#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodekits/SoAppearanceKit.h>
#include <Inventor/fields/SoMFVec3d.h>
#include <Inventor/fields/SoMFTime.h>
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
  SO_KIT_ADD_FIELD(pointInterval, (1));
  SO_KIT_ADD_FIELD(lineInterval, (0));
  SO_KIT_ADD_FIELD(tickInterval, (0));
  SO_KIT_ADD_FIELD(tickSize, (1.0f));
  SO_KIT_INIT_INSTANCE();
  
  getTrack()->trackLength.connectFrom(&this->trackLength);
  getTrack()->pointInterval.connectFrom(&this->pointInterval);
  getTrack()->lineInterval.connectFrom(&this->lineInterval);
  getTrack()->tickInterval.connectFrom(&this->tickInterval);
  getTrack()->tickSize.connectFrom(&this->tickSize);
  this->set("appearanceKit.drawStyle { pointSize 3 }");
}

SmTrackPointKit::~SmTrackPointKit()
{}

SmTrack * SmTrackPointKit::getTrack()
{
  return (SmTrack *)this->getAnyPart("track", TRUE);
}

void
SmTrackPointKit::addTrackPoint(const SbVec3d & pos, 
                               const SbTime & timestamp)
{
  getTrack()->append(pos, timestamp);
}

void 
SmTrackPointKit::addTrackPoints( const SoMFVec3d & positions, const SoMFTime & timestamps )
{
  assert( positions.getNum() == timestamps.getNum() && "Size of positions and timestamps fields must be equal");

  SmTrack * track = getTrack();
  track->track.startEditing(); 
  track->timeStamps.startEditing(); 

  for(int i = 0; i<positions.getNum(); i++)
    addTrackPoint( positions[i], timestamps[i] ); 

  track->timeStamps.finishEditing(); 
  track->track.finishEditing();
}

void
SmTrackPointKit::setTrackPoints( const SoMFVec3d & positions, const SoMFTime & timestamps )
{
  getTrack()->deleteValues();
  addTrackPoints(positions, timestamps);
}
