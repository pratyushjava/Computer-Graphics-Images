
/****
@Author = Pratyush Singh
* The purpose of this program is to read the given image and then Display its warped image on the screen and then read back the warped image from the screen and write it on the disc.
* steps to run
* 1. build the project through make file.
* 2. Then use command inimage.png [outimage.png]optional. example (./warp dhouse.png del1.png).
* 3. select the warp image window and then press W or w to write the image (if not provided the optional [output file name]).
* Note: In this program I have 2 warp one which is given by the professor and one i have used by myself.
* 		There are 2 variations of each warp as well, one is clean warp where i have removed the artifacts after warping the other is without cleaning.
*		After running the program, user will be prompt with the following questions
*		1. Press Y for clean warp / N for normal warp?
*		   Here if you press (y) then the program will do clean warp by doing bileanear interpolation and if you press (N) it will just give output of inverse map for the warp, the main purpose of this is to test before and after image of cleanig.
*		2. Press Y to use oKwarp / N for use my own warp function?
*		   Here if you press (y) then the program will use the warp function given by the professor, but we also have to use one of our function for warp so see the warp through this warp press (N)
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
int outputWidth = 0;
int outputHight = 0;
bool invWarp = false;
bool iscleanWarp = false;
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


// inverse warp, different than what we got in the homework, program will use this function when user press (N) for second question.
void inv_map2(float x, float y, float &u, float &v,
	int inwidth, int inheight, int outwidth, int outheight){
			
	  x /= outwidth;		// normalize (x, y) to (0...1, 0...1)
	  y /= outheight;
	
	  u = pow(x, 0.25);			        // inverse in x direction is sqrt
	  v = pow(sin(3.14 * 0.5 * y), 2); // inverse in y direction is offset sine
	
	  u *= inwidth;			// scale normalized (u, v) to pixel coords
	  v *= inheight;
		
}


// inverse warp, different than what we got in the homework, program will use this function when user press (Y) for second question.
void inv_map(float x, float y, float &u, float &v,
	int inwidth, int inheight, int outwidth, int outheight){
  
  if(invWarp)
  {
	x /= outwidth;		// normalize (x, y) to (0...1, 0...1)
	y /= outheight;
	
	u = sqrt(x);			        // inverse in x direction is sqrt
	v = 0.5 * (1 + sin(y * 3.1415926536));  // inverse in y direction is offset sine
	
	u *= inwidth;			// scale normalized (u, v) to pixel coords
	v *= inheight;
  }
  else{
      inv_map2(x,  y, u, v, inwidth,  inheight,  outwidth,  outheight);
  }

}

// warping function which does not do cleaning of warp, used whem user press (N) for the first question. basically to find how image look before cleaning of the warp.
void do_Warp()
{
	// setting the input image size same as output image.
	outputWidth = ImageWidth;
	outputHight = ImageHeight;
	mWidth  = outputWidth;  // setting width and height for glut.
	mHeight = outputHight;

	// allocating memory for warp image
	gPixels = new unsigned char[outputWidth * outputHight * 4];
	
	// Using inverse map, so iterating through the output image.
	for(int i=0;i<outputHight;i++){
		for(int j=0;j<outputWidth;j++){
						
			//converting coordinates to input indexes.
			float u = 0.0;
			float v = 0.0;
			
			inv_map(j,i,u,v,ImageWidth,ImageHeight,outputWidth,outputHight);
			
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





// Logic to find min & max (x , y), required for finding width and height of output image
void findMinMaxXY(float u00, float u01, float u11, float u10, float v00, float v01, float v11, float v10, float &minX , float &minY, float &maxX , float &maxY ){
	
	// Storing x and y in array so that it can be iterated later. to find min, max X and Y 
	float xCord[4], yCord[4];
	xCord[0] = u00;
	xCord[1] = u01;
	xCord[2] = u11;
	xCord[3] = u10;
	
	yCord[0] = v00;
	yCord[1] = v01;
	yCord[2] = v11;
	yCord[3] = v10;
	
	//setting first coordinate to be both min , max -> x and y.
	minX = u00;
	maxX = u00;
	for(int i =1; i < 4 ; i++){
		
		if(xCord[i] < minX){ //if find smaller make this as smaller -> x
			minX = xCord[i];
		}
		
		if(xCord[i] > maxX){ //if find grater make this as greater -> x
			maxX = xCord[i];
		}
	}		
	
	minY = v00;
	maxY = v00;
	
	for(int i =1; i < 4 ; i++){
		
		if(yCord[i] < minY){ //if find smaller make this as smaller -> y
			minY = yCord[i];
		}
		
		if(yCord[i] > maxY){  //if find grater make this as greater -> y
			maxY = yCord[i];
		}
	}
	
}



bool isMagnification(float u00, float u01, float u11, float u10, float v00, float v01, float v11, float v10){
	
	//float dx = (pow((u01 - u00),2) + pow((u10 - u11),2) + pow((u11 - u00),2) + pow((u10 - u01),2))/2;
	//float dy = (pow((v01 - v00),2) + pow((v10 - v11),2) + pow((v11 - v00),2) + pow((v10 - v01),2))/2;
	float minX = 0.0 , minY = 0.0, maxX = 0.0 , maxY = 0.0;
	findMinMaxXY( u00,  u01,  u11,  u10,  v00,  v01,  v11,  v10,  minX ,  minY,  maxX ,  maxY );
	float dx = maxX - minX; // for dx dy i have used difference for min and max, x and y. i have tried many ways but got same result after computing dx , dy.
	float dy = maxY - minY;
	if(dx > 0 && dy > 0){
		return false;	
	}
	return true;
}


// this funstion is called for those pixels which are magnified.
void bileanearInterpolation(float x , float y, int &R, int &G, int &B){

		float u = 0.0, v = 0.0;
		
        //do inverse transform
        inv_map(x,y,u,v,ImageWidth,ImageHeight,outputWidth,outputHight);
        //in coor of src
        int x1= (int)floor(u);
        int y1= (int)floor(v);
        int x2= x1+1;
        int y2= y1+1;
        
        // calculate weight
        float dx1= u-x1;
        float dx2= 1-dx1;
        float dy1= v-y1;
        float dy2= 1-dy1;
		
		if(x1 >= 0 && x2 >=0 && y1 >= 0 && y2 >= 0 && x1 < ImageWidth && x2 < ImageWidth && y1 < ImageHeight && y2 < ImageHeight ){
	   		// finding average color value
	   		
	        B = ( dy2 * (image[y1*ImageWidth*ImageChannels+x1*ImageChannels+2] * dx2 + image[y1*ImageWidth*ImageChannels+x2*ImageChannels+2] * dx1) +
	                dy1 * (image[y2*ImageWidth*ImageChannels+x1*ImageChannels+2] * dx2 + image[y2*ImageWidth*ImageChannels+x2*ImageChannels+2] * dx1));
	                
	        G = (dy2 * (image[y1*ImageWidth*ImageChannels+x1*ImageChannels+1] * dx2 + image[y1*ImageWidth*ImageChannels+x2*ImageChannels+1] * dx1)+
	                dy1 * (image[y2*ImageWidth*ImageChannels+x1*ImageChannels+1] * dx2 + image[y2*ImageWidth*ImageChannels+x2*ImageChannels+1] * dx1));
	                
	        R = (dy2 * (image[y1*ImageWidth*ImageChannels+x1*ImageChannels+0] * dx2 + image[y1*ImageWidth*ImageChannels+x2*ImageChannels+0] * dx1)+
	                dy1 * (image[y2*ImageWidth*ImageChannels+x1*ImageChannels+0] * dx2 + image[y2*ImageWidth*ImageChannels+x2*ImageChannels+0] * dx1));
	               
		}
		else{ // this is executed when on the edges few pixels fall outside of image. to remove the black like in center.
			R = image[y1*ImageWidth*ImageChannels+x1*ImageChannels+0];
			G = image[y1*ImageWidth*ImageChannels+x1*ImageChannels+1];
			B = image[y1*ImageWidth*ImageChannels+x1*ImageChannels+2];
		}

}
	
// this funstion is called for those pixels which are minified.
void superSampling(float x , float y, int &R, int &G, int &B){
	
	float u = 0.0, v = 0.0;
	
	//do inverse transform
	float u00 = 0.0, u01 = 0.0, u11 = 0.0, u10 = 0.0, v00 = 0.0, v01 = 0.0, v11 = 0.0, v10 = 0.0;
	
	// mapping input pixels for the bounding box.
	inv_map(x,y,u00,v00,ImageWidth,ImageHeight,outputWidth,outputHight);
	inv_map(x,y+1,u01,v01,ImageWidth,ImageHeight,outputWidth,outputHight);
	inv_map(x+1,y+1,u11,v11,ImageWidth,ImageHeight,outputWidth,outputHight);
	inv_map(x+1,y,u10,v10,ImageWidth,ImageHeight,outputWidth,outputHight);
	
	//finding min max x, y . to identify the size of square of bounding box in input image 
	float minX = 0.0 , minY = 0.0, maxX = 0.0 , maxY = 0.0;
	findMinMaxXY( u00,  u01,  u11,  u10,  v00,  v01,  v11,  v10,  minX ,  minY,  maxX ,  maxY );
	
	// if the size is smaller than 1, then get the color from the input pixel which maps to center of the output pixel.
	if( maxX - minX < 1 && maxY - minY < 1){
		float centerX = (maxX - minX)/2;
		float centerY = (maxY - minY)/2;
		inv_map(x + centerX , y + centerY ,u , v , ImageWidth , ImageHeight , outputWidth , outputHight );
		int x1= (int)floor(u);
        int y1= (int)floor(v);
        R = image[y1*ImageWidth*ImageChannels+x1*ImageChannels+0];
        G = image[y1*ImageWidth*ImageChannels+x1*ImageChannels+1];
        B = image[y1*ImageWidth*ImageChannels+x1*ImageChannels+2];
	}
	else{
		/*
		 * Here i am just finding the nine neighbouring points, getting there inverse map and picking there R , G , B values from there input pixmap.
		 * Later at last i have calculated the weighted average of these R, G, B values.
		*/
		float a0x = x;
		float a0y = y;
		float a1x = x;
		float a1y = y + 0.5;
		float a2x = x;
		float a2y = y + 1;
		float a3x = x + 0.5;
		float a3y = y + 1;
		float a4x = x + 1;
		float a4y = y + 1;
		float a5x = x + 1;
		float a5y = y + 0.5;
		float a6x = x + 1;
		float a6y = y;
		float a7x = x + 0.5;
		float a7y = y;
		float centerx = x + 0.5;
		float centery = y + 0.5;
		inv_map(a0x , a0y ,u , v , ImageWidth , ImageHeight , outputWidth , outputHight );
		
		int x1= (int)floor(u);
        int y1= (int)floor(v);
        float R0 = 0.0;
        float G0 = 0.0;
        float B0 = 0.0;
        if(x1 >= 0  && y1 >= 0 && x1 < ImageWidth && y1 < ImageHeight){
	        R0 = image[y1*ImageWidth*ImageChannels+x1*ImageChannels+0];
	        G0 = image[y1*ImageWidth*ImageChannels+x1*ImageChannels+1];
	        B0 = image[y1*ImageWidth*ImageChannels+x1*ImageChannels+2];
        }
        x1 = 0.0;
        y1 = 0.0;
        u  = 0.0;
        v  = 0.0;
        
        inv_map(a1x , a1y ,u , v , ImageWidth , ImageHeight , outputWidth , outputHight );
		x1= (int)floor(u);
        y1= (int)floor(v);
        float R1 = 0.0;
        float G1 = 0.0;
        float B1 = 0.0;
        if(x1 >= 0  && y1 >= 0 && x1 < ImageWidth && y1 < ImageHeight){
	        R1 = image[y1*ImageWidth*ImageChannels+x1*ImageChannels+0];
	        G1 = image[y1*ImageWidth*ImageChannels+x1*ImageChannels+1];
	        B1 = image[y1*ImageWidth*ImageChannels+x1*ImageChannels+2];
        }
        x1 = 0.0;
        y1 = 0.0;
        u  = 0.0;
        v  = 0.0;
        
        inv_map(a2x , a2y ,u , v , ImageWidth , ImageHeight , outputWidth , outputHight );
		x1= (int)floor(u);
        y1= (int)floor(v);
        float R2 = 0.0;
        float G2 = 0.0;
        float B2 = 0.0;
        if(x1 >= 0  && y1 >= 0 && x1 < ImageWidth && y1 < ImageHeight){
	         R2 = image[y1*ImageWidth*ImageChannels+x1*ImageChannels+0];
	         G2 = image[y1*ImageWidth*ImageChannels+x1*ImageChannels+1];
	         B2 = image[y1*ImageWidth*ImageChannels+x1*ImageChannels+2];
        }
        x1 = 0.0;
        y1 = 0.0;
        u  = 0.0;
        v  = 0.0;
        
        inv_map(a3x , a3y ,u , v , ImageWidth , ImageHeight , outputWidth , outputHight );
		x1= (int)floor(u);
        y1= (int)floor(v);
        float R3 = 0.0;
        float G3 = 0.0;
        float B3 = 0.0;
        if(x1 >= 0  && y1 >= 0 && x1 < ImageWidth && y1 < ImageHeight){
	         R3 = image[y1*ImageWidth*ImageChannels+x1*ImageChannels+0];
	         G3 = image[y1*ImageWidth*ImageChannels+x1*ImageChannels+1];
	         B3 = image[y1*ImageWidth*ImageChannels+x1*ImageChannels+2];
        }
        x1 = 0.0;
        y1 = 0.0;
        u  = 0.0;
        v  = 0.0;
        
        inv_map(a4x , a4y ,u , v , ImageWidth , ImageHeight , outputWidth , outputHight );
		x1= (int)floor(u);
        y1= (int)floor(v);
        float R4 = 0.0;
        float G4 = 0.0;
        float B4 = 0.0;
        if(x1 >= 0  && y1 >= 0 && x1 < ImageWidth && y1 < ImageHeight){
	         R4 = image[y1*ImageWidth*ImageChannels+x1*ImageChannels+0];
	         G4 = image[y1*ImageWidth*ImageChannels+x1*ImageChannels+1];
	         B4 = image[y1*ImageWidth*ImageChannels+x1*ImageChannels+2];
        }
        x1 = 0.0;
        y1 = 0.0;
        u  = 0.0;
        v  = 0.0;
        
        inv_map(a5x , a5y ,u , v , ImageWidth , ImageHeight , outputWidth , outputHight );
		x1= (int)floor(u);
        y1= (int)floor(v);
        float R5 = 0.0;
        float G5 = 0.0;
        float B5 = 0.0;
        if(x1 >= 0  && y1 >= 0 && x1 < ImageWidth && y1 < ImageHeight){
	         R5 = image[y1*ImageWidth*ImageChannels+x1*ImageChannels+0];
	         G5 = image[y1*ImageWidth*ImageChannels+x1*ImageChannels+1];
	         B5 = image[y1*ImageWidth*ImageChannels+x1*ImageChannels+2];
        }
        x1 = 0.0;
        y1 = 0.0;
        u  = 0.0;
        v  = 0.0;
        
        inv_map(a6x , a6y ,u , v , ImageWidth , ImageHeight , outputWidth , outputHight );
		x1= (int)floor(u);
        y1= (int)floor(v);
        float R6 = 0.0;
        float G6 = 0.0;
        float B6 = 0.0;
        if(x1 >= 0  && y1 >= 0 && x1 < ImageWidth && y1 < ImageHeight){
	         R6 = image[y1*ImageWidth*ImageChannels+x1*ImageChannels+0];
	         G6 = image[y1*ImageWidth*ImageChannels+x1*ImageChannels+1];
	         B6 = image[y1*ImageWidth*ImageChannels+x1*ImageChannels+2];
        }
        x1 = 0.0;
        y1 = 0.0;
        u  = 0.0;
        v  = 0.0;
        
        inv_map(a7x , a7y ,u , v , ImageWidth , ImageHeight , outputWidth , outputHight );
		x1= (int)floor(u);
        y1= (int)floor(v);
        float R7 = 0.0;
        float G7 = 0.0;
        float B7 = 0.0;
        if(x1 >= 0  && y1 >= 0 && x1 < ImageWidth && y1 < ImageHeight){
	         R7 = image[y1*ImageWidth*ImageChannels+x1*ImageChannels+0];
	         G7 = image[y1*ImageWidth*ImageChannels+x1*ImageChannels+1];
	         B7 = image[y1*ImageWidth*ImageChannels+x1*ImageChannels+2];
        }
        x1 = 0.0;
        y1 = 0.0;
        u  = 0.0;
        v  = 0.0;
        
        inv_map(centerx , centery ,u , v , ImageWidth , ImageHeight , outputWidth , outputHight );
		x1= (int)floor(u);
        y1= (int)floor(v);
        float Rcenter = 0.0;
        float Gcenter = 0.0;
        float Bcenter = 0.0;
        if(x1 >= 0  && y1 >= 0 && x1 < ImageWidth && y1 < ImageHeight){
	         Rcenter = image[y1*ImageWidth*ImageChannels+x1*ImageChannels+0];
	         Gcenter = image[y1*ImageWidth*ImageChannels+x1*ImageChannels+1];
	         Bcenter = image[y1*ImageWidth*ImageChannels+x1*ImageChannels+2];
        }
        x1 = 0.0;
        y1 = 0.0;
        u  = 0.0;
        v  = 0.0;
        
        // center weigt = 1 / 4, corners weight = 1 / 16, mid point of side = 1/ 8
        
        R = (R0/16) + (R1/8) + (R2/16) + (R3/8) + (R4 / 16) + (R5 / 8) + (R6 / 16) + (R7 / 8) + (Rcenter / 4);
        
        G = (G0/16) + (G1/8) + (G2/16) + (G3/8) + (G4 / 16) + (G5 / 8) + (G6 / 16) + (G7 / 8) + (Gcenter / 4);
        
        B = (B0/16) + (B1/8) + (B2/16) + (B3/8) + (B4 / 16) + (B5 / 8) + (B6 / 16) + (B7 / 8) + (Bcenter / 4);

		
	}
	
}

