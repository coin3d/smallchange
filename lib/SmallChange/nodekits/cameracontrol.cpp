#include "cameracontrol.h"

#include <Inventor/SbLinear.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoOrthographicCamera.h>


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
  
  camera->orientation = SbRotation(camerarot);
}
