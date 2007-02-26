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
#include <Inventor/projectors/SbSphereSheetProjector.h>
#include <Inventor/SbDict.h>

static SbDict * cameradict = NULL;
static SbSphereSheetProjector * spinprojector = NULL;

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

static void set_pos(SoCamera * camera, 
                    const SbVec3d & position)
{
  camera->isOfType(UTMCamera::getClassTypeId()) ?
    ((UTMCamera *)camera)->utmposition.setValue(position) :
    camera->position.setValue(SbVec3f(float(position[0]), float(position[1]), float(position[2])));
}

static void set_pos(SoCamera * camera, 
                    const SbVec3f & position)
{
  set_pos(camera, SbVec3d(position[0], position[1], position[2]));
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
  
  set_pos(thisp->camera, thisp->startpoint + (thisp->endpoint - thisp->startpoint) * t);
  
  if (end) {
    thisp->sensor->unschedule();
    cameradict->remove((unsigned long)thisp->camera);
    delete thisp;
  }
}

void cam_reset_roll(SoCamera * camera, const SbVec3f & up)
{
  assert(camera != NULL && up != SbVec3f(0.0f, 0.0f, 0.0f));

  SbMatrix matrix;
  matrix.setRotate(camera->orientation.getValue());
  SbVec3f lookat(matrix[2]);

  if (fabs(lookat.dot(up)) > 0.99f) {
    // just give up
    return;
  }
  
  SbVec3f side = up.cross(lookat);
  SbVec3f camup = lookat.cross(side);
  
  side.normalize();
  camup.normalize();

  matrix[0][0] = side[0];
  matrix[0][1] = side[1];
  matrix[0][2] = side[2];
  
  matrix[1][0] = camup[0];
  matrix[1][1] = camup[1];
  matrix[1][2] = camup[2];
  
  camera->orientation.enableNotify(FALSE);  
  camera->orientation = SbRotation(matrix);
  camera->orientation.enableNotify(TRUE);
}

void cam_spin(SoCamera * camera,
              const SbVec2f & dp,
              const SbVec3f & up)
{
  if (!spinprojector) {
    spinprojector = new SbSphereSheetProjector(SbSphere(SbVec3f(0, 0, 0), 0.8f));
    SbViewVolume volume;
    volume.ortho(-1, 1, -1, 1, -1, 1);
    spinprojector->setViewVolume(volume);
  }

  assert(camera != NULL);
  
  SbVec2f prevpos(0.5f, 0.5f);
  SbVec2f currpos = prevpos + dp;

  // find the rotation from prevpos to currpos
  SbVec3f to = spinprojector->project(prevpos);
  SbVec3f from = spinprojector->project(currpos);
  SbRotation rot = spinprojector->getRotation(from, to);

  // find the matrices for current rotation and new rotation
  SbMatrix matrix, newmatrix;
  matrix.setRotate(camera->orientation.getValue());
  newmatrix.setRotate(rot * camera->orientation.getValue());

  SbVec3f camup(matrix[1]);
  SbVec3f lookat(matrix[2]);
  SbVec3f newcamup(newmatrix[1]);
  SbVec3f newlookat(newmatrix[2]);

  // Do not allow the camera up vector to cross the plane defined by
  // the world up vector by returning the old values
  if (newcamup.dot(up) < 0.0f) {
      return;
  }

  // calculate new camera position and orientation
  SbVec3f campos = camera->position.getValue();
  float focaldist = camera->focalDistance.getValue();
  SbVec3f focalpoint = campos - focaldist * lookat;
  SbVec3f newpos = (focalpoint + focaldist * newlookat) - campos;

  if (camera->isOfType(UTMCamera::getClassTypeId())) {
    UTMCamera * utmcamera = (UTMCamera *) camera;
    SbVec3d utmpos = utmcamera->utmposition.getValue();
    newpos += SbVec3f(float(utmpos[0]), float(utmpos[1]), float(utmpos[2]));
  }
  
  set_pos(camera, newpos);
  camera->orientation = SbRotation(newmatrix);
}

void cam_seek_to_point(SoCamera * camera,
                       const SbVec3d & endpoint,
                       const SbRotation & endorient,
                       const float seektime)
{
  // if seektime is zero, get straight to the point
  if (seektime == 0.0f) {
    set_pos(camera, endpoint);
    camera->orientation.setValue(endorient);
    return;
  }

  if (!cameradict) {
    cameradict = new SbDict;
  }

  void * seekdata_ptr = NULL;
  SeekData * seekdata;
  if (cameradict->find((unsigned long) camera, seekdata_ptr)) {
    seekdata = (SeekData*) seekdata_ptr;
  } else {
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
  set_pos(camera, startpoint);
  
  // seek to the computed point
  cam_seek_to_point(camera, endpoint, endorient, seektime);
}

SbBool cam_is_seeking(SoCamera * camera)
{
  void * dummy = NULL;
  return cameradict->find((unsigned long) camera, dummy);
}
