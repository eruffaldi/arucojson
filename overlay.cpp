#include <Eigen/Core>
#include <fstream>
#include <json/json.h>
#include <aruco.h>
#include <cvdrawingutils.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv/cv.h>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc/imgproc.hpp>

Json::Value mat2json(cv::Mat m)
{
    Json::Value r;
    for(int i = 0; i < m.rows; i++)
    {
        Json::Value q;
      for(int j = 0; j < m.cols; j++)
            q[j] = m.at<double>(i,j);
        r[i] = q;
    }
    return r;
}

Json::Value vec2json(double * p, int n)
{
    Json::Value r;
    for(int i = 0; i < n; i++)
        r[i] = p[i];
    return r;
}

Json::Value mat2json1(Eigen::Matrix4d m)
{
    Json::Value r;
    double *v_ptr = m.data();
    for (int i = 0; i < 16; ++i)
        r.append(Json::Value(v_ptr[i]));
    return r;
}

Json::Value mat2json(const Eigen::Matrix4d & m)
{
    Json::Value r;
    for(int i = 0; i < m.rows(); i++)
    {
        Json::Value q;
      for(int j = 0; j < m.cols(); j++)
            q[j] = m(i,j);
        r[i] = q;
    }    
    return r;
}


int main(int argc, char const *argv[])
{
	// first is camera as OpenCV
	// second is file
	// output is array of markers	
	bool y_axis_perpendicular = false;

	if(argc < 5)
	{
		std::cerr << "Expected: calibrationfile.yaml command.yaml imagefile dist [outfile|-]\n";
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

    std::cout << "K:\n" <<camera_matrix << std::endl;
    std::cout << "dist:\n" << dist_coeffs << std::endl;

    cv::Mat frame = cv::imread(argv[3]);
    if(frame.rows == 0)
    {
    	std::cerr << "cannot open image:" << argv[3]<<std::endl;
    	return -3;
    }

    if(atoi(argv[4]) != 0)
    {
        // setZero
        for(int i = 0; i < dist_coeffs.cols; i++)
            dist_coeffs.at<float>(i) = 0;
    }


    aruco::CameraParameters cp(camera_matrix,dist_coeffs, cv::Size(frame.cols,frame.rows));

    cv::FileStorage overlay(argv[2], cv::FileStorage::READ);


    for(int i = 0; i < 10; i++)
    {
        aruco::Marker marker;
        char name[128];
        sprintf(name,"markerpose%d",i);
        cv::Mat pose;
        overlay[name] >> pose;
        if(pose.rows == 0)
            continue;
        int id;
        sprintf(name,"markerid%d",i);
        overlay[name] >> id;
        std::cout << "found marker " << i << " with id " << id << " at " << pose << std::endl;
        sprintf(name,"markersize%d",i);
        double size = 1.0;
        overlay[name] >> size;
        sprintf(name,"mode%d",i);
        int flag = 0;
        overlay[name] >> flag;

        marker.id = id;
        marker.ssize = size;
        marker.resize(4);

        // decompose the matrix? not needed
        // marker.Rvec = 
        marker.Tvec.create(3, 1, CV_32FC1);
        marker.Rvec.create(3, 1, CV_32FC1);
        marker.Tvec.at<float>(0,0) = pose.at<double>(0,3);
        marker.Tvec.at<float>(1,0) = pose.at<double>(1,3);
        marker.Tvec.at<float>(2,0) = pose.at<double>(2,3);
        cv::Rodrigues(pose(cv::Rect(0,0,3,3)),marker.Rvec);

/*
void MarkerDetector::distortPoints(vector< cv::Point2f > in, vector< cv::Point2f > &out, const Mat &camMatrix, const Mat &distCoeff) {
    // trivial extrinsics
    cv::Mat Rvec = cv::Mat(3, 1, CV_32FC1, cv::Scalar::all(0));
    cv::Mat Tvec = Rvec.clone();
    // calculate 3d points and then reproject, so opencv makes the distortion internally
    vector< cv::Point3f > cornersPoints3d;
    for (unsigned int i = 0; i < in.size(); i++)
        cornersPoints3d.push_back(cv::Point3f((in[i].x - camMatrix.at< float >(0, 2)) / camMatrix.at< float >(0, 0), // x
                                              (in[i].y - camMatrix.at< float >(1, 2)) / camMatrix.at< float >(1, 1), // y
                                              1)); // z
    cv::projectPoints(cornersPoints3d, Rvec, Tvec, camMatrix, distCoeff, out);
}

cv::undistortPoints(contour2f, contour2f, camMatrix, distCoeff, cv::Mat(), camMatrix);

*/

    cv::Mat ImagePoints(4, 1, CV_32FC2, cv::Scalar::all(0));
        std::vector<cv::Point3f> corners;
        corners.push_back(cv::Point3f(size/2,size/2,0));
        corners.push_back(cv::Point3f(size/2,-size/2,0));
        corners.push_back(cv::Point3f(-size/2,-size/2,0));
        corners.push_back(cv::Point3f(-size/2,size/2,0));
        std::cout << "T  " << marker.Tvec << std::endl;
        std::cout << "R  " << marker.Rvec << std::endl;
        std::cout << "C  " << corners << std::endl;
        std::cout << "K  " << camera_matrix << std::endl;
        std::cout << "DI " << dist_coeffs << std::endl;
        cv::projectPoints(corners,marker.Rvec,marker.Tvec,camera_matrix,dist_coeffs,ImagePoints);
        std::cout << "IP " << ImagePoints << std::endl;
        for(int j = 0; j < 4; j++)
        {
            marker[j].x = ImagePoints.at<cv::Point2f>(j).x;
            marker[j].y = ImagePoints.at<cv::Point2f>(j).y;
        }
        if(flag & 1)
        {
            marker.draw(frame, cv::Scalar(0,0,255), 2);
        }
        if(flag & 2)
        {
            aruco::CvDrawingUtils::draw3dAxis(frame, marker, cp);
        }

    }
    if(argc < 6 || strcmp(argv[5],"-") == 0)
    {
        cv::imshow("ciao",frame);
        cv::waitKey(0);
    }
    else
        cv::imwrite(argv[5],frame);
    return 0;
}