
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
#include <SmallChange/nodes/SoText2Set.h>


// *************************************************************************


int
main(
  int argc,
  char ** argv )
{
  assert(argc >= 2);
  QWidget * window = SoQt::init( argv[0] );

  SoText2Set::initClass();

  SoInput input;
  SbBool ret = input.openFile(argv[1]);
  assert(ret);

  SoSeparator * root = SoDB::readAll(&input);
  assert(root);
  root->ref();

  SoQtExaminerViewer * examinerviewer = new SoQtExaminerViewer( window );
  examinerviewer->setSceneGraph( root );
  examinerviewer->show();
  SoQt::show( window );
  SoQt::mainLoop();

  delete examinerviewer;

  root->unref();

  return 0;
} // main()

