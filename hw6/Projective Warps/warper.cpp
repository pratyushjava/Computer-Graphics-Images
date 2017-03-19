
/****
@Author = Pratyush Singh
* The purpose of this program is to read the given image and then Display it then takes the input of transformation and display the warped image on the screen and then read back the warped image from the screen and write it on the disc.
* steps to run
* 1. Build the project through make file. [warper]
* 2. Then use command -> warper inimage.png [outimage.png] optional. example (./warper dhouse.png del1.png).
* 3. Then provide series of input for transformation and press d when finish entering the transformation.
* 4. provide input transformation as below:-
*    r theta     counter clockwise rotation about image origin, theta in degrees
*    s sx sy     scale (watch out for scale by 0!)
*    t dx dy     translate
*    f xf yf     flip - if xf = 1 flip horizontal, yf = 1 flip vertical
*    h hx hy     shear
*    p px py     perspective
*    d           done
* 5. select the warp image window and then press W or w to write the image (if not provided the optional [output file name]).
****/

#include<OpenImageIO/imageio.h>
#include "matrix.h"
#include<cstdlib>
#include<iostream>
#include<math.h>
#include <sstream>
#include <vector>
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
unsigned char *gPixels = NULL; // This is a global pixel which will be having warped image
unsigned char *imageRGBA = NULL; // This will be a input image in RGBA format

// these are global variables which are used in multiple function. 
int ImageWidth = 0; // input image width.
int ImageHeight = 0; // input image height.
int ImageChannels = 0; // input image channels.

int outputWidth = 0; // output image width
int outputHight = 0; // output image height

//transformation resultant matrix;
Matrix3D M;

//Inverse of transformation resultant matrix;
Matrix3D MInv; 

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

// compute rotation matrix
void rotateTransformation(float theta){
	
	theta = theta * (3.14 / 180); //convert theta from radiyan to degree
	
	// creating rotation matrix.
	Matrix3D r;
	r.M[0][0] = cos(theta);
	r.M[0][1] = -sin(theta);
	r.M[1][0] = sin(theta);
	r.M[1][1] = cos(theta);
	
	M = r * M; // multiplying rotation matrix to resultant transformation matrix
}

// compute translation matrix
void translateTransformation(float dx, float dy){
	// creating translation matrix.
	Matrix3D t;
	t.M[0][2] = dx;
	t.M[1][2] = dy;
	
	M = t * M;  // multiplying translation matrix to resultant transformation matrix
}

// compute Scale matrix
void scaleTransformation(float sx, float sy){
	// creating Scale matrix.
	Matrix3D s;
	s.M[0][0] = sx;
	s.M[1][1] = sy;
	
	M = s * M;  // multiplying Scale matrix to resultant transformation matrix
}

// compute flip matrix
void flipTransformation(int xf, int yf){
	// creating flip matrix.
	Matrix3D f;
	f.M[0][0] = xf;
	f.M[1][1] = yf;
	
	M = f * M;   // multiplying flip matrix to resultant transformation matrix

}

// compute shear matrix
void shearTransformation(float hx, float hy){
	// creating shear matrix.
	Matrix3D sh;
	sh.M[0][1] = hx;
	sh.M[1][0] = hy;
	
	M = sh * M;   // multiplying shear matrix to resultant transformation matrix

}

// compute perspective matrix
void perspectiveTransformation(float px, float py){
	// creating perspective matrix.
	Matrix3D p;
	p.M[2][0] = px;
	p.M[2][1] = py;
	
	M = p * M;		// multiplying perspective matrix to resultant transformation matrix

}

// find the inverse if matrix M for computiing inverse warp.
void computeMInverse(){
	MInv = M.inverse();
}


// Logic to find min & max (x , y), required for finding width and height of output image
void findMinMaxXY(Vector2D xycorners[4], float &minX , float &minY, float &maxX , float &maxY ){
	
	// Storing x and y in array so that it can be iterated later. to find min, max X and Y 
	float xCord[4], yCord[4];
	xCord[0] = xycorners[0].x;
	xCord[1] = xycorners[1].x;
	xCord[2] = xycorners[2].x;
	xCord[3] = xycorners[3].x;
	
	yCord[0] = xycorners[0].y;
	yCord[1] = xycorners[1].y;
	yCord[2] = xycorners[2].y;
	yCord[3] = xycorners[3].y;
	
	//setting first coordinate to be both min , max -> x and y.
	minX = xycorners[0].x;
	maxX = xycorners[0].x;
	for(int i =1; i < 4 ; i++){
		
		if(xCord[i] < minX){ //if find smaller make this as smaller -> x
			minX = xCord[i];
		}
		
		if(xCord[i] > maxX){ //if find grater make this as greater -> x
			maxX = xCord[i];
		}
	}		
	
	minY = xycorners[0].y;
	maxY = xycorners[0].y;
	
	for(int i =1; i < 4 ; i++){
		
		if(yCord[i] < minY){ //if find smaller make this as smaller -> y
			minY = yCord[i];
		}
		
		if(yCord[i] > maxY){  //if find grater make this as greater -> y
			maxY = yCord[i];
		}
	}
	
}

