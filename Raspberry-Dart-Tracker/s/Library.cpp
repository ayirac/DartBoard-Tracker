#include "Library.h"

double distance_between(Point p1, Point p2)
{
	return sqrt(pow(p2.x - p1.x, 2) + pow(p2.x - p1.x, 2));
}

Point find_most_centered_point(Point p1, Point p2, Point center)
{
	if (distance_between(p1, center) > distance_between(p2, center))
		return p2;
	return p1;
}

Point find_further_point(Point p1, Point p2, Point center)
{
	if (distance_between(p1, center) < distance_between(p2, center))
		return p2;
	return p1;
}

/*
int get_biggest_contour_id(vector<vector<Point>> contours)
{
	double max_area = 0;
	int max_area_contour_id = 0;
	for (int i = 0; i < contours.size(); i++)
	{
		double new_area = contourArea(contours.at(i));
		if (new_area > max_area)
		{
			max_area = new_area;
			max_area_contour_id = i;
		}
	}
	return max_area_contour_id;
}

vector<Point2f> sort_points_quad(vector<Point2f> unsorted)
{
	// init vector
	vector<Point2f> sorted;
	for (int i = 0; i < 4; i++) sorted.push_back(Point2f(0, 0));
	// Get center points for reference
	int middleX = (unsorted[0].x + unsorted[1].x + unsorted[2].x + unsorted[3].x) / 4;
	int middleY = (unsorted[0].y + unsorted[1].y + unsorted[2].y + unsorted[3].y) / 4;
	// Sort off of criteria tl, tr, bl, br
	for (int i = 0; i < unsorted.size(); i++) {
		if (unsorted.at(i).x < middleX && unsorted.at(i).y < middleY)sorted[0] = unsorted.at(i);
		if (unsorted.at(i).x > middleX && unsorted.at(i).y < middleY)sorted[1] = unsorted.at(i);
		if (unsorted.at(i).x < middleX && unsorted.at(i).y > middleY)sorted[2] = unsorted.at(i);
		if (unsorted.at(i).x > middleX && unsorted.at(i).y > middleY)sorted[3] = unsorted.at(i);
	}
	return sorted;
}

double distance_between_points(Point pnt1, Point pnt2)
{
	return sqrt(pow(pnt2.x - pnt1.x, 2) + pow(pnt2.y - pnt1.y, 2));
}

bool isInside(int circle_x, int circle_y, int rad, int x, int y)
{
	// Compare radius of circle with distance
	// of its center from given point
	if ((x - circle_x) * (x - circle_x) +
		(y - circle_y) * (y - circle_y) <= rad * rad)
		return true;
	else
		return false;
}*/