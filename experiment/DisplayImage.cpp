#include <stdio.h>
#include <opencv2/opencv.hpp>
using namespace cv;
int main(int argc, char** argv )
{
    if ( argc != 4 )
    {
        printf("usage: DisplayImage.out <Calibration_Data_Path> <Image_Path> <Output_Path>\n");
        return -1;
    }
    char* imageName = argv[2];
    char* calibDataFileName = argv[1];
    char* outputName = argv[3];
    FileStorage fs(calibDataFileName, FileStorage::READ);
    if( !fs.isOpened() )
    {
        printf( " No calibration data \n " );
        return -1;
    }
    Mat cameraMatrix, distCoeffs;

    fs["camera_matrix"] >> cameraMatrix;
    fs["distortion_coefficients"] >> distCoeffs;

    Mat image;

    image = imread( imageName, IMREAD_COLOR );
    if( !image.data )
    {
        printf( " No image data \n " );
        return -1;
    }

    Mat corrected_image;
    undistort(image, corrected_image, cameraMatrix, distCoeffs);
    Mat cropped_image(corrected_image, Rect(0, 315, 1820, 1629));
    imwrite( outputName, cropped_image );
    return 0;
}
