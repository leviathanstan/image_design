#include "stdafx.h"
#include "ImageProcess.h"
#include <iostream>
#include <io.h>
#include <vector>
#include <string>
using namespace std;
# define M_PI           3.14159265358979323846  /* pi */
#define   WIDTHBYTES(bits) (((bits)+31)/32*4)//用于使图像宽度所占字节数为4byte的倍数  

ImageProcess::ImageProcess() :CDIB()
{
}

ImageProcess::~ImageProcess()
{
}
/*记录额外的图片信息*/
bool ImageProcess::LoadFromFile(LPCTSTR lpszFileName) {
	if (!CDIB::LoadFromFile(lpszFileName))
		return false;
	/*记录对齐后的图像宽度*/
	actualWidth = 3 * GetDIBWidth();
	while (actualWidth % 4 != 0)actualWidth++;
	return TRUE;
}
/*缩放图像*/
double** ImageProcess::resize(int** input, int srcRow, int srcCol, int row, int col) {

	double** output = new double*[row];
	for (int i = 0; i < row; i++) {
		output[i] = new double[col];
	}
	float h_scale_rate = (float)srcRow / row;
	float w_scale_rate = (float)srcCol / col;
	for (int i = 0; i < row; i++) {
		double* p = output[i];
		for (int j = 0; j < col; j++) {
			float i_scale = h_scale_rate * i;
			float j_scale = w_scale_rate * j;
			p[j] = get_scale_value(input, i_scale, j_scale);
		}
	}
	return output;
}
/*双线性插值*/
int ImageProcess::get_scale_value(int** input, float raw_i, float raw_j)
{
	int i = (int)raw_i;
	int j = (int)raw_j;
	float u = raw_i - i;
	float v = raw_j - j;

	int x1 = input[i][j];  //f(i,j)
	int x2 = input[i][j + 1];  //f(i,j+1)
	int x3 = input[i + 1][j];   //(i+1,j)
	int x4 = input[i + 1][j + 1];  //f(i+1,j+1) 

	return (int)((1 - u)*(1 - v)*x1 + (1 - u)*v*x2 + u * (1 - v)*x3 + u * v*x4);
}

/*综合RGB到二维数组（参考java实现）*/
int** ImageProcess::getRGB() {
	int width = GetDIBWidth(), height = GetDIBHeight();
	int** res = new int*[height];
	for (int i = 0; i < height; i++) {
		res[i] = new int[width];
	}
	for (int ni = 0; ni < height; ni++)
		for (int nj = 0; nj < width; nj++) {
			int index = ni * actualWidth + nj * 3;
			res[ni][nj] = m_pDIBData[index] << 16
				| (m_pDIBData[index + 1] << 8)
				| (m_pDIBData[index + 2] << 0);
		}
	return res;
}

/*基于直方图的相似度计算*/
double ImageProcess::compare(ImageProcess* img1, ImageProcess* img2) {
	float* f1;
	float* f2;
	f1 = img1->filter();
	f2 = img2->filter();
	return calcSimilarity(f1, f2, 16 * 16 * 16);
}

