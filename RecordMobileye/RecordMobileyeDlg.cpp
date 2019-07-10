
// RecordMobileyeDlg.cpp : ʵ���ļ�
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
// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���


class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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

// CRecordMobileyeDlg �Ի���
Mat gdiScreenCapture(HDC hCDC, HDC hDDC, int nWidth, int nHeight) {
	HBITMAP hBitmap = CreateCompatibleBitmap(hDDC, nWidth, nHeight);//�õ�λͼ    
	SelectObject(hCDC, hBitmap); //�����ܵ���ôд��             
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
	int nWidth = GetSystemMetrics(SM_CXSCREEN);//�õ���Ļ�ķֱ��ʵ�x    
	int nHeight = GetSystemMetrics(SM_CYSCREEN);//�õ���Ļ�ֱ��ʵ�y    
	HDC hDDC = GetDC(GetDesktopWindow());//�õ���Ļ��dc    
	HDC hCDC = CreateCompatibleDC(hDDC);//    
	Mat img = gdiScreenCapture(hCDC, hDDC, nWidth, nHeight);
#ifdef RECORD_FILE
	VideoWriter vw;
	vw.open("d:\\2.avi", CV_FOURCC('M', 'P', '4', '2'), 8, cvSize(img.cols, img.rows));
#endif
	//���ͱ���
	boost::system::error_code error;
	
	const int blocksize = 1440;
	//int nSendBuf = 1024 * 1024 * 50;//���ջ���10M 
	//������ṹ 
	struct recvbuf//����ʽ  
	{
		char buf[blocksize];//������ݵı���  
		int flag;//��־   
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
		//��������
		/*
		CString str;
		str.Format(L"Width %d Height %d", nWidth, nHeight);
		AfxMessageBox(str);
		*/
		//ѹ����ʽ
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
			
				//��������
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
		//ָ���֡�����ݾ���  
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
				//��������
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


// CRecordMobileyeDlg ��Ϣ�������

BOOL CRecordMobileyeDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CRecordMobileyeDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CRecordMobileyeDlg::OnQueryDragIcon()
{	return static_cast<HCURSOR>(m_hIcon);
}



void CRecordMobileyeDlg::OnBnClickedOk()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	std::thread th(savevideo);
	th.detach();
	//CDialogEx::OnOK();
}

