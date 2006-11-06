#include "cameracontrol.h"

#include <Inventor/SbLinear.h>
#include <Inventor/SbTime.h>
#include <Inventor/SbViewportRegion.h>
#include <Inventor/nodes/SoNode.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoOrthographicCamera.h>
#include <Inventor/nodekits/SoBaseKit.h>
#include <SmallChange/nodes/UTMCamera.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/sensors/SoTimerSensor.h>
#include <Inventor/SbDict.h>

static SbDict * cameradict = NULL;

class SeekData {
public:
  SeekData(void);
  ~SeekData();

  SoCamera * camera;
  SbVec3d startpoint;
  SbRotation startorient;
  SbVec3d endpoint;
  SbRotation endorient;

  float seektime;
  SoTimerSensor * sensor;

private:
  static void seeksensorCB(void * closure, SoSensor * sensor);
};

static void cam_set_pos(SoCamera * camera, 
                        const SbVec3d & position)
{
  camera->isOfType(UTMCamera::getClassTypeId()) ?
    ((UTMCamera *)camera)->utmposition.setValue(position) :
    camera->position.setValue(SbVec3f(position[0], position[1], position[2]));
}

static SbVec3d cam_get_pos(SoCamera * camera)
{
  SbVec3d pos;
  camera->isOfType(UTMCamera::getClassTypeId()) ?
    pos = ((UTMCamera *)camera)->utmposition.getValue() :
    pos.setValue(camera->position.getValue());

  return pos;
}

SeekData::SeekData() 
{
  this->seektime= 2.0f;
  this->sensor = new SoTimerSensor(SeekData::seeksensorCB, this);
}
  
SeekData::~SeekData() 
{
  delete this->sensor;
}

void 
SeekData::seeksensorCB(void * closure, SoSensor * sensor)
{
  SeekData * thisp = (SeekData *) closure;
  SoTimerSensor * timersensor = (SoTimerSensor *) sensor;
  
  SbTime dt = SbTime::getTimeOfDay() - timersensor->getBaseTime();
  double t = double(dt.getValue()) / thisp->seektime;
  
  if (t > 1.0f) t = 1.0f;
  if (t + timersensor->getInterval().getValue() > 1.0f) t = 1.0f;
  
  SbBool end = (t == 1.0f);
  
  t = (1.0 - cos(M_PI*t)) * 0.5;
  
  thisp->camera->orientation = SbRotation::slerp(thisp->startorient,
                                                 thisp->endorient,
                                                 (float) t);
  
  cam_set_pos(thisp->camera, thisp->startpoint + (thisp->endpoint - thisp->startpoint) * t);
  
  if (end) {
    thisp->sensor->unschedule();
    cameradict->remove((unsigned long)thisp->camera);
    delete thisp;
  }
}

void cam_reset_roll(SoCamera * camera, const SbVec3f & viewup)
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

void cam_seek_to_point(SoCamera * camera,
                       const SbVec3d & endpoint,
                       const SbRotation & endorient,
                       const float seektime)
{
  // if seektime is zero, get straight to the point
  if (seektime == 0.0f) {
    cam_set_pos(camera, endpoint);
    camera->orientation.setValue(endorient);
    return;
  }

  if (!cameradict) {
    cameradict = new SbDict;
  }
  
  SeekData * seekdata;
  if (!cameradict->find((unsigned long) camera, (void*&) seekdata)) {
    seekdata = new SeekData;
    seekdata->camera = camera;
    cameradict->enter((unsigned long) camera, seekdata);
  }

  seekdata->startpoint = cam_get_pos(camera);
  seekdata->startorient = camera->orientation.getValue();

  seekdata->endpoint = endpoint;
  seekdata->endorient = endorient;
  seekdata->seektime = seektime;
  
  if (seekdata->sensor->isScheduled()) {
    seekdata->sensor->unschedule();
  }
  seekdata->sensor->setBaseTime(SbTime::getTimeOfDay());
  seekdata->sensor->schedule();
}

void cam_seek_to_node(SoCamera * camera,
                      SoNode * node,
                      SoNode * root,
                      const SbViewportRegion & vp,
                      const float seektime)
{
  assert(camera && node && root);
  
  // save camera values
  SbVec3d startpoint = cam_get_pos(camera);
  SbRotation startorient = camera->orientation.getValue();
  
  // use camera to calculate an appropriate viewpoint
  SbBool searchchildren = SoBaseKit::isSearchingChildren();
  SoBaseKit::setSearchingChildren(TRUE);
  SoSearchAction sa;
  sa.setNode(node);
  sa.setSearchingAll(TRUE);
  sa.apply(root);

  SoBaseKit::setSearchingChildren(searchchildren);
  
  SoGetBoundingBoxAction ba(vp);
  SoFullPath * path = (SoFullPath *) sa.getPath();
  ba.apply(path);

  camera->viewAll(sa.getPath(), vp);
  camera->pointAt(ba.getCenter());

  SbVec3d endpoint = cam_get_pos(camera);
  SbRotation endorient = camera->orientation.getValue();

  // restore camera values
  camera->orientation = startorient;
  cam_set_pos(camera, startpoint);
  
  // seek to the computed point
  cam_seek_to_point(camera, endpoint, endorient, seektime);
}

SbBool cam_is_seeking(SoCamera * camera)
{
  void * dummy = NULL;
  return cameradict->find((unsigned long) camera, dummy);
}
