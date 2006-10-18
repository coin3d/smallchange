#include "cameracontrol.h"

#include <Inventor/SbLinear.h>
#include <Inventor/SbTime.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoOrthographicCamera.h>
#include <SmallChange/nodes/UTMCamera.h>
#include <Inventor/sensors/SoTimerSensor.h>

class SeekData {
public:
  SeekData() 
    : seeking(FALSE), distance(50.0f), period(2.0f)
  {
    this->sensor = new SoTimerSensor(SeekData::seeksensorCB, this);
  }
  
  ~SeekData() {
    delete this->sensor;
  }

  static void seeksensorCB(void * closure, SoSensor * sensor)
  {
    SeekData * thisp = (SeekData *) closure;
    SbTime currenttime = SbTime::getTimeOfDay();
    SoTimerSensor * timersensor = (SoTimerSensor *) sensor;
    
    SbTime dt = currenttime - timersensor->getBaseTime();
    double t = double(dt.getValue()) / thisp->period;
    
    if (t > 1.0f) t = 1.0f;
    if (t + timersensor->getInterval().getValue() > 1.0f) t = 1.0f;
    
    SbBool end = (t == 1.0f);
    
    t = (1.0 - cos(M_PI*t)) * 0.5;
    
    thisp->camera->orientation = SbRotation::slerp(thisp->startorient,
                                                   thisp->endorient,
                                                   (float) t);

    SbVec3d newpos = thisp->startpoint + (thisp->endpoint - thisp->startpoint) * t;

    thisp->camera->isOfType(UTMCamera::getClassTypeId()) ?
      ((UTMCamera *)thisp->camera)->utmposition = newpos :
      thisp->camera->position.setValue(SbVec3f(newpos[0], newpos[1], newpos[2]));
    
    if (end) {
      thisp->seeking = FALSE;
      thisp->sensor->unschedule();
    }
  }

  SoCamera * camera;
  SbVec3d startpoint;
  SbRotation startorient;
  SbVec3d endpoint;
  SbRotation endorient;
  
  float distance;
  float period;
  SoTimerSensor * sensor;
  SbBool seeking;
};

static SeekData seekdata;

void resetRoll(SoCamera * camera, const SbVec3f & viewup)
{
  assert(camera);

  SbVec3f newy = viewup;
  if (newy == SbVec3f(0.0f, 0.0f, 0.0f)) return;

  SbMatrix camerarot;
  camerarot.setRotate(camera->orientation.getValue());
  
  SbVec3f Z;
  Z[0] = camerarot[2][0];
  Z[1] = camerarot[2][1];
  Z[2] = camerarot[2][2];
  
  if (fabs(Z.dot(newy)) > 0.99f) {
    // just give up
    return;
  }
  SbVec3f newx = newy.cross(Z);
  newy = Z.cross(newx);
  
  newx.normalize();
  newy.normalize();

  camerarot[0][0] = newx[0];
  camerarot[0][1] = newx[1];
  camerarot[0][2] = newx[2];
  
  camerarot[1][0] = newy[0];
  camerarot[1][1] = newy[1];
  camerarot[1][2] = newy[2];

  camera->orientation.enableNotify(FALSE);  
  camera->orientation = SbRotation(camerarot);
  camera->orientation.enableNotify(TRUE);
}

void seekToPoint(SoCamera * camera,
                 const SbVec3d & endpoint,
                 const SbRotation & endorient)
{
  seekdata.camera = camera;
  seekdata.endpoint = endpoint;
  seekdata.endorient = endorient;
  
  camera->isOfType(UTMCamera::getClassTypeId()) ?
    seekdata.startpoint = ((UTMCamera *)camera)->utmposition.getValue() :
    seekdata.startpoint.setValue(camera->position.getValue());

  seekdata.startorient = camera->orientation.getValue();

  if (seekdata.sensor->isScheduled()) {
    seekdata.sensor->unschedule();
  }
  
  seekdata.seeking = TRUE;
  seekdata.sensor->setBaseTime(SbTime::getTimeOfDay());
  seekdata.sensor->schedule();
}

SbBool isSeeking(void)
{
  return seekdata.seeking;
}
