#ifndef COIN_SOLODEXTRUSION_H
#define COIN_SOLODEXTRUSION_H

#include <Inventor/nodes/SoShape.h>
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/fields/SoSFFloat.h>
#include <Inventor/fields/SoSFVec3f.h>
#include <Inventor/fields/SoSFInt32.h>
#include <Inventor/fields/SoMFVec2f.h>
#include <Inventor/fields/SoMFVec3f.h>
#include <Inventor/fields/SoMFRotation.h>

class SoLODExtrusion : public SoShape
{
  typedef SoShape inherited;
  SO_NODE_HEADER(SoLODExtrusion);

public:
  static void initClass(void);
  SoLODExtrusion(void);

  SoSFBool beginCap;
  SoSFBool ccw;
  SoSFFloat creaseAngle;
  SoMFVec2f crossSection;
  SoSFBool endCap;
  SoMFVec3f spine;
  SoSFFloat radius;
  SoSFInt32 circleSegmentCount;
  SoSFFloat lodDistance1;
  SoSFFloat lodDistance2;
  SoSFVec3f zAxis;
  SoMFVec3f color;

  virtual void GLRender(SoGLRenderAction * action);
  virtual void getPrimitiveCount(SoGetPrimitiveCountAction * action);
  virtual void computeBBox(SoAction * action,
                           SbBox3f & bbox, SbVec3f & center);

protected:
  virtual ~SoLODExtrusion();

  virtual void notify(SoNotList * list);
  virtual void generatePrimitives( SoAction * action );

  virtual SoDetail * createTriangleDetail(SoRayPickAction * action,
                                          const SoPrimitiveVertex * v1,
                                          const SoPrimitiveVertex * v2,
                                          const SoPrimitiveVertex * v3,
                                          SoPickedPoint * pp);
private:
  void updateCache(void);
  class SoLODExtrusionP * pimpl;
};
#endif // ! COIN_SOLODEXTRUSION_H