// This function is called when user press y for 1st question.
void cleanWarp(){
	// setting the input image size same as output image.
	outputWidth = ImageWidth;
	outputHight = ImageHeight;
	mWidth  = outputWidth;  // setting width and height for glut.
	mHeight = outputHight;
	cout <<" outputWidth "<<outputWidth<<" outputHight "<<outputHight;
	// allocating memory for warp image
	gPixels = new unsigned char[outputWidth * outputHight * 4];
	
	int countMag = 0;
	int countMin = 0;
	// Using inverse map, so iterating through the output image.
	for(int i=0;i<outputHight;i++){
		for(int j=0;j<outputWidth;j++){
			
			// Get the bounding box of each pixel, to find minification or magnification case.
			float u00 = 0.0, u01 = 0.0, u11 = 0.0, u10 = 0.0, v00 = 0.0, v01 = 0.0, v11 = 0.0, v10 = 0.0;
			inv_map(j,i,u00,v00,ImageWidth,ImageHeight,outputWidth,outputHight);
			inv_map(j,i+1,u01,v01,ImageWidth,ImageHeight,outputWidth,outputHight);
			inv_map(j+1,i+1,u11,v11,ImageWidth,ImageHeight,outputWidth,outputHight);
			inv_map(j+1,i,u10,v10,ImageWidth,ImageHeight,outputWidth,outputHight);
			int R = 0, G = 0, B = 0;
			
			// if magnification do bileanear interpolation and if minification do supersampling
			if(isMagnification( u00, u01, u11, u10, v00, v01, v11, v10))
			{
				bileanearInterpolation(j , i , R, G, B);
				// set the R G B value got afer bileanearInterpolation
				gPixels[i*outputWidth*4+j*4+0]    =  R; // red
				gPixels[i*outputWidth*4+j*4+1]    =  G; // green
				gPixels[i*outputWidth*4+j*4+2]    =  B; // blue
				gPixels[i*outputWidth*4+j*4+3] 		=  255;
				countMag++;
			}
			else{
				
				superSampling(j , i , R, G, B);
				// set the R G B value got afer superSampling
				gPixels[i*outputWidth*4+j*4+0]    =  R; // red
				gPixels[i*outputWidth*4+j*4+1]    =  G; // green
				gPixels[i*outputWidth*4+j*4+2]    =  B; // blue
				gPixels[i*outputWidth*4+j*4+3] 		=  255;
				countMin++;
			}
			
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
		imageRGBA[i*ImageWidth*4+j*4+3]    =  255;
		
	}
}	
	

	// call to the warp function
	if(iscleanWarp){
		cleanWarp();
	}
	else{
		do_Warp();
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
	
	char ch = ' ';
	cout<<" Press Y for clean warp / N for normal warp"<<endl;
	cin>>ch;
	if(ch == 'Y' || ch == 'y'){
		iscleanWarp = true;
	}
	
	ch = ' ';
	cout<<" Press Y to use oKwarp / N for use my own warp function"<<endl;
	cin>>ch;
	if(ch == 'Y' || ch == 'y'){
		invWarp = true;
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