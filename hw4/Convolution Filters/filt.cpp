
/****
@Author = Pratyush Singh
* The purpose of this program is to read the given image and the kernel for that image and Display its convolve image on the screen and then read back the convolve image from the screen and write it on the disc.
* steps to run
* 1. build the project through make file.
* 2. Then use command -f [input file name] [filter name] [output file name]
* 3. to test the gabor kernel give -g [theta] [sigma] [period] -f [input file name]
* 4. for gabor kernel after step 3, select the output image window and then press W or w to write the image.
* 5. for command line option -f is used to input the files name and -g is used to input [theta] [sigma] [period].
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
unsigned char *gPixels; // This is a global pixel
int theta =0, sigma = 0, period = 0; // for gabor filter
// these are global variables which are used in multiple function. 
int ImageWidth = 0;
int ImageHeight = 0;
int ImageChannels = 0;
int kernelSize = 0;
float negativekernelScale = 0.0, positivekernelScale = 0.0, kernelWeight = 0.0;
float** kernelArr = NULL;


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


// this function will read the kernel from the file
void readKernel(string kernel){
	
	if(kernel.empty()){ // if no name provided through command line argument, ask the user to input the kernal name
		cout<<"Please enter the kernel name"<<endl;
		cin>>kernel;
	}
	
	// open the file
	FILE *fp;
	if((fp = fopen(kernel.c_str(), "r")) == NULL){
		cout<<"can not open filter"<<endl;
		return;
	}
	
	// get the kernel Size from the file
	fscanf(fp, "%d %f", &kernelSize, &positivekernelScale);
	cout<<" kernelSize "<<kernelSize<<endl;
	
	
	// populate the kernel weight from the file
  kernelArr = new float*[kernelSize];
  for(int i = 0; i < kernelSize; i++){
  	kernelArr[i] = new float[kernelSize];
  	for(int j = 0; j < kernelSize; j++){
  		fscanf(fp, "%f", &kernelWeight);
  		kernelArr[i][j] = kernelWeight;
  	}
  }
   
  // calculate the scale values.
  positivekernelScale = 0.0;
  for(int i = 0; i < kernelSize; i++){
  	for(int j = 0; j < kernelSize; j++){
  		if(kernelArr[i][j] < 0){
  			negativekernelScale +=kernelArr[i][j];
  			}
  			else{
  				positivekernelScale +=kernelArr[i][j]; 
  			}
  	}	
  }
    
	return;
}

void calculateGaborKernel()
{
	/*
	gabor weight = exp(- ((xprime * xprime)+ (yprimme * yprime)/ 2* sigma * sigma) * cos (2 * 3.14 * xprime/ period)
	*/
	int distanceCenter = 2 * sigma; // distance from the center
	cout<<" distanceCenter "<<distanceCenter<<endl; // 
	kernelSize = distanceCenter*2 +1; // 0 will be on center, and 2 * sigma on both side
	kernelArr = new float*[kernelSize];
	for(int i = 0; i< distanceCenter*2 +1 ; i++){
		kernelArr[i] = new float[kernelSize];
		for(int j = 0; j< distanceCenter*2 +1 ; j++){
			int x = j - distanceCenter; // finding the value of X
			int y = i - distanceCenter; // finding the value of y
			cout<<" x "<<x<<" y "<<y<<endl;
			float xPrime = (x * cos(theta)) + (y * sin(theta)); // xPrime = x cos(theta) + y sin(theta)
			float yPrime = (-x * sin(theta)) + (y * cos(theta));// yPrime = -x sin(theta) + y cos(theta)
			cout<<" xPrime "<<xPrime<<" yPrime "<<yPrime<<endl;
			float gaborWeight = exp(- (((xPrime * xPrime) + (yPrime * yPrime))/ (2 * sigma * sigma))) * cos((2 * xPrime * 3.14)/period); // calculating final weight
			cout<<" gaborWeight "<<gaborWeight<<endl;
			kernelArr[i][j] = gaborWeight; // filling the final kernel with calculated gabor weight.
		}
	}
	
	  for(int i = 0; i < kernelSize; i++){
  	for(int j = 0; j < kernelSize; j++){
  		cout<<"arr["<<i<<"]["<<j<<"] = "<<kernelArr[i][j]<<endl;
  	}
  }
  
  // calculating the kernel scale size.
  positivekernelScale = 0.0;
  negativekernelScale = 0.0;
  for(int i = 0; i < kernelSize; i++){
  	for(int j = 0; j < kernelSize; j++){
  		if(kernelArr[i][j] < 0){
  			negativekernelScale +=kernelArr[i][j];
  			}
  			else{
  				positivekernelScale +=kernelArr[i][j]; 
  			}
  	}
  	
  }
  cout<<"gabour kernel done"<<endl;
  
  return;
}

