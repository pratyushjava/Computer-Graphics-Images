
/****
@Author = Pratyush Singh
* This is a first program of a project, the output of this program will be used as a input to the second program(CompositeView).
* The purpose of this program is to read the given image and Display its associated color image on the screen and then read back the associated color image from the screen and write it on the disc.
* steps to run
* 1. give filename to read as commandline argument, for this project it has to be dhouse.png because i have done green screening.
* 2. press r or R to re read. (will prompt to enter the name of input file)
* 3. press w or W to write. (will prompt to enter the output file name)
* 4. press q or Q or esc to exit.
****/

#include<OpenImageIO/imageio.h>

#include<cstdlib>
#include<iostream>

#ifdef __APPLE__
#	pragma clang diagnostic ignored "-Wdeprecated-declarations"
#	include<GLUT/glut.h>
#else
#	include<GL/glut.h>
#endif

using namespace std;
OIIO_NAMESPACE_USING

#define WIDTH 800
#define HEIGHT 800



#define maximum(x, y, z) ((x) > (y)? ((x) > (z)? (x) : (z)) : ((y) > (z)? (y) : (z)))  // returns maximum of 3 values
#define minimum(x, y, z) ((x) < (y)? ((x) < (z)? (x) : (z)) : ((y) < (z)? (y) : (z))) // returns minimum of 3 values
	
int mWidth, mHeight, mChannels; // These variables will be filled when reading image, they will have values for width, height and channel of the image.
unsigned char *gPixels; // This is a global pixel


void writeImgToDisc()
{
	//get window width and height, this will be used to read the pixels from the screen, these are input for glReadPixels which read pixels from opengl framebuffer.
	const int w = glutGet(GLUT_WINDOW_WIDTH);
	const int h = glutGet(GLUT_WINDOW_HEIGHT);

	string outPutName; // name of the file which will be inputed by the user and same will be created to store the associated image.

	cout << "Enter the name of associated image";
	cin >> outPutName;
	
	// create handler for imageoutput, this is required to open and write image
	ImageOutput *outF = ImageOutput::create(outPutName);
	if(!outF)
	{
		cout << "Error in creating a file, please try with different name!!";
		return;
	}
	
	//open the file
	ImageSpec spec(w,h,4,TypeDesc::UINT8);
	if(!outF->open(outPutName, spec)){
		cout << "Can not open the file, you entered."<< endl;
		return;
	}
	cout<<"filling pixmap"<<endl;
	
	unsigned char *pixmap = NULL;// image will be stored in pixmap after reading from opengl framebuffer.
	pixmap = new unsigned char[w* h * 4]; // allocating the memmory for pixmap according to widh, height of window, channel will be 4
	
	glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE, pixmap);// read pixels from opengl framebuffer and store in pixmap
	
	// Calculating one Scanline length, this is required in order to write a image correct
	// As opengl starts up down so if we need to write it we need to write in reverse order otherwise reverse image will be written, -scanlinesize enshure that image is writen straight.
	int scanlinesize = w * 4 * sizeof(pixmap[0]); // 1 scan line will be equal to width * channel * size of one memory chunk
	
	if (pixmap)
	{
		outF->write_image(TypeDesc::UINT8, (char *)pixmap + (h - 1)*scanlinesize,
			AutoStride, -scanlinesize, AutoStride);
	}
	cout<<"end writing"<<endl;
	//close and delete the imageoutput handler.
	outF->close();
	delete outF;
	return;
}

/* 
  Input RGB color primary values: r, g, and b on scale 0 - 255 
  Output HSV colors: h on scale 0-360, s and v on scale 0-1 
*/

void RGBtoHSV(int r, int g, int b, double &h, double &s, double &v){

  double red, green, blue; 
  double max, min, delta;

  red = r / 255.0; green = g / 255.0; blue = b / 255.0;  /* r, g, b to 0 - 1 scale */

  max = maximum(red, green, blue); 
  min = minimum(red, green, blue);

  v = max;        /* value is maximum of r, g, b */

  if(max == 0){    /* saturation and hue 0 if value is 0 */ 
     s = 0; 
     h = 0; 
  } 
  else{ 
    s = (max - min) / max;           /* saturation is color purity on scale 0 - 1 */

    delta = max - min; 
    if(delta == 0)                         /* hue doesn't matter if saturation is 0 */ 
       h = 0; 
    else{ 
       if(red == max)                    /* otherwise, determine hue on scale 0 - 360 */ 
          h = (green - blue) / delta; 
      else if(green == max) 
          h = 2.0 + (blue - red) / delta; 
      else /* (green == max) */ 
          h = 4.0 + (red - green) / delta; 

      h = h * 60.0;                       /* change hue to degrees */
      if(h < 0) 
          h = h + 360.0;                /* negative hue rotated to equivalent positive around color wheel */
    } 
  } 
}


