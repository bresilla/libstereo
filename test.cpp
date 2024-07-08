#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <algorithm>

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
        try {
            if (!fs::exists(folder_path) || !fs::is_directory(folder_path)) {
                throw std::runtime_error("Folder does not exist or is not a directory");
            }

            std::vector<std::string> temp_images;
            for (const auto& entry : fs::directory_iterator(folder_path)) {
                if (entry.is_regular_file()) {
                    temp_images.push_back(entry.path().string());
                }
            }
            
            // Sort the images by name
            std::sort(temp_images.begin(), temp_images.end());

            // Transfer sorted images to the main vector
            images = std::move(temp_images);

        } catch (const std::exception& e) {
            std::cerr << "Error loading images from " << folder_path << ": " << e.what() << std::endl;
        }
    }
};


int main(int argc, char** argv) {
    std::string right_folder = "/dat/KITI/odometry/dataset/sequences/00/image_0";
    std::string left_folder = "/dat/KITI/odometry/dataset/sequences/00/image_1";

	ImageLoader loader(right_folder, left_folder);
	

    while (1) {
        auto [right_image, left_image] = loader.getNextImages();
		if (right_image.empty() || left_image.empty()) {
			break;
		}
        cv::Mat img1 = cv::imread(right_image, cv::COLOR_BGR2GRAY);
        cv::Mat img2 = cv::imread(left_image, cv::COLOR_BGR2GRAY);
        cv::Mat disparity;

        int numDisparities = 64;
        int blockSize = 9;
        // cv::Ptr<cv::StereoBM> stereo = cv::StereoBM::create(numDisparities, blockSize);
        // stereo->setPreFilterType(cv::StereoBM::PREFILTER_XSOBEL);
        // stereo->setPreFilterSize(9);
        // stereo->setPreFilterCap(31);
        // stereo->setTextureThreshold(10);
        // stereo->setUniquenessRatio(15);
        // stereo->setSpeckleRange(32);
        // stereo->setSpeckleWindowSize(100);
        // stereo->setMinDisparity(0);

        cv::Ptr<cv::StereoSGBM> stereo = cv::StereoSGBM::create(
            0, // minDisparity
            numDisparities,
            blockSize,
            8*blockSize*blockSize,
            32*blockSize*blockSize,
            2, // P1 (control parameter for disparity smoothness)
            63, // P2 (control parameter for disparity smoothness)
            10, // disp12MaxDiff
            100, // preFilterCap
            32, // uniquenessRatio
            true // mode
        );


        stereo->compute(img1, img2, disparity);

        cv::Mat disparity8U;
        cv::normalize(disparity, disparity8U, 0, 255, cv::NORM_MINMAX, CV_8U);
        
        cv::imshow("Disparity", disparity8U);
        imshow("Right", img1);
        imshow("Left", img2);

        int k = cv::waitKey(0);
        if (k==27 || k == int('q')) {
            break;
        }
    }
    
	return 0;
}