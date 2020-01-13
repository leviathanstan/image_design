
// image_designView.cpp: CimagedesignView 类的实现
//

#include "stdafx.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "image_design.h"
#endif

#include "image_designDoc.h"
#include "image_designView.h"

#include <iostream>
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CimagedesignView

IMPLEMENT_DYNCREATE(CimagedesignView, CView)

BEGIN_MESSAGE_MAP(CimagedesignView, CView)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_COMMAND(ID_OPEN_IMG, &CimagedesignView::OnOpenImg)
	ON_COMMAND(ID_TEST_SHOWALL, &CimagedesignView::OnTestShowall)
END_MESSAGE_MAP()

// CimagedesignView 构造/析构

CimagedesignView::CimagedesignView() noexcept
{
	// TODO: 在此处添加构造代码
	AllocConsole();
	FILE *stream;
	freopen_s(&stream, "CON", "r", stdin);
	freopen_s(&stream, "CON", "w", stdout);
	freopen_s(&stream, "CON", "w", stderr);
	cout << "启动控制台/n/n" << endl;
}

CimagedesignView::~CimagedesignView()
{
	FreeConsole();
}

BOOL CimagedesignView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CView::PreCreateWindow(cs);
}

// CimagedesignView 绘图

void CimagedesignView::OnDraw(CDC* pDC)
{
	CimagedesignDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;
	if (m_dib.m_bLoaded == true) {
		m_dib.resize(200, 200);
		int nw = m_dib.GetDIBWidth();
		int nh = m_dib.GetDIBHeight();
		m_dib.ShowDIB(pDC, 10, 10, nw, nh, m_dib.m_pDIBData, m_dib.m_pBMI);
		if (m_dib.status == 1) {
			m_dib.status = 2;
			CStatic *cs;
			cs = new CStatic();
			cs->Create("wait for some min", WS_CHILD | WS_VISIBLE | SS_CENTER, CRect(300, 0, 1000, 200), this);
			string str = m_dib.showAll(pDC, folder);
			cs->SetWindowTextA(str.c_str());
			return;
		}
	}
}

void CimagedesignView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CimagedesignView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CimagedesignView 诊断

#ifdef _DEBUG
void CimagedesignView::AssertValid() const
{
	CView::AssertValid();
}

void CimagedesignView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CimagedesignDoc* CimagedesignView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CimagedesignDoc)));
	return (CimagedesignDoc*)m_pDocument;
}
#endif //_DEBUG


// CimagedesignView 消息处理程序


void CimagedesignView::OnOpenImg()
{
	// TODO: 在此添加命令处理程序代码
	static char szFilter[] = "BMP文件(*.bmp)|*.bmp||";  //定义过滤文件的类型
	CFileDialog dlg(TRUE, "bmp", NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter);//定义文件对话框对象
	int ret = dlg.DoModal();  //运行打开文件对方框
	if (ret == IDOK)
	{
		CString path = dlg.GetFolderPath() + "\\" + dlg.GetFileName();
		m_dib.LoadFromFile(path);   //加载图像
		if (!m_dib.m_bLoaded)            //判断是否加载图像成功
		{
			AfxMessageBox("图像打不开");
			return;
		}
	}
	Invalidate(1);	//刷新屏幕
}

void CimagedesignView::OnTestShowall()
{
	// TODO: 在此添加命令处理程序代码
	BROWSEINFO bi;
	bi.hwndOwner = NULL;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = NULL;
	bi.lpszTitle = NULL;
	bi.ulFlags = 0;
	bi.lpfn = NULL;
	bi.iImage = 0;
	LPCITEMIDLIST pidl = SHBrowseForFolder(&bi);
	if (!pidl)
		return;
	TCHAR szDisplayName[255];
	SHGetPathFromIDList(pidl, szDisplayName);
	CString str(szDisplayName);
	folder = str;
	m_dib.status = 1;
	Invalidate(1);
}