void readImgFromDisc(string filename)
{
	
	if(filename.empty()){ // if no name provided through command line argument, ask the user to input the file name
		cout<<"Please enter the picture name"<<endl;
		cin>>filename;
	}
	
	ImageInput *in = ImageInput::open(filename);// create handel of input imageinput this will be required to read a file
	if(!in){
		cout << "Error in opening a file, please try with different name!!";
		return;
	}
		
	const ImageSpec &spec = in->spec();// to get image specification-> width, height, channel.
	mWidth = spec.width;
	mHeight = spec.height;
	mChannels = spec.nchannels;

	//allocate space for input image  and global associated image (gPixels)
	unsigned char *image;
	image = new unsigned char[mWidth*mHeight* mChannels];
	gPixels = new unsigned char[mWidth * mHeight * 4];
  
	// Get the Size of scanline, required in order to read into buffer in reverse order.
	int scanlinesize = mWidth * mChannels * sizeof(image[0]);
	// read the image and fill the pixmap in reverse order.
	// This pixmap is already filled in a reverse order so we do not need to inverse it. when opengl will display, it will show in correct form.
	// -scanlinesize enshure that the image is read in correct order that is reverse from opengl perspective, and will be seen straight by the user.
	in->read_image(TypeDesc::UINT8,
		(char *)image + (mHeight - 1)*scanlinesize,
		AutoStride, -scanlinesize, AutoStride);
		
	in->close();
	ImageInput::destroy(in);
		
	// fill pixels alpha value
	// fill associated global pixmap, in order to do green screning
	double h = 0, s = 0, v = 0, alpha =0;
	for(int i=0;i<mHeight;i++){
		for(int j=0;j<mWidth;j++){
			//assign the same rgb for global pixel as it is in the image, associated image calculation are done further.
			gPixels[i*mWidth*4+j*4+0]=image[i*mWidth*3+j*3+0];
			gPixels[i*mWidth*4+j*4+1]=image[i*mWidth*3+j*3+1];
			gPixels[i*mWidth*4+j*4+2]=image[i*mWidth*3+j*3+2];
			
			// here h,s,v values of corresponding R,G,B is get so that alpha value of pixel can be decided.
			RGBtoHSV(image[i*mWidth*3+j*3+0],image[i*mWidth*3+j*3+1],image[i*mWidth*3+j*3+2],h,s,v);
			// By further experiment, i have found that hue value for green screening is between 70 and 150. 
			// i am also checking for saturation value because i found some dots on the left side in hairs, checking saturation value solved the problem.
			if(h>=110 && h<=160 && s>0.1)
			{
				/* when the hue and saturation value found in this range, 
				 * that means it is green color so multiplying with zero will remove that part from the image(green screening)
				 */
				gPixels[i*mWidth*4+j*4+0]*=0;
				gPixels[i*mWidth*4+j*4+1]*=0;
				gPixels[i*mWidth*4+j*4+2]*=0;		
				gPixels[i*mWidth*4+j*4+3]=0;		
			}
			else
			{
				/* if not found within the range, then multiply 1 to each pixel,
				 * actually multiplying for 1 is not required but still i am doing for sake of readibility and understanding.
				 * setting alpha value to 255 so that it will be usefull at the time of compositing.
				 */
				gPixels[i*mWidth*4+j*4+0]*=1;
				gPixels[i*mWidth*4+j*4+1]*=1;
				gPixels[i*mWidth*4+j*4+2]*=1;
				gPixels[i*mWidth*4+j*4+3]=255;
			}
 }
}
	glutReshapeWindow(mWidth,mHeight);
	glutPostRedisplay();
	glFlush();
}

//Callback for key press event
void handleKey(unsigned char key, int x, int y){
	switch(key){
		case 'r':
		case 'R':
			readImgFromDisc("");	//'r' to read image.
			break;
		case 'w':
		case 'W':
			writeImgToDisc();	//'w' to write image.
			break;
		case 'q':
		case 'Q':
		case 27:
			exit(0);		//'q' or ESC to quit
		default:
			return;
	}
}

void handleReshape(int w, int h){

	//make the full viewport, this will keep the image at center
	glViewport(0,0,w,h);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0,w,0,h);
	glMatrixMode(GL_MODELVIEW);
	glFlush();
}

void drawImage(){
	// To keep the window black, when no image is inputed through command line argument
	glClearColor(0,0,0,1); 
	glClear(GL_COLOR_BUFFER_BIT);

	// Get currend window size, this will be required to position the image if size of the window is changed
	const int w = glutGet(GLUT_WINDOW_WIDTH);
	const int h = glutGet(GLUT_WINDOW_HEIGHT);

	/* The total (length of the current window - length of image) will be a total size of the image exceded. 
	 * to show image in center we need to devide the extra length on both side, so devide by 2 
	 */
	int xstart = (w-mWidth)/2;
	int ystart = (h-mHeight)/2;

	//Handling the case when window is shrinked
	if(w < mWidth || h < mHeight){

		// when we are shrinking in x direction-> that is along the width.
		if((float)w/mWidth <(float)h/mHeight){
			glRasterPos2i(0,0); // Drawing Coordinates
			glPixelZoom((float)w/mWidth,(float)w/mWidth); // the image will zoom in, as the ratio will be smaller than 1.
		}
		else{ // when we are shrinking in y direction-> that is along the height.
			glRasterPos2i(0,0);
			glPixelZoom((float)h/mHeight,(float)h/mHeight);
		}

	}

	//making the image to be in the middle
	else{
		glRasterPos2i(xstart,ystart);
		glViewport(xstart, ystart, w - xstart, h - ystart); // when straetching the image, defining new view port
	}
	glDrawPixels(mWidth,mHeight,GL_RGBA,GL_UNSIGNED_BYTE,&gPixels[0]); // drawing the image on screen

	glFlush();
}

int main(int argc, char* argv[]){

	glutInit(&argc,argv);

	// Initialising glut window.
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
	glutInitWindowSize(WIDTH,HEIGHT);
	glutCreateWindow("Associated Image");

	// registering callback functions.
	glutDisplayFunc(drawImage);
	glutKeyboardFunc(handleKey);
	glutReshapeFunc(handleReshape);

	// Call to open the image given by command line argument.
	if(argv[1]){
		string file = argv[1];
		readImgFromDisc(file);
	}

	//opengl mainloop
	glutMainLoop();

}