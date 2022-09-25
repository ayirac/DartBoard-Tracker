#include <fstream>
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
		if (pBoard->get_state() == 8)
			pBoard->check_hit(pt);
	}
}

int main()
{
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
	namedWindow("Trackbar Controls", WINDOW_AUTOSIZE);
	resizeWindow("Trackbar Controls", 5, 5);
	moveWindow("Trackbar Controls", 5, 5);
	moveWindow("Frame", 700, 20);
	bool first_calibration = false;

	// Load profiles for calibration values
	char eater;
	int prof = 0, dist;
	bool corrupt_profile = false;
	ifstream profiles("profile.dat");
	CircleParams profile_params[50][7];
	LinesParams profile_line_params[5];
	if (profiles.good())
	{
		int n = 0; // current profile
		int numberParams = 0;

		while (!profiles.eof() && !corrupt_profile) // Loads all profiles into an array
		{
			for (int i = 0; i < 7; i++)
			{
				profiles >> dist >> eater; // Check for invalid profile #999
				if (dist == 999) {
					cout << "Profile not complete, configure manually.\n";
					for (int i = 0; i < 8 - numberParams; i++)
						profile_params[0][i].dist = 80, profile_params[0][i].p1 = 100, profile_params[0][i].p2 = 120, profile_params[0][i].minR = 100, profile_params[0][i].maxR = 200;
					profile_line_params[n - 1].p1 = 100, profile_line_params[n - 1].p2 = 100, profile_line_params[n - 1].p3 = 100, profile_line_params[n - 1].e_p1 = 80, profile_line_params[n - 1].e_p2 = 80;
					corrupt_profile = true;
					break;
				}
				profile_params[n][i].dist = dist;
				profiles >> profile_params[n][i].p1 >> eater;
				profiles >> profile_params[n][i].p2 >> eater;
				profiles >> profile_params[n][i].minR >> eater;
				profiles >> profile_params[n][i].maxR;
				numberParams++;
			}
			profiles >> profile_line_params[n].p1 >> eater;
			profiles >> profile_line_params[n].p2 >> eater;
			profiles >> profile_line_params[n].p3 >> eater;
			profiles >> profile_line_params[n].e_p1 >> eater;
			profiles >> profile_line_params[n].e_p2;
			numberParams++;
			n++;
		}
		profiles.close();


		do
		{
			cout << "Choose between profile 1 and profile " << n << ".\n";
			cin >> prof;
		}
		while (prof > n || prof < 0);
		cout << "Loaded profile " << prof << endl;
		prof--;
	}
	else
	{
		for (int i = 0; i < 8; i++)
			profile_params[0][i].dist = 80, profile_params[0][i].p1 = 100, profile_params[0][i].p2 = 120, profile_params[0][i].minR = 100, profile_params[0][i].maxR = 200;
	}

	int* p_dist = &profile_params[prof][0].dist, *p_p1 = &profile_params[prof][0].p1, *p_p2 = &profile_params[prof][0].p2, *p_minR = &profile_params[prof][0].minR, *p_maxR = &profile_params[prof][0].maxR;
	Mat gray;
	ostringstream profile;
	
	// Main loop for getting frames & performing operations
	while (true)
	{

		// Restarts the video when it ends
		double frame_pos = cap.get(CAP_PROP_POS_FRAMES);
		if (frame_pos == frames)
			cap.set(CAP_PROP_POS_AVI_RATIO, 0);
		cap >> frame;

		// Calibration pipeline //
		if (Board.get_state() == 0)
			Board.calibrate_board(frame, profile_params[prof][0].dist, profile_params[prof][0].p1, profile_params[prof][0].p2, profile_params[prof][0].minR, profile_params[prof][0].maxR);
		else if (Board.get_state() < 7)
			Board.locate_boundaries(profile_params[prof]);
		else if (Board.get_state() == 7) // locate segments
			gray = Board.locate_singles(profile_line_params[prof].p1, profile_line_params[prof].p2, profile_line_params[prof].p3, profile_line_params[prof].e_p1, profile_line_params[prof].e_p2);

		// Update trackbar pointers according current step
		p_dist = &profile_params[prof][Board.get_state()].dist;
		p_p1 = &profile_params[prof][Board.get_state()].p1;
		p_p2 = &profile_params[prof][Board.get_state()].p2;
		p_minR = &profile_params[prof][Board.get_state()].minR;
		p_maxR = &profile_params[prof][Board.get_state()].maxR;
									
		// Display & Update windows, taskbars //
		if (Board.state_change())
		{
			destroyWindow("Trackbar Controls");
			namedWindow("Trackbar Controls", WINDOW_AUTOSIZE);
			resizeWindow("Trackbar Controls", 250, 300);
			moveWindow("Trackbar Controls", 325, 20);
			createTrackbar("track_distL", "Trackbar Controls", p_dist, 200);
			createTrackbar("p1", "Trackbar Controls", p_p1, 200);
			createTrackbar("p2", "Trackbar Controls", p_p2, 200);
			createTrackbar("minR", "Trackbar Controls", p_minR, 500);
			createTrackbar("maxR", "Trackbar Controls", p_maxR, 600);
			createTrackbar("edge_p1", "Trackbar Controls", &profile_line_params[prof].e_p1, 400);
			createTrackbar("edge_p2", "Trackbar Controls", &profile_line_params[prof].e_p2, 400);
			createTrackbar("thresh", "Trackbar Controls", &profile_line_params[prof].p1, 300);
			createTrackbar("min len", "Trackbar Controls", &profile_line_params[prof].p2, 300);
			createTrackbar("max gap", "Trackbar Controls", &profile_line_params[prof].p3, 100);
			
		}
				
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
			if (Board.get_state() == 0) {
				Board.take_snapshot(frame);
				Board.set_state(1);
			}
			else if (Board.get_state() < 7) {
				profile << profile_params[prof][Board.get_state()].dist << ',' << profile_params[prof][Board.get_state()].p1 << ',' << profile_params[prof][Board.get_state()].p2 << ','
					<< profile_params[prof][Board.get_state()].minR << ',' << profile_params[prof][Board.get_state()].maxR << '\n';
				Board.set_state(Board.get_state() + 1);
			}
			else if (Board.get_state() == 7) {
				profile << profile_line_params[prof].p1 << ',' << profile_line_params[prof].p2 << ',' << profile_line_params[prof].p3 << ','
					<< profile_line_params[prof].e_p1 << ',' << profile_line_params[prof].e_p2 << '\n';
				Board.lock_in_segment_lines();
				Board.set_state(8);
			}
			else if (Board.get_state() == 8)
			{
				// take a picture of a dart & show it
				Mat dart_frame = Board.check_darts(frame);
				imshow("Darts", dart_frame);
			}
			

		}
		else if (key_code == 102 || key_code == 707) { // 'f' to reset the DartBoard Calibration
			Board.set_state(0);
		}
		else if (key_code == 27) // 'esc' to exit the program
			break;
		else if (key_code == 115) { // 's' to save the current calib values to the first available profile
			ifstream iprofile("profile.dat");
			if (iprofile.good())
			{
				// Add an extra profile, TODO
				ofstream oprofile("profile.dat", ios_base::app);
				oprofile << "\n" << profile.str();
				for (int i = 0; i < 8 - Board.get_state(); i++) // Add -, to indicate a missing field
					if (i != 7 - Board.get_state())
						oprofile << "-\n";
			}
			else {
				ofstream oprofile("profile.dat");
				oprofile << profile.str();
				for (int i = 0; i < 8 - Board.get_state(); i++) // Add -, to indicate a missing field
					oprofile << "-\n";

			}
			iprofile.close();
			
		}
	}
}