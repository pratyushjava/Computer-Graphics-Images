
/****
@Author = Pratyush Singh
* This is a program of hiding a secret image in cover image. The method is called steganography.
* The purpose of this program is to read the given 2 image and hide the complete second image in the last 2 bit of pixmap of first image.
* Note to function this program properly it is necessary that the second image is 3 times smaller than the first image.
* 1. give file names to read as commandline argument as, ./stagnography coverImage secretImage.
* 2. Note there was a issue as openimageIO was missing last bits in reading and writing, i have encoded the image and decoded the image in one go.
* 3. when decoding is done after encoding, openimageIO do not come in picture. but when decoded image is read from the file there is a problem.
* 4. press q or Q or esc to exit.
****/

#include<OpenImageIO/imageio.h>

#include<cstdlib>
#include<iostream>

#include <fstream>

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
unsigned char *image = NULL;
unsigned char *bgImage = NULL;
unsigned char *decodeImage = NULL;
int bgWidth = 0;        // used by pixmap To store the secret image
int bgHeight = 0;
int bgChannels = 0;
int maskWidth = 0;      // used by pixmap to store the first image pixmap
int maskHeight = 0;
int maskChannels = 0;
int curX = 0;
int curY = 0;
bool doDecode = false;
int decodedImageWidth = 0;  // These variables are used for pixmap where decoded (extracted) image will be stored
int decodedImageHeight = 0;
int decodedImageChannels = 0;




/*
    two seperate function for writing the image is used one for writing encoded image but since openimageIo is loosing last bit decoding of encoded 
    image is also done after doing encoding.
    so one function is for writing encoded image just to explain how encoded image looks after hiding the secret image in it.
    second function is for writing the decoded image (extarcted).
    two seperate function is used to make it easier for the grader to understand which function is writing which pixmap.
    and how that pixmap is filled. in the writeEncodedImageWhichHasSecretImageHiden() encoded image is written and that pixmap is filled in encoded image.
    in writeExtractedImageWhichIsDecodedFromEncodedImage() extarcted image is writen and the pixmap it use is filled in decoded function where it
    extract the the image from last two bit of the encoded image pixmap.
*/


// this function write encoded image to the file.
void writeEncodedImageWhichHasSecretImageHiden(string outPutName)
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
	ImageSpec spec(mWidth,mHeight,3,TypeDesc::UINT8);
	if(!outF->open(outPutName, spec)){
		cout << "Can not open the file, you entered."<< endl;
		return;
	}
	
	// Calculating one Scanline length, this is required in order to write a image correct
	// As opengl starts up down so if we need to write it we need to write in reverse order otherwise reverse image will be written, -scanlinesize enshure that image is writen straight.
	int scanlinesize = mWidth * 3 * sizeof(image[0]); // 1 scan line will be equal to width * channel * size of one memory chunk
	
	if (gPixels)
	{
		outF->write_image(TypeDesc::UINT8, (char *)image + (mHeight - 1)*scanlinesize,
			AutoStride, -scanlinesize, AutoStride);
	}
	
	//close and delete the imageoutput handler.
	outF->close();
	delete outF;
	return;
	return;
}


// This function writes the decoded (extracted ) image which is extracted from last 2 bit of encoded image.
void writeExtractedImageWhichIsDecodedFromEncodedImage(string outPutName)
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
	ImageSpec spec(decodedImageWidth,decodedImageHeight,decodedImageChannels,TypeDesc::UINT8);
	if(!outF->open(outPutName, spec)){
		cout << "Can not open the file, you entered."<< endl;
		return;
	}
	
	// Calculating one Scanline length, this is required in order to write a image correct
	// As opengl starts up down so if we need to write it we need to write in reverse order otherwise reverse image will be written, -scanlinesize enshure that image is writen straight.
	int scanlinesize = decodedImageWidth * decodedImageChannels * sizeof(decodeImage[0]); // 1 scan line will be equal to width * channel * size of one memory chunk
	
	if (gPixels)
	{
		outF->write_image(TypeDesc::UINT8, (char *)decodeImage + (decodedImageHeight - 1) * scanlinesize,
			AutoStride, -scanlinesize, AutoStride);
	}
	
	//close and delete the imageoutput handler.
	outF->close();
	delete outF;
	return;
	return;
}