void roundOF(float &minX , float &minY, float &maxX , float &maxY){
	// logic for rounding.
	// negative coordinate needs to be rounded by subtracting 0.5 before truncating
	//  while a positive coordinate is rounded by adding 0.5 before truncating
	
	if(minX < 0){
		minX -= 0.5;
	}
	else{
		minX += 0.5;
	}
	
	if(maxX < 0){
		maxX -= 0.5;
	}
	else{
		maxX += 0.5;
	}
	
	if(minY < 0){
		minY -= 0.5;
	}
	else{
		minY += 0.5;
	}
	
	if(maxY < 0){
		maxY -= 0.5;
	}
	else{
		maxY += 0.5;
	}
}

// allocat memory for output pix map
void constuctOutputPixmap(){
	
	Vector2D minUV, maxUminV, minUmaxV, maxUV, xycorners[4];
	// input image corners to find output image corners 
	minUV.x = 0;
	minUV.y = 0;
	maxUminV.x = ImageWidth;
	maxUminV.y = 0;
	minUmaxV.x = 0;
	minUmaxV.y = ImageHeight;
	maxUV.x = ImageWidth;
	maxUV.y = ImageHeight;
	
	// Find the output coordinates of the corners of image through forward map.
	xycorners[0] = M * minUV;
	xycorners[1] = M * maxUminV;
	xycorners[2] = M * minUmaxV;
	xycorners[3] = M * maxUV;
	
	// Logic to find min & max (x , y)
	float minX = 0.0 , minY = 0.0, maxX = 0.0, maxY = 0.0;
	findMinMaxXY(xycorners, minX , minY, maxX , maxY );

	// roundof by adding 0.5 or subtracting 0.5, based on negative and positive value. 
	roundOF(minX , minY, maxX , maxY);

	
	int x0 = static_cast <int> (floor(minX));
	int x1 = static_cast <int> (floor(maxX));
	int y0 = static_cast <int> (floor(minY));
	int y1 = static_cast <int> (floor(maxY));
	
	// If found negative minx, miny, we need to translate that much 
	int translatex = 0;
	int translatey = 0;
	
	if(x0 < 0){
		translatex = x0 * -1;
	}
	
	if(y0 < 0){
		translatey = y0 * -1;
	}
	
	// Width and Height of output image.
	outputWidth = (x1 - x0);
	
	outputHight = (y1 - y0);
		
	// Translate equivalent to negative minx or miny. so that the complete image fitts in pixmap.
	translateTransformation(translatex, translatey);
	
	// Allocate memory for output pixmap
	gPixels = new unsigned char[outputWidth * outputHight * 4];
	
}

// This function calls the desired transformation required by the user.
void computeTransformation(char transformationType, float xTransform, float yTransform, float theta, int flipX, int flipY){
	
	switch ( transformationType )
      {
         case 'r':
         case 'R':
	         rotateTransformation(theta);
            break;
         case 's':
         case 'S':
         	scaleTransformation(xTransform, yTransform);
            break;
         case 't':
         case 'T':
         	translateTransformation(xTransform, yTransform);
         	break;
         case 'f':
         case 'F':
         	flipTransformation(flipX, flipY);
         	break;
         case 'h':
         case 'H':
        	 shearTransformation(xTransform, yTransform);
         	break;
         case 'p':
         case 'P':
         	perspectiveTransformation(xTransform, yTransform);
         	break;
         default:
            break;
      }
     
}

