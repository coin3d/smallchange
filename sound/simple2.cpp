#include <math.h>
#include <stdlib.h>
#include <conio.h>

/*
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>
*/

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

#include <SmallChange/nodes/SoListener.h>
#include <SmallChange/nodes/SoSound.h>
#include <SmallChange/nodes/SoAudioClip.h>
#include <SmallChange/nodes/SoAudioClipStreaming.h>
#include <SmallChange/misc/SoAudioDevice.h>
// #include <SmallChange/misc/ALTools.h>

#include <SmallChange/misc/SbAudioWorkerThread.h>


#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>


SoSeparator * root;
SoPerspectiveCamera * camera;
SoTransform *xf;


static void timerSensorCallback(void *data, SoSensor *)
{

  float x, y, z;
  static counter=0;
  SbVec3f cpos;
  cpos=xf->translation.getValue();
  cpos.getValue(x, y, z);
//  x=x+0.3;
  x=x+0.03;
  xf->translation.setValue(x, y, z);
}

int user_callback(void *userdata)
{
  printf(".");
  return 1;
}

FILE *oggfile;
OggVorbis_File vf;
int current_section;

// #include <io.h>
// #include <fcntl.h>

void openoggfile(char *filename)
{
  oggfile = fopen(filename, "rb");
  if (oggfile == NULL)
  {
      fprintf(stderr,"Unknown file.\n");
      exit(1);
  }
//  _setmode( _fileno( stdin ), _O_BINARY );

  if(ov_open(oggfile, &vf, NULL, 0) < 0) {
//  if(ov_open(stdin, &vf, NULL, 0) < 0) {
      fprintf(stderr,"Input does not appear to be an Ogg bitstream.\n");
      exit(1);
  }

  /* Throw the comments plus a few lines about the bitstream we're
     decoding */
  {
    char **ptr=ov_comment(&vf,-1)->user_comments;
    vorbis_info *vi=ov_info(&vf,-1);
    while(*ptr){
      fprintf(stderr,"%s\n",*ptr);
      ++ptr;
    }
    fprintf(stderr,"\nBitstream is %d channel, %ldHz\n",vi->channels,vi->rate);
    fprintf(stderr,"\nDecoded length: %ld samples\n",
	    (long)ov_pcm_total(&vf,-1));
    fprintf(stderr,"Encoded by: %s\n\n",ov_comment(&vf,-1)->vendor);
  }


};

void closeoggfile()
{
  ov_clear(&vf);
  fclose(oggfile);
};

static SbBool fill_from_ogg_callback(void *buffer, int length, void *userdata)
{
  int numread = 0;
  int ret;
  char *ptr = (char *)buffer;
  while (numread<length*2)
  {
    ret=ov_read(&vf, ptr+numread, length*2-numread, 0, 2, 1, &current_section);
    numread+=ret;
    if (ret == 0)
      return FALSE;
  };

  return TRUE;
};


static SbBool fill_callback(void *buffer, int length, void *userdata)
{
    short int *ibuffer = (short int *)buffer;
  static double freq = 600.0;
  static double ffreq = 600.0;
  static int counter = 0;
  //ffreq +=10.0;
  freq +=10.0;
  int a=100;
  int h=800;
  int d=100;
  int p=44100.0/(ffreq/110.0);

  // det klikker av og til fordi vi forandrer p og mod'er med denne

  int c;
  double value;
  for (int i=0; i< length; i++)
  {
    c = (counter+i)%p;
    value = sin( ((float)(counter+i))/44100.0*2*3.14159265358979323846264383*freq*2);
    value=0;

    value = 32000.0*sin( ((float)(counter+i))/44100.0*2*3.14159265358979323846264383*(freq) + 2*value);


    if (c<=a)
      ibuffer[i]= (double)c/(double)a * value;
    else if (c<=a+h)
      ibuffer[i]= value;
    else if (c<=a+h+d)
      ibuffer[i]= (1.0-(double)(c-(a+h))/(double)d) * value;
    else
      ibuffer[i] = 0.0;


//      ibuffer[i]= value;

  }
  counter+=length;

  return TRUE;

};

void
main(
  int argc,
  char ** argv )
{

  HWND window = SoWin::init( argv[0] );

  SoListener::initClass();
  SoSound::initClass();
  SoAudioClip::initClass();
  SoAudioClipStreaming::initClass();
  SoAudioRenderAction::initClass();

  SoAudioDevice audioDevice;
  SbBool ret = audioDevice.init("OpenAL", "DirectSound3D");
  if (!ret)
  {
    printf("audioDevice::init failed\n");
  };

  SoListener *listener;
  SoSound *sourcenode;
//  SoAudioClip *buffernode;
  SoAudioClipStreaming *buffernode;

  root = new SoSeparator;

  root->addChild( camera = new SoPerspectiveCamera );
  root->addChild(listener = new SoListener );

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
  sep->addChild(sourcenode = new SoSound);
//  sep->addChild(listener = new SoListener);
  
//  root->addChild(buffernode = new SoAudioClip);
  root->addChild(buffernode = new SoAudioClipStreaming);
//  buffernode->setAsyncMode(FALSE);
  buffernode->setAsyncMode(TRUE);
//  buffernode->setBufferInfo(4410, 3);
//  buffernode->setBufferInfo(44100/100*3, 10); 
  buffernode->setBufferInfo(44100/10, 8); 
  // ^^ 20010809 - if buffer*num == 1 sec, we will have jitter/loops at the beginning
  // or "very round values) (0.1 sec, 0.2 sec, 
  // I have no idea why this happens !!!
//  buffernode->url.setValue("lyd1.wav");
//  buffernode->loop.setValue(TRUE);
  buffernode->loop.setValue(FALSE); 
  buffernode->startTime.setValue(SbTime::getTimeOfDay() + SbTime(2));
  buffernode->stopTime.setValue(SbTime::getTimeOfDay() + SbTime(100));

  openoggfile("allways.ogg");

  //  buffernode->setUserCallback(fill_callback, NULL);
  buffernode->setUserCallback(fill_from_ogg_callback, NULL);
//  buffernode->pitch.setValue(2.0f);

  sourcenode->source.setValue(buffernode);

//  root->addChild( node=new SoCone );
//  add separator and add several nodes
//  root->addChild( node=new SoListener );

  listener->orientation.connectFrom(&camera->orientation);
  listener->position.connectFrom(&camera->position);

  root->addChild(sep);

	SbViewportRegion vp;
	vp.setWindowSize(SbVec2s(400, 400));

  SoWinRenderArea * viewer = new SoWinRenderArea( window );

  viewer->setSceneGraph( root );

	viewer->setViewportRegion(vp);
  camera->viewAll( node, vp);

	SoTimerSensor *timerSensor;
	timerSensor = new SoTimerSensor(timerSensorCallback, NULL);
//	timerSensor->setInterval(1);
	timerSensor->setInterval(SbTime(0, 1000*500));
	timerSensor->schedule();

  audioDevice.setSceneGraph(root);
  audioDevice.setGLRenderAction(viewer->getGLRenderAction());

  audioDevice.enable();

//  SbAudioWorkerThread mt(user_callback);
//  SbAudioWorkerThread mt(NULL);
//  mt.start();

  viewer->show();
  SoWin::show( window );

  MMRESULT mmres = joySetCapture(window, JOYSTICKID1, 20, TRUE);

  SoWin::mainLoop();

//  mt.stop();

  joyReleaseCapture(mmres);

  audioDevice.disable();

  closeoggfile();

// fixme: do the deletion - but wait for mbm to fix sowin-bug
//  delete viewer;

//  getch();

} // main()


