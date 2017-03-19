
/****
@Author = Pratyush Singh
* The purpose of this program is to read the given image and then Display its warped image on the screen and then read back the warped image from the screen and write it on the disc.
* steps to run
* 1. build the project through make file.
* 2. Then use command inimage.png [outimage.png]optional. example (./warp dhouse.png del1.png).
* 3. select the warp image window and then press W or w to write the image (if not provided the optional [output file name]).
****/

#include<OpenImageIO/imageio.h>

#include<cstdlib>
#include<iostream>
#include<math.h>
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

//allocate space for input image
unsigned char *image = NULL; // pixmap where the original image will be stored.
int mWidth, mHeight, mChannels; // These variables will be filled when reading image, they will have values for width, height and channel of the image.
unsigned char *gPixels = NULL; // This is a global pixel
unsigned char *imageRGBA = NULL;

// these are global variables which are used in multiple function. 
int ImageWidth = 0;
int ImageHeight = 0;
int ImageChannels = 0;

// write the warped image to the output file provided in the command line argument.
void writeOutput(string outPutName)
{

	
	// Reading the image.
	if(outPutName.empty()){ // if no name provided through command line argument, ask the user to input the image name
		cout<<"Please enter the  output image name"<<endl;
		cin>>outPutName;
	}
	
	// create handler for imageoutput, this is required to open and write image
	ImageOutput *outF = ImageOutput::create(outPutName);
	if(!outF)
	{
		cout << "Error in creating a file, please try with different name!!";
		return;
	}
	
	//open the file
	ImageSpec spec(mWidth,mHeight,4,TypeDesc::UINT8);
	if(!outF->open(outPutName, spec)){
		cout << "Can not open the file, you entered."<< endl;
		return;
	}
	
	// Calculating one Scanline length, this is required in order to write a image correct
	// As opengl starts up down so if we need to write it we need to write in reverse order otherwise reverse image will be written, -scanlinesize enshure that image is writen straight.
	int scanlinesize = mWidth * 4 * sizeof(gPixels[0]); // 1 scan line will be equal to width * channel * size of one memory chunk
	
	if (gPixels)
	{
		outF->write_image(TypeDesc::UINT8, (char *)gPixels + (mHeight - 1)*scanlinesize,
			AutoStride, -scanlinesize, AutoStride);
	}
	
	//close and delete the imageoutput handler.
	outF->close();
	delete outF;
	return;
	return;
}

// function to write the image to disc when the user press w.
void writeImgToDisc(string outPutName)
{
	//get window width and height, this will be used to read the pixels from the screen, these are input for glReadPixels which read pixels from opengl framebuffer.
	const int w = glutGet(GLUT_WINDOW_WIDTH);
	const int h = glutGet(GLUT_WINDOW_HEIGHT);
	
	// Reading the image.
	if(outPutName.empty()){ // if no name provided through command line argument, ask the user to input the image name
		cout<<"Please enter the  output image name"<<endl;
		cin>>outPutName;
	}
	
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
	
	//close and delete the imageoutput handler.
	outF->close();
	delete outF;
	return;
}


void do_Warp()
{
	// setting the input image size same as output image.
	int outputWidth = ImageWidth;
	int outputHight = ImageHeight;
	mWidth  = outputWidth;  // setting width and height for glut.
	mHeight = outputHight;

	// allocating memory for warp image
	gPixels = new unsigned char[outputWidth * outputHight * 4];
	
	// Using inverse map, so iterating through the output image.
	for(int i=0;i<outputHight;i++){
		for(int j=0;j<outputWidth;j++){
			
			// normalising the coordinates between (0-1).
			float normalY = (i+0.5) / outputHight;
			float normalX = (j+0.5) / outputWidth;
			
			// Using warp function (x = u ^ 4) and (v = (2 / 3.14) sin ^-1 (sqrt(v)))
			// inverse function for this warp function is (u = x ^ 0.25) and ( v = (sin (3.14 / 2 * y)) ^ 2)
			float normalU = pow(normalX, 0.25);
			float normalv = pow(sin(3.14 * 0.5 * normalY), 2);
			
			//converting coordinates to input indexes.
			float u = normalU * ImageWidth;
			float v = normalv * ImageHeight;
			int inputU = static_cast <int> (floor(u));
			int inputV = static_cast <int> (floor(v));
			
			// skip those which falls outside the boundary of input image
			if(inputU < 0 || inputU > ImageWidth || inputV < 0 || inputV > ImageHeight){
				gPixels[i*outputWidth*4+j*4+0]    =  0; // red
				gPixels[i*outputWidth*4+j*4+1]    =  0; // green
				gPixels[i*outputWidth*4+j*4+2]    =  0; // blue
				gPixels[i*outputWidth*4+j*4+3] 		=  255;
				continue;
			}

		// Filling the desired input image pixels into output image pixels.
		gPixels[i*outputWidth*4+j*4+0]    =  image[inputV*ImageWidth*ImageChannels+inputU*ImageChannels+0]; // red
		gPixels[i*outputWidth*4+j*4+1]    =  image[inputV*ImageWidth*ImageChannels+inputU*ImageChannels+1]; // green
		gPixels[i*outputWidth*4+j*4+2]    =  image[inputV*ImageWidth*ImageChannels+inputU*ImageChannels+2]; // blue
		gPixels[i*outputWidth*4+j*4+3] 		=  255;
		
	}
}

}


