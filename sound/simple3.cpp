#include <math.h>
#include <stdlib.h>

#include <Inventor/Win/SoWin.h>
#include <Inventor/Win/viewers/SoWinExaminerViewer.h>

#include <Inventor/SoDB.h>
#include <Inventor/SoInput.h>
#include <Inventor/SbRotation.h>
#include <Inventor/nodes/SoBaseColor.h>
#include <Inventor/nodes/SoCone.h>
#include <Inventor/nodes/SoCube.h>
#include <Inventor/nodes/SoCylinder.h>
#include <Inventor/nodes/SoShapeHints.h>
#include <Inventor/nodes/SoSphere.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoText3.h>
#include <Inventor/nodes/SoDirectionalLight.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoTransform.h>
#include <Inventor/Win/viewers/SoWinViewer.h>
#include <Inventor/sensors/SoTimerSensor.h>

#include <Inventor/fields/SoMFString.h>
#include <Inventor/sensors/SoFieldSensor.h>
#include <Inventor/errors/SoDebugError.h>

#include <Inventor/nodes/SoShape.h>
#include <Inventor/elements/SoCoordinateElement.h>
#include <Inventor/elements/SoModelMatrixElement.h>

#include <Inventor/actions/SoGLRenderAction.h>

#if HAVE_CONFIG_H
#include <config.h>
#endif

SoSeparator * root;
SoPerspectiveCamera * camera;
SoTransform *xf;

void
main(
  int argc,
  char ** argv )
{

  HWND window = SoWin::init( argv[0] );

  root = new SoSeparator;

  root->addChild( camera = new SoPerspectiveCamera );

  root->addChild( new SoDirectionalLight );

  SoShapeHints * hints = new SoShapeHints;
  hints->vertexOrdering = SoShapeHints::COUNTERCLOCKWISE;
  hints->shapeType = SoShapeHints::SOLID;
  hints->creaseAngle = 0.91f;
  root->addChild( hints );
  SoBaseColor * basecol = new SoBaseColor;
  basecol->rgb.setValue( float(rand())/float(RAND_MAX),
                         float(rand())/float(RAND_MAX),
                         float(rand())/float(RAND_MAX) );
  root->addChild( basecol );


	SoNode *node;
  SoSeparator *sep = new SoSeparator;

  xf = new SoTransform;
  xf->translation.setValue(-1.3, 0, 0);
  
  sep->addChild(new SoCone);
  sep->addChild(xf);
  sep->addChild(node=new SoCone);

  root->addChild(sep);

	SbViewportRegion vp;
	vp.setWindowSize(SbVec2s(400, 400));

  SoWinRenderArea * viewer = new SoWinRenderArea( window );

  viewer->setSceneGraph( root );

	viewer->setViewportRegion(vp);
  camera->viewAll( node, vp);

  viewer->show();
  SoWin::show( window );

  SoWin::mainLoop();

  delete viewer;

} // main()