void writeImgToDisc()
{
	//get window width and height, this will be used to read the pixels from the screen, these are input for glReadPixels which read pixels from opengl framebuffer.
	const int w = glutGet(GLUT_WINDOW_WIDTH);
	const int h = glutGet(GLUT_WINDOW_HEIGHT);

	string outPutName; // name of the file which will be inputed by the user and same will be created to store the encoded image.

	cout << "Enter the name of encoded image";
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

// encoding part start
// This function insert the bits to the last 2 bit of the cover image.
void mergeTwoBit(unsigned char r, unsigned char g, unsigned char b){
    // here RGB will be having bits only last 2 bits rest will be zero
    // image pixmap will be having last 2 bit as zero so what ever will be there RGB will be set in pixmap
    image[curY*maskWidth*3+curX*3+0] |= r; // red
    image[curY*maskWidth*3+curX*3+1] |= g; // green
    image[curY*maskWidth*3+curX*3+2] |= b; // blue
    cout<<" after mergeTwoBit  "<<" r1 "<<int(image[curY*maskWidth*3+curX*3+0])<<" g1 "<<int(image[curY*maskWidth*3+curX*3+1])<<" b1 "<<int(image[curY*maskWidth*3+curX*3+2])<<endl;
}

// this function will set last 2 bit of pixmap to zero
void setLastTwoBitZero(){
    // AND operation with 252 will set last 2 bit to zero.
    cout<<" before setting last bit to zero "<<" r "<<int(image[curY*maskWidth*3+curX*3+0])<<" g "<<int(image[curY*maskWidth*3+curX*3+1])<<" b "<<int(image[curY*maskWidth*3+curX*3+2])<<endl;
    image[curY*maskWidth*3+curX*3+0] &= 252; // red
    image[curY*maskWidth*3+curX*3+1] &= 252; // green
    image[curY*maskWidth*3+curX*3+2] &= 252; // blue
    cout<<" after setting last bit to zero "<<" r "<<int(image[curY*maskWidth*3+curX*3+0])<<" g "<<int(image[curY*maskWidth*3+curX*3+1])<<" b "<<int(image[curY*maskWidth*3+curX*3+2])<<endl;
}

void hide(unsigned char r, unsigned char g, unsigned char b){
    cout<<" setLastTwoBitZero 1 "<<endl;
    setLastTwoBitZero();
    unsigned char r1 = r, r2 = r, r3 = r, b1 =b, b2 = b, b3 = b, g1 = g, g2 = g , g3 = g;
    
    // getting first 2 bit of RGB to set in a cover image pixmap
    r1 = r1>>6;
    g1 = g1>>6;
    b1 = b1>>6;
    
    cout<<" send to mergeTwoBit 1 "<<" r1 "<<int(r1)<<" g1 "<<int(g1)<<" b1 "<<int(b1)<<endl;
    mergeTwoBit(r1, g1, b1);
    cout<<" merge 1 done "<<endl;
    curX++;
    
    cout<<" setLastTwoBitZero 2 "<<endl;
    setLastTwoBitZero();
    
    // getting third and fourth bit of RGB to set in a cover image pixmap
    r2 = r2<<2;
    r2 = r2>>6;
    g2 = g2<<2;
    g2 = g2>>6;
    b2 = b2<<2;
    b2 = b2>>6;
    cout<<" send to mergeTwoBit 2 "<<" r2 "<<int(r2)<<" g2 "<<int(g2)<<" b2 "<<int(b2)<<endl;
    mergeTwoBit(r2, g2, b2);
    cout<<" merge 2 done "<<endl;
    curX++;
    
    cout<<" setLastTwoBitZero 3 "<<endl;
    setLastTwoBitZero();
    
    // getting fifth and sixth bit of RGB to set in a cover image pixmap
    r3 = r3<<4;
    r3 = r3>>6;
    g3 = g3<<4;
    g3 = g3>>6;
    b3 = b3<<4;
    b3 = b3>>6;
    
    cout<<" send to mergeTwoBit 3 "<<" r3 "<<int(r3)<<" g3 "<<int(g3)<<" b3 "<<int(b3)<<endl;
    mergeTwoBit(r3, g3, b3);
    cout<<" merge 3 done "<<endl;
    curX++;
    
}

// The encoding function will get the first 6 bits of the secret image and store it in the last 2bit of the pixmap of cover image.
void encode(){
    int pos = 0 ;
    curY = 0;
    unsigned char r = 0, g = 0, b = 0;
    for(int i = 0; i < bgHeight; i++){
        curX = 0;
        for(int j = 0; j < bgWidth ; j++){
            
            cout<<" i "<<i<<" j "<<j<<" curX "<<curX<<" curY "<<curY<<endl;
            // getting the rgb to set in pixmap of cover image
            r =  bgImage[i* bgWidth*3+j*3+0]; // red
			g =  bgImage[i* bgWidth*3+j*3+1]; // green
			b =  bgImage[i* bgWidth*3+j*3+2]; // blue
            
            cout<<"actual r g b "<<" r "<<int(r)<<" g "<<int(g)<<" b "<<int(b)<<endl;
            // sending the RGb values to set in last 2 bit of pixmap of cover image.
            hide(r, g, b);
        }
        curY++;
    }
    cout<<" done hiding "<<endl;
}

// Decoding part start

// this function set the fifth and sixth bit of rgb values
void setFifthSixthBit(unsigned char &r, unsigned char &g, unsigned char &b){
    //cout<<"in setFifthSixthBit rgb "<<" image[curY*maskWidth*3+curX*3+0] "<< int(image[curY*maskWidth*3+curX*3+0])<<" image[curY*maskWidth*3+curX*3+1] "<<int(image[curY*maskWidth*3+curX*3+1])<<" image[curY*maskWidth*3+curX*3+2] "<<int(image[curY*maskWidth*3+curX*3+2])<<endl;
    
    // AND operation with 3 will give last 2 bit of pixmap.
    unsigned char r1 = image[curY*maskWidth*3+curX*3+0] & 3; // red
    unsigned char g1 = image[curY*maskWidth*3+curX*3+1] & 3; // green
    unsigned char b1 = image[curY*maskWidth*3+curX*3+2] & 3;
    //cout<<" gettin last 2 bit of FifthSixth "<<" r1 "<<int(r1)<<" g1 "<<int(g1)<<" b1 "<<int(b1)<<endl;
    
    // left shift of 2 will give the fifth and sixth bit of secret image.
    r1 = r1 << 2;
    g1 = g1 << 2;
    b1 = b1 << 2;
    //cout<<" making last 2 bit five 6 "<<" r1 "<<int(r1)<<" g1 "<<int(g1)<<" b1 "<<int(b1)<<endl;
    // OR operation will set fifth and sixth position
    r = r | r1;
    g = g | g1;
    b = b | b1;
    //cout<<" after last 2 bit five 6 rgb is "<<" r "<<int(r)<<" g "<<int(g)<<" b "<<int(b)<<endl;
}

// this function set the third and fourth bit of rgb values
void setThirdFourthBit(unsigned char &r, unsigned char &g, unsigned char &b){
    //cout<<"in setThirdFourthBit rgb "<<" image[curY*maskWidth*3+curX*3+0] "<< int(image[curY*maskWidth*3+curX*3+0])<<" image[curY*maskWidth*3+curX*3+1] "<<int(image[curY*maskWidth*3+curX*3+1])<<" image[curY*maskWidth*3+curX*3+2] "<<int(image[curY*maskWidth*3+curX*3+2])<<endl;
    
    // AND operation with 3 will give last 2 bit of pixmap.
    unsigned char r1 = image[curY*maskWidth*3+curX*3+0] & 3; // red
    unsigned char g1 = image[curY*maskWidth*3+curX*3+1] & 3; // green
    unsigned char b1 = image[curY*maskWidth*3+curX*3+2] & 3;
    //cout<<" gettin last 2 bit of three four "<<" r1 "<<int(r1)<<" g1 "<<int(g1)<<" b1 "<<int(b1)<<endl;
    
    // left shift of 4 will give the third and fourth bit of secret image.
    r1 = r1 << 4;
    g1 = g1 << 4;
    b1 = b1 << 4;
    //cout<<" making last 2 bit three 4 "<<" r1 "<<int(r1)<<" g1 "<<int(g1)<<" b1 "<<int(b1)<<endl;
    
    // OR operation will set third and forth bit position
    r = r | r1;
    g = g | g1;
    b = b | b1;
    //cout<<" after last 2 bit three 4 rgb is "<<" r "<<int(r)<<" g "<<int(g)<<" b "<<int(b)<<endl;
}

// this function set the first and second bit bit of rgb values
void setFirstTwoBit(unsigned char &r, unsigned char &g, unsigned char &b){
    //cout<<"in setFirstTwoBit rgb "<<" image[curY*maskWidth*3+curX*3+0] "<< int(image[curY*maskWidth*3+curX*3+0])<<" image[curY*maskWidth*3+curX*3+1] "<<int(image[curY*maskWidth*3+curX*3+1])<<" image[curY*maskWidth*3+curX*3+2] "<<int(image[curY*maskWidth*3+curX*3+2])<<endl;
    
    // AND operation with 3 will give last 2 bit of pixmap.
    unsigned char r1 = image[curY*maskWidth*3+curX*3+0] & 3; // red
    unsigned char g1 = image[curY*maskWidth*3+curX*3+1] & 3; // green
    unsigned char b1 = image[curY*maskWidth*3+curX*3+2] & 3;
    
   // cout<<" gettin last 2 bit of first2"<<" r1 "<<int(r1)<<" g1 "<<int(g1)<<" b1 "<<int(b1)<<endl;
    
    // left shift of 6 will give the first and second bit of secret image.
    r1 = r1 << 6;
    g1 = g1 << 6;
    b1 = b1 << 6;
    //cout<<" making last 2 bit first 2 "<<" r1 "<<int(r1)<<" g1 "<<int(g1)<<" b1 "<<int(b1)<<endl;
    
    // OR operation will set first and second bit position
    r = r | r1;
    g = g | g1;
    b = b | b1;
    //cout<<" after last 2 bit first 2 rgb is "<<" r "<<int(r)<<" g "<<int(g)<<" b "<<int(b)<<endl;
    
}


// This function controls the setting of appropriate bit
void reveal(unsigned char &r, unsigned char &g, unsigned char &b){
 cout<<" setFirstTwoBit called "<<endl;
 setFirstTwoBit(r,g,b);
 curX++;
 cout<<" setThirdFourthBit called "<<endl;
 setThirdFourthBit(r,g,b);
 curX++;
 cout<<" setFifthSixthBit called "<<endl;
 setFifthSixthBit(r,g,b);
 curX++;
    
}

// This function will decode the image from the encoded image pixmap
void decode(){
    int pos = 0 ;
    decodeImage = new unsigned char[decodedImageWidth * decodedImageHeight * decodedImageChannels];
    curY = 0;
    for(int i = 0; i < decodedImageWidth; i++){
        curX = 0;
        for(int j = 0; j < decodedImageHeight ; j++){
            cout<<" i "<<i<<" j "<<j<<" curX "<<curX<<" curY "<<curY<<endl;
            unsigned char r = 0, g = 0, b = 0;
            reveal(r , g ,b );
            cout<<" final rgb "<<" i "<<i<<" j "<<j<<" curX "<<curX<<" curY "<<curY<<" r "<<int(r)<<" g "<<int(g)<<" b "<<int(b)<<endl;
            cout<<"one"<<endl;
            decodeImage[i* decodedImageWidth*decodedImageChannels+j*decodedImageChannels+0] = r; // red
            cout<<"two"<<endl;
			decodeImage[i* decodedImageWidth*decodedImageChannels+j*decodedImageChannels+1] = g; // green
            cout<<"three"<<endl;
			decodeImage[i* decodedImageWidth*decodedImageChannels+j*decodedImageChannels+2] = b; // blue
            
        }
        curY++;
    }
    cout<<" done revealing "<<endl;
    
}

void readImgFromDisc(string CoverImage , string secretImage)
{
	
	// Reading the associate alphamask image.
	if(CoverImage.empty()){ // if no name provided through command line argument, ask the user to input the associate alphamask image name
		cout<<"Please enter the associate alphamask image name"<<endl;
		cin>>CoverImage;
	}
	
	ImageInput *in = ImageInput::open(CoverImage);// create handel of input imageinput this will be required to read a file
	if(!in){
		cout << "Error in opening a associate alphamask image, please try again!!";
		return;
	}
		
	const ImageSpec &spec = in->spec();// to get image specification-> width, height, channel.
    maskWidth = spec.width;
    maskHeight = spec.height;
    maskChannels = spec.nchannels;

	//allocate space for input image
	
	image = new unsigned char[maskWidth * maskHeight * maskChannels];
    cout<<"maskWidth "<<maskWidth<<"maskHeight "<<maskHeight<<"maskChannels "<<maskChannels<<endl;
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
	cout<<"image read done"<<endl;
		
	//****************************************************

	// Reading the secretImage image.
	if(secretImage.empty()){ // if no name provided through command line argument, ask the user to input the secretImage image name
		cout<<"Please enter the secretImage image name"<<endl;
		cin>>secretImage;
    }
	
		
	in = ImageInput::open(secretImage);// create handel of input imageinput this will be required to read a file
	if(!in){
		cout << "Error in opening a secretImage image, please try again!!";
		return;
	}
	
     //read image specification
	 const ImageSpec &bgspec = in->spec();
	 bgWidth = bgspec.width;
	 bgHeight = bgspec.height;
	 bgChannels = bgspec.nchannels;
	
	//allocate space for secretImage image
	
	bgImage = new unsigned char[bgWidth * bgHeight * bgChannels];
	cout<<"bgWidth "<<bgWidth<<"bgHeight "<<bgHeight<<"bgChannels "<<bgChannels<<endl;
		
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
	
    gPixels = new unsigned char[mWidth * mHeight * 4];
    
    
    /*
        as mentioned above decoding will be done just after encoding.
        the decoding function will extract the secret image from the pixmap of encoded image.
    
    */

    encode();
    decodedImageWidth = bgWidth;
    decodedImageHeight = bgHeight;
    decodedImageChannels = bgChannels;
    decode();
    
    // for display of decoded image on screen.
    for(int i=0;i < maskHeight;i++){
        for(int j=0;j < maskWidth;j++){
            gPixels[i* maskWidth*4+j*4+0]    =  image[i* maskWidth*3+j*3+0]; // red
            gPixels[i* maskWidth*4+j*4+1]    =  image[i* maskWidth*3+j*3+1]; // green
            gPixels[i* maskWidth*4+j*4+2]    =  image[i* maskWidth*3+j*3+2]; // blue
            gPixels[i* maskWidth*4+j*4+3]    =  255;   
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
	glutCreateWindow("Decoded Image");
    glutDisplayFunc(drawImage);
	glutKeyboardFunc(handleKey);
	glutReshapeFunc(handleReshape);
    // registering callback functions.
	glutDisplayFunc(drawImage);
	glutKeyboardFunc(handleKey);
	glutReshapeFunc(handleReshape);
    
    // Call to open the image given by command line argument.
	if(argc == 3){
        
		string coverImage = "";
		string secretImage = "";
        if(argv[1]){
            coverImage = argv[1];
        }

        if(argv[2]){
            secretImage = argv[2];
        }
        cout<<"hello there"<<endl;
		readImgFromDisc(coverImage, secretImage);
        string outPutEncoded = "encoded.jpg"; // this image is the one which has hidden secret image.
        writeEncodedImageWhichHasSecretImageHiden(outPutEncoded);
        string outPutExtracted = "extracted.jpg"; // this image is the one which is extracted from the encoded image.
        writeExtractedImageWhichIsDecodedFromEncodedImage(outPutExtracted);
	}
    else{
        cout<<"Please Provide the Cover Image followed by secret image through commandline argument"<<endl;
        exit(0);
    }

	//opengl mainloop
	glutMainLoop();

}