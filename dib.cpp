//DIB.CPP :DIB implementation file
//

#include "stdafx.h"
#include "dib.h"
//#include "mainfrm.h"
#include "math.h"
#include <iostream>
using namespace std;

////////////////////
CDIB::CDIB()
{
	m_pBMI = NULL;
	m_pDIBData = NULL;
	m_pDumpDIBData = NULL;
	m_pR = NULL;
	m_pG = NULL;
	m_pB = NULL;
	m_bLoaded = false;
	dataBytes = 0;
	m_pdata = NULL;
	m_pPalette = NULL;
	m_nImType = 0;


}

CDIB::~CDIB()
{
	//�ͷ��ڴ�
	if (m_pBMI != NULL)
		delete m_pBMI;
	if (m_pDIBData != NULL)
		delete m_pDIBData;
	if (m_pDumpDIBData != NULL)
		delete m_pDumpDIBData;

	if (m_pR != NULL)
		free(m_pR);
	if (m_pG != NULL)
		free(m_pG);
	if (m_pB != NULL)
		free(m_pB);
	if (m_pdata != NULL)
		delete m_pdata;

}

bool CDIB::LoadFromFile(LPCTSTR lpszFileName)
{
	CFile file;
	BITMAPINFO* pBMI = NULL;
	BYTE* pDIBData = NULL;
	BYTE* pDumpDIBData = NULL;

	//��ָ�����ļ�
	if (!file.Open(lpszFileName, CFile::modeRead | CFile::typeBinary))
	{
		AfxMessageBox("�򲻿��ļ�!");
		return FALSE;
	}

	//����BMP�ļ�ͷ��Ϣ
	//	BITMAPFILEHEADER bfh;
	if (file.Read(&bfh, sizeof(bfh)) != sizeof(bfh))
	{
		AfxMessageBox("error in reading file");
		return FALSE;
	}

	//������BMP,���账��
	if (bfh.bfType != 0x4d42)      //'BM'
	{
		AfxMessageBox("����BMP�ļ�");
		return FALSE;
	}

	//����λͼ��Ϣͷ
		//
	if (file.Read(&bih, sizeof(bih)) != sizeof(bih))
	{
		AfxMessageBox("���ļ�����!");
		return FALSE;
	}

	//������/24λ���ɫλͼ,���账��
	/*	if(bih.biBitCount<24)
		{
			AfxMessageBox("this is not 24bit bitmap");
			return FALSE;
		}
	*/
	if (bih.biBitCount == 8)
	{
		////��ȡͼ����Ϣ
		if (m_pBMI != NULL)
			delete pBMI;
		file.Seek(-bih.biSize, CFile::current);
		m_pBMI = (BITMAPINFO*) new BYTE[sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256];
		if (file.Read(m_pBMI, sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256) != sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256)
		{
			AfxMessageBox("��ͼ�����ݳ���");
			return false;
		}

		////////////////������ɫ��
		int ncolor = bih.biClrUsed ? bih.biClrUsed : 1 << bih.biBitCount;
		UINT nsize = sizeof(LOGPALETTE) + sizeof(PALETTEENTRY)*ncolor;

		LOGPALETTE *pLP = (LOGPALETTE*)new BYTE(nsize);
		pLP->palVersion = 0x300;
		pLP->palNumEntries = ncolor;

		/*for(int i=0;i<ncolor;i++)
		{
			pLP->palPalEntry[i].peRed=m_pBMI->bmiColors[i].rgbRed;
			pLP->palPalEntry[i].peGreen=m_pBMI->bmiColors[i].rgbGreen;
			pLP->palPalEntry[i].peBlue=m_pBMI->bmiColors[i].rgbBlue;
			pLP->palPalEntry[i].peFlags=0;
		}
		*/
		if (m_pPalette != NULL)
			delete m_pPalette;
		m_pPalette = new CPalette;

		m_pPalette->CreatePalette(pLP);


		/////��ͼ������
		int nW = m_pBMI->bmiHeader.biWidth;
		int nH = m_pBMI->bmiHeader.biHeight;
		if (m_pDIBData != NULL)
			delete m_pDIBData;
		m_pDIBData = new BYTE[nW*nH];
		if (!m_pDIBData)
		{
			AfxMessageBox("�����ڴ����!");
			return FALSE;
		}

		file.Seek(-nH * nW, CFile::end);
		if (file.Read(m_pDIBData, nW*nH) != (LONG)nW*nH)
		{
			AfxMessageBox("��ͼ�����ݳ���");
			delete m_pBMI;
			delete m_pDIBData;
			return FALSE;
		}
		file.Close();

		if (m_pdata != NULL)
			delete m_pdata;
		m_pdata = new BYTE[nW*nH];
		if (m_pdata == NULL)
		{
			AfxMessageBox("�����ڴ����");
			return FALSE;
		}

		memcpy(m_pdata, m_pDIBData, nW*nH);

		//		delete pLP;
		m_bLoaded = true;
		m_nImType = 8;
		return TRUE;


	}

	//ΪBITMAPINFO�ṹָ�������ڴ�,�������ɫλͼû����ɫ��,������BITMAPINFO=BITMAPINFOHEADER
	pBMI = (BITMAPINFO*) new char[sizeof(BITMAPINFOHEADER)];
	if (!pBMI)
	{
		AfxMessageBox("�����ڴ����!");
		return FALSE;
	}

	//����ǰ���ѽ�BITMAPINFOHERAER�������ڴ�,��������ֻ�追��һ��
	memcpy(pBMI, &bih, sizeof(BITMAPINFOHEADER));

	//ΪDIBͼ������ָ�������ڴ�
	int nW = pBMI->bmiHeader.biWidth;
	int nH = pBMI->bmiHeader.biHeight;
	int length = 3 * nW;
	while (length % 4 != 0)length++;
	length = length - 3 * nW;
	dataBytes = 3 * nW*nH + nH * length;
	pDIBData = (BYTE*) new char[dataBytes];
	if (!pDIBData)
	{
		AfxMessageBox("�����ڴ����");
		return FALSE;
	}
	//	dataBytes=bfh.bfSize-bfh.bfOffBits;//ͼ�����ݵ��ֽ��� ALPHA NO DWORD
	pDumpDIBData = (BYTE*) new char[dataBytes];
	if (!pDIBData)
	{
		AfxMessageBox("�����ڴ����");
		return FALSE;
	}

	if (!pDumpDIBData)
	{
		AfxMessageBox("�����ڴ����");
		return FALSE;
	}

	//����λͼ��ͼ������
	if (file.Read(pDIBData, dataBytes) != dataBytes)
	{
		AfxMessageBox("��ͼ�����ݳ���");
		delete pBMI;
		delete pDIBData;
		return FALSE;
	}

	file.Close();

	if (m_pBMI != NULL)
		delete m_pBMI;
	m_pBMI = pBMI;

	if (m_pDIBData != NULL)
		delete m_pDIBData;
	m_pDIBData = pDIBData;

	memcpy(pDumpDIBData, pDIBData, dataBytes);

	if (m_pDumpDIBData != NULL)
		delete m_pDumpDIBData;
	m_pDumpDIBData = pDumpDIBData;

	if (m_pR != NULL)
		free(m_pR);
	if (m_pG != NULL)
		free(m_pG);
	if (m_pB != NULL)
		free(m_pB);

	//m_pR=(BYTE*)malloc(sizeof(BYTE)*nW*nH);
	//m_pG=(BYTE*)malloc(sizeof(BYTE)*nW*nH);
	//m_pB=(BYTE*)malloc(sizeof(BYTE)*nW*nH);

	if (m_pdata != NULL)
		delete m_pdata;

	m_pdata = new BYTE[nW*nH];
	if (m_pdata == NULL)
	{
		AfxMessageBox("�����ڴ����");
		return FALSE;
	}


	int ni, nj;
	for (ni = 0; ni < nH; ni++)
		for (nj = 0; nj < nW; nj++)
		{
			//	m_pB[(nH-1-nj)*nW+ni]=m_pDIBData[(nj*nW+ni)*3+nj*length];
			//	m_pG[(nH-1-nj)*nW+ni]=m_pDIBData[(nj*nW+ni)*3+1+nj*length];
			//	m_pR[(nH-1-nj)*nW+ni]=m_pDIBData[(nj*nW+ni)*3+2+nj*length];
			m_pdata[ni*nW + nj] = (unsigned char)(m_pDIBData[ni*(3 * nW + length) + 3 * nj] * 0.11 + m_pDIBData[ni*(3 * nW + length) + 3 * nj + 1] * 0.59 + m_pDIBData[ni*(3 * nW + length) + 3 * nj + 2] * 0.3);
		}

	m_bLoaded = true;
	m_nImType = 24;
	return TRUE;
}

