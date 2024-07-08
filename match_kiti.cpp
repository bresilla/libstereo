#include "stereomatch.h"
#include <boost/thread/thread.hpp>
#include <iostream>
#include <filesystem>
#include <vector>
#include <string>

namespace fs = std::filesystem;

class ImageLoader {
public:
    ImageLoader(const std::string& right_folder, const std::string& left_folder)
        : right_folder_path(right_folder), left_folder_path(left_folder), right_index(0), left_index(0) {
        loadImages(right_folder, right_images);
        loadImages(left_folder, left_images);
    }

    std::pair<std::string, std::string> getNextImages() {
        if (right_index >= right_images.size() || left_index >= left_images.size()) {
            return {"", ""}; // No more images
        }

        std::string right_image = right_images[right_index++];
        std::string left_image = left_images[left_index++];

        return {right_image, left_image};
    }

private:
    std::string right_folder_path;
    std::string left_folder_path;
    std::vector<std::string> right_images;
    std::vector<std::string> left_images;
    size_t right_index;
    size_t left_index;

    void loadImages(const std::string& folder_path, std::vector<std::string>& images) {
        for (const auto& entry : fs::directory_iterator(folder_path)) {
            if (entry.is_regular_file()) {
                images.push_back(entry.path().string());
            }
        }
    }
};

#define FRAME_H 240
#define FRAME_W 320

int startx, starty, endx, endy;
bool drawing = false;
Mat disp, img1, img2;
StereoBlockMatcher* sm;

static void onMouse(int event, int x, int y, int flags, void*);

boost::shared_ptr<pcl::visualization::PCLVisualizer> createVisualizer (pcl::PointCloud<pcl::PointXYZRGB>::ConstPtr cloud) {
    boost::shared_ptr<pcl::visualization::PCLVisualizer> viewer (new pcl::visualization::PCLVisualizer ("3D Viewer"));
    viewer->setBackgroundColor (255, 255, 255);
    pcl::visualization::PointCloudColorHandlerRGBField<pcl::PointXYZRGB> rgb(cloud);
    viewer->addPointCloud<pcl::PointXYZRGB> (cloud, rgb, "reconstruction");
    viewer->addCoordinateSystem ( 1.0 );
    viewer->initCameraParameters ();
    return viewer;
}




int main(int argc, char** argv) {
    std::string right_folder = "/dat/KITI/odometry/dataset/sequences/00/image_0";
    std::string left_folder = "/dat/KITI/odometry/dataset/sequences/00/image_1";

	ImageLoader loader(right_folder, left_folder);

	namedWindow("disparity", WINDOW_AUTOSIZE);
	setMouseCallback("disparity", onMouse);
	sm = new StereoBlockMatcher();

	pcl::PointCloud<pcl::PointXYZRGB>::Ptr point_cloud_ptr (new pcl::PointCloud<pcl::PointXYZRGB>);
	boost::shared_ptr<pcl::visualization::PCLVisualizer> viewer;
    	viewer = createVisualizer( point_cloud_ptr );

	while (1) {
		auto [right_image, left_image] = loader.getNextImages();
		if (right_image.empty() || left_image.empty()) {
			break;
		}

		img1 = imread(right_image, IMREAD_COLOR);
		img2 = imread(left_image, IMREAD_COLOR);
		disp = sm -> getDisparity(img1, img2);
	
		sm -> reproject(disp, img1, point_cloud_ptr);
		cout << "PointCloud size: "<< point_cloud_ptr->size()<<"\n";
		imshow("left", img1);
		imshow("right", img2);
		imshow("disparity", disp);

		pcl::visualization::PointCloudColorHandlerRGBField<pcl::PointXYZRGB> rgb(point_cloud_ptr);
		viewer->updatePointCloud<pcl::PointXYZRGB> (point_cloud_ptr, rgb, "reconstruction");
		viewer->spinOnce();
   	
		int k = waitKey(0);
		if (k == 27) break;
	}
	destroyAllWindows();
	delete sm;	

	return 0;
}

static void onMouse(int event, int x, int y, int flags, void*) {
	if (event==EVENT_LBUTTONDOWN) {
		drawing = true;
		startx = x; starty = y;
	}
	else if (event == EVENT_LBUTTONUP) {
		drawing = false;
		endx = x; endy = y;
		Mat cropped = disp(Rect(startx, starty, abs(startx-endx), abs(starty-endy)));
		cout << "mean: " << mean(cropped)[0] <<"\n";
		rectangle(disp, Point(startx, starty), Point(x, y), Scalar(0, 0, 255), 1);
		imshow("disparity", disp);
		sm->getDepth(startx, starty, endx, endy);
	}
}