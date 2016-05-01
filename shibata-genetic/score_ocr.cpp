/*
 * score_ocr.cpp
 * Get blue (first) player score from a image through color range analysis and
 * run optical character recognition.
 */

#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/text/ocr.hpp>

int main(int argc, char **argv) {
	cv::Mat image;

	if (argc < 2) {
		std::cout << "Usage: " << argv[0] << " target_image\n";
		return -1;
	}

	image = cv::imread(argv[1], CV_LOAD_IMAGE_COLOR);
	if (!image.data) {
		std::cout << "Failed to load image\n";
		return -1;
	}

	// Crop score section.
	image = image(cv::Rect(165, 37, 25, 200));

	// Mask only blue player score.
	cv::inRange(image, cv::Scalar(170, 80, 0), cv::Scalar(255, 190, 10), image);

	// Morphological closing.
	cv::dilate(image, image, getStructuringElement(cv::MORPH_RECT, cv::Size(2, 2)));
	cv::erode(image, image, getStructuringElement(cv::MORPH_RECT, cv::Size(2, 2)));

	// Get blue selection and invert image.
	image = ~image(boundingRect(image) + cv::Size(4, 4) - cv::Point(2, 2));

#ifdef DEBUG
	cv::namedWindow("Masked score", CV_WINDOW_AUTOSIZE);
	cv::imshow("Masked score", image);
	cv::waitKey(-1);
#endif

	// Run OCR.
	// Options: Tessdata = default, language = default (english), chars,
	// OCR engine modes = default, page segmentation mode = single line.
	cv::Ptr<cv::text::OCRTesseract> ocr =
		cv::text::OCRTesseract::create(NULL, NULL, "0123456789-", 3, 7);

	std::string out_text;
	ocr->run(image, out_text);

	// Remove extra lines.
	std::string number_text(out_text.begin(), std::find(out_text.begin(), out_text.end(), '\n'));
	std::cout << number_text << "\n";

	return 0;
}