bool CDIB::SaveFile(LPCTSTR lpszFileName)
{
	CFile newFile;
	newFile.Open(lpszFileName, CFile::modeCreate | CFile::modeWrite);
	newFile.Write(&bfh, sizeof(bfh));
	newFile.Write(m_pBMI, sizeof(BITMAPINFOHEADER));
	newFile.Write(m_pDumpDIBData, dataBytes);
	newFile.Close();

	return true;
}


void CDIB::ShowDIB(CDC *pDC, int nLeft, int nTop, int nRight, int nBottom, BYTE *pDIBData, BITMAPINFO* pBMI)
{
	if (m_pBMI->bmiHeader.biBitCount == 8)
	{
		//	pDC->SelectPalette(m_pPalette,FALSE);
		//	pDC->RealizePalette();
	}

	pDC->SetStretchBltMode(COLORONCOLOR);    //������������ģʽ
	StretchDIBits(pDC->GetSafeHdc(),         //DC�ľ��
		nLeft,                     //Ŀ��������Ͻǵ�X����
		nTop,                      //Ŀ��������Ͻǵ�Y����
		nRight,                    //Ŀ����εĿ��
		nBottom,                   //Ŀ����εĸ߶�
		0,                         //Դ�������Ͻǵ�X����
		0,                         //Դ�������Ͻǵ�Y����
		GetDIBWidth(),             //Դ���εĿ��
		GetDIBHeight(),            //Դ���εĸ߶�
		pDIBData,                  //λͼͼ�����ݵĵ�ַ
		pBMI,                      //λͼ��Ϣ�ṹ��ַ
		DIB_RGB_COLORS,            //��־ѡ��
		SRCCOPY);                  //��դ������

}





