@Author Pratyush Singh.

In this project we have 2 programs which needs to be run one after the other. The first one is alphamask and second is compose.
alphamask program can be found in alphamask folder, where as compose program can be found in compose folder.
The output of alphamask program is input to the compose program. 
Compose program also requires one background image as input.

Please follow below steps to run and test the applications.

1. In alphamask folder first build the program through make file present in the folder.

2. Run the program by giving image name through command line argument. (in our case it will be only dhouse.png present in alphamask folder)

3. If in case you don't provide the image name through command line argument, you can still press r or R and enter the image name.

4. after step 2 or 3 you can see the associated image on the screen.

5. press w or W to write the image in the same folder(alphamask) by giving the file name.

6. copy this new associated image from alphamask folder to compose folder

7. In compose folder build the program through make file present in the folder.

8. Run the program by giving associated image file name(out put of alphamask program) and background file through command line argument.
NOTE: The first argument has to be name of alphamask associated image, second argument has to be name of background image. 
			I have saved few background image in the compose folder.
			
9. If in case you don't provide the commandline argument, just like alphamask program, you can still press r or R and inter the image name.
		when you press r or R first the program will prompt you to enter the name of alphamask associated image then it will ask for background image.
		
10. After steps 8 or 9 you will be able to see composite image on screen.

11. press w or W, to write the image to the same folder by giving the name of the file.