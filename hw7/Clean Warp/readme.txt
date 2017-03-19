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

 