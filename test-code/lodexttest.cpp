#include <Inventor/@Gui@/So@Gui@.h>
#include <Inventor/@Gui@/viewers/So@Gui@ExaminerViewer.h>
#include <Inventor/SoDB.h>
#include <Inventor/SoInput.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoMatrixTransform.h>
#include <SmallChange/nodes/SoLODExtrusion.h>
#include <cstdlib>

static void
setup_lod(SoLODExtrusion * lod)
{
  int i;
  lod->radius = 1.0f;
  lod->spine.setNum(100);
  SbVec3f * ptr = lod->spine.startEditing();
  for (i = 0; i < 100; i++) {
    ptr[i] = SbVec3f(0.0f, (float)i*4, 0.0f);
  }
  lod->spine.finishEditing();
  lod->lodDistance1 = 400.0f;

#if 0 // to test multiple colors
  lod->color.setNum(100);
  SbColor * col = lod->color.startEditing();
  for (i = 0; i < 100; i++) {
    col[i] = SbColor(float(rand())/RAND_MAX,
                     float(rand())/RAND_MAX,
                     float(rand())/RAND_MAX);
  }
  lod->color.finishEditing();

#endif //colors

}

int main(int argc, char ** argv)
{
  @WIDGET@ * window = So@Gui@::init( argv[0] );

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

  So@Gui@ExaminerViewer * examinerviewer = new So@Gui@ExaminerViewer(window);
  examinerviewer->setBackgroundColor(SbColor(0.2f,0.4f,0.6f));
  examinerviewer->setSceneGraph(root);
  examinerviewer->show();
  So@Gui@::show( window );
  So@Gui@::mainLoop();

  delete examinerviewer;

  root->unref();

  return 0;
} // main()
