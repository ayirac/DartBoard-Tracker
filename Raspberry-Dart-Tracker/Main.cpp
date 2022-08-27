#include <iostream>
#include <opencv2/opencv.hpp>
#include "DartBoard.h"


using namespace std;
using namespace cv;

void mouse_callback(int event, int x, int y, int flags, void* userdata)
{
	Point pt(x, y);
	DartBoard* pBoard = (DartBoard*)userdata;
	if (event == EVENT_LBUTTONDOWN) {
		pBoard->check_hit(pt);
	}
}

int main()
{
	const int max_value_H = 360 / 2;
	const int max_value = 255;
	int low_H = 11, low_S = 0, low_V = 45;
	int high_H = 165, high_S = 173, high_V = 255;
	
    string fp = "input/dartboard-edit.mp4";
	Mat frame, frame_gray, frame_dartboard_gray;
	vector<vector<Point>> contours_frame;
	vector<Vec4i> hierachy_frame;
	vector<Vec3f> bullseyes;
	DartBoard Board;

	VideoCapture cap(0);
	if (!cap.isOpened()) // Setup camera, if not plugged in use a pre-recorded video for dev
	{
		cout << "Camera not found." << endl;
		cap.open(fp);
		if (!cap.isOpened())
		{
			cout << "Video not found." << endl;
			return -1;
		}
	}
	double frames = cap.get(CAP_PROP_FRAME_COUNT);
	// Create Windows
	namedWindow("Frame", WINDOW_AUTOSIZE);
	//namedWindow("Frame Snapshot", WINDOW_AUTOSIZE);
	namedWindow("Trackbar Controls1", WINDOW_AUTOSIZE);
	namedWindow("Trackbar Controls2", WINDOW_AUTOSIZE);
	moveWindow("Frame", 700, 20);
	moveWindow("Trackbar Controls1", 325, 20);
	moveWindow("Trackbar Controls2", 20, 20);
	//setMouseCallback("Frame Snapshot", mouse_callback, &Board);

	// Create Trackbars
	int track_dist = 200, p1 = 101, p2 = 96, minR = 160, maxR = 200; // int track_dist = 200, p1 = 90, p2 = 217, minR = 191, maxR = 396; big radius
	int track_dist2 = 100, p12 = 13, p22 = 15, minR2 = 6, maxR2 = 20;
	int track_dist3 = 40, p13 = 17, p23 = 15, minR3 = 6, maxR3 = 16;
	int track_dist5 = 200, p15 = 101, p25 = 80, minR5 = 70, maxR5 = 150; // inner doubles 
	int p14 = 43, p24 = 122;
	int lines_p1 = 147, lines_p2 = 92, lines_p3 = 25;
	int edges_p1 = 242, edges_p2 = 201;
	bool first_calibration = false;
	//createTrackbar("Low H", "Trackbar Controls1", &low_H, max_value_H); // #1
	//createTrackbar("High H", "Trackbar Controls1", &high_H, max_value_H);
	//createTrackbar("Low S", "Trackbar Controls1", &low_S, max_value);
	//createTrackbar("High S", "Trackbar Controls1", &high_S, max_value);
	//createTrackbar("Low V", "Trackbar Controls1", &low_V, max_value);
	//createTrackbar("High V", "Trackbar Controls1", &high_V, max_value);

	createTrackbar("track_distL", "Trackbar Controls1", &track_dist3, 200); // #2
	createTrackbar("p1", "Trackbar Controls1", &p13, 150);
	createTrackbar("p2", "Trackbar Controls1", &p23, 150);
	createTrackbar("minR", "Trackbar Controls1", &minR3, 500);
	createTrackbar("maxR", "Trackbar Controls2", &maxR3, 600);
	createTrackbar("edge_p1", "Trackbar Controls2", &edges_p1, 400);
	createTrackbar("edge_p2", "Trackbar Controls2", &edges_p2, 400);

	createTrackbar("thresh", "Trackbar Controls2", &lines_p1, 300);
	createTrackbar("min len", "Trackbar Controls2", &lines_p2, 300);
	createTrackbar("max gap", "Trackbar Controls2", &lines_p3, 100);
	Mat gray;
	// Main loop for getting frames & performing operations
	while (true)
	{
		// Restarts the video when it ends
		double frame_pos = cap.get(CAP_PROP_POS_FRAMES);
		if (frame_pos == frames)
			cap.set(CAP_PROP_POS_AVI_RATIO, 0);
		cap >> frame;

		// Calibration pipeline //
		if (!Board.snapshotted())
			Board.calibrate_board(frame, track_dist, p1, p2, minR, maxR);
		else //if (!Board.calibrated())
		{
			while (!Board.bullseye_located(false)) // locate outer bullseye
				if (!Board.locate_bullseye(track_dist2, p12, p22, minR2, maxR2, false))
					break;

			while (!Board.bullseye_located(true)) // locate inner bullseye
				if (!Board.locate_bullseye(track_dist3, p13, p23, minR3, maxR3, true))
					break;

			while (!Board.triples_located()) // locate triples	
				if (!Board.locate_triples(p14, p24))
					break;

			while (!Board.doubles_located()) // locate doubles	
				if (!Board.locate_doubles(p14, p24))
					break;

			if (!Board.segment_lines_calibrated())
				gray = Board.locate_singles(lines_p1, lines_p2, lines_p3, edges_p1, edges_p2);

			
			// locate singles
		}

		// Display & Update windows //
		if (!Board.snapshotted())
			imshow("Frame", Board.get_frame_circles());
		else
		{
			imshow("Frame Snapshot", Board.get_frame_segments());
			imshow("Frame Doubles", gray);


			if (!first_calibration) // move windows into position on first cycle
			{
				first_calibration = true;
				moveWindow("Frame Snapshot", 20, 330);
				moveWindow("Frame Doubles", 365, 330);
				setMouseCallback("Frame Snapshot", mouse_callback, &Board);
			}
		}
			

		// Keybinds // 
		int key_code = waitKey(20);
		if (key_code == 32) 							// 'Space' to snapshot the DartBoard during Calibration
		{
			if (!Board.snapshotted())
				Board.take_snapshot(frame);
			else if (!Board.segment_lines_calibrated()) // calibrate singles, if 20 & space... then assign the lines to segments
 				Board.lock_in_segment_lines();
			else
			{
				// take a picture of a dart & show it
				Mat dart_frame = Board.check_darts(frame);
				imshow("Darts", dart_frame);
			}

		}
		else if (key_code == 102 || key_code == 707)	// 'f' to reset the DartBoard Calibration
			Board.new_snapshot();
		else if (key_code == 27)						// 'esc' to exit the program
			break;

	}
}