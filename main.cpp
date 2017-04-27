#include <Eigen/Core>
#include <fstream>
#include <sstream>
#include "json/json.h"
#include <aruco/aruco.h>
#include <aruco/cvdrawingutils.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv/cv.h>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/eigen.hpp>

void rotateXAxis(cv::Mat &rotation) {
    cv::Mat R(3, 3, CV_32F);
    cv::Rodrigues(rotation, R);
    // create a rotation matrix for x axis
    cv::Mat RX = cv::Mat::eye(3, 3, CV_32F);
    float angleRad = M_PI / 2;
    RX.at< float >(1, 1) = cos(angleRad);
    RX.at< float >(1, 2) = -sin(angleRad);
    RX.at< float >(2, 1) = sin(angleRad);
    RX.at< float >(2, 2) = cos(angleRad);
    // now multiply
    R = R * RX;
    // finally, the the rodrigues back
    cv::Rodrigues(R, rotation);
}


double calculateExtrinsics(aruco::Marker & m, float markerSizeMeters, cv::Mat camMatrix, cv::Mat distCoeff, bool setYPerpendicular,cv::Mat &Rvec,cv::Mat &Tvec)
     {
    if (!m.isValid())
        return -1;
    if (markerSizeMeters <= 0)
        return -1; 
    if (camMatrix.rows == 0 || camMatrix.cols == 0)
        return -1;

    double halfSize = markerSizeMeters / 2.;
    cv::Mat ObjPoints(4, 3, CV_32FC1);
    ObjPoints.at< float >(1, 0) = -halfSize;
    ObjPoints.at< float >(1, 1) = halfSize;
    ObjPoints.at< float >(1, 2) = 0;
    ObjPoints.at< float >(2, 0) = halfSize;
    ObjPoints.at< float >(2, 1) = halfSize;
    ObjPoints.at< float >(2, 2) = 0;
    ObjPoints.at< float >(3, 0) = halfSize;
    ObjPoints.at< float >(3, 1) = -halfSize;
    ObjPoints.at< float >(3, 2) = 0;
    ObjPoints.at< float >(0, 0) = -halfSize;
    ObjPoints.at< float >(0, 1) = -halfSize;
    ObjPoints.at< float >(0, 2) = 0;

    cv::Mat ImagePoints(4, 1, CV_32FC2);
    cv::Mat ximagePoints(4, 1, CV_32FC2);

    // Set image points from the marker
    for (int c = 0; c < 4; c++) {
        ImagePoints.at< cv::Point2f >(c, 0) = m[c];
    }

    cv::Mat raux, taux;
    bool x = cv::solvePnP(ObjPoints, ImagePoints, camMatrix, distCoeff, raux, taux);
    if(!x)
        return -2;
    std::cout << "solved\n";
    raux.convertTo(Rvec, CV_32F);
    taux.convertTo(Tvec, CV_32F);
    // rotate the X axis so that Y is perpendicular to the marker plane
    cv::projectPoints(ObjPoints,raux,taux,camMatrix,distCoeff,ximagePoints);
    std::cout << " op " << ImagePoints.rows << " " << ImagePoints.cols << " c " << ImagePoints.channels() << " t " << ImagePoints.type() << std::endl;
    std::cout << " op " << ximagePoints.rows << " " << ximagePoints.cols << " c " << ximagePoints.channels() << " t" << ximagePoints.type() << std::endl;
    std::cout << "reproject " << ImagePoints-ximagePoints << std::endl;
    //double r = 0;
    //for(int i = 0; i < 4; i++)
    //    r += cv::norm(ImagePoints.at<cv::Point2f>(i,0)-ximagePoints.at<cv::Point2f>(i,0));
    double r = cv::norm(ImagePoints-ximagePoints);
    if (setYPerpendicular)
        rotateXAxis(Rvec);
    // and now reprojection error
    return r;
}


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

void help(){
        std::cerr << "Arguments: [-xy] calibrationfile size imagefile alreadyundist [-|outfilename]\n\t-x mirror image along x\n\t-y mirror image along y\n\t-xy mirror image along xy\n";
        exit(1);
}