//获取图像信息，重写父类加载图片方法
BYTE* ImageProcess::getImg(LPCTSTR lpszFileName) {
	CFile file;

	if (!file.Open(lpszFileName, CFile::modeRead | CFile::typeBinary))	return NULL;
	//读取文件元信息
	BITMAPFILEHEADER bitAder;
	if (file.Read(&bitAder, sizeof(bitAder)) != sizeof(bitAder))	return NULL;
	if (bitAder.bfType != 0x4d42)	return NULL;
	BITMAPINFOHEADER bitMap;
	if (file.Read(&bitMap, sizeof(bitMap)) != sizeof(bitMap))	return NULL;
	if (bitMap.biBitCount < 24)	return NULL;
	BITMAPINFO* bitMapInfo = (BITMAPINFO*) new char[sizeof(BITMAPINFOHEADER)];
	if (!bitMapInfo)	return NULL;
	memcpy(bitMapInfo, &bitMap, sizeof(BITMAPINFOHEADER));

	//内存对齐
	int width = bitMapInfo->bmiHeader.biWidth;
	int height = bitMapInfo->bmiHeader.biHeight;
	int length = 3 * width;
	while (length % 4 != 0)length++;
	length = length - 3 * width;
	dataBytes = 3 * width*height + height * length;
	
	BYTE* data = (BYTE*) new char[dataBytes];
	if (!data)	return NULL;

	//读如位图的图像数据
	if (file.Read(data, dataBytes) != dataBytes) {
		delete data;
		delete bitMapInfo;
		return NULL;
	}

	file.Close();

	BYTE* imgData = new BYTE[width*height];
	if (imgData == NULL)	return NULL;

	int ni, nj;
	for (ni = 0; ni < height; ni++)
		for (nj = 0; nj < width; nj++) {
			imgData[ni*width + nj] = (unsigned char)(data[ni*(3 * width + length) + 3 * nj] * 0.11 + data[ni*(3 * width + length) + 3 * nj + 1] * 0.59 + data[ni*(3 * width + length) + 3 * nj + 2] * 0.3);
		}
	return imgData;
}

/*巴氏系数 大于等于0.8可以简单判断这两张图片内容一致*/
double ImageProcess::calcSimilarity(float* sourceData, float* candidateData, int len) {
	double* mixedData = new double[len];
	for (int i = 0; i < len; i++) {
		//cout << sourceData[i] << " " << candidateData[i] << endl;
		mixedData[i] = sqrt(sourceData[i] * candidateData[i]);
	}

	double similarity = 0;
	for (int i = 0; i < len; i++) {
		similarity += mixedData[i];
	}

	return similarity;
}