//bileanear warp
void do_bilenearWarp(){
	cout<<"bileanear"<<endl;
	// input image corners to find output image corners 
	Vector2D minUV, maxUminV, minUmaxV, maxUV, r0, r1, r2, r3;
	BilinearCoeffs coeff;
	minUV.x = 0;
	minUV.y = 0;
	maxUminV.x = ImageWidth;
	maxUminV.y = 0;
	minUmaxV.x = 0;
	minUmaxV.y = ImageHeight;
	maxUV.x = ImageWidth;
	maxUV.y = ImageHeight;
	
	// Find the output coordinates of the corners of image.
	r0 = M * minUV;
	r1 = M * minUmaxV;
	r2 = M * maxUV;
	r3 = M * maxUminV;
	Vector2D xycorners[4];
	
	xycorners[0] = r0;
	xycorners[1] = r1;
	xycorners[2] = r2;
	xycorners[3] = r3;
	
	// Set bileanear coeff to use it later
	setbilinear(ImageWidth, ImageHeight, xycorners, coeff );
	
	mWidth  = outputWidth;  // setting width and height for glut.
	mHeight = outputHight;
	
	// Using inverse map, so iterating through the output image.
	for(int i=0;i<outputHight;i++){
		for(int j=0;j<outputWidth;j++){
			
			// inverse bileanear
			Vector2D normalXY, normalUV;
			float normalY = (float)((float)i+0.5);
			float normalX = (float)((float)j+0.5);
			normalXY.x = normalX;
			normalXY.y = normalY;
			invbilinear(coeff, normalXY, normalUV);
			
			//converting coordinates to input indexes.
			float u = normalUV.x ;
			float v = normalUV.y; 
			
			// roundof 
			int inputU = static_cast <int> (floor(u));
			int inputV = static_cast <int> (floor(v));
			
			
			// skip those which falls outside the boundary of input image
			if(inputU < 0 || inputU > ImageWidth || inputV < 0 || inputV > ImageHeight){
				gPixels[i*outputWidth*4+j*4+0]    =  0; // red
				gPixels[i*outputWidth*4+j*4+1]    =  0; // green
				gPixels[i*outputWidth*4+j*4+2]    =  0; // blue
				gPixels[i*outputWidth*4+j*4+3] 	  =  255;
				continue;
			}
			
			// Filling the desired input image pixels into output image pixels.
			gPixels[i*outputWidth*4+j*4+0]    =  image[inputV*ImageWidth*ImageChannels+inputU*ImageChannels+0]; // red
			gPixels[i*outputWidth*4+j*4+1]    =  image[inputV*ImageWidth*ImageChannels+inputU*ImageChannels+1]; // green
			gPixels[i*outputWidth*4+j*4+2]    =  image[inputV*ImageWidth*ImageChannels+inputU*ImageChannels+2]; // blue
			gPixels[i*outputWidth*4+j*4+3] 	  =  255; // A
		}
	}
}

// projective warp
void do_Warp()
{
	cout<<"normal warp"<<endl;
	mWidth  = outputWidth;  // setting width and height for glut.
	mHeight = outputHight;
	
	// Using inverse map, so iterating through the output image.
	for(int i=0;i<outputHight;i++){
		for(int j=0;j<outputWidth;j++){
					
			float normalY = i;
			float normalX = j;
			
			// Finding the desired input pixel for the output pixmap by inverse resultant matrix
			Vector2D normalXY, normalUV;
			normalXY.x = normalX;
			normalXY.y = normalY;
			normalUV = MInv * normalXY;

			//converting coordinates to input indexes.
			float u = normalUV.x ;
			float v = normalUV.y; 
			int inputU = static_cast <int> (floor(u));
			int inputV = static_cast <int> (floor(v));
			
			
			// skip those which falls outside the boundary of input image
			if(inputU < 0 || inputU > ImageWidth || inputV < 0 || inputV > ImageHeight){
				gPixels[i*outputWidth*4+j*4+0]    =  0; // red
				gPixels[i*outputWidth*4+j*4+1]    =  0; // green
				gPixels[i*outputWidth*4+j*4+2]    =  0; // blue
				gPixels[i*outputWidth*4+j*4+3] 	  =  255;
				continue;
			}
		
		// Filling the desired input image pixels into output image pixels.
		gPixels[i*outputWidth*4+j*4+0]    =  image[inputV*ImageWidth*ImageChannels+inputU*ImageChannels+0]; // red
		gPixels[i*outputWidth*4+j*4+1]    =  image[inputV*ImageWidth*ImageChannels+inputU*ImageChannels+1]; // green
		gPixels[i*outputWidth*4+j*4+2]    =  image[inputV*ImageWidth*ImageChannels+inputU*ImageChannels+2]; // blue
		gPixels[i*outputWidth*4+j*4+3] 	  =  255;	//A
		
	}
}

}


