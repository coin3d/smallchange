// #define DOREAD 1


/*

  mulig ogg vorbis fix:
  bygg static isteden ??

  se bugliste, feil nr. 8 og 20

  */

/* debugging 
#if     (defined(_MT) || defined(_DLL)) && !defined(_MAC)
int * __cdecl _errno(void)
{
  return 0;
};
#define errno   (*_errno())
#else   // ndef _MT && ndef _DLL 
int errno;
#endif  // _MT || _DLL 
*/

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
#include <Inventor/actions/SoWriteAction.h>

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


/*
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>
*/

SoSeparator * root;
SoPerspectiveCamera * camera;
SoTransform *xf;
SoTransform *xf2;


static void timerSensorCallback(void *data, SoSensor *)
{

  float x, y, z;
  static int counter=0;
  counter++;

  SbVec3f cpos;
  cpos=xf->translation.getValue();
  cpos.getValue(x, y, z);
//  x=x+0.3;
  x=x+0.003;
  xf->translation.setValue(x, y, z);
  x=2.0*sin((double)counter/20.0*2.0*3.1415);
  xf2->translation.setValue(-x, y, z);
}

int user_callback(void *userdata)
{
  printf(".");
  return 1;
}

/*
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
*/

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

SoSeparator *readFile(const char *filename) 
{
  SoInput mySceneInput;
  if (!mySceneInput.openFile(filename)) {
    fprintf(stderr, "couldn't open file\n");
    return NULL;
  }
  SoSeparator *myGraph = SoDB::readAll(&mySceneInput);
  if (myGraph==NULL) {
    mySceneInput.closeFile();
    fprintf(stderr, "Problem reading file\n");
    return NULL;
  }
  mySceneInput.closeFile();
  return myGraph;
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

#ifdef DOREAD
  root = readFile("file2.iv");

  SoListener *listener;

  root->addChild( camera = new SoPerspectiveCamera );
  root->addChild(listener = new SoListener );


  listener->orientation.connectFrom(&camera->orientation);
  listener->position.connectFrom(&camera->position);

	SbViewportRegion vp;
	vp.setWindowSize(SbVec2s(400, 400));

  SoWinRenderArea * viewer = new SoWinRenderArea( window );

  viewer->setSceneGraph( root );

	viewer->setViewportRegion(vp);
//  camera->viewAll( node, vp);
  camera->viewAll( root, vp);

	SoTimerSensor *timerSensor;
/*
  timerSensor = new SoTimerSensor(timerSensorCallback, NULL);
//	timerSensor->setInterval(1);
	timerSensor->setInterval(SbTime(0, 1000*50));
	timerSensor->schedule();
*/

#else
  SoListener *listener;
  SoSound *sourcenode;
//  SoAudioClip *buffernode;
  SoAudioClipStreaming *buffernode;
  SoAudioClip *clip2;
  SoSound *source2;

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
  
  root->addChild(buffernode = new SoAudioClipStreaming);
//  buffernode->setAsyncMode(TRUE);
//  buffernode->setBufferInfo(4410, 3);
//  buffernode->setBufferInfo(44100/100*3, 10); 
//  buffernode->setBufferInfo((44100*30)/1000, 8); 
  // ^^ 20010809 - if buffer*num == 1 sec, we will have jitter/loops at the beginning
  // or "very round values) (0.1 sec, 0.2 sec, 
  // I have no idea why this happens !!!
//  buffernode->url.setValue("lyd1.wav");
  buffernode->url.setValue("allways.ogg");
//  buffernode->loop.setValue(TRUE);
//  buffernode->loop.setValue(FALSE); 
  buffernode->startTime.setValue(SbTime::getTimeOfDay() + SbTime(2));
  buffernode->stopTime.setValue(SbTime::getTimeOfDay() + SbTime(100));

  sourcenode->source.setValue(buffernode);
  root->addChild(sep);

//  openoggfile("allways.ogg");

//    buffernode->setUserCallback(fill_callback, NULL);
//  buffernode->setUserCallback(fill_from_ogg_callback, NULL);
//  buffernode->pitch.setValue(2.0f);

//  root->addChild( node=new SoCone );
//  add separator and add several nodes
//  root->addChild( node=new SoListener );


  SoSeparator *sep2 = new SoSeparator;
  xf2 = new SoTransform;
  xf2->translation.setValue(1.3, 0, 0);
  sep2->addChild(xf2);
  SoSphere *ball = new SoSphere();
//  sep2->addChild(node=new SoSphere);
  ball->radius = 0.3f;
  sep2->addChild(ball);
  sep2->addChild(source2 = new SoSound);
  root->addChild(clip2 = new SoAudioClip);
  clip2->url.setValue("lyd1.wav");
  clip2->loop.setValue(TRUE);
//  clip2->loop.setValue(FALSE);
  clip2->startTime.setValue(SbTime::getTimeOfDay() + SbTime(1));
  clip2->stopTime.setValue(SbTime::getTimeOfDay() + SbTime(100));
  source2->source.setValue(clip2);
  root->addChild(sep2);


  listener->orientation.connectFrom(&camera->orientation);
  listener->position.connectFrom(&camera->position);


	SbViewportRegion vp;
	vp.setWindowSize(SbVec2s(400, 400));

  SoWinRenderArea * viewer = new SoWinRenderArea( window );

  viewer->setSceneGraph( root );

	viewer->setViewportRegion(vp);
  camera->viewAll( node, vp);

	SoTimerSensor *timerSensor;
	timerSensor = new SoTimerSensor(timerSensorCallback, NULL);
//	timerSensor->setInterval(1);
	timerSensor->setInterval(SbTime(0, 1000*50));
	timerSensor->schedule();

//  SbAudioWorkerThread mt(user_callback);
//  SbAudioWorkerThread mt(NULL);
//  mt.start();

  SoWriteAction writeAction;
  writeAction.apply(root);

#endif

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

//  closeoggfile();

// fixme: do the deletion - but wait for mbm to fix sowin-bug
//  delete viewer;

//  getch();

} // main()


