#pragma once
#include "dib.h"
#include <iostream>
#include <vector>
using namespace std;
class ImageProcess :
	public CDIB
{
public:
	ImageProcess();
	~ImageProcess();
	bool LoadFromFile(LPCTSTR lpszFileName);
	void DCT(int ** input, double ** output, int row, int col);
	int** getRGB();
	int** gray();
	double** resize(int** input, int srcRow, int srcCol, int row, int col);
	int get_scale_value(int** input, float raw_i, float raw_j);
	double computeHashSimilar(ImageProcess* p1, ImageProcess* p2);
	char* getHash(int len);
	float* filter();
	float getBinIndex(int binCount, int color, int colorMaxValue);
	BYTE* getImg(LPCTSTR lpszFileName);
	double calcSimilarity(float* sourceData, float* candidateData, int len);
	double compare(ImageProcess* img1, ImageProcess* img2);
	int TraverseFiles(string path, vector<string*>* files);
	string showAll(CDC* pDC, LPCTSTR folder);
	void resize(int height, int width);
	int compute(BYTE* source, float x, float y, int offset);
	void save();
public:
	int status;
private:
	int actualWidth;
};

