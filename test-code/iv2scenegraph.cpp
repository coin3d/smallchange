#include <Inventor/SoDB.h>
#include <Inventor/SoInput.h>
#include <Inventor/SoOutput.h>
#include <Inventor/SoInteraction.h>
#include <Inventor/SoOffscreenRenderer.h>
#include <Inventor/nodekits/SoNodeKit.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoOrthographicCamera.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/actions/SoWriteAction.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <SmallChange/actions/SoGenerateSceneGraphAction.h>
#include <SmallChange/actions/SoTweakAction.h>
#include <cstdio>
#include <cmath>

/*
 * --inventor-out
 * --image-out
 * --image-scale-factor
 * --image-background
 */

int
main(int argc, char ** argv)
{
  if ( argc != 3 ) {
    fprintf(stderr, "Usage: %s <infile.iv> <outfile.iv>\n", argv[0]);
    return -1;
  }

  SoDB::init();
  SoNodeKit::init();
  SoInteraction::init();

  SoGenerateSceneGraphAction::initClass();
  SoTweakAction::initClass();

  SoInput in;
  SoNode * scene, * graph;
  if ( !in.openFile(argv[1]) ) {
    fprintf(stderr, "%s: error opening \"%s\" for reading.\n", argv[0], argv[1]);
    return -1;
  }
  scene = SoDB::readAll(&in);
  if ( scene == NULL ) {
    fprintf(stderr, "%s: error parsing \"%s\"\n", argv[0], argv[1]);
    return -1;
  }
  scene->ref();

  SoGenerateSceneGraphAction action;
  // action.setDropTypeIfNameEnabled(TRUE);
  action.apply(scene);
  graph = action.getGraph();
  if ( graph == NULL ) {
    fprintf(stderr, "%s: error generating scene graph\n", argv[0]);
    return -1;
  }
  graph->ref();
  scene->unref();
  scene = NULL;

  // figure out camera settings and needed rendering canvas size
  SoGetBoundingBoxAction bbaction(SbViewportRegion(64,64)); // just something
  bbaction.apply(graph);

  SbBox3f bbox = bbaction.getBoundingBox();
  SbVec3f min = bbox.getMin();
  SbVec3f max = bbox.getMax();
  float bwidth = max[0] - min[0];
  float bheight = max[1] - min[1];
  // fprintf(stdout, "min: %g %g %g\n", min[0], min[1], min[2]);
  // fprintf(stdout, "max: %g %g %g\n", max[0], max[1], max[2]);

  // place camera
  SoSearchAction search;
  search.setType(SoCamera::getClassTypeId());
  search.setInterest(SoSearchAction::FIRST);
  search.apply(graph);
  SoPath * campath = search.getPath();
  SoOrthographicCamera * cam = (SoOrthographicCamera *) campath->getTail();
  assert(cam != NULL);
  SbVec3f pos = cam->position.getValue();
  cam->position.setValue(SbVec3f(min[0] + ((max[0]-min[0])/2.0f),
                                 min[1] + ((max[1]-min[1])/2.0f),
                                 pos[2]));
  cam->height.setValue(bheight);

  if ( TRUE ) { // FIXME: only write .iv-scene if asked
    SoOutput out;
    if ( !out.openFile(argv[2]) ) {
      fprintf(stderr, "%s: error opening \"%s\" for writing.\n", argv[0], argv[2]);
      return -1;
    }
    SoWriteAction writer(&out);
    // writer.setCoinFormattingEnabled(TRUE);
    writer.apply(graph);
  }

  int width = (int) ceil(bwidth * 150.0f) + 2;
  int height = (int) ceil(bheight * 150.0f);
  fprintf(stderr, "image: %d x %d\n", width, height);
  if ( TRUE ) { // FIXME: only write image if asked
    SoOffscreenRenderer renderer(SbViewportRegion(width, height));
    SoGLRenderAction * glra = renderer.getGLRenderAction();
    glra->setNumPasses(9);
    // FIXME: auto-crop image afterwards?  seems like it's a perfect fit right now
    renderer.setComponents(SoOffscreenRenderer::RGB_TRANSPARENCY);
    renderer.setBackgroundColor(SbColor(1.0f,1.0f,1.0f));
    renderer.render(graph);
    // FIXME: support command line option filename
    // FIXME: also support .eps
    renderer.writeToFile("output.png", "png");
  }

  graph->unref();
  return 0;
}
