
#include <Inventor/Qt/SoQt.h>
#include <Inventor/Qt/viewers/SoQtExaminerViewer.h>
#include <Inventor/SoDB.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoTranslation.h>
#include <Inventor/nodes/SoCamera.h>
#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/actions/SoWriteAction.h>
#include <Inventor/SoInput.h>
#include <Inventor/SoOutput.h>
#include <assert.h>
#include <SmallChange/misc/Init.h>
#include <SmallChange/nodekits/LegendKit.h>
#include <SmallChange/elements/GLDepthBufferElement.h>
#include <SmallChange/nodes/ViewportRegion.h>
#include <SmallChange/nodes/DepthBuffer.h>
#include <Inventor/actions/SoSearchAction.h>


// *************************************************************************

#ifdef CLOD_TRICOUNT // dummy 
int clod_tricount;
#endif

static uint32_t color_cb(double val)
{
  if (val < 0.25) {
    val += 0.5f;
    return (uint32_t(val*255)<<24)|0x2222ff;
  }
  else if (val < 0.6) {
    val += 0.3f;
    return (uint32_t(val*255)<<16)|0x220022ff;
  }
  else if (val < 0.8) {
    val += 0.2f;
    return (uint32_t(val*255)<<8)|0x222200ff;
  }
  else {
    return (uint32_t(val*255.0)<<24)|(uint32_t(val*255.0)<<16) | 0x22ff;
  }
}

int
main(int argc, char ** argv)
{
  if (argc != 2) {
    (void)fprintf(stderr, "\n\tUsage: %s legendkitfile.iv\n\n", argv[0]);
    exit(1);
  }

  QWidget * window = SoQt::init(argv[0]);
  smallchange_init();
  // LegendKit::initClass();

  SoInput input;
  SbBool ret = input.openFile(argv[1]);
  assert(ret);

  SoSeparator * root = SoDB::readAll(&input);
  assert(root);
  root->ref();

  SoSearchAction sa;
  sa.setType(LegendKit::getClassTypeId());
  sa.setInterest(SoSearchAction::FIRST);
  sa.setSearchingAll(FALSE);
  sa.apply(root);
  SoPath * path = sa.getPath();
  if (path) {
    LegendKit * kit = (LegendKit*) path->getTail();
    fprintf(stderr,"found LegendKit\n");
    
    kit->setColorCB(color_cb);
    kit->setImageTransparency(0.2);
    kit->useTextureNotImage(TRUE);
    kit->setBackgroundColor(SbColor(1.0f, 1.0f, 1.0f), 0.2f);
    kit->setTickAndLinesColor(SbColor(0.0f, 0.0f, 0.0f), 0.0f);
    kit->bigTickSize = 8;
    kit->smallTickSize = 4;
    kit->description.setNum(3);
    kit->description.set1Value(0, "Juba, juba, juba, juba");
    kit->description.set1Value(1, "Juba2, juba2, juba2, juba2");
    kit->description.set1Value(2, "Juba333, juba333, juba333, juba333");
    kit->setPosition(SbVec2s(20, 20));
    kit->descriptionOnTop = FALSE;
    kit->topSpace = 10.0f;

    SbString discrete("juba");
    for (int i = 0; i <= 16; i++) {
      if (!(i & 3)) {
        kit->addBigTick(double(i)/double(16), double(i), &discrete);
      }
      else {
        kit->addSmallTick(double(i)/double(16));
      }
    }
    //    kit->discreteUseLower = TRUE;
    kit->setDiscreteMode(TRUE);
    //    kit->enableBackground(FALSE);
    //    kit->enableImage(FALSE);
  }

  SoQtExaminerViewer * examinerviewer = new SoQtExaminerViewer( window );
  examinerviewer->setSceneGraph( root );
  examinerviewer->show();
  SoQt::show( window );
  SoQt::mainLoop();

  delete examinerviewer;

#if 0
  SoOutput out;
  if (out.openFile("legend_out.iv")) {
    SoWriteAction wa(&out);
    wa.apply(root);
  }
#endif  

  root->unref();

  return 0;
} // main()