// read the image
void readImgFromDisc(string filename)
{
	// Reading the image.
	if(filename.empty()){ // if no name provided through command line argument, ask the user to input the image name
		cout<<"Please enter the  input image name"<<endl;
		cin>>filename;
	}
	
	ImageInput *in = ImageInput::open(filename);// create handel of input imageinput this will be required to read a file
	if(!in){
		cout << "Error in opening a input image, please try again!!";
		return;
	}
		
	const ImageSpec &spec = in->spec();// to get image specification-> width, height, channel.
	ImageWidth = spec.width;
	ImageHeight = spec.height;
	ImageChannels = spec.nchannels;
	
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

void split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
}


int main(int argc, char* argv[]){
	//parsing the command line argument
	string image = "", output = "";
	bool bilenear = false;
	for(int i = 1 ; i < argc ; i++){
		if(strncmp(argv[i],"-b",2) == 0){
			bilenear = true;
		}
		else{
			image = argv[i];			
			if(argv[i + 1]){
				output = argv[i +1];
				break;
			}
			
		}
	}
	
	if(image.empty()){
		cout<<"please provide input image file name "<<endl;
		exit(0);
	}



	glutInit(&argc,argv);

	// Initialising glut window.
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
	glutInitWindowSize(WIDTH,HEIGHT);
	glutCreateWindow("warp Image");

	// registering callback functions.
	glutDisplayFunc(drawOutputImage);
	glutKeyboardFunc(handleKey);
	glutReshapeFunc(handleReshape);
	

	char oper;
	double xop=0;
	double yop=0;
	int flipX = 1;
	int flipY = 1;
	
	/*
			//computeTransformation(char transformationType, float xTransform, float yTransform, float theta, int flipX, int flipY)
	//computeTransformation('s', 1, 1, 0.0, 1, -1);
	computeTransformation('t', 100.0, 0.0, 30, 1, -1);
	//computeTransformation('f', 0.0, 0.0, 30, -1, 1);
	//computeTransformation('h', 2.0, 0.0, 30, 1, -1);
	//computeTransformation('p', 0.001, 0.005, 30, 1, -1);
			constuctOutputPixmap();
	M.print();
	computeMInverse();
	MInv.print();
	
	do_bilenearWarp();
	//do_Warp();
	
	*/
	
	while(oper!='d')
	{
	   cin >> oper;
  	   if(oper=='r')
	   {
	    cin >> xop;
	    computeTransformation('r', 0.0, 0.0, xop, 0, 0);
	   }
	   if(oper=='s')
	   {
	    cin >> xop;
	    cin >> yop;
		if(xop==0)
		{
		   xop=1.0;
		}
		if(yop==0)
		{
		   yop=1.0;
		}
	    computeTransformation('s', xop, yop, 0.0, 0, 0);
	   }
	   if(oper=='t')
	   {
		  cin >> xop;
		  cin >> yop;
		  computeTransformation('t', xop, yop, 0.0, 0, 0);
	   }
	   if(oper=='f')
	   {
	      cin >> flipX;
		  cin >> flipY;
		  flipX = flipX;
		  flipY = flipY;
		  if(flipX==0)
		  {
		      flipX=1;
		  }
		  if(flipY==0)
		  {
		      flipY=1;
		  }
		  computeTransformation('f', 0.0, 0.0, 0.0, flipX, flipY);
		}
		if(oper=='h')
		{
		    cin >> xop;
		    cin >> yop;
		    computeTransformation('h', xop, yop, 0.0, 0, 0);
		}
		if(oper=='p')
		{
		   cin >> xop;
		   cin >> yop;
		   computeTransformation('p', xop, yop, 0.0, 0, 0);
		}	
	}

		
	if(!image.empty()){
		readImgFromDisc(image);
	}
	

	// find the width and height of pixmap
	constuctOutputPixmap();

	// find the inverse resultant matrix
	computeMInverse();
	
	// if -b the do bileanear warp else normal inverse warp
	if(bilenear){
		do_bilenearWarp();
	}
	else{
		do_Warp();
	}
	
	glutReshapeWindow(mWidth,mHeight);
	glutPostRedisplay();
	glFlush();

	if(!output.empty()){
		writeOutput(output);
	}
	


	// Initialising glut window.
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
	glutInitWindowSize(ImageWidth,ImageHeight);
	glutCreateWindow("Input Image");

	// registering callback functions.mWidth,mHeight,
	glutDisplayFunc(drawInputImage);
	glutKeyboardFunc(handleKey);
	glutReshapeFunc(handleReshape);

	//opengl mainloop
	glutMainLoop();

}
