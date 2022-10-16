#include <iostream>
#include <opencv2/opencv.hpp>
#include "DartBoard.h"
#include "ProfileManager.h"


using namespace std;
using namespace cv;

// Callback function for a window containing a DartBoard. If the board is calibrated, it allows for the user to click on the board to check for hits
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
	bool first_calibration = false, live = true;
	const int max_value_H = 360 / 2;
	const int max_value = 255;
	string fp = "input/1.mp4";
	Mat calib_frame;
	DartBoard board;
	ProfileManager profile_manager;

	VideoCapture cap(5);
	if (!cap.isOpened()) // Setup camera, if not plugged in use a pre-recorded video 'fp' for development
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
	cap >> board.get_cam_frame(); // Read one frame to get metadata
	double frames = cap.get(CAP_PROP_FRAME_COUNT);
	namedWindow("Frame", WINDOW_NORMAL);
	namedWindow("Frame Calib", WINDOW_NORMAL);
	namedWindow("Frame Snapshot", WINDOW_NORMAL);
	namedWindow("Trackbar Controls", WINDOW_AUTOSIZE);
	resizeWindow("Frame", board.get_cam_frame().cols / 3, board.get_cam_frame().rows / 3);
	resizeWindow("Frame Calib", board.get_cam_frame().cols / 3, board.get_cam_frame().rows / 3);
	resizeWindow("Frame Snapshot", board.get_cam_frame().cols / 3, board.get_cam_frame().rows / 3);
	resizeWindow("Trackbar Controls", 5, 5);
	moveWindow("Frame", 0, 0);
	moveWindow("Frame Calib", board.get_cam_frame().cols / 3, 0);
	moveWindow("Frame Snapshot", board.get_cam_frame().cols / 1.5, 0);
	moveWindow("Trackbar Controls", 5, 5);
	
	// Load profiles
	profile_manager.load_profile_file();

	// Main loop for getting frames & performing operations
	int frame_start = frames - frames, frame_end = frames - 0;
	cap.set(CAP_PROP_POS_FRAMES, frame_start);
	try
	{
		while (true)
		{
			if (!live)// Restarts the video when it ends, not working..
			{
				double frame_pos = cap.get(CAP_PROP_POS_FRAMES);
				if (frame_pos == frame_end)
					cap.set(CAP_PROP_POS_FRAMES, frame_start);
			}
			cap >> board.get_cam_frame();


			// Calibration pipeline //
			if (board.get_state() == -1)
				calib_frame = board.locate_four_corners(board.get_cam_frame(), Scalar(profile_manager.get_thresh().lowH, profile_manager.get_thresh().lowS, profile_manager.get_thresh().lowV),
					Scalar(Scalar(profile_manager.get_thresh().highH, profile_manager.get_thresh().highS, profile_manager.get_thresh().highV)), profile_manager.get_thresh().warpX, profile_manager.get_thresh().warpY);
			else if (board.get_state() == 0)
				calib_frame = board.calibrate_board(profile_manager.get_circle(0).dist, profile_manager.get_circle(0).p1, profile_manager.get_circle(0).p2, profile_manager.get_circle(0).minR, profile_manager.get_circle(0).maxR);
			else if (board.get_state() < 7)
				calib_frame = board.locate_boundaries(profile_manager.get_circle(board.get_state())); // cont here, yada
			else if (board.get_state() == 7) // locate segments
				calib_frame = board.locate_singles(profile_manager.get_line().p1, profile_manager.get_line().p2, profile_manager.get_line().p3, profile_manager.get_line().e_p1, profile_manager.get_line().e_p2);
			else // Capture frames to build MOG2 background to detect darts
				board.capture_MOG2(profile_manager.get_thresh().warpX, profile_manager.get_thresh().warpY);

			// Update Trackbars //
			if (board.state_change())
			{
				destroyWindow("Trackbar Controls");
				namedWindow("Trackbar Controls", WINDOW_AUTOSIZE);
				resizeWindow("Trackbar Controls", 300, 600);
				moveWindow("Trackbar Controls", 0, board.get_cam_frame().rows / 2.7);
				if (board.get_state() == -1)
				{
					cv::createTrackbar("lowH", "Trackbar Controls", &profile_manager.get_thresh().lowH, max_value_H);
					cv::createTrackbar("lowS", "Trackbar Controls", &profile_manager.get_thresh().lowS, max_value);
					cv::createTrackbar("lowV", "Trackbar Controls", &profile_manager.get_thresh().lowV, max_value);
					cv::createTrackbar("maxH", "Trackbar Controls", &profile_manager.get_thresh().highH, max_value_H);
					cv::createTrackbar("maxS", "Trackbar Controls", &profile_manager.get_thresh().highS, max_value);
					cv::createTrackbar("maxV", "Trackbar Controls", &profile_manager.get_thresh().highV, max_value);
					cv::createTrackbar("thresh", "Trackbar Controls", &profile_manager.get_thresh().thresh, 50);
					createTrackbar("warpX", "Trackbar Controls", &profile_manager.get_thresh().warpX, 200);
					createTrackbar("warpY", "Trackbar Controls", &profile_manager.get_thresh().warpY, 200);
				}
				else if (board.get_state() < 7)
				{
					createTrackbar("track_distL", "Trackbar Controls", &profile_manager.get_circle(board.get_state()).dist, 200);
					createTrackbar("p1", "Trackbar Controls", &profile_manager.get_circle(board.get_state()).p1, 200);
					createTrackbar("p2", "Trackbar Controls", &profile_manager.get_circle(board.get_state()).p2, 200);
					createTrackbar("minR", "Trackbar Controls", &profile_manager.get_circle(board.get_state()).minR, 500);
					createTrackbar("maxR", "Trackbar Controls", &profile_manager.get_circle(board.get_state()).maxR, 600);

				}
				else if (board.get_state() == 7)
				{
					createTrackbar("edge_p1", "Trackbar Controls", &profile_manager.get_line().e_p1, 400);
					createTrackbar("edge_p2", "Trackbar Controls", &profile_manager.get_line().e_p2, 400);
					createTrackbar("thresh", "Trackbar Controls", &profile_manager.get_line().p1, 300);
					createTrackbar("min len", "Trackbar Controls", &profile_manager.get_line().p2, 500);
					createTrackbar("max gap", "Trackbar Controls", &profile_manager.get_line().p3, 300);
				}
			}

			// Update Windows //
			if (board.get_state() == -1)
				imshow("Frame", board.get_frame());
			else
				imshow("Frame", board.get_cam_frame());
			if (board.get_state() < 1)
				imshow("Frame Snapshot", board.get_frame());
			else
				imshow("Frame Snapshot", board.get_frame_segments());
			imshow("Frame Calib", calib_frame);

			// First calibration window adjustments
			if (!first_calibration)
			{
				first_calibration = true;
				//moveWindow("Frame Snapshot", 850, 0);
				//resizeWindow("Frame Snapshot", board.get_frame().cols / 3, board.get_frame().rows / 3);
				if (board.get_state() == 6)
					moveWindow("Frame Doubles", 365, 330);
				setMouseCallback("Frame Snapshot", mouse_callback, &board);
			}

			// Keybinds // 
			int key_code = waitKey(20);
			if (key_code == 32) 								// 'Space' to snapshot the DartBoard during Calibration
			{
				profile_manager.save_state(board.get_state());		// Save the current state to current profile

				if (board.get_state() == -1) {						// Warps the dartboard to get a perfect circle

					board.take_perspective_transform(profile_manager.get_thresh().warpX, profile_manager.get_thresh().warpY);
					resizeWindow("Frame Snapshot", board.get_frame().cols / 1.5, board.get_frame().rows / 1.5);
					board.set_state(0);
				}
				else if (board.get_state() == 0) {					// Isolates the dartboard playing area
					board.take_snapshot();
					board.set_state(1);
				}
				else if (board.get_state() < 7) {					// Move the state forward by one in the boundary detection states
					board.set_state(board.get_state() + 1);
				}
				else if (board.get_state() == 7) {					// Sets the lines found as the dartboard's segment lines
					board.lock_in_segment_lines();
					board.set_state(8);
				}
				else if (board.get_state() == 8)					// Get game type info from user			
				{
					int numb_points;
					string game_type;
					Game* type;
					char* args[4];
					bool valid_config = false;


					cout << "What game do you want to play? (Options: FixedScore)"; // Only one game type rn
					cin >> game_type;
					do
					{
						if (game_type == "FixedScore")
						{
							type = &FixedScore();
							cout << "How many points do you want to start with for the FixedScore game? (i.e 301): ";
							cin >> numb_points;
							if (numb_points < 2001)
								valid_config = true;
						}

						if (!valid_config)
							cout << "Invalid selection, try again.\n";
					}
					while (!valid_config);


					// game selected
					board.start_game(type, args);
					

					calib_frame = board.check_darts(profile_manager.get_thresh().warpX, profile_manager.get_thresh().warpY); // Dart Detection
					cv::Mat foreground = board.locate_dart_MOG2(profile_manager.get_thresh().warpX, profile_manager.get_thresh().warpY);
					imshow("mask", foreground);
				}
				else if (board.get_state() == 9) // game running
					board.game_input(key_code);


			}
			else if (key_code == 102 || key_code == 707) {		// 'f' to reset the DartBoard Calibration
				if (board.get_state() == 8)
					board.reset_segments();
				else if (board.get_state() == 9) // game running
					board.game_input(key_code);
				board.set_state(board.get_state() - 1);
			}
			else if (key_code == 27)							// 'esc' to exit the program
				break;
			else if (key_code == 115) {							// 's' to save the current calib values to the first available profile

				profile_manager.save_state(board.get_state());
				profile_manager.save_profile(board.get_state());
			}
			// add r , cont ya-da for ... the 301
		}
	}
	catch (const cv::Exception& e)
	{
		cout << e.msg << endl;
	}
	
}