int main(int argc, char const *argv[])
{
	// first is camera as OpenCV
	// second is file
	// output is array of markers	
	bool y_axis_perpendicular = false;
    int mirror = 0;
    bool show = false;

    if(argc == 1)
        help();

    for(; argv[1][0] == '-'; argc--, argv++)
    {
        if(strcmp(argv[1],"-xy") == 0)
            mirror = 3;
        else if(strcmp(argv[1],"-x") == 0)
            mirror |= 1;
        else if(strcmp(argv[1],"-y") == 0)
            mirror |= 2;
        else if(strcmp(argv[1],"-s") == 0)
            show = true;
    }

	if(argc < 5)
	{
        help();
		return -1;
	}

    std::string saveframe;
    if(argc > 5)
    {
        if(strcmp(argv[5],"-") == 0)
            show = true;
        else
            saveframe = argv[5];
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

	float marker_size = atof(argv[2]);

    if(atoi(argv[4]) != 0)
    {
        // setZero
        for(int i = 0; i < dist_coeffs.cols; i++)
            dist_coeffs.at<float>(i) = 0;
    }

    std::cout << "K:"<<camera_matrix << std::endl;
    std::cout << "dist:"<<dist_coeffs << std::endl;
    std::cout << "marker_size:"<<marker_size << std::endl;

    cv::VideoCapture cap(argv[3]);
    if(!cap.isOpened()) 
    {
        std::cerr << "wrong image/video file " << argv[3] << std::endl;
        return -3;
    }


    std::vector<aruco::Marker> markers;
    aruco::MarkerDetector marker_detector;
    marker_detector.setMinMaxSize(0.01,0.1);

    cv::Mat frame;
    bool singleframe = cap.get(CV_CAP_PROP_FRAME_COUNT) == 1;

    std::ofstream onf((std::string(argv[3]) +".json").c_str());

    for(;;)
    {
        cap >> frame;
        if(frame.rows == 0)
        {
            break;
        }
        if(mirror != 0)
        {
            cv::Mat dst;
            cv::flip(frame,dst,mirror == 1 ? 0 : mirror == 2 ? 1 : -1);
            frame = dst; // ref
        }
        aruco::CameraParameters cp(camera_matrix,dist_coeffs, cv::Size(frame.cols,frame.rows));

        marker_detector.detect(
        	frame, 
        	markers, 
        	cp,
            marker_size, 
            y_axis_perpendicular);
        double pmat[16];
        cp.glGetProjectionMatrix(cp.CamSize,cp.CamSize,pmat,0.1,100,false);

        // TODO YML output for frames
        std::string xin = argv[3];
        std::unique_ptr<cv::FileStorage> recreate;
        if(singleframe)
            recreate = std::unique_ptr<cv::FileStorage>(new cv::FileStorage(xin+".yml", cv::FileStorage::WRITE));

        Json::Value jmarkers(Json::arrayValue);
        int found = 0;

        std::cout << "frame\n";

    	for (auto marker : markers)
        {
            if (!marker.isValid())
            {
                continue;
            }
            found++;
            cv::Mat Rvec,Tvec;
            auto e = calculateExtrinsics(marker,marker_size,camera_matrix,dist_coeffs,y_axis_perpendicular,Rvec,Tvec);

            marker.draw(frame, cv::Scalar(0,0,255), 2);
            aruco::CvDrawingUtils::draw3dAxis(frame, marker, cp);



            Eigen::Matrix4d marker_pose = Eigen::Matrix4d::Identity();
            marker_pose.block<3, 1>(0, 3) = Eigen::Vector3d(Tvec.at<float>(0),
                                                  Tvec.at<float>(1),
                                                  Tvec.at<float>(2));
            cv::Mat marker_rot;
            cv::Rodrigues(Rvec, marker_rot);
            marker_pose(0, 0) = marker_rot.at<float>(0, 0);
            marker_pose(0, 1) = marker_rot.at<float>(0, 1);
            marker_pose(0, 2) = marker_rot.at<float>(0, 2);
            marker_pose(1, 0) = marker_rot.at<float>(1, 0);
            marker_pose(1, 1) = marker_rot.at<float>(1, 1);
            marker_pose(1, 2) = marker_rot.at<float>(1, 2);
            marker_pose(2, 0) = marker_rot.at<float>(2, 0);
            marker_pose(2, 1) = marker_rot.at<float>(2, 1);
            marker_pose(2, 2) = marker_rot.at<float>(2, 2);

            //void eigen2cv(const Eigen::Matrix<_Tp, _rows, _cols, _options, _maxRows, _maxCols>& src, Mat& dst)
            cv::Mat marker_posecv(4,4,CV_32F);
            //marker_rot.copyTo(marker_posecv(cv::Rect(0,0,3,3)));
            //Tvec.copyTo(marker_posecv(cv::Rect(0,3,3,1)));
            cv::eigen2cv(marker_pose,marker_posecv);

            //cv::Mat cvT(4,4,CV_32FC1); 
            //Eigen::Map<Matrix4f> eigenT( cvT.data() ); 

            //std::cout << "marker mid:" << marker.id << " error:" << e << "\n\tTvec:" << Tvec << "\n\tRvec:" << Rvec << std::endl;

    	    double mat[16];
            marker.glGetModelViewMatrix(mat);
            cv::Point2f center = marker.getCenter();

            Json::Value jmarker;
            jmarker["id"] = marker.id;
            jmarker["center"][0] = center.x;
            jmarker["center"][1] = center.y;
            jmarker["error"] = e;
            jmarker["areapx"] = marker.getArea();
            jmarker["areau"] = marker.getArea()/(cp.CamSize.width*cp.CamSize.height);
            jmarker["Tvec"][0] = Tvec.at<float>(0,0);
            jmarker["Tvec"][1] = Tvec.at<float>(1,0);
            jmarker["Tvec"][2] = Tvec.at<float>(2,0);
            jmarker["Rvec"][0] = Rvec.at<float>(0,0);
            jmarker["Rvec"][1] = Rvec.at<float>(1,0);
            jmarker["Rvec"][2] = Rvec.at<float>(2,0);
            jmarker["pose"] = mat2json(marker_pose);
            jmarker["glmodelview"] = vec2json(mat,16);
            for(int q = 0; q < marker.size(); q++)
            {
                jmarker["points"][q][0] = marker[q].x;
                jmarker["points"][q][1] = marker[q].y;
            }
            jmarker["points"][(int)marker.size()][0] = center.x;
            jmarker["points"][(int)marker.size()][1] = center.y;
            // and then the center
            jmarkers.append(jmarker);
            std::cout <<" mid:" << marker.id << " error:" << e << " area:" << marker.getArea() << std::endl;


            //markerpose
            //markerid
            //markersize
            //mode
            if(recreate)
            {
                *recreate << ((std::ostringstream() << "markerid" << found ).str().c_str()) << marker.id;
                *recreate << ((std::ostringstream() << "markersize" << found  ).str().c_str()) << marker_size;
                *recreate << ((std::ostringstream() << "markerpose" << found  ).str().c_str()) << marker_posecv;
                *recreate << ((std::ostringstream() << "mode" << found ).str().c_str()) << 3;
                *recreate << ((std::ostringstream() << "corners" << found ).str().c_str()) << marker;
            }
        }

        Json::Value jroot;
        jroot["markers"] = jmarkers;
        jroot["glprojection"] = vec2json(pmat,16);
        jroot["K"] = mat2json(camera_matrix);
        jroot["dist"] = mat2json(dist_coeffs);
        jroot["markersize"] = marker_size;
        jroot["yaxisup"] = y_axis_perpendicular;
        jroot["imagesize"][0] = cp.CamSize.width;
        jroot["imagesize"][1] = cp.CamSize.height;

        // multiple frames == multiple JSON messages, one per line
        onf << Json::FastWriter().write(jroot);
        
        if(show && found)
        {
            cv::imshow("ciao",frame);
            cv::waitKey(1);
        }
        if(!saveframe.empty() && found)
        {
            // Multiframe => overwrite
            if(!singleframe)
                std::cout << "Warning: overwriting output " << saveframe << std::endl;
            cv::imwrite(saveframe.c_str(),frame);            
        }
    }
    return 0;
}