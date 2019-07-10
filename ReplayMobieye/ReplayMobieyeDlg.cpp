
// ReplayMobieyeDlg.cpp : 实现文件
//
//#define  ASYNC
#include "stdafx.h"
#include "ReplayMobieye.h"
#include "ReplayMobieyeDlg.h"
#include "afxdialogex.h"
#include <thread>
#include "CvvImage.h"
#include "resource.h"
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread/thread.hpp>
#ifdef ASYNC
#include "talk_to_client.h"
#endif
using namespace boost::asio;
using namespace boost::posix_time;
using boost::system::error_code;
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CReplayMobieyeDlg* g_pDlg = NULL;


#ifndef ASYNC
const int blocksize = 1440;
//定义包结构 
struct recvbuf//包格式  
{
	char buf[blocksize];//存放数据的变量  
	int flag;//标志   
	int cnt;
	int pack_num;
};
#endif // !1
struct recvbuf g_data;
boost::mutex g_data_mutex;
io_service service;

size_t read_complete(char * buf, const error_code & err, size_t bytes) {

	if (err) return 0;
	bool found = std::find(buf, buf + bytes, '\n') < buf + bytes;
	return found ? 0 : 1;

}

#ifdef ASYNC
ip::tcp::acceptor acceptor(service, ip::tcp::endpoint(ip::tcp::v4(), 6000));
 
void handle_accept(talk_to_client::ptr client, const boost::system::error_code & err) {
	static Mat dst(cvSize(1600, 900), CV_8UC3);
	static int npack = 0;
	static int flag_sum = 0;
	client->start();
	
	memcpy(dst.data + g_data.cnt*blocksize, g_data.buf, blocksize * sizeof(char));

	flag_sum = flag_sum + (int)g_data.flag;
	npack++;
	if (g_data.flag == 2)
	{
		if (flag_sum == 3001 && npack == 3000)
		{
			if (g_pDlg != NULL)
				g_pDlg->SetImage(dst);
		}
		npack = 0;
		flag_sum = 0;
	}

	talk_to_client::ptr new_client = talk_to_client::new_();
	acceptor.async_accept(new_client->sock(), boost::bind(handle_accept, new_client, _1));
}
#endif

void replay_image()
{
#ifndef ASYNC
	Mat dst;
	dst.create(cvSize(1600, 900), CV_8UC3);
	int npack = 0;
//	int flag_sum = 0;
#endif

	try {
		/*
#ifndef ASYNC
		ip::tcp::acceptor acceptor(service, ip::tcp::endpoint(ip::tcp::v4(), 6000));
		char buff[1500] = { 0 };
		while (true) 
		{
			ip::tcp::socket sock(service);
			acceptor.accept(sock);

			boost::system::error_code error;
			std::size_t bytes = sock.read_some(boost::asio::buffer(buff), error);
			if (error)
			{
				throw boost::system::system_error(error);
			}
			memcpy(&g_data, buff, sizeof(g_data));

			if (bytes < sizeof(recvbuf))
			{
				TRACE("len error!\n");
			}
			else
			{
				memcpy(&g_data, buff, sizeof(g_data));
			}
			memcpy(dst.data + g_data.cnt*blocksize, g_data.buf, blocksize * sizeof(char));

			flag_sum = flag_sum + (int)g_data.flag;
			npack++;
			if (g_data.flag == 2)
			{
				if (flag_sum == 3001 && npack == 3000)
				{
					if (dst.empty())
						continue;
					if (g_pDlg != NULL)
						g_pDlg->SetImage(dst);
				}
				npack = 0;
				flag_sum = 0;
			}

			std::string msg = "ok";
			sock.write_some(buffer(msg));
			sock.close();
		}
#else
		
		talk_to_client::ptr client = talk_to_client::new_();
		acceptor.async_accept(client->sock(), boost::bind(handle_accept, client, _1));
			
		service.run();
			
#endif // !
*/
		ip::tcp::acceptor acceptor(service, ip::tcp::endpoint(ip::tcp::v4(), 6000));
		char buff[1500] = { 0 };
		std::vector<uchar> img_data;
		

		while (true)
		{
			ip::tcp::socket sock(service);
			acceptor.accept(sock);

			boost::system::error_code error;
			std::size_t bytes = sock.read_some(boost::asio::buffer(buff), error);
			if (error)
			{
				throw boost::system::system_error(error);
			}
			memcpy(&g_data, buff, sizeof(g_data));

			if (bytes < sizeof(recvbuf))
			{
				TRACE("len error!\n");
			}
			else
			{
				memcpy(&g_data, buff, sizeof(g_data));
			}

			for (int i = 0; i < g_data.cnt; i++)
				img_data.emplace_back(g_data.buf[i]);

			npack++;
			if (g_data.flag == 2)
			{
				if (g_data.pack_num == npack)
				{
					dst = imdecode(img_data, CV_LOAD_IMAGE_COLOR);
					if (dst.empty())
						continue;
					if (g_pDlg != NULL)
						g_pDlg->SetImage(dst);
				}
				npack = 0;
				img_data.clear();
						
			}

			std::string msg = "ok";
			sock.write_some(buffer(msg));
			sock.close();
		}
	}
	catch (std::exception e)
	{
		TRACE(e.what());
	}
}

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


