
// image_designView.h: CimagedesignView 类的接口
//

#pragma once

#include "ImageProcess.h" 

class CimagedesignView : public CView
{
protected: // 仅从序列化创建
	CimagedesignView() noexcept;
	DECLARE_DYNCREATE(CimagedesignView)

// 特性
public:
	CimagedesignDoc* GetDocument() const;
	ImageProcess m_dib;
	CString folder;
// 操作
public:

// 重写
public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// 实现
public:
	virtual ~CimagedesignView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成的消息映射函数
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnOpenImg();
	afx_msg void OnTestShowall();
};

#ifndef _DEBUG  // image_designView.cpp 中的调试版本
inline CimagedesignDoc* CimagedesignView::GetDocument() const
   { return reinterpret_cast<CimagedesignDoc*>(m_pDocument); }
#endif

