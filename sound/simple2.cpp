// #define DOREAD 1


static double magdata[] = {
  472973.00,  8669053.00,  576.900, 
  472997.76,  8669056.48,  563.000, 
  473022.51,  8669059.96,  537.200, 
  473047.27,  8669063.44,  538.500, 
  473072.03,  8669066.92,  574.200, 
  473096.78,  8669070.40,  562.700, 
  473121.54,  8669073.88,  558.400, 
  473146.30,  8669077.36,  554.200, 
  473171.05,  8669080.83,  557.500, 
  473195.81,  8669084.31,  529.200, 
  473220.57,  8669087.79,  696.000, 
  473245.32,  8669091.27,  1015.300, 
  473270.08,  8669094.75,  703.700, 
  473294.84,  8669098.23,  659.100, 
  473319.59,  8669101.71,  646.800, 
  473344.35,  8669105.19,  642.200, 
  473369.11,  8669108.67,  631.600, 
  473393.86,  8669112.15,  643.000, 
  473418.62,  8669115.63,  642.900, 
  473443.38,  8669119.11,  640.100, 
  473468.13,  8669122.59,  638.900, 
  473492.89,  8669126.07,  640.800, 
  473517.65,  8669129.55,  650.400, 
  473542.40,  8669133.02,  658.300, 
  473567.16,  8669136.50,  660.300, 
  473591.92,  8669139.98,  649.400, 
  473616.67,  8669143.46,  645.600, 
  473641.43,  8669146.94,  661.700, 
  473666.19,  8669150.42,  663.100, 
  473690.94,  8669153.90,  657.900, 
  473715.70,  8669157.38,  643.800, 
  473740.46,  8669160.86,  631.200, 
  473765.21,  8669164.34,  640.000, 
  473789.97,  8669167.82,  643.200, 
  473814.73,  8669171.30,  633.900, 
  473839.48,  8669174.78,  635.900, 
  473864.24,  8669178.26,  639.100, 
  473889.00,  8669181.74,  632.600, 
  473913.75,  8669185.21,  631.200, 
  473938.51,  8669188.69,  641.500, 
  473963.27,  8669192.17,  681.500, 
  473988.02,  8669195.65,  683.000, 
  474012.78,  8669199.13,  665.800, 
  474037.54,  8669202.61,  682.700, 
  474062.29,  8669206.09,  657.800, 
  474087.05,  8669209.57,  642.200, 
  474111.81,  8669213.05,  641.900, 
  474136.56,  8669216.53,  636.100, 
  474161.32,  8669220.01,  640.300, 
  474186.08,  8669223.49,  639.100, 
  474210.84,  8669226.97,  631.600, 
  474235.59,  8669230.45,  631.900, 
  474260.35,  8669233.93,  634.100, 
  474285.11,  8669237.40,  639.400, 
  474309.86,  8669240.88,  647.500, 
  474334.62,  8669244.36,  651.100, 
  474359.38,  8669247.84,  656.600, 
  474384.13,  8669251.32,  653.200, 
  474408.89,  8669254.80,  649.500, 
  474433.65,  8669258.28,  649.800, 
  474458.40,  8669261.76,  638.800, 
  474483.16,  8669265.24,  638.200, 
  474507.92,  8669268.72,  638.800, 
  474532.67,  8669272.20,  649.000, 
  474557.43,  8669275.68,  647.900, 
  474582.19,  8669279.16,  648.400, 
  474606.94,  8669282.64,  648.300, 
  474631.70,  8669286.11,  649.300, 
  474656.46,  8669289.59,  661.400, 
  474681.21,  8669293.07,  656.400, 
  474705.97,  8669296.55,  658.800, 
  474730.73,  8669300.03,  658.100, 
  474755.48,  8669303.51,  671.000, 
  474780.24,  8669306.99,  674.700, 
  474805.00,  8669310.47,  672.300, 
  474829.75,  8669313.95,  669.300, 
  474854.51,  8669317.43,  660.100, 
  474879.27,  8669320.91,  661.900, 
  474904.02,  8669324.39,  667.400, 
  474928.78,  8669327.87,  672.700, 
  474953.54,  8669331.35,  668.900, 
  474978.29,  8669334.83,  669.000, 
  475003.05,  8669338.30,  661.700, 
  475027.81,  8669341.78,  662.700, 
  475052.56,  8669345.26,  670.800, 
  475077.32,  8669348.74,  679.000, 
  475102.08,  8669352.22,  680.900, 
  475126.83,  8669355.70,  683.200, 
  475151.59,  8669359.18,  684.500, 
  475176.35,  8669362.66,  688.900, 
  475201.10,  8669366.14,  692.500, 
  475225.86,  8669369.62,  692.300, 
  475250.62,  8669373.10,  690.000, 
  475275.37,  8669376.58,  692.900, 
  475300.13,  8669380.06,  688.100, 
  475324.89,  8669383.54,  688.800, 
  475349.64,  8669387.02,  688.300, 
  475374.40,  8669390.49,  690.300, 
  475399.16,  8669393.97,  690.600, 
  475423.91,  8669397.45,  687.200, 
  475448.67,  8669400.93,  684.300, 
  475473.43,  8669404.41,  679.800, 
  475498.18,  8669407.89,  678.300, 
  475522.94,  8669411.37,  677.300, 
  475547.70,  8669414.85,  669.800, 
  475572.45,  8669418.33,  669.800, 
  475597.21,  8669421.81,  668.800, 
  475621.97,  8669425.29,  670.000, 
  475646.72,  8669428.77,  671.100, 
  475671.48,  8669432.25,  680.000, 
  475696.24,  8669435.73,  689.900, 
  475720.99,  8669439.21,  695.000, 
  475735.85,  8669441.29,  696.900
};