// CReplayMobieyeDlg 对话框

CReplayMobieyeDlg::CReplayMobieyeDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_REPLAYMOBIEYE_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CReplayMobieyeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CReplayMobieyeDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CReplayMobieyeDlg::OnBnClickedOk)
	ON_WM_TIMER()
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CReplayMobieyeDlg 消息处理程序

BOOL CReplayMobieyeDlg::OnInitDialog()
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
	g_pDlg = this;
	//加入OnSize的初始值

	CRect rect;
	GetClientRect(&rect);     //取客户区大小  
	old.x = rect.right - rect.left;
	old.y = rect.bottom - rect.top;
	
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CReplayMobieyeDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CReplayMobieyeDlg::OnPaint()
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
	ShowImg(IDC_PIC, resize_img);

}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CReplayMobieyeDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}




Mat CReplayMobieyeDlg::ResizeImg(UINT ID, cv::Mat &_Image)
{
	CRect rect;
	GetDlgItem(ID)->GetClientRect(rect);
	int rectW = rect.right - rect.left;
	int rectH = rect.bottom - rect.top;
	// 读取图片的宽和高
	int w = _Image.cols;
	int h = _Image.rows;

	// 计算将图片缩放到TheImage区域所需的比例因子,找到最大的比例尺度
	float w_scale = (float)w / (float)rectW;
	float h_scale = (float)h / (float)rectH;
	float scale = w_scale > h_scale ? w_scale : h_scale;
	//缩放后图片的宽和高
	int nw = (int)(w / scale);
	int nh = (int)(h / scale);
	Size Rsize;
	Rsize.width = nw; Rsize.height = nh;
	Mat TheImage;
	resize(_Image, TheImage, Rsize);
	return TheImage;
}

//
void  CReplayMobieyeDlg::ShowImg(UINT ID, cv::Mat &Image)
{
	//////////////得到控件窗口////////////////////////////////
	CDC *pDC = GetDlgItem(ID)->GetDC();
	HDC hDC = pDC->GetSafeHdc();

	CRect Pic_rect;
	GetDlgItem(ID)->GetClientRect(Pic_rect);
	int rectW = Pic_rect.right - Pic_rect.left;
	int rectH = Pic_rect.bottom - Pic_rect.top;
	int nw = Image.cols;
	int nh = Image.rows;
	//int tlx=nw>nh?0:(int)(rectW-nw)/2;
	//int tly=nw>nh?(int)(rectH-nh)/2:0;
	int tlx = (rectW - nw) / 2;
	int tly = (rectH - nh) / 2;
	Pic_rect.SetRect(tlx, tly, tlx + nw, tly + nh);

	IplImage IpImg = Image;
	CvvImage cimg;
	cimg.CopyOf(&IpImg);
	cimg.DrawToHDC(hDC, &Pic_rect);
	ReleaseDC(pDC);
}

void CReplayMobieyeDlg::ShowVideoImage()
{
	if (src.empty())
		return;
	resize_img = ResizeImg(IDC_PIC, src);
	ShowImg(IDC_PIC, resize_img);
	//Invalidate(1);
}

void  CReplayMobieyeDlg::SetImage(cv::Mat& img)
{
	
	if (img.empty())
		return;
	//AfxMessageBox(L"Enter Image");
	img.copyTo(src);
}

void CReplayMobieyeDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	
	std::thread th(replay_image);
	th.detach();

	SetTimer(1, 25, NULL);

	//CDialogEx::OnOK();
}


void CReplayMobieyeDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	ShowVideoImage();
	CDialogEx::OnTimer(nIDEvent);
}


void CReplayMobieyeDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
	if (nType == SIZE_RESTORED || nType == SIZE_MAXIMIZED)
	{
		ReSize();
	}
}



void CReplayMobieyeDlg::ReSize()
{

	float fsp[2];

	POINT Newp; //获取现在对话框的大小

	CRect recta;

	GetClientRect(&recta);     //取客户区大小  

	Newp.x = recta.right - recta.left;

	Newp.y = recta.bottom - recta.top;

	fsp[0] = (float)Newp.x /old.x;

	fsp[1] = (float)Newp.y /old.y;

	CRect Rect;

	int woc;

	CPoint OldTLPoint, TLPoint; //左上角

	CPoint OldBRPoint, BRPoint; //右下角

	HWND  hwndChild = ::GetWindow(m_hWnd, GW_CHILD);  //列出所有控件  

	while (hwndChild)
	{

		woc = ::GetDlgCtrlID(hwndChild);//取得ID

		GetDlgItem(woc)->GetWindowRect(Rect);

		ScreenToClient(Rect);

		OldTLPoint = Rect.TopLeft();

		TLPoint.x = long(OldTLPoint.x*fsp[0]);

		TLPoint.y = long(OldTLPoint.y*fsp[1]);

		OldBRPoint = Rect.BottomRight();

		BRPoint.x = long(OldBRPoint.x *fsp[0]);

		BRPoint.y = long(OldBRPoint.y *fsp[1]);

		Rect.SetRect(TLPoint, BRPoint);

		GetDlgItem(woc)->MoveWindow(Rect, TRUE);

		hwndChild = ::GetWindow(hwndChild, GW_HWNDNEXT);

	}

	old = Newp;
}

