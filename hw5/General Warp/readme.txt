@Author Pratyush Singh.

In this project we have 2 programs. The first one is warp and second is tile. both are independent program.
warp is a program which simpley takes input image name through commandline argument and warp it.
tile is a program which takes row and column for tile image and image name through commandline argument and perform tile operation on it.

Testing is done through dhouse.png for both the program.
for tile program i have also used one-tile.jpg to test the program.

Steps to run the programs.

to run warp program

1. build the project through make file.
2. Then use command inimage.png [outimage.png]optional. example (./warp dhouse.png del1.png).
3. select the warp image window and then press W or w to write the image (if not provided the optional [output file name]).


to run tile program

1. build the project through make file [make tile].
2. Then use command ./tile [numer of row] [number of column] [input file name] [output file name]Optional. example (./tile 3 2  dhouse.png del4.png).
3. select the warp image window and then press W or w to write the image (if not provided the optional [output file name]).


warp program is in warp folder
tile program is in tile folder
The test results are present in output folder within each warp and tile.

 