void convolove(){
	
	/*
	 * seperating r, g , b values from pixmap and creating an different planes for R, G ,B.
	 */
	 
	float RedChannel[ImageWidth][ImageHeight]; // array to store red channel value
	float GreenChannel[ImageWidth][ImageHeight]; // array to store green channel value
	float BlueChannel[ImageWidth][ImageHeight]; //array to store blue channel value
	
	// seperating red green and blue channel value from the image
	for(int i=0;i<ImageHeight;i++){
	for(int j=0;j<ImageWidth;j++){
		
		RedChannel[i][j]    =  (float)image[i*ImageWidth*ImageChannels+j*ImageChannels+0]/255; // red
		GreenChannel[i][j]  =  (float)image[i*ImageWidth*ImageChannels+j*ImageChannels+1]/255; // green
		BlueChannel[i][j]   =  (float)image[i*ImageWidth*ImageChannels+j*ImageChannels+2]/255; // blue
	}
}

    gPixels = new unsigned char[ImageWidth * ImageHeight * 4]; // allocating memory for global pixmap
    mWidth  = ImageWidth;  // setting width and height for glut.
		mHeight = ImageHeight;
		int rowSkip, columnSkip;
		// this is calculated to skip edges of images and to map the final result with the center of kernel.
		cout<<"kernelSize "<<kernelSize<<endl;
    int convolveRow = kernelSize /2; // how much to skip the margin
    int convolveCol = kernelSize /2;
    for(int ic = 0; ic < ImageHeight - convolveRow ;  ic++){ // skip the margin 
    	for(int ir = 0; ir < ImageWidth - convolveCol; ir++){ // skip the margin 
    		double sumR = 0.0, sumG = 0.0, sumB = 0.0; // used for suming the product of kernel and image
    		int  rch = 0, gch = 0, bch = 0; // used for final value after scaling
    		
    		// calculate the sum of product of kernal and image
    		for(int kc = 0 ; kc < kernelSize ; kc++){
    			rowSkip = kernelSize - kc - 1 - convolveRow; // for mapping kernel row with image row
    			for(int kr = 0 ; kr < kernelSize; kr++){
    				columnSkip = kernelSize - kr - 1 - convolveRow; // for mapping kernel row with image row
    				sumR += kernelArr[kc][kr] * RedChannel[ic+rowSkip][ir+columnSkip];   // calcualting sum of product for R
    				sumG += kernelArr[kc][kr] * GreenChannel[ic+rowSkip][ir+columnSkip]; // calcualting sum of product for G
    				sumB += kernelArr[kc][kr] * BlueChannel[ic+rowSkip][ir+columnSkip]; // calcualting sum of product for B
    			}
    		}
    		
    		// scaling
    		rch = 255 * (float)abs(sumR);
    		gch = 255 * (float)abs(sumG);
    		bch = 255 * (float)abs(sumB);
    		
    		
    		// To fitt the value between the range 0 to 255
    		if(rch > 255) rch = 255;
    		if(rch < 0) rch =0;
    		if(gch > 255) gch = 255;
    		if(gch < 0) gch =0;
    		if(bch > 255) bch = 255;
    		if(bch < 0) bch =0;
    			
    			// storing final values in the pixel.
    		gPixels[(ic+convolveRow)*ImageWidth*4+(ir+convolveCol)*4+0] = rch;

    		gPixels[(ic+convolveRow)*ImageWidth*4+(ir+convolveCol)*4+1] = gch;
    		
    		gPixels[(ic+convolveRow)*ImageWidth*4+(ir+convolveCol)*4+2] = bch;
    		
    		gPixels[(ic+convolveRow)*ImageWidth*4+(ir+convolveCol)*4+3] = 255;
    		
    		
    	}
   }
}

// read the image
void readImgFromDisc(string filename , string kernel)
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
		
	if(theta || sigma || period){
		calculateGaborKernel();
	}
	else{
		// read the kernel from .filt file.
		readKernel(kernel);
	}
	
	// perform convolution through the kernel 
	convolove();	
	
	 	//mWidth  = ImageWidth;  // setting width and height for glut.
		//mHeight = ImageHeight;
	
	
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
	if(ImageChannels == 1){
		glDrawPixels(mWidth,mHeight,GL_LUMINANCE,GL_UNSIGNED_BYTE,&image[0]); // drawing the image on screen
	}
	else if(ImageChannels == 3){
		glDrawPixels(mWidth,mHeight,GL_RGB,GL_UNSIGNED_BYTE,&image[0]); // drawing the image on screen
	}
	else if(ImageChannels == 4){
		glDrawPixels(mWidth,mHeight,GL_RGBA,GL_UNSIGNED_BYTE,&image[0]); // drawing the image on screen
	}

	glFlush();
}


int main(int argc, char* argv[]){

	glutInit(&argc,argv);

	// Initialising glut window.
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
	glutInitWindowSize(WIDTH,HEIGHT);
	glutCreateWindow("convolved Image");

	// registering callback functions.
	glutDisplayFunc(drawOutputImage);
	glutKeyboardFunc(handleKey);
	glutReshapeFunc(handleReshape);

	// Call to open the image given by command line argument.
	string image = "", filter = "", output = "";
	cout<<"argc"<<argc;
	for(int i = 1 ; i < argc ; i++){
		if(strncmp(argv[i],"-g",2) == 0){

			theta  = stoi(argv[++i]);
			sigma  = stoi(argv[++i]);
			period = stoi(argv[++i]);
		}
		else if(strncmp(argv[i],"-f",2) == 0){
			image = argv[i + 1];
			if(argv[i + 2] && strncmp(argv[i + 2],"-g",2) != 0){
				filter = argv[i + 2];
			}
			
			if(argv[i + 3] && strncmp(argv[i + 3],"-g",2) != 0){
				output = argv[i +3];
			}
			
		}
	}
	if(!image.empty() && filter.empty()){
		readImgFromDisc(image, "");
	}
	if(!image.empty() && !filter.empty()){
		readImgFromDisc(image, filter);
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