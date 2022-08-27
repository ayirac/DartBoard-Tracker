#pragma once
#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;


double distance_between(Point p1, Point p2);
Point find_most_centered_point(Point p1, Point p2, Point center);
Point find_further_point(Point p1, Point p2, Point center);