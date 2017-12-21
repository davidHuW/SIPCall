#pragma once
#include "afxwin.h"
#include "DataM.h"

struct SMonitorDeviceInfo{
	char czTunnelName[MAX_PATH];
	char czETName[MAX_PATH];
	SMonitorDeviceInfo(){
		init();
	}
	void init(){
		memset(czETName, 0, MAX_PATH * sizeof(char));
		memset(czTunnelName, 0, MAX_PATH * sizeof(char));
	}
};
// CDlgMonitor dialog

class CDlgMonitor : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgMonitor)

public:
	CDlgMonitor(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgMonitor();
	void SetMonitorInfo(char* czTunnelName, char* czETName);
	void GetMonitorInfo(int &nTunnelIdx, int& nDeviceIdx);
	SDevice* FindDevice(char* czName, int nTunnelIdx);
	SDevice* GetDevice();
private:
	SDevice* m_pDevice;
	STunnel* m_pTunnelInfo;
	int m_nTunnelNum;
	SMonitorDeviceInfo m_stMonitor;
// Dialog Data
	enum { IDD = IDD_DIALOG_MONITOR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeComboTunnel();
	afx_msg void OnCbnSelchangeComboDevice();
	CComboBox m_ctrlComboTunnel;
	CComboBox m_ctrlComboDevice;
};
