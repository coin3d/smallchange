#include <SmallChange/nodes/SmTrack.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#ifdef __COIN__
#include <Inventor/system/gl.h>
#else // SGI/TGS Inventor
#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif // HAVE_WINDOWS_H
#include <GL/gl.h>
#endif // SGI/TGS Inventor

#include <Inventor/bundles/SoMaterialBundle.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/sensors/SoFieldSensor.h>

#define PRIVATE(obj) (obj)->pimpl
#define PUBLIC(obj) (obj)->master

class SmTrackP {
public:
  SmTrackP(void) 
    : startix(0) {}

  void updateInterval(void) 
  {
    float t = PUBLIC(this)->trackLength.getValue();
    int n = PUBLIC(this)->timeStamps.getNum();
    int i = n-1;
    SbTime starttime = PUBLIC(this)->timeStamps[i];

    for (; i >= 0; i--) {
      SbTime tracktime = PUBLIC(this)->timeStamps[i];
      SbTime interval(starttime - tracktime);
      if (interval.getValue() > PUBLIC(this)->trackLength.getValue()) {
        this->startix = i;
        break;
      }
    }
  }

  static void updateIntervalCB(void * closure, SoSensor *)
  {
    SmTrackP * thisp = (SmTrackP *) closure;
    thisp->updateInterval();
  }

  int startix, endix;
  SmTrack * master;
  SoFieldSensor * tracklengthsensor;
};

SO_NODE_SOURCE(SmTrack);

void
SmTrack::initClass(void)
{
  SO_NODE_INIT_CLASS(SmTrack, SoShape, "Shape");
}

SmTrack::SmTrack(void)
{
  SO_NODE_CONSTRUCTOR(SmTrack);
  SO_NODE_ADD_FIELD(track, (SbVec3d()));
  SO_NODE_ADD_FIELD(timeStamps, (SbTime::getTimeOfDay()));
  SO_NODE_ADD_FIELD(trackLength, (22.0f));

  PRIVATE(this) = new SmTrackP;
  PRIVATE(this)->master = this;

  PRIVATE(this)->tracklengthsensor = 
    new SoFieldSensor(SmTrackP::updateIntervalCB, PRIVATE(this));
  PRIVATE(this)->tracklengthsensor->attach(&this->trackLength);
}

SmTrack::~SmTrack()
{
  delete PRIVATE(this);
}

void 
SmTrack::GLRender(SoGLRenderAction * action)
{
  if (!this->shouldGLRender(action)) return;

  SoMaterialBundle mb(action);
  mb.sendFirst();

  glBegin(GL_POINTS);
  for (int i = PRIVATE(this)->startix; i < this->track.getNum(); i++) {
    glVertex3dv(this->track[i].getValue());
  }
  glEnd();
}

void 
SmTrack::append(const SbVec3d & pos, 
                const SbTime & timestamp)
{
  int index = this->timeStamps.getNum();
  assert(index == this->track.getNum());
  this->timeStamps.set1Value(index, timestamp);
  this->track.set1Value(index, pos);

  PRIVATE(this)->updateInterval();
}


void 
SmTrack::computeBBox(SoAction * action, SbBox3f & box, 
                     SbVec3f & center)
{
  box.makeEmpty();

  for (int i = PRIVATE(this)->startix; i < this->track.getNum(); i++) {
    SbVec3d coord = this->track[i].getValue();
    box.extendBy(SbVec3f(coord[0], coord[1], coord[2]));
  }
  if (!box.isEmpty()) center = box.getCenter();
}

void 
SmTrack::generatePrimitives(SoAction * action)
{
  // FIXME: implement! (20060602 frodo)
}

#undef PRIVATE
#undef PUBLIC