/*直方图归一化*/
float* ImageProcess::filter() {
	int width = GetDIBWidth();
	int height = GetDIBHeight();
	int bins = 16;

	float* histogramData = new float[bins * bins * bins];
	for (int i = 0; i < bins * bins * bins; i++) {
		histogramData[i] = 0;
	}
	int** inPixels = getRGB();
	int index = 0;
	int redIdx = 0, greenIdx = 0, blueIdx = 0;
	int singleIndex = 0;
	float total = 0;
	for (int i = 0; i < height; i++) {
		unsigned int tr = 0, tg = 0, tb = 0;
		for (int j = 0; j < width; j++) {
			tr = (inPixels[i][j] >> 16) & 0xff;
			tg = (inPixels[i][j] >> 8) & 0xff;
			tb = inPixels[i][j] & 0xff;
			redIdx = (int)getBinIndex(bins, tr, 255);
			greenIdx = (int)getBinIndex(bins, tg, 255);
			blueIdx = (int)getBinIndex(bins, tb, 255);
			singleIndex = redIdx + greenIdx * bins + blueIdx * bins * bins;
			histogramData[singleIndex] += 1;
			total += 1;
		}
	}
	for (int i = 0; i < bins*bins*bins; i++) {
		histogramData[i] = histogramData[i] / total;
	}

	return histogramData;
}
/*RGB映射（如果用一个24位有效的int型保存所有RGB信息，那么直方图数组的长度位2^24，所以需要映射减少长度*/
float ImageProcess::getBinIndex(int binCount, int color, int colorMaxValue) {
	float binIndex = (((float)color) / ((float)colorMaxValue)) * ((float)binCount);
	if (binIndex >= binCount)
		binIndex = (float)(binCount - 1);
	return binIndex;
}
/*保存图片到文件*/
void  ImageProcess::save() {
	//CFile newFile;
	//BITMAPFILEHEADER writebitHead = bfh;
	//BITMAPINFOHEADER writebitInfoHead = bih;
	//writebitInfoHead.biHeight = row;//为截取文件重写位图高度  
	//writebitInfoHead.biWidth = col;//为截取文件重写位图宽度
	//int mywritewidth = WIDTHBYTES(writebitInfoHead.biWidth*writebitInfoHead.biBitCount);
	//writebitInfoHead.biSizeImage = mywritewidth * writebitInfoHead.biHeight;//计算位图实际数据区大小  
	//writebitHead.bfSize = 54 + writebitInfoHead.biSizeImage;//位图文件头大小为位图数据区大小加上54byte  
	//newFile.Open("res.bmp", CFile::modeCreate | CFile::modeWrite);
	//newFile.Write(&writebitHead, sizeof(bfh));
	//newFile.Write(&writebitInfoHead, sizeof(bih));
	//newFile.Write(output, mywritewidth * writebitInfoHead.biHeight);
	//newFile.Close();
}
/*获取文件夹下图片数*/
int ImageProcess::TraverseFiles(string path, vector<string*>* files)
{
	_finddata_t file_info;
	string current_path = path + "/*.bmp";
	//打开文件查找句柄
	int handle = _findfirst(current_path.c_str(), &file_info);
	int count = 0;
	if (-1 == handle)
		return count;
	do {
		if (file_info.attrib == _A_SUBDIR) //是目录
			continue;
		string* name = new string;
		*name = file_info.name;
		*name = "img\\" + *name;
		files->push_back(name);
		count++;
	} while (!_findnext(handle, &file_info));
	_findclose(handle);
	return count;
}
/*缩放*/
void ImageProcess::resize(int height, int width) {

	int row = height, col = width;
	int awidth = WIDTHBYTES(col * 24);
	BYTE* output = (BYTE*) new char[row * awidth * 3];
	memset(output, 0, awidth*row);
	float h_scale_rate = (float)GetDIBHeight() / row;
	float w_scale_rate = (float)GetDIBWidth() / col;
	for (int i = 0; i < row; i++) {
		for (int j = 0; j < col; j++) {
			float i_scale = h_scale_rate * i;
			float j_scale = w_scale_rate * j;
			for (int k = 0; k < 3; k++) {
				int res = compute(m_pDIBData, i_scale, j_scale, k);
				BYTE b = (char)abs(res);
				output[i*awidth + j * 3 + k] = b;
			}
		}
	}
	m_pDIBData = output;
	m_pBMI->bmiHeader.biWidth = col;
	m_pBMI->bmiHeader.biHeight = row;
	actualWidth = awidth;
}
/*双线性插值*/
int ImageProcess::compute(BYTE* source, float x, float y, int offset) {
	int res = 0;
	int i = (int)x;
	int j = (int)y;
	float u = x - i;
	float v = y - j;
	int x1 = source[j * 3 + i * actualWidth + offset];  //f(i,j)
	int x2 = source[(j + 1) * 3 + i * actualWidth + offset];  //f(i,j+1)
	int x3 = source[j * 3 + (i + 1) * actualWidth + offset];   //(i+1,j)
	int x4 = source[(j + 1) * 3 + (i + 1) * actualWidth + offset];  //f(i+1,j+1) 
	return (int)((1 - u)*(1 - v)*x1 + (1 - u)*v*x2 + u * (1 - v)*x3 + u * v*x4);
}
/*转为256阶*/
int** ImageProcess::gray() {
	int width = GetDIBWidth(), height = GetDIBHeight();
	int** m_pdata = new int*[width*height];
	for (int i = 0; i < height; i++) {
		m_pdata[i] = new int[width];
	}
	for (int ni = 0; ni < height; ni++)
		for (int nj = 0; nj < width; nj++) {
			int index = ni * actualWidth + nj * 3;
			m_pdata[ni][nj] = m_pDIBData[index] * 0.11
				+ m_pDIBData[index + 1] * 0.59
				+ m_pDIBData[index + 2] * 0.3;
		}
	return m_pdata;
}

