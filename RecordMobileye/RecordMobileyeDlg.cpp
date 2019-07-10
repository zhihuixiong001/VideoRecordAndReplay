
// RecordMobileyeDlg.cpp : 实现文件
//
//#define ASYNC
#include "stdafx.h"
#include "RecordMobileye.h"
#include "RecordMobileyeDlg.h"
#include "afxdialogex.h"
#include <thread>
#include <opencv2/opencv.hpp>

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#ifdef ASYNC
#include "talk_to_svr.h"
#endif
using namespace boost::asio;
using boost::system::error_code;
io_service service;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace cv;
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框


class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// CRecordMobileyeDlg 对话框
Mat gdiScreenCapture(HDC hCDC, HDC hDDC, int nWidth, int nHeight) {
	HBITMAP hBitmap = CreateCompatibleBitmap(hDDC, nWidth, nHeight);//得到位图    
	SelectObject(hCDC, hBitmap); //好像总得这么写。             
	BitBlt(hCDC, 0, 0, nWidth, nHeight, hDDC, 0, 0, SRCCOPY);
	Mat dst;
	dst.create(cvSize(nWidth, nHeight), CV_8UC4);
	GetBitmapBits(hBitmap, nWidth*nHeight * 4, dst.data);
	cvtColor(dst, dst, CV_BGRA2BGR);
	return dst;
}

void savevideo()
{
	
	bool ifwrite = true;
	int nWidth = GetSystemMetrics(SM_CXSCREEN);//得到屏幕的分辨率的x    
	int nHeight = GetSystemMetrics(SM_CYSCREEN);//得到屏幕分辨率的y    
	HDC hDDC = GetDC(GetDesktopWindow());//得到屏幕的dc    
	HDC hCDC = CreateCompatibleDC(hDDC);//    
	Mat img = gdiScreenCapture(hCDC, hDDC, nWidth, nHeight);
#ifdef RECORD_FILE
	VideoWriter vw;
	vw.open("d:\\2.avi", CV_FOURCC('M', 'P', '4', '2'), 8, cvSize(img.cols, img.rows));
#endif
	//发送报文
	boost::system::error_code error;
	
	const int blocksize = 1440;
	//int nSendBuf = 1024 * 1024 * 50;//接收缓存10M 
	//定义包结构 
	struct recvbuf//包格式  
	{
		char buf[blocksize];//存放数据的变量  
		int flag;//标志   
		int cnt;
		int pack_num;
	};
	struct recvbuf data;
	ip::tcp::endpoint endpoint(boost::asio::ip::address_v4::from_string("192.168.1.56"), 6000);
	
	while (ifwrite)
	{
		img = gdiScreenCapture(hCDC, hDDC, nWidth, nHeight);
		if (!img.data)
			continue;;
#ifdef RECORD_FILE
		vw << img;
#endif
		//发送数据
		/*
		CString str;
		str.Format(L"Width %d Height %d", nWidth, nHeight);
		AfxMessageBox(str);
		*/
		//压缩格式
		std::vector<uchar> data_encode;
		imencode(".jpg", img, data_encode);
		TRACE("data_encode size=%d",data_encode.size());
		int nPack = data_encode.size() / blocksize;
		int nResidue = data_encode.size() % blocksize;
		if (nResidue != 0)
			nPack++;
		for (int i = 0; i < nPack; i++)
		{
			try {
				boost::system::error_code error;
				ip::tcp::socket sock(service);

				sock.connect(endpoint, error);
				if (error)
				{
					throw boost::system::system_error(error);
				}
				
				if (i < nPack - 1)
				{
					data.flag = 1;
					std::copy(data_encode.begin() + blocksize*i, data_encode.begin() + blocksize*(i + 1), data.buf);
					data.cnt = blocksize;
					data.pack_num = nPack;
				}
				else
				{
					data.flag = 2;
					std::copy(data_encode.begin() + blocksize*i, data_encode.end(), data.buf);
					data.cnt = nResidue;
					data.pack_num = nPack;
					
				}
			
				//发送数据
				sock.write_some(boost::asio::buffer((char*)(&data), sizeof(data)));
				char buf[1024] = { 0 };

				boost::system::error_code error1;
				std::size_t bytes = sock.read_some(boost::asio::buffer(buf), error);
				if (error1)
				{
					throw boost::system::system_error(error1);
				}
				std::string copy(buf, bytes - 1);
				strcat_s(buf, " server reply \n ");
				TRACE(buf);
				sock.close();
			}
			catch (std::exception e)
			{
				TRACE(e.what());
			}
		}


/*
		char* pImgData = (char*)img.data;
		//指向该帧的数据矩阵  
		for (int i = 0; i < 3000; i++)                    
		{
#ifndef ASYNC
			try {
				boost::system::error_code error;
				ip::tcp::socket sock(service);
				
				sock.connect(endpoint, error);
				if (error)
				{
					throw boost::system::system_error(error);
				}
							
				memcpy(data.buf, pImgData + i*blocksize, blocksize);
				if (i < 2999)
					data.flag = 1;
				else
					data.flag = 2;
				data.cnt = i;
				//发送数据
				sock.write_some(boost::asio::buffer((char*)(&data), sizeof(data)));
				char buf[1024] = {0};

				boost::system::error_code error1;
				std::size_t bytes = sock.read_some(boost::asio::buffer(buf), error);
				if (error1)
				{
					throw boost::system::system_error(error1);
				}
				std::string copy(buf, bytes - 1);
				strcat_s(buf, " server reply \n ");
				TRACE(buf);
				sock.close();
			}
			catch (std::exception e)
			{
				TRACE(e.what());
			}
#endif

#ifdef ASYNC
			memcpy(data.buf, pImgData + i*blocksize, blocksize);
			if (i < 2999)
				data.flag = 1;
			else
				data.flag = 2;
			data.cnt = i;
			char tmp_buf[1500] = { 0 };
			memcpy(tmp_buf, &data, sizeof(data));
			std::string msg;
			std::copy(tmp_buf, tmp_buf + sizeof(data), msg);
			talk_to_svr::start(endpoint, msg);
			boost::this_thread::sleep(0);
			service.run()
#endif
		}
*/
		waitKey(0);
	}

#ifdef RECORD_FILE
	vw.release();
#endif
	
}


CRecordMobileyeDlg::CRecordMobileyeDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_RECORDMOBILEYE_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRecordMobileyeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CRecordMobileyeDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CRecordMobileyeDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CRecordMobileyeDlg 消息处理程序

BOOL CRecordMobileyeDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CRecordMobileyeDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CRecordMobileyeDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CRecordMobileyeDlg::OnQueryDragIcon()
{	return static_cast<HCURSOR>(m_hIcon);
}



void CRecordMobileyeDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	std::thread th(savevideo);
	th.detach();
	//CDialogEx::OnOK();
}

