#include <Inventor/@Gui@/So@Gui@.h>
#include <Inventor/@Gui@/viewers/So@Gui@ExaminerViewer.h>
#include <Inventor/SoDB.h>
#include <Inventor/SoInput.h>
#include <Inventor/SoOutput.h>
#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/actions/SoWriteAction.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoTranslation.h>
#include <Inventor/nodes/SoCamera.h>
#include <SmallChange/nodes/SoText2Set.h>
#include <cassert>

// *************************************************************************

int main(int argc, char ** argv )
{
  assert(argc >= 2);
  @WIDGET@ * window = So@Gui@::init( argv[0] );

  SoText2Set::initClass();

  SoInput input;
  SbBool ret = input.openFile(argv[1]);
  assert(ret);

  SoSeparator * root = SoDB::readAll(&input);
  assert(root);
  root->ref();

  So@Gui@ExaminerViewer * examinerviewer = new So@Gui@ExaminerViewer( window );
  examinerviewer->setSceneGraph( root );
  examinerviewer->show();
  So@Gui@::show( window );
  So@Gui@::mainLoop();

  delete examinerviewer;

  root->unref();

  return 0;
} // main()