// read the image
void readImgFromDisc(string filename)
{
	
	// Reading the image.
	if(filename.empty()){ // if no name provided through command line argument, ask the user to input the image name
		cout<<"Please enter the  image name"<<endl;
		cin>>filename;
	}
	
	ImageInput *in = ImageInput::open(filename);// create handel of input imageinput this will be required to read a file
	if(!in){
		cout << "Error in opening a image, please try again!!";
		return;
	}
		
	const ImageSpec &spec = in->spec();// to get image specification-> width, height, channel.
	ImageWidth = spec.width;
	ImageHeight = spec.height;
	ImageChannels = spec.nchannels;
	cout<<" image "<< ImageChannels<<endl;
	// allocate memory for reading the image.
	image = new unsigned char[ImageWidth * ImageHeight * ImageChannels];
  
	// Get the Size of scanline, required in order to read into buffer in reverse order.
	int scanlinesize = ImageWidth * ImageChannels * sizeof(image[0]);
	
	// read the image and fill the pixmap in reverse order.
	// This pixmap is already filled in a reverse order so we do not need to inverse it. when opengl will display, it will show in correct form.
	// -scanlinesize enshure that the image is read in correct order that is reverse from opengl perspective, and will be seen straight by the user.
	in->read_image(TypeDesc::UINT8,
		(char *)image + (ImageHeight - 1)*scanlinesize,
		AutoStride, -scanlinesize, AutoStride);
		
	// close input file
	in->close();
	ImageInput::destroy(in);


	// Creating RGBA input image to display
	imageRGBA = new unsigned char[ImageWidth * ImageHeight * 4];
	for(int i=0;i<ImageHeight;i++){
		for(int j=0;j<ImageWidth;j++){
			
		imageRGBA[i*ImageWidth*4+j*4+0]    =  image[i*ImageWidth*ImageChannels+j*ImageChannels+0]; // red
		imageRGBA[i*ImageWidth*4+j*4+1]    =  image[i*ImageWidth*ImageChannels+j*ImageChannels+1]; // green
		imageRGBA[i*ImageWidth*4+j*4+2]    =  image[i*ImageWidth*ImageChannels+j*ImageChannels+2]; // blue
		imageRGBA[i*ImageWidth*4+j*4+3] 	 =  255;
		
	}
}	
	

	// call to the warp function
	do_Warp();
	
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
			writeImgToDisc("");	//'w' to write image.
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

void drawOutputImage(){
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


void drawInputImage(){
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

	glDrawPixels(ImageWidth,ImageHeight,GL_RGBA,GL_UNSIGNED_BYTE,&imageRGBA[0]); // drawing the image on screen


	glFlush();
}


int main(int argc, char* argv[]){

	glutInit(&argc,argv);

	// Initialising glut window.
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
	glutInitWindowSize(WIDTH,HEIGHT);
	glutCreateWindow("warp Image");

	// registering callback functions.
	glutDisplayFunc(drawOutputImage);
	glutKeyboardFunc(handleKey);
	glutReshapeFunc(handleReshape);


	//parsing the command line argument
	string image = "", output = "";
	
	if(argc < 2){
		cout<<"please provide input image file name "<<endl;
		exit(0);
	}
		
	if(argv[1]){
		image = argv[1];
	}
	
	if(argv[2]){
		output = argv[2];
	}
		
		
		
	if(!image.empty()){
		readImgFromDisc(image);
	}
	
	if(!output.empty()){
		writeOutput(output);
	}
	
	
	
		// Initialising glut window.
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
	glutInitWindowSize(mWidth,mHeight);
	glutCreateWindow("Input Image");

	// registering callback functions.mWidth,mHeight,
	glutDisplayFunc(drawInputImage);
	glutKeyboardFunc(handleKey);
	glutReshapeFunc(handleReshape);

	//opengl mainloop
	glutMainLoop();

}