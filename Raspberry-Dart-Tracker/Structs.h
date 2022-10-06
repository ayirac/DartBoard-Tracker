#pragma once

struct CircleParams
{
	int dist, p1, p2, minR, maxR;
	CircleParams() : dist(1), p1(1), p2(1), minR(1), maxR(1) {};
	CircleParams(int d, int p1, int p2, int min, int max) : dist(d), p1(p1), p2(p2), minR(min), maxR(max) {};
};

struct LinesParams
{
	int p1, p2, p3, e_p1, e_p2;
};

struct ThreshParams
{
	int lowH, lowS, lowV, highH, highS, highV, thresh, warpX, warpY;
};