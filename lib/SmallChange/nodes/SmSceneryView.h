#ifndef SM_SCENERYVIEW_H
#define SM_SCENERYVIEW_H

#include <Inventor/SbBasic.h>

#include <Inventor/fields/SoSFBool.h>
#include <Inventor/fields/SoSFInt32.h>
#include <Inventor/fields/SoSFFloat.h>
#include <Inventor/fields/SoMFVec3f.h>
#include <Inventor/fields/SoMFInt32.h>

#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/nodes/SoNode.h>

#include <SmallChange/basic.h>

class SoFieldSensor;
class SoSensor;
class SoPrimitiveVertex;
class SoFaceDetail;
class SoGLImage;

class SMALLCHANGE_DLL_API SmSceneryView : public SoNode {
  typedef SoNode inherited;

  SO_NODE_HEADER(SmSceneryView);

public:
  static void initClass(void);
  SmSceneryView(void);

  SoSFInt32 viewId;
  SoMFVec3f hotspots;
  SoSFBool hotCamera;

  SoSFFloat blockRottger;
  SoSFFloat loadRottger;

  SoMFInt32 renderSequence;

  SoSFBool visualDebug;

protected:
  virtual ~SmSceneryView(void);

private:

};

#endif // !SM_SCENERYVIEW_H