void CDIB::InvalidateData()
{
	if (m_bLoaded == false)
	{
		AfxMessageBox("���ȴ�Ҫ������ͼ");
		return;
	}

	int nW = GetDIBWidth();
	int nH = GetDIBHeight();
	if (bih.biBitCount == 8)
	{
		memcpy(m_pDIBData, m_pdata, nW*nH);
		return;
	}

	int lenght = 3 * nW;
	while (lenght % 4 != 0)lenght++;
	lenght = lenght - 3 * nW;
	int ni, nj;
	for (ni = 0; ni < nH; ni++)
		for (nj = 0; nj < nW; nj++)
		{
			m_pDumpDIBData[ni*(3 * nW + lenght) + nj * 3] = m_pdata[ni*nW + nj];
			m_pDumpDIBData[ni*(3 * nW + lenght) + nj * 3 + 1] = m_pdata[ni*nW + nj];
			m_pDumpDIBData[ni*(3 * nW + lenght) + nj * 3 + 2] = m_pdata[ni*nW + nj];
		}
}

void CDIB::close()
{

	//�ͷ��ڴ�
	if (m_pBMI != NULL)
		delete m_pBMI;
	if (m_pDIBData != NULL)
		delete m_pDIBData;
	if (m_pDumpDIBData != NULL)
		delete m_pDumpDIBData;

	if (m_pR != NULL)
		free(m_pR);
	if (m_pG != NULL)
		free(m_pG);
	if (m_pB != NULL)
		free(m_pB);
	if (m_pdata != NULL)
		delete m_pdata;


}

void CDIB::UpdateData()
{
	if (m_bLoaded == false)
	{
		AfxMessageBox("���ȴ�Ҫ������ͼ");
		return;
	}

	int nW = GetDIBWidth();
	int nH = GetDIBHeight();
	if (bih.biBitCount == 8)
	{
		memcpy(m_pDIBData, m_pdata, nW*nH);
		return;
	}
	//ͼ�����ֽڶ��������ͼ�����4�ı�������ô��ܻ��ÿ�����ݽ������(Ϊ�˼ӿ�洢�Ͷ�ȡ���������ȡ������4�ֽ�Ϊ��λ)��ʹ�����Ϊ4�ı��������������ƫ������Ҫ���м���
	int lenght = 3 * nW;
	while (lenght % 4 != 0)lenght++;
	lenght = lenght - 3 * nW;
	int ni, nj;
	for (ni = 0; ni < nH; ni++)
		for (nj = 0; nj < nW; nj++)
		{
			m_pDIBData[ni*(3 * nW + lenght) + nj * 3] = m_pdata[ni*nW + nj];
			m_pDIBData[ni*(3 * nW + lenght) + nj * 3 + 1] = m_pdata[ni*nW + nj];
			m_pDIBData[ni*(3 * nW + lenght) + nj * 3 + 2] = m_pdata[ni*nW + nj];
		}
}