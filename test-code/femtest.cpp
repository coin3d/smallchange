#include <Inventor/Qt/SoQt.h>
#include <Inventor/Qt/viewers/SoQtExaminerViewer.h>
#include <Inventor/SoDB.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/SoInput.h>
#include <SmallChange/nodekits/SoFEMKit.h>

#define XSIZE 30
#define YSIZE 30
#define ZSIZE 30

#define NODEIDX(x, y, z) (((z)*XSIZE*YSIZE)+((y)*XSIZE)+x)
#define ELEMENTIDX(x, y, z) (((z)*(XSIZE-1)*(YSIZE-1))+((y)*(XSIZE-1))+x)


static void
setup_fem(SoFEMKit * fem)
{
  int x, y, z;
  for (z = 0; z < ZSIZE; z++) {
    for (y = 0; y < YSIZE; y++) {
      for (x = 0; x < XSIZE; x++) {
        fem->addNode(NODEIDX(x,y,z), SbVec3f(x, y, z));
        fem->setNodeColor(NODEIDX(x,y,z), SbColor(float(x)/XSIZE,
                                                  float(y)/YSIZE,
                                                  float(z)/ZSIZE));
      }
    }
  }

  int nodeidx[8];

  for (z = 0; z < ZSIZE-1; z++) {
    for (y = 0; y < YSIZE-1; y++) {
      for (x = 0; x < XSIZE-1; x++) {
        if (rand() & 1) {
          nodeidx[0] = NODEIDX(x,y,z);
          nodeidx[1] = NODEIDX(x,y+1,z);
          nodeidx[2] = NODEIDX(x+1,y+1,z);
          nodeidx[3] = NODEIDX(x+1,y,z);
          nodeidx[4] = NODEIDX(x,y,z+1);
          nodeidx[5] = NODEIDX(x,y+1,z+1);
          nodeidx[6] = NODEIDX(x+1,y+1,z+1);
          nodeidx[7] = NODEIDX(x+1,y,z+1);
          fem->add3DElement(ELEMENTIDX(x,y,z), nodeidx);
          fem->setElementColor(ELEMENTIDX(x,y,z), SbColor(float(x)/XSIZE,
                                                          float(y)/YSIZE,
                                                          float(z)/ZSIZE));
        }
      }
    }
  }
  fem->removeHidden(TRUE);
}

int
main(int argc, char ** argv)
{
  QWidget * window = SoQt::init( argv[0] );

  SoFEMKit::initClass();
  SoSeparator * root = new SoSeparator;
  root->ref();
  SoFEMKit * fem = new SoFEMKit;

  setup_fem(fem);

  root->addChild(fem);

  SoQtExaminerViewer * examinerviewer = new SoQtExaminerViewer(window);
  examinerviewer->setSceneGraph(root);
  examinerviewer->show();
  SoQt::show( window );
  SoQt::mainLoop();

  delete examinerviewer;

  root->unref();

  return 0;
} // main()

