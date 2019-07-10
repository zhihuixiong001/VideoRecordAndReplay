
// ReplayMobieyeDlg.h : 头文件
//

#pragma once
#include <opencv2/opencv.hpp>
using namespace cv;

// CReplayMobieyeDlg 对话框
class CReplayMobieyeDlg : public CDialogEx
{
// 构造
public:
	CReplayMobieyeDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_REPLAYMOBIEYE_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


public:
	void  ShowVideoImage();
	void  SetImage(cv::Mat& img);
protected:
	void  ShowImg(UINT ID, cv::Mat &Image);
	Mat ResizeImg(UINT ID, cv::Mat &_Image);

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

	DECLARE_MESSAGE_MAP()
protected:
	cv::Mat src;
	cv::Mat resize_img;
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSize(UINT nType, int cx, int cy);
protected:
	void ReSize();
private:
	POINT old;
};
