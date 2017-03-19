
/****
@Author = Pratyush Singh
* This is a second program of a project, the input to this program will be the output of first program(alphamask).
* The purpose of this program is to read the given 2 image and Display its composite image on the screen and then read back the composite image image from the screen and write it on the disc.
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

	
int mWidth, mHeight, mChannels; // These variables will be filled when reading image, they will have values for width, height and channel of the image.
unsigned char *gPixels; // This is a global pixel


void writeImgToDisc()
{
	//get window width and height, this will be used to read the pixels from the screen, these are input for glReadPixels which read pixels from opengl framebuffer.
	const int w = glutGet(GLUT_WINDOW_WIDTH);
	const int h = glutGet(GLUT_WINDOW_HEIGHT);

	string outPutName; // name of the file which will be inputed by the user and same will be created to store the associated image.

	cout << "Enter the name of composite image";
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

// This function does A over B calculations
void getCompositeColor(int fg, int  bg, int alphaF, int &compositeChannel)
{
	//we know that composition Ch = CA + (1-alphaA)Cb
	compositeChannel = static_cast<int>(((float)fg/255 + (float)(1 - (float)alphaF/255) * (float)bg/255)*255);
}

// this function does A over B operation (composition)
void compositeForeGroundOverBackGround(unsigned char *foreground, unsigned char *background, int bgWidth, int bgHeight, int bgChannels)
{
	/*
	 * we know that composition Ch = CA + (1-alphaA)Cb
	 * it means for those pixels for which there is alpha value not equal to zero in foreground, we set pixels to foreground image.
	 * and for pixels whose alpha value is zero in foreground, we set pixels of background.
	 */
		for(int i=0;i<mHeight;i++){
			for(int j=0;j<mWidth;j++){
					int compositeChannel = 0;
					getCompositeColor(static_cast<int>(foreground[i*mWidth*4+j*4+0]),  static_cast<int>(background[i*bgWidth*bgChannels+j*bgChannels+0]),  static_cast<int>(foreground[i*mWidth*4+j*4+3]), compositeChannel);
				  gPixels[i*mWidth*4+j*4+0] = compositeChannel;
				  compositeChannel = 0;
					getCompositeColor( static_cast<int>(foreground[i*mWidth*4+j*4+1]),  static_cast<int>(background[i*bgWidth*bgChannels+j*bgChannels+1]),  static_cast<int>(foreground[i*mWidth*4+j*4+3]), compositeChannel);
				  gPixels[i*mWidth*4+j*4+1] = compositeChannel;
				  compositeChannel = 0;
					getCompositeColor( static_cast<int>(foreground[i*mWidth*4+j*4+2]),  static_cast<int>(background[i*bgWidth*bgChannels+j*bgChannels+2]),  static_cast<int>(foreground[i*mWidth*4+j*4+3]), compositeChannel);
				  gPixels[i*mWidth*4+j*4+2] = compositeChannel;
				  compositeChannel = 0;
					gPixels[i*mWidth*4+j*4+3] = 255;
			}
		}
return;
}

void readImgFromDisc(string filename , string backGround)
{
	
	// Reading the associate alphamask image.
	if(filename.empty()){ // if no name provided through command line argument, ask the user to input the associate alphamask image name
		cout<<"Please enter the associate alphamask image name"<<endl;
		cin>>filename;
	}
	
	ImageInput *in = ImageInput::open(filename);// create handel of input imageinput this will be required to read a file
	if(!in){
		cout << "Error in opening a associate alphamask image, please try again!!";
		return;
	}
		
	const ImageSpec &spec = in->spec();// to get image specification-> width, height, channel.
	int maskWidth = spec.width;
	int maskHeight = spec.height;
	int maskChannels = spec.nchannels;

	//allocate space for input image
	unsigned char *image = NULL;
	image = new unsigned char[maskWidth * maskHeight * maskChannels];
  
	// Get the Size of scanline, required in order to read into buffer in reverse order.
	int scanlinesize = maskWidth * maskChannels * sizeof(image[0]);
	
	// read the image and fill the pixmap in reverse order.
	// This pixmap is already filled in a reverse order so we do not need to inverse it. when opengl will display, it will show in correct form.
	// -scanlinesize enshure that the image is read in correct order that is reverse from opengl perspective, and will be seen straight by the user.
	in->read_image(TypeDesc::UINT8,
		(char *)image + (maskHeight - 1)*scanlinesize,
		AutoStride, -scanlinesize, AutoStride);
		
	in->close();
	ImageInput::destroy(in);
		
	//****************************************************
	
	// Reading the background image.
	if(backGround.empty()){ // if no name provided through command line argument, ask the user to input the background image name
		cout<<"Please enter the background image name"<<endl;
		cin>>backGround;
	}
		
	in = ImageInput::open(backGround);// create handel of input imageinput this will be required to read a file
	if(!in){
		cout << "Error in opening a background image, please try again!!";
		return;
	}
	
		//read image specification
	const ImageSpec &bgspec = in->spec();
	int bgWidth = bgspec.width;
	int bgHeight = bgspec.height;
	int bgChannels = bgspec.nchannels;
	
	//allocate space for background image
	unsigned char *bgImage = NULL;
	bgImage = new unsigned char[bgWidth * bgHeight * bgChannels];
		
	// Get the Size of scanline, required in order to read into buffer in reverse order.
	scanlinesize = bgWidth * bgChannels * sizeof(bgImage[0]);
	
	// read the image and fill the pixmap in reverse order.
	// This pixmap is already filled in a reverse order so we do not need to inverse it. when opengl will display, it will show in correct form.
	// -scanlinesize enshure that the image is read in correct order that is reverse from opengl perspective, and will be seen straight by the user.
	in->read_image(TypeDesc::UINT8,
		(char *)bgImage + (bgHeight - 1)*scanlinesize,
		AutoStride, -scanlinesize, AutoStride);
		
	in->close();
	ImageInput::destroy(in);

	mWidth = maskWidth;
	mHeight = maskHeight;
	
	// allocating the memory for composite image, size will be in accordance to the greater image.
	gPixels = new unsigned char[mWidth * mHeight * 4];
	
	// call to composite operation function. This will set the global pixemap (gPixels)
	compositeForeGroundOverBackGround(image,bgImage, bgWidth, bgHeight, bgChannels);
		
	glutReshapeWindow(mWidth,mHeight);
	glutPostRedisplay();
	glFlush();
}

//Callback for key press event
void handleKey(unsigned char key, int x, int y){
	switch(key){
		case 'r':
		case 'R':
			readImgFromDisc("","");	//'r' to read image.
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
	if(argv[1] && argv[2]){
		string associatedImage = argv[1];
		string backgroundImage = argv[2];
		readImgFromDisc(associatedImage, backgroundImage);
	}

	//opengl mainloop
	glutMainLoop();

}