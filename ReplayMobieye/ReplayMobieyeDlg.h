
// ReplayMobieyeDlg.h : ͷ�ļ�
//

#pragma once
#include <opencv2/opencv.hpp>
using namespace cv;

// CReplayMobieyeDlg �Ի���
class CReplayMobieyeDlg : public CDialogEx
{
// ����
public:
	CReplayMobieyeDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_REPLAYMOBIEYE_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


public:
	void  ShowVideoImage();
	void  SetImage(cv::Mat& img);
protected:
	void  ShowImg(UINT ID, cv::Mat &Image);
	Mat ResizeImg(UINT ID, cv::Mat &_Image);

// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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
