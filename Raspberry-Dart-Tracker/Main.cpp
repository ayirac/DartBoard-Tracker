#include <iostream>
#include <opencv2/opencv.hpp>
#include "DartBoard.h"


struct CircleParams
{
	int dist, p1, p2, minR, maxR;
	CircleParams() : dist(1), p1(1), p2(1), minR(1), maxR(1) {};
	CircleParams(int d, int p1, int p2, int min, int max) : dist(d), p1(p1), p2(p2), minR(min), maxR(max) {};
};

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
	namedWindow("Trackbar Controls1", WINDOW_AUTOSIZE);
	namedWindow("Trackbar Controls2", WINDOW_AUTOSIZE);
	moveWindow("Frame", 700, 20);
	moveWindow("Trackbar Controls1", 325, 20);
	moveWindow("Trackbar Controls2", 20, 20);

	// old values
	int lines_p1 = 147, lines_p2 = 92, lines_p3 = 25;
	int edges_p1 = 242, edges_p2 = 201;
	bool first_calibration = false;

	// Create Trackbar
	CircleParams params[8];
	params[0] = CircleParams(200, 101, 96, 160, 200); // snap
	params[1] = CircleParams(100, 13, 15, 6, 20); // bulls-o
	params[2] = CircleParams(61, 38, 15, 3, 16); // bulls-i
	params[3] = CircleParams(61, 54, 16, 102, 131); // triple-o
	params[4] = CircleParams(56, 52, 31, 92, 114); // triple-i
	params[5] = CircleParams(137, 161, 94, 156, 200); // double-o
	params[6] = CircleParams(122, 59, 125, 75, 220); // double-i
	int *p_dist = new int, *p_p1 = new int, *p_p2 = new int, *p_minR = new int, *p_maxR = new int;
	int *e_p1 = new int, *e_p2 = new int;
	*p_dist = params[0].dist;
	*p_p1 = params[0].p1;
	*p_p2 = params[0].p2;
	*p_minR = params[0].minR;
	*p_maxR = params[0].maxR;
	*e_p1 = edges_p1;
	*e_p2 = edges_p2;
	bool loaded = true;
	//createTrackbar("Low H", "Trackbar Controls1", &low_H, max_value_H); // #1
	//createTrackbar("High H", "Trackbar Controls1", &high_H, max_value_H);
	//createTrackbar("Low S", "Trackbar Controls1", &low_S, max_value);
	//createTrackbar("High S", "Trackbar Controls1", &high_S, max_value);
	//createTrackbar("Low V", "Trackbar Controls1", &low_V, max_value);
	//createTrackbar("High V", "Trackbar Controls1", &high_V, max_value);

	createTrackbar("track_distL", "Trackbar Controls1", p_dist, 200); // #2
	createTrackbar("p1", "Trackbar Controls1", p_p1, 200);
	createTrackbar("p2", "Trackbar Controls1", p_p2, 200);
	createTrackbar("minR", "Trackbar Controls1", p_minR, 500);
	createTrackbar("maxR", "Trackbar Controls1", p_maxR, 600);

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
		int state = Board.get_state();
		if (!loaded)
		{
			*p_dist = params[state].dist;
			*p_p1 = params[state].p1;
			*p_p2 = params[state].p2;
			*p_minR = params[state].minR;
			*p_maxR = params[state].maxR;
			loaded = true;
		}
		if (Board.get_state() == 0)
			Board.calibrate_board(frame, *p_dist, *p_p1, *p_p2, *p_minR, *p_maxR);
		else if (Board.get_state() == 1) // locate outer bullseye
						Board.locate_bullseye(*p_dist, *p_p1, *p_p2, *p_minR, *p_maxR, false);
		else if (Board.get_state() == 2)  // locate inner bullseye
			Board.locate_bullseye(*p_dist, *p_p1, *p_p2, *p_minR, *p_maxR, true);
		else if (Board.get_state() == 3)  // locate triples
			Board.locate_triples(*e_p1, *e_p2, *p_dist, *p_p1, *p_p2, *p_minR, *p_maxR, false);
		else if (Board.get_state() == 4)
			Board.locate_triples(*e_p1, *e_p2, *p_dist, *p_p1, *p_p2, *p_minR, *p_maxR, true);
		else if (Board.get_state() == 5) // locate doubles
			Board.locate_doubles(*e_p1, *e_p2, *p_dist, *p_p1, *p_p2, *p_minR, *p_maxR, false);
		else if (Board.get_state() == 6)
			Board.locate_doubles(*e_p1, *e_p2, *p_dist, *p_p1, *p_p2, *p_minR, *p_maxR, true);
		else if (Board.get_state() == 7) // locate segments
			gray = Board.locate_singles(lines_p1, lines_p2, lines_p3, edges_p1, edges_p2);

		// Display & Update windows //
		if (Board.get_state() == 0)
			imshow("Frame", Board.get_frame_circles());
		else
		{
			imshow("Frame Snapshot", Board.get_frame_segments());
			if (Board.get_state() == 7)
				imshow("Frame Doubles", gray);


			if (!first_calibration) // move windows into position on first cycle
			{
				first_calibration = true;
				moveWindow("Frame Snapshot", 20, 330);
				if (Board.get_state() == 6)
					moveWindow("Frame Doubles", 365, 330);
				setMouseCallback("Frame Snapshot", mouse_callback, &Board);
			}
		}
			

		// Keybinds // 
		int key_code = waitKey(20);
		if (key_code == 32) 							// 'Space' to snapshot the DartBoard during Calibration
		{
			loaded = false;
			if (Board.get_state() == 0)
				Board.take_snapshot(frame);
			else if (Board.get_state() == 1)
				Board.set_state(2);
			else if (Board.get_state() == 2)
				Board.set_state(3);
			else if (Board.get_state() == 3) 
				Board.set_state(4);
			else if (Board.get_state() == 4) 
				Board.set_state(5);
			else if (Board.get_state() == 5)
				Board.set_state(6);
			else if (Board.get_state() == 6)
				Board.set_state(7);
			else if (Board.get_state() == 7)
				Board.lock_in_segment_lines();
			else if (Board.get_state() == 8)
			{
				// take a picture of a dart & show it
				Mat dart_frame = Board.check_darts(frame);
				imshow("Darts", dart_frame);
			}

		}
		else if (key_code == 102 || key_code == 707) { // 'f' to reset the DartBoard Calibration
			Board.set_state(0);
			loaded = false;
		}
		else if (key_code == 27)						// 'esc' to exit the program
			break;

	}
}