/*
  root = readFile("file.iv");

  SoListener *listener;
  SoSound *sourcenode;
//  SoAudioClip *buffernode;
  SoAudioClipStreaming *buffernode;
  SoAudioClip *clip2;
  SoSound *source2;

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
  
  root->addChild(buffernode = new SoAudioClipStreaming);
//  buffernode->setAsyncMode(TRUE);
//  buffernode->setBufferInfo(4410, 3);
//  buffernode->setBufferInfo(44100/100*3, 10); 
//  buffernode->setBufferInfo((44100*30)/1000, 8); 
  // ^^ 20010809 - if buffer*num == 1 sec, we will have jitter/loops at the beginning
  // or "very round values) (0.1 sec, 0.2 sec, 
  // I have no idea why this happens !!!
//  buffernode->url.setValue("lyd1.wav");
  buffernode->url.setValue("allways.ogg");
//  buffernode->loop.setValue(TRUE);
//  buffernode->loop.setValue(FALSE); 
  buffernode->startTime.setValue(SbTime::getTimeOfDay() + SbTime(2));
  buffernode->stopTime.setValue(SbTime::getTimeOfDay() + SbTime(100));

  sourcenode->source.setValue(buffernode);
  root->addChild(sep);

//  openoggfile("allways.ogg");

//    buffernode->setUserCallback(fill_callback, NULL);
//  buffernode->setUserCallback(fill_from_ogg_callback, NULL);
//  buffernode->pitch.setValue(2.0f);

//  root->addChild( node=new SoCone );
//  add separator and add several nodes
//  root->addChild( node=new SoListener );


  SoSeparator *sep2 = new SoSeparator;
  xf2 = new SoTransform;
  xf2->translation.setValue(1.3, 0, 0);
  sep2->addChild(xf2);
  SoSphere *ball = new SoSphere();
//  sep2->addChild(node=new SoSphere);
  ball->radius = 0.3f;
  sep2->addChild(ball);
  sep2->addChild(source2 = new SoSound);
  root->addChild(clip2 = new SoAudioClip);
  clip2->url.setValue("lyd1.wav");
  clip2->loop.setValue(TRUE);
  clip2->startTime.setValue(SbTime::getTimeOfDay() + SbTime(1));
  clip2->stopTime.setValue(SbTime::getTimeOfDay() + SbTime(100));
  source2->source.setValue(clip2);
  root->addChild(sep2);


  listener->orientation.connectFrom(&camera->orientation);
  listener->position.connectFrom(&camera->position);


	SbViewportRegion vp;
	vp.setWindowSize(SbVec2s(400, 400));

  SoWinRenderArea * viewer = new SoWinRenderArea( window );

  viewer->setSceneGraph( root );

	viewer->setViewportRegion(vp);
  camera->viewAll( node, vp);

	SoTimerSensor *timerSensor;
	timerSensor = new SoTimerSensor(timerSensorCallback, NULL);
//	timerSensor->setInterval(1);
	timerSensor->setInterval(SbTime(0, 1000*50));
	timerSensor->schedule();

  audioDevice.setSceneGraph(root);
  audioDevice.setGLRenderAction(viewer->getGLRenderAction());
  audioDevice.enable();

//  SbAudioWorkerThread mt(user_callback);
//  SbAudioWorkerThread mt(NULL);
//  mt.start();

  SoWriteAction writeAction;
  writeAction.apply(root);
*/


/*
  // open a wave file
  riff_t * file;
  file = riff_file_open( "lyd1.wav");
  if ( file == NULL ) {
    printf("Couldn't open file\n");
  }

  if ( riff_file_is_type( file, "WAVE" ) == 1 )
  {
    printf("We have a WAV file\n");
  }
  else
  {
    printf("We don't have a WAV file\n");
  }

  void *chunk;
  chunk = NULL;

  while (!file->at_eof) {
    if (riff_next_chunk_is_type(file, "fmt ")) {
      printf("'fmt '\n");
      int size;
      size = riff_next_chunk_size( file );
      chunk = riff_chunk_read(file);
            
      short int format;
      format = riff_chunk_shortword(chunk , 0);
      printf("    Format: %d\n", format);

      short int channels;
      channels = riff_chunk_shortword(chunk , 1);
      printf("    Channels: %d\n", channels);

      long int samplerate;
      samplerate = riff_chunk_longword(chunk , 2);
      printf("    Samplerate: %d\n", samplerate);

      // (void) riff_chunk_longword(chunk , 4); // ignore bpsec (longword)
      // (void) riff_chunk_shortword(chunk , 6); // ignore blockallign (shortword)

      short int bitsPerSample;
      bitsPerSample = riff_chunk_shortword(chunk , 7);
      printf("    bitsPerSample: %d\n", bitsPerSample);


      riff_chunk_free(chunk);
    }
    else if (riff_next_chunk_is_type(file, "data")) {
      printf("'data'\n");
      int size;
      size = riff_next_chunk_size( file );
      printf("    Size=%d\n", size);
//      chunk = riff_chunk_read(file);
//      riff_chunk_free(chunk);

      char *buf = new char[size];
      riff_chunk_read_data(file, buf, 0);
    }
    else {
       riff_chunk_skip(file);
    }

  };

  riff_file_close( file );
*/