/*离散余弦变换*/
void ImageProcess::DCT(int ** input, double ** output, int row, int col) {
	double ALPHA, BETA;

	for (int u = 0; u < row; u++) {
		for (int v = 0; v < col; v++) {
			ALPHA = u == 0 ? sqrt(1.0 / row) : sqrt(2.0 / row);
			BETA = v == 0 ? sqrt(1.0 / col) : sqrt(2.0 / col);

			double tmp = 0.0;
			for (int i = 0; i < row; i++) {
				for (int j = 0; j < col; j++) {
					//tmp += *((double*)input + col * i + j) * cos((2 * i + 1)*u*M_PI / (2.0 * row)) * cos((2 * j + 1)*v*M_PI / (2.0 * col));
					tmp += input[i][j] * cos((2 * i + 1)*u*M_PI / (2.0 * row)) * cos((2 * j + 1)*v*M_PI / (2.0 * col));
				}
			}
			output[u][v] = ALPHA * BETA * tmp;
			//*((double*)output + col * u + v) = ALPHA * BETA * tmp;
		}
	}
}
/*计算相似度*/
string ImageProcess::showAll(CDC* pDC, LPCTSTR folder) {
	vector<string*> files;
	string path = folder;
	const int len = TraverseFiles(path, &files);
	int x = 10, y = 200;
	int col = 0;
	string str;
	double* rate = new double[len];
	ImageProcess** ips = new ImageProcess*[len];
	//compute
	for (int i = 0; i < len; i++) {
		ImageProcess* ip = new ImageProcess();
		ImageProcess* ip2 = new ImageProcess();
		ip->LoadFromFile((LPCTSTR)files[i]->c_str());
		ip2->LoadFromFile((LPCTSTR)files[i]->c_str());
		double res = computeHashSimilar(this, ip2);
		rate[i] = res;
		ips[i] = ip;

		////double res = compare(this, &ip);
	}
	//排序
	for (int i = 0; i < len; i++) {
		for (int j = 0; j < len - 1 - i; j++) {
			if (rate[j + 1] > rate[j]) {
				double temp = rate[j + 1];
				rate[j + 1] = rate[j];
				rate[j] = temp;

				ImageProcess* tmp = ips[j + 1];
				ips[j + 1] = ips[j];
				ips[j] = tmp;
			}
		}
	}
	//show
	for (int i = 0; i < len; i++, col++) {
		str += to_string(rate[i]) + "    ";
		ips[i]->resize(200, 200);
		ips[i]->ShowDIB(pDC, x, y, ips[i]->GetDIBWidth(), ips[i]->GetDIBHeight(), ips[i]->m_pDIBData, ips[i]->m_pBMI);
		if (col == 7) {
			y += ips[i]->GetDIBHeight();
			x = 10;
			col = -1;
			str += "\n";
		}
		else {
			x += ips[i]->GetDIBWidth();
		}
	}
	delete rate;
	delete [] ips;
	return str;
}

char* ImageProcess::getHash(int len) {

	//缩小
	resize(len, len);
	//降低灰度
	int** data = gray();
	/*离散余弦变换*/
	/*	double** dct = new double*[len];
	for (int i = 0; i < len; i++) {
		dct[i] = new double[len];
	}
	DCT(data, dct, len, len);
	*/
	//平均
	double total = 0;
	for (int x = 0; x < len; x++) {
		for (int y = 0; y < len; y++) {
			//total += dct[x][y];
			total += data[x][y];
		}
	}
	double avg = total / (len * len);
	//计算哈希
	char* hash = new char[len * len];
	for (int x = 0, index = 0; x < len; x++) {
		for (int y = 0; y < len; y++) {
			hash[index++] = (data[x][y] > avg ? '1' : '0');
		}
	}
	return hash;
}

/*基于PHash计算相似度*/
double ImageProcess::computeHashSimilar(ImageProcess* p1, ImageProcess* p2) {
	int len = 32;
	char* hash1 = p1->getHash(len);
	char* hash2 = p2->getHash(len);
	int count = 0;
	int size = len*len;
	for (int i = 0; i < size; i++) {
		if (*(hash1 + i) == *(hash2 + i)) {
			count++;
		}
	}
	return count / (double)size;
}