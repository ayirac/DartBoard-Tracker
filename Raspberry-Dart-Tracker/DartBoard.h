#pragma once
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/video/background_segm.hpp>
#include "Game.h"
#include "FixedScore.h"
#include "Structs.h"


typedef enum {BULLSEYE_INNER, BULLEYES_OUTER, TRIPLE_INNER, TRIPLE_OUTER, DOUBLE_INNER, DOUBLE_OUTER } TYPE;

struct Segment
{
	int ID;
	cv::Vec4i lines[2];
};

struct BoundaryCircle
{
	TYPE type;
	cv::Vec3f circ;
};

class DartBoard
{
private:
	cv::Mat frame_, cam_frame_, temp_frame_, original_frame_;
	std::vector<Segment> segments_;
	std::vector<BoundaryCircle*> boundaries_;
	int c_state_; // Calibration State: Perspective, Snapshot, Bullseye_O, Bullseye_I, Triples_O, Triples_I, Doubles_O, Doubles_I, Segment LInes (-1 - 7)
	cv::Vec3f outer_circle_;
	std::vector<cv::Vec4i> lines_;
	bool new_state_;
	cv::Point2f src_pnts[4], dst_pnts[4];
	cv::Ptr<cv::BackgroundSubtractor> MOG2_;
	int MOG_frame_target_, MOG_frame_count_;
	Game* game_; // cont here <-
	
public:
	DartBoard();
	cv::Mat calibrate_board(int dist, int p1, int p2, int min_R, int max_R);
	cv::Mat locate_four_corners(cv::Mat& input_frame, cv::Scalar lows, cv::Scalar highs, int warpX, int warpY);
	bool take_snapshot();
	void take_perspective_transform(int warpX, int warpY);
	cv::Mat take_perspective_transform(int warpX, int warpY, bool);
	cv::Mat& get_frame();
	cv::Mat& get_frame_bullseye_circles();
	cv::Mat locate_boundary(int dist, int p1, int p2, int min_R, int max_R, BoundaryCircle& boundary, int offcenter_threshold);
	cv::Mat locate_boundaries(CircleParams arr);
	cv::Mat get_frame_segments();
	cv::Mat locate_singles(int p1, int p2, int p3, int e_p1, int e_p2);
	Hit check_hit(cv::Point& hit);
	bool segment_hit(cv::Point& hit, cv::Vec3f outer, cv::Vec3f inner);
	bool segment_hit(cv::Point& hit, cv::Vec3f outer);
	void create_segment(int x, int g);
	void lock_in_segment_lines();
	cv::Point check_darts(int p1, int p2);
	int& get_state();
	void set_state(int s);
	bool state_change();
	BoundaryCircle* get_boundary(TYPE t);
	cv::Mat& get_cam_frame() { return cam_frame_; }
	void reset_segments();
	cv::Mat get_playing_area(int p1, int p2);
	cv::Mat capture_MOG2(int warpX, int warpY);
	cv::Mat locate_dart_MOG2(int warpX, int warpY);
	void start_game(Game* type, int score, int darts, bool double_in);
	Game* get_game() { return game_; }
	void game_input(int key_code, int warpX, int warpY);
	cv::Mat& get_temp_frame() { return this->temp_frame_; }
	cv::Vec3f get_outer() { return this->outer_circle_; }
};
