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
	bool live = true;
	const int max_value_H = 360 / 2;
	const int max_value = 255;
	string fp = "input/new.mp4"; //dartboard-edit
	vector<vector<Point>> contours_frame;
	vector<Vec4i> hierachy_frame;
	vector<Vec3f> bullseyes;
	DartBoard Board;

	VideoCapture cap(0);
	if (!cap.isOpened()) // Setup camera, if not plugged in use a pre-recorded video for dev
	{
		cout << "Camera not found." << endl;
		live = false;
		cap.open(fp);
		if (!cap.isOpened())
		{
			cout << "Video not found." << endl;
			return -1;
		}
	}
	
	// Create Windows
	cap >> Board.get_cam_frame(); // Read one frame to get metadata
	double frames = cap.get(CAP_PROP_FRAME_COUNT);
	namedWindow("Frame", WINDOW_NORMAL);
	namedWindow("Frame Calib", WINDOW_NORMAL);
	resizeWindow("Frame", Board.get_cam_frame().cols / 3, Board.get_cam_frame().rows / 3);
	resizeWindow("Frame Calib", Board.get_cam_frame().cols / 3, Board.get_cam_frame().rows / 3);
	moveWindow("Frame", 0, 0);
	moveWindow("Frame Calib", Board.get_cam_frame().cols / 3, 0);
	namedWindow("Trackbar Controls", WINDOW_AUTOSIZE);
	resizeWindow("Trackbar Controls", 5, 5);
	moveWindow("Trackbar Controls", 5, 5);
	bool first_calibration = false;

	// Load profiles for calibration values
	char eater;
	int prof = 0, dist;
	bool corrupt_profile = false;
	ifstream profiles("profile.dat");
	ThreshParams profile_thresh_params[5]; // thresh, lhsv, hhsv, warps
	CircleParams profile_params[50][7];
	LinesParams profile_line_params[5];
	if (profiles.good())
	{
		int n = 0; // current profile


		while (!profiles.eof() && !corrupt_profile) // Loads all profiles into an array
		{
			int numberParams = 0;
			// Loads THRESH params
			profiles >> profile_thresh_params[n].thresh >> eater >> profile_thresh_params[n].lowH >> eater >> profile_thresh_params[n].lowS >> eater >> profile_thresh_params[n].lowV >> eater
				>> profile_thresh_params[n].highH >> eater >> profile_thresh_params[n].highS >> eater >> profile_thresh_params[n].highV >> eater >> profile_thresh_params[n].warpX >> eater >> profile_thresh_params[n].warpY;
			// Loads CIRCLE params
			for (int i = 0; i < 7; i++)
			{
				profiles >> dist >> eater; // Check for invalid profile #999 & set default values
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
			} // Loads the LINE params
			profiles >> dist >> eater; // Check for invalid profile #999 & set default values
			if (dist == 999) {
				cout << "Profile not complete, configure manually.\n";
				for (int i = 0; i < 8 - numberParams; i++)
					profile_params[0][i].dist = 80, profile_params[0][i].p1 = 100, profile_params[0][i].p2 = 120, profile_params[0][i].minR = 100, profile_params[0][i].maxR = 200;
				profile_line_params[n - 1].p1 = 100, profile_line_params[n - 1].p2 = 100, profile_line_params[n - 1].p3 = 100, profile_line_params[n - 1].e_p1 = 80, profile_line_params[n - 1].e_p2 = 80;
				corrupt_profile = true;
				break;
			}
			profile_line_params[n].p1 = dist;
			profiles >> profile_line_params[n].p2 >> eater;
			profiles >> profile_line_params[n].p3 >> eater;
			profiles >> profile_line_params[n].e_p1 >> eater;
			profiles >> profile_line_params[n].e_p2;
			n++;
		}
		profiles.close();

		if (n != 1)
		{
			do
			{
				cout << "Choose between profile 0 and profile " << n-1 << ".\n";
				cin >> prof;
			} while (prof > n || prof < 0);
		}
		cout << "Loaded profile " << prof << endl;

	}
	else { // Set default values if no profile file exists
		profile_thresh_params[0].thresh = 20, profile_thresh_params[0].lowH = 100, profile_thresh_params[0].lowS = 100, profile_thresh_params[0].lowV = 100,
			profile_thresh_params[0].highH = 100, profile_thresh_params[0].highS = 100, profile_thresh_params[0].highV = 100, profile_thresh_params[0].warpX = 100, profile_thresh_params[0].warpY = 100;
		for (int i = 0; i < 8; i++)
			profile_params[0][i].dist = 80, profile_params[0][i].p1 = 100, profile_params[0][i].p2 = 120, profile_params[0][i].minR = 100, profile_params[0][i].maxR = 200;
		profile_line_params[0].e_p1 = 80, profile_line_params[0].e_p2 = 80, profile_line_params[0].p1 = 100, profile_line_params[0].p2 = 120, profile_line_params[0].p3 = 120;
	}

	int* p_dist = &profile_params[prof][0].dist, * p_p1 = &profile_params[prof][0].p1, * p_p2 = &profile_params[prof][0].p2, * p_minR = &profile_params[prof][0].minR, * p_maxR = &profile_params[prof][0].maxR;

	ostringstream profile;

	// Main loop for getting frames & performing operations
	while (true)
	{
		if (!live)// Restarts the video when it ends, not working..
		{
			double frame_pos = cap.get(CAP_PROP_POS_FRAMES);
			if (frame_pos == frames)
				cap.set(CAP_PROP_POS_FRAMES, 0);
		}
		
		cap >> Board.get_cam_frame();
		Mat calib_frame;

		// Calibration pipeline //
		if (Board.get_state() == -1)
			calib_frame = Board.locate_four_corners(Board.get_cam_frame(), Scalar(profile_thresh_params[prof].lowH, profile_thresh_params[prof].lowS, profile_thresh_params[prof].lowV),
				Scalar(Scalar(profile_thresh_params[prof].highH, profile_thresh_params[prof].highS, profile_thresh_params[prof].highV)), profile_thresh_params[prof].thresh, profile_thresh_params[prof].warpX, profile_thresh_params[prof].warpY);
		else if (Board.get_state() == 0)
			calib_frame = Board.calibrate_board(profile_params[prof][0].dist, profile_params[prof][0].p1, profile_params[prof][0].p2, profile_params[prof][0].minR, profile_params[prof][0].maxR);
		else if (Board.get_state() < 7)
			calib_frame = Board.locate_boundaries(profile_params[prof]); // cont here, yada
		else if (Board.get_state() == 7) // locate segments
			calib_frame = Board.locate_singles(profile_line_params[prof].p1, profile_line_params[prof].p2, profile_line_params[prof].p3, profile_line_params[prof].e_p1, profile_line_params[prof].e_p2);

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
			resizeWindow("Trackbar Controls", 300, 600);
			moveWindow("Trackbar Controls", 0, Board.get_cam_frame().rows / 2.7);
			if (Board.get_state() == -1)
			{
				cv::createTrackbar("lowH", "Trackbar Controls", &profile_thresh_params[prof].lowH, max_value_H);
				cv::createTrackbar("lowS", "Trackbar Controls", &profile_thresh_params[prof].lowS, max_value);
				cv::createTrackbar("lowV", "Trackbar Controls", &profile_thresh_params[prof].lowV, max_value);
				cv::createTrackbar("maxH", "Trackbar Controls", &profile_thresh_params[prof].highH, max_value_H);
				cv::createTrackbar("maxS", "Trackbar Controls", &profile_thresh_params[prof].highS, max_value);
				cv::createTrackbar("maxV", "Trackbar Controls", &profile_thresh_params[prof].highV, max_value);
				cv::createTrackbar("thresh", "Trackbar Controls", &profile_thresh_params[prof].thresh, 50);
				createTrackbar("warpX", "Trackbar Controls", &profile_thresh_params[prof].warpX, 200);
				createTrackbar("warpY", "Trackbar Controls", &profile_thresh_params[prof].warpY, 200);
			}
			else if (Board.get_state() < 7)
			{
				createTrackbar("track_distL", "Trackbar Controls", p_dist, 200);
				createTrackbar("p1", "Trackbar Controls", p_p1, 200);
				createTrackbar("p2", "Trackbar Controls", p_p2, 200);
				createTrackbar("minR", "Trackbar Controls", p_minR, 500);
				createTrackbar("maxR", "Trackbar Controls", p_maxR, 600);
				
			}
			else if (Board.get_state() == 7)
			{
				createTrackbar("edge_p1", "Trackbar Controls", &profile_line_params[prof].e_p1, 400);
				createTrackbar("edge_p2", "Trackbar Controls", &profile_line_params[prof].e_p2, 400);
				createTrackbar("thresh", "Trackbar Controls", &profile_line_params[prof].p1, 300);
				createTrackbar("min len", "Trackbar Controls", &profile_line_params[prof].p2, 500);
				createTrackbar("max gap", "Trackbar Controls", &profile_line_params[prof].p3, 300);
			}
		}


		if (Board.get_state() == -1)
		{
			imshow("Frame", Board.get_frame());
		}
		else
		{
			imshow("Frame", Board.get_cam_frame());
		}

		if (Board.get_state() < 1)
			imshow("Frame Snapshot", Board.get_frame());
		else
			imshow("Frame Snapshot", Board.get_frame_segments());

		if (!first_calibration) // move windows into position on first cycle
		{
			first_calibration = true;
			moveWindow("Frame Snapshot", 850, 0);
			if (Board.get_state() == 6)
				moveWindow("Frame Doubles", 365, 330);
			setMouseCallback("Frame Snapshot", mouse_callback, &Board);
		}
		if (Board.get_state() != 8) imshow("Frame Calib", calib_frame);



		// Keybinds // 
		int key_code = waitKey(20);
		if (key_code == 32) 							// 'Space' to snapshot the DartBoard during Calibration
		{
			// Save profiles as state progresses
			if (Board.get_state() == -1) {
				profile << profile_thresh_params[prof].thresh << ',' << profile_thresh_params[prof].lowH << ',' << profile_thresh_params[prof].lowS << ',' << profile_thresh_params[prof].lowV << ','
					<< profile_thresh_params[prof].highH << ',' << profile_thresh_params[prof].highS << ',' << profile_thresh_params[prof].highV << ','
					<< profile_thresh_params[prof].warpX << ',' << profile_thresh_params[prof].warpY << '\n';
			}
			else if (Board.get_state() < 7 && Board.get_state() >= 0) {
				profile << profile_params[prof][Board.get_state()].dist << ',' << profile_params[prof][Board.get_state()].p1 << ',' << profile_params[prof][Board.get_state()].p2 << ','
					<< profile_params[prof][Board.get_state()].minR << ',' << profile_params[prof][Board.get_state()].maxR << '\n';
			}
			else if (Board.get_state() == 7) {
				profile << profile_line_params[prof].p1 << ',' << profile_line_params[prof].p2 << ',' << profile_line_params[prof].p3 << ','
					<< profile_line_params[prof].e_p1 << ',' << profile_line_params[prof].e_p2 << '\n';
			}

			// Actions based on state & set states
			if (Board.get_state() == -1) {

				Board.take_perspective_transform(profile_thresh_params[prof].warpX, profile_thresh_params[prof].warpY);
				Board.set_state(0);
			}
			else if (Board.get_state() == 0) {
				Board.take_snapshot();
				Board.set_state(1);
			}
			else if (Board.get_state() < 7) {
				Board.set_state(Board.get_state() + 1);
			}
			else if (Board.get_state() == 7) {
				Board.lock_in_segment_lines();
				Board.set_state(8);
			}
			else if (Board.get_state() == 8)
			{
				// take a picture of a dart & show it
				Mat dart_frame = Board.check_darts(profile_thresh_params[prof].warpX, profile_thresh_params[prof].warpY);
				imshow("Darts", dart_frame);
			}
			
		}
		else if (key_code == 102 || key_code == 707) { // 'f' to reset the DartBoard Calibration
			if (Board.get_state() == 8)
				Board.reset_segments();
			Board.set_state(Board.get_state() - 1);
			
		}
		else if (key_code == 27) // 'esc' to exit the program
			break;
		else if (key_code == 115) { // 's' to save the current calib values to the first available profile

			if (Board.get_state() == -1) {
				profile << profile_thresh_params[prof].thresh << ',' << profile_thresh_params[prof].lowH << ',' << profile_thresh_params[prof].lowS << ',' << profile_thresh_params[prof].lowV << ','
					<< profile_thresh_params[prof].highH << ',' << profile_thresh_params[prof].highS << ',' << profile_thresh_params[prof].highV << ','
					<< profile_thresh_params[prof].warpX << ',' << profile_thresh_params[prof].warpY << '\n';
			}
			else if (Board.get_state() < 7 && Board.get_state() >= 0) {
				profile << profile_params[prof][Board.get_state()].dist << ',' << profile_params[prof][Board.get_state()].p1 << ',' << profile_params[prof][Board.get_state()].p2 << ','
					<< profile_params[prof][Board.get_state()].minR << ',' << profile_params[prof][Board.get_state()].maxR << '\n';
			}
			else if (Board.get_state() == 7) {
				profile << profile_line_params[prof].p1 << ',' << profile_line_params[prof].p2 << ',' << profile_line_params[prof].p3 << ','
					<< profile_line_params[prof].e_p1 << ',' << profile_line_params[prof].e_p2 << '\n';
			}


			ifstream iprofile("profile.dat");
			if (iprofile.good())
			{
				// Add an extra profile, TODO
				ofstream oprofile("profile.dat", ios_base::app);
				oprofile << "\n" << profile.str();
				for (int i = 0; i < 8 - Board.get_state(); i++) // Add -, to indicate a missing field
					if (i != 7 - Board.get_state())
						oprofile << "999\n";
			}
			else {
				ofstream oprofile("profile.dat");
				oprofile << profile.str();
				for (int i = 0; i < 8 - Board.get_state(); i++) // Add -, to indicate a missing field
					oprofile << "999\n";

			}
			iprofile.close();

		}
	}
}