static const int NUM_MAGDATA = sizeof(magdata) / (sizeof(double)*3);


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

SbList<SoSound *> sounds;

void deleteFinishedSounds()
{
  SoSound *sound = NULL;
  int length = sounds.getLength();
  for (int i=0; i<length; i++)
  {
    sound = sounds[i];
    SoAudioClip *audioClip = NULL;
    audioClip = (SoAudioClip *)sound->source.getValue();

    if (audioClip != NULL)
    {
      if (!audioClip->isActive.getValue())
      { // it's not playing
        SbTime stop;
        stop = audioClip->stopTime.getValue();
        if (SbTime::getTimeOfDay() > stop)
        {
          // it's not scheduled to start
          printf("Deleting sound\n");
          // delete sosound and audioclip
          sounds.remove(i);
          break;
        }
      };
    };
  }
};

SoSeparator * root;
SoPerspectiveCamera * camera;
SoTransform *xf;
SoTransform *xf2;


static void timerSensorCallback(void *data, SoSensor *)
{
  deleteFinishedSounds();

  float x, y, z;
  static int counter=0;
  counter++;

  SbVec3f cpos;
  cpos=xf->translation.getValue();
  cpos.getValue(x, y, z);
//  x=x+0.3;
  x=x+0.003;
  xf->translation.setValue(x, y, z);

  cpos=xf2->translation.getValue();
  cpos.getValue(x, y, z);
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

int numchannels = 1;

static SbBool fill_callback(void *buffer, int length, void *userdata)
{
  printf("<");
    short int *ibuffer = (short int *)buffer;
  static double freq = 600.0;
  static double ffreq = 600.0;
  static int counter = 0;
  //ffreq +=10.0;
  // freq +=10.0;
  int a=100;
  int h=800;
  int d=100;
  int p=44100.0/(ffreq/110.0);

  // det klikker av og til fordi vi forandrer p og mod'er med denne

  int c;
  static bool flip = false;
  double value;
  for (int i=0; i< length; i++)
  {
    c = (counter+i)%p;
    if (c==0)
      flip = !flip;
    value = sin( ((float)(counter+i))/44100.0*2*3.14159265358979323846264383*freq*2);
    value=0;

    value = 32000.0*sin( ((float)(counter+i))/44100.0*2*3.14159265358979323846264383*(freq) + 2*value);


    if (c<=a)
      value= (double)c/(double)a * value;
    else if (c<=a+h)
      value= value;
    else if (c<=a+h+d)
      value= (1.0-(double)(c-(a+h))/(double)d) * value;
    else
      value = 0.0;

    value = 32000.0*sin( (float)(counter + i)/44100.0*2*3.14159*100.0 ); // debug !!!
    if (numchannels==1)
      ibuffer[i] = value;
    else
    {
      if (flip)
      {
        ibuffer[i*2] = 0;
        ibuffer[i*2+1] = value;
      }
      else
      {
        ibuffer[i*2] = value;
        ibuffer[i*2+1] = 0;
      }
    }

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

short int *createMagSound(int length, int pick_index)
{
  float freq = magdata[pick_index*3+2];
//  int bliplength = 44100/8;
  int bliplength = 44100/(freq/100);
  int blippos = 0;

  short int *mybuf = new short int[length*2];
  double value;
  int a = bliplength*0.02;
  int b = bliplength*0.05;
  int c = bliplength*0.05;

  int fade = length*0.5;
  for (int i=0; i<length; i++)
  {

    value = sin( ((float)i)/44100.0*2*3.14159*freq*2);
    value = 32000.0*sin( ((float)i)/44100.0*2*3.14159*(freq) + 2*value);

    blippos = i%bliplength;
    if (blippos<a)
      value=(double)blippos/(double)a*value;
    else if (blippos<a+b)
      value=1.0*value;
    else if (blippos<a+b+c)
      value=(1.0-(double)(blippos-(a+b))/(double)c)*value;
    else 
      value=0;

    if (i>fade)
      value=(1.0-(double)(i-fade)/(double)(length-fade)) * value;

    mybuf[i*2]=value;
    mybuf[i*2+1]=value;
  }
  return mybuf; // remember to delete[]
}


void
main(
  int argc,
  char ** argv )
{

  int i;
/*  for (i=0; i<NUM_MAGDATA; i++)
  {
    printf("%3d : %6.2f, : %6.2f\n", i, magdata[i*3+2], log(magdata[i*3+2]));
  }

  getch();
*/

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
  buffernode->setAsyncMode(FALSE);
  buffernode->setBufferInfo(4410, 3);
//  buffernode->setBufferInfo(44100/100*3, 10); 
//  buffernode->setBufferInfo((44100*30)/1000, 8); 
  // ^^ 20010809 - if buffer*num == 1 sec, we will have jitter/loops at the beginning
  // or "very round values) (0.1 sec, 0.2 sec, 
  // I have no idea why this happens !!!
//  buffernode->url.setValue("lyd1.wav");
  
//  buffernode->url.setValue("allways.ogg");

  //  buffernode->loop.setValue(TRUE);
//  buffernode->loop.setValue(FALSE); 
  buffernode->startTime.setValue(SbTime::getTimeOfDay() + SbTime(2));
  buffernode->stopTime.setValue(SbTime::getTimeOfDay() + SbTime(100));

  sourcenode->source.setValue(buffernode);
//  root->addChild(sep);

//  openoggfile("allways.ogg");

    buffernode->setUserCallback(fill_callback, NULL);
    buffernode->setSampleFormat(numchannels);
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
//  clip2->url.setValue("lyd1.wav");

  int pick_index = 12;

  short int *mybuf = createMagSound(44100, pick_index);
  /*
  float freq = magdata[pick_index*3+2];

  int mybuf[44100*2];
  for (i=0; i<44100; i++)
  {
    mybuf[i*2]=(int)32000.0*sin( (float)(i)/44100.0*2*3.14159*freq );
    mybuf[i*2+1]=(int)32000.0*sin( (float)(i)/44100.0*2*3.14159*freq );
  }
  */
  clip2->setBuffer(mybuf, 44100, 2, 16, 44100);
//  clip2->url.setValue("lyd2.wav");
  delete[] mybuf;
  
  clip2->loop.setValue(TRUE);
//  clip2->loop.setValue(FALSE);
  clip2->startTime.setValue(SbTime::getTimeOfDay() + SbTime(0.0));
  clip2->stopTime.setValue(SbTime::getTimeOfDay() + SbTime(60));
  source2->source.setValue(clip2);
  source2->intensity.setValue(0.5f);
  root->addChild(sep2);
  sounds.push(source2);


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
