
/****
@Author = Pratyush Singh
* This is a program of hiding a secret image in cover image. The method is called steganography.
* The purpose of this program is to read the given 2 image and hide the complete second image in the last 2 bit of pixmap of first image.
* Note to function this program properly it is necessary that the second image is 3 times smaller than the first image.
* 1. give file names to read as commandline argument as, ./stagnography coverImage secretImage.
* 2. Note there was a issue as openimageIO was missing last bits in reading and writing, i have encoded the image and decoded the image in one go.
* 3. when decoding is done after encoding, openimageIO do not come in picture. but when decoded image is read from the file there is a problem.

two seperate function for writing the image is used one for writing encoded image but since openimageIo is loosing last bit so decoding of encoded  image is also done after doing encoding.
so one function is for writing encoded image just to explain how encoded image looks after hiding the secret image in it.
second function is for writing the decoded image (extarcted).
two seperate function is used to make it easier for the grader to understand which function is writing which pixmap.
and how that pixmap is filled. in the writeEncodedImageWhichHasSecretImageHiden() encoded image is written and that pixmap is filled in encoded image.
in writeExtractedImageWhichIsDecodedFromEncodedImage() extarcted image is writen and the pixmap it use is filled in decoded function where it
extract the the image from last two bit of the encoded image pixmap.

in this program first six bit of the secret image is used to hide it in the cover image

to run the program give command.

./stagnography coverImage secretImage.

as a result to image will be created encoded.jpg and extracted.jpg

encoded.jpg is the one which has secret image hidden in it.
extracted.jpg is the one which is extracted from the encoded image pixmap.

For testing this program two given images {tiger.jpg , penguin.jpg } can be used to hide the secret image.
for the secret image the given cat images can be used.

any other image can also be used but the secret image should be 3 times smaller than cover image.

Test result are present in resultofencodedimage and resultofextractedImage folder.

resultofencodedimage folder has images which are encoded they have hidden secret images.

resultofextractedImage folder has images which are extracted from the encoded images present in resultofencodedimage by decoding algorithm
****/

 