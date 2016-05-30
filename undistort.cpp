#include <fstream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv/cv.h>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc/imgproc.hpp>


int main(int argc, char const *argv[])
{

	if(argc != 4)
	{
		std::cerr << "Undistor: calibrationfile input output\n";
		return -1;
	}

	cv::Mat camera_matrix,dist_coeffs;
	cv::FileStorage camera_calibration_file(argv[1], cv::FileStorage::READ);
    if (!camera_calibration_file.isOpened())
    {
    	std::cerr << "wrong calibration file" << std::endl;
    	return -2;
    }
	camera_calibration_file["rgb_intrinsics"] >> camera_matrix;
	camera_calibration_file["rgb_distortion"] >> dist_coeffs;
	if(!camera_matrix.rows || !dist_coeffs.rows)
	{
		std::cerr << "expected rgb_intrinsics or rgb_distortion in file" << std::endl;
		return -1;
	}


    std::cout << "K:\n"<<camera_matrix << std::endl;
    std::cout << "dist:\n"<<dist_coeffs << std::endl;

    cv::Mat frame = cv::imread(argv[2]);
    if(frame.rows == 0)
    {
    	std::cerr << "wrong image file" << std::endl;
    	return -3;
    }
    cv::Mat oframe;
    cv::undistort(frame,oframe,camera_matrix,dist_coeffs);
    std::cout << "writing to " << argv[3] << " " << oframe.rows << std::endl;
    cv::imwrite(argv[3],oframe);
    return 0;
}