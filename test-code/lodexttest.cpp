#include <Inventor/Qt/SoQt.h>
#include <Inventor/Qt/viewers/SoQtExaminerViewer.h>
#include <Inventor/SoDB.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoMatrixTransform.h>
#include <Inventor/SoInput.h>
#include <SmallChange/nodes/SoLODExtrusion.h>

static void
setup_lod(SoLODExtrusion * lod)
{
  lod->radius = 1.0f;
  lod->spine.setNum(100);
  SbVec3f * ptr = lod->spine.startEditing();
  for (int i = 0; i < 100; i++) {
    ptr[i] = SbVec3f(0.0f, (float)i*4, 0.0f);
  }
  lod->spine.finishEditing();
  lod->lodDistance1 = 400.0f;
}

int
main(int argc, char ** argv)
{
  QWidget * window = SoQt::init( argv[0] );

  SoLODExtrusion::initClass();
  SoSeparator * root = new SoSeparator;
  root->ref();

  SoLODExtrusion * lod = new SoLODExtrusion;

#if 0 // enable to test left handed coordinate system
  SoMatrixTransform * mt = new SoMatrixTransform;
  mt->matrix = SbMatrix(1,0,0,0,
                        0,1,0,0,
                        0,0,-1,0,
                        0,0,0,1);
  root->addChild(mt);
  lod->ccw = FALSE;
#endif // left handed
  setup_lod(lod);

  root->addChild(lod);

  SoQtExaminerViewer * examinerviewer = new SoQtExaminerViewer(window);
  examinerviewer->setBackgroundColor(SbColor(0.2,0.4,0.6));
  examinerviewer->setSceneGraph(root);
  examinerviewer->show();
  SoQt::show( window );
  SoQt::mainLoop();

  delete examinerviewer;

  root->unref();

  return 0;
} // main()

