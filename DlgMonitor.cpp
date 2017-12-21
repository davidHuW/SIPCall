// DlgMonitor.cpp : implementation file
//

#include "stdafx.h"
#include "SIPCall.h"
#include "DlgMonitor.h"
#include "afxdialogex.h"
#include "SIPCallDlg.h"

// CDlgMonitor dialog

IMPLEMENT_DYNAMIC(CDlgMonitor, CDialogEx)

CDlgMonitor::CDlgMonitor(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgMonitor::IDD, pParent)
{
	m_pTunnelInfo = NULL;
	m_nTunnelNum = 0;
	m_pDevice = NULL;
}

CDlgMonitor::~CDlgMonitor()
{
}

void CDlgMonitor::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_TUNNEL, m_ctrlComboTunnel);
	DDX_Control(pDX, IDC_COMBO_DEVICE, m_ctrlComboDevice);
}


BEGIN_MESSAGE_MAP(CDlgMonitor, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgMonitor::OnBnClickedOk)
	ON_CBN_SELCHANGE(IDC_COMBO_TUNNEL, &CDlgMonitor::OnSelchangeComboTunnel)
	ON_CBN_SELCHANGE(IDC_COMBO_DEVICE, &CDlgMonitor::OnCbnSelchangeComboDevice)
END_MESSAGE_MAP()


// CDlgMonitor message handlers


void CDlgMonitor::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	

	if (m_nTunnelNum > 0)
	{
		int nTunnelIdx = m_ctrlComboTunnel.GetCurSel();
		/*
		int nDeviceIdx = m_ctrlComboDevice.GetCurSel();
		int upSz = m_pTunnelInfo[nTunnelIdx].vecDeviceUp.size();
		int downSz = m_pTunnelInfo[nTunnelIdx].vecDeviceDown.size();
		if (nDeviceIdx >= upSz)
		{
			nDeviceIdx -= upSz; 
			// 取下面
			vector<SDevice>::iterator vecIter = m_pTunnelInfo[nTunnelIdx].vecDeviceDown.begin() + nDeviceIdx;
			m_pDevice = &(*vecIter);
		}
		else{
			// 取上面
			vector<SDevice>::iterator vecIter = m_pTunnelInfo[nTunnelIdx].vecDeviceUp.begin() + nDeviceIdx;
			m_pDevice = &(*vecIter);
		}
		*/
		CString strtext;
		GetDlgItemText(IDC_COMBO_DEVICE, strtext);
		char czDeviceName[MAX_PATH] = {0};
		WideCharToMultiByte(CP_ACP, 0, strtext, -1, czDeviceName, MAX_PATH, NULL, NULL);
		m_pDevice = FindDevice(czDeviceName, nTunnelIdx);

	}
	

	CDialogEx::OnOK();
}
SDevice* CDlgMonitor::FindDevice(char* czName, int nTunnelIdx)
{
	for (int i = 0; i < m_pTunnelInfo[nTunnelIdx].vecDeviceUp.size(); i++)
	{
		if (strncmp(czName, m_pTunnelInfo[nTunnelIdx].vecDeviceUp[i].sDB.Name, strlen(czName)) == 0)
		{
			vector<SDevice>::iterator vecIter = m_pTunnelInfo[nTunnelIdx].vecDeviceUp.begin() + i;
			return  &(*vecIter);
		}
	}
	for (int i = 0; i < m_pTunnelInfo[nTunnelIdx].vecDeviceDown.size(); i++)
	{
		if (strncmp(czName, m_pTunnelInfo[nTunnelIdx].vecDeviceDown[i].sDB.Name, strlen(czName)) == 0)
		{
			vector<SDevice>::iterator vecIter = m_pTunnelInfo[nTunnelIdx].vecDeviceDown.begin() + i;
			return  &(*vecIter);
		}
	}
	return NULL;
}

BOOL CDlgMonitor::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	m_pTunnelInfo = g_data.GetData(m_nTunnelNum);
		
	if (m_nTunnelNum > 0)
	{
		int nTunnelIdx = 0;
		int nDeviceIdx = 0;
		//GetMonitorInfo(nTunnelIdx, nDeviceIdx);

		for (int i = 0; i < m_nTunnelNum; i++)
		{
			wchar_t wzName[MAX_PATH] = {0};
			MultiByteToWideChar(CP_ACP, 0, m_pTunnelInfo[i].Name, -1, wzName, MAX_PATH);
			m_ctrlComboTunnel.AddString(wzName);
			// 得到idx
			if (strlen(m_stMonitor.czTunnelName) > 0)
			{
				if (strncmp(m_stMonitor.czTunnelName, m_pTunnelInfo[i].Name, strlen(m_pTunnelInfo[i].Name)) == 0)
				{
					nTunnelIdx = i;
				}
			}
		}
		m_ctrlComboTunnel.SetCurSel(nTunnelIdx);

		int nAddIdx = 0;
		for (int i = 0; i < m_pTunnelInfo[nTunnelIdx].vecDeviceUp.size(); i++)
		{
			if (!m_pTunnelInfo[nTunnelIdx].vecDeviceUp[i].sDB.IsVisible())
			{
				continue;
			}
			wchar_t wzName[MAX_PATH] = {0};
			MultiByteToWideChar(CP_ACP, 0, m_pTunnelInfo[nTunnelIdx].vecDeviceUp[i].sDB.Name, -1, wzName, MAX_PATH);
			m_ctrlComboDevice.AddString(wzName);
			
			// 得到idx
			if (strlen(m_stMonitor.czETName) > 0)
			{
				if (strncmp(m_stMonitor.czETName, m_pTunnelInfo[nTunnelIdx].vecDeviceUp[i].sDB.Name, strlen(m_pTunnelInfo[nTunnelIdx].vecDeviceUp[i].sDB.Name)) == 0)
				{
					nDeviceIdx = nAddIdx;
				}
			}
			nAddIdx++;
		}
		for (int i = 0; i < m_pTunnelInfo[nTunnelIdx].vecDeviceDown.size(); i++)
		{
			if (!m_pTunnelInfo[nTunnelIdx].vecDeviceDown[i].sDB.IsVisible())
			{
				continue;
			}
			wchar_t wzName[MAX_PATH] = {0};
			MultiByteToWideChar(CP_ACP, 0, m_pTunnelInfo[nTunnelIdx].vecDeviceDown[i].sDB.Name, -1, wzName, MAX_PATH);
			m_ctrlComboDevice.AddString(wzName);
			// 得到idx
			if (strlen(m_stMonitor.czETName) > 0)
			{
				if (strncmp(m_stMonitor.czETName, m_pTunnelInfo[nTunnelIdx].vecDeviceDown[i].sDB.Name, strlen(m_pTunnelInfo[nTunnelIdx].vecDeviceDown[i].sDB.Name)) == 0)
				{
					nDeviceIdx = nAddIdx;
				}
			}
			nAddIdx++;
		}
		m_ctrlComboDevice.SetCurSel(nDeviceIdx);
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CDlgMonitor::OnSelchangeComboTunnel()
{
	// TODO: Add your control notification handler code here
	m_ctrlComboDevice.ResetContent();
	int idx = m_ctrlComboTunnel.GetCurSel();
	for (int i = 0; i < m_pTunnelInfo[idx].vecDeviceUp.size(); i++)
	{
		if (!m_pTunnelInfo[idx].vecDeviceUp[i].sDB.IsVisible())
		{
			continue;
		}
		wchar_t wzName[MAX_PATH] = {0};
		MultiByteToWideChar(CP_ACP, 0, m_pTunnelInfo[idx].vecDeviceUp[i].sDB.Name, -1, wzName, MAX_PATH);
		m_ctrlComboDevice.AddString(wzName);
	}
	for (int i = 0; i < m_pTunnelInfo[idx].vecDeviceDown.size(); i++)
	{
		if (!m_pTunnelInfo[idx].vecDeviceDown[i].sDB.IsVisible())
		{
			continue;
		}
		wchar_t wzName[MAX_PATH] = {0};
		MultiByteToWideChar(CP_ACP, 0, m_pTunnelInfo[idx].vecDeviceDown[i].sDB.Name, -1, wzName, MAX_PATH);
		m_ctrlComboDevice.AddString(wzName);
	}
	m_ctrlComboDevice.SetCurSel(0);
}


void CDlgMonitor::OnCbnSelchangeComboDevice()
{
	// TODO: Add your control notification handler code here
	int a = 0;
	a = m_ctrlComboDevice.GetCurSel();


}
SDevice* CDlgMonitor::GetDevice()
{
	return m_pDevice;
}

void CDlgMonitor::SetMonitorInfo(char* czTunnelName, char* czETName)
{
	m_stMonitor.init();
	strncpy(m_stMonitor.czTunnelName, czTunnelName, strlen(czTunnelName));
	strncpy(m_stMonitor.czETName, czETName, strlen(czETName));
}
void CDlgMonitor::GetMonitorInfo(int &nTunnelIdx, int& nDeviceIdx)
{
	nTunnelIdx = nDeviceIdx = 0;
	if ((strlen(m_stMonitor.czETName) > 0) && 
		(strlen(m_stMonitor.czTunnelName) > 0))
	{
		CString strTunnel;
		GetDlgItemText(IDC_COMBO_TUNNEL, strTunnel);
		char czTunnelName[MAX_PATH] = {0};
		WideCharToMultiByte(CP_ACP, 0, strTunnel, -1, czTunnelName, MAX_PATH, NULL, NULL);

		CString strDevice;
		GetDlgItemText(IDC_COMBO_DEVICE, strDevice);
		char czDeviceName[MAX_PATH] = {0};
		WideCharToMultiByte(CP_ACP, 0, strDevice, -1, czDeviceName, MAX_PATH, NULL, NULL);

		
		for (int i = 0; i < m_nTunnelNum; i++)
		{
			if (strncmp(m_pTunnelInfo[i].Name, czTunnelName, strlen(czTunnelName)) == 0)
			{
				int nIdx = 0;
				for (int j = 0; j < m_pTunnelInfo[i].vecDeviceUp.size(); j++)
				{
					if (!m_pTunnelInfo[i].vecDeviceUp[j].sDB.IsVisible())
					{
						continue;
					}
					if (strncmp(m_pTunnelInfo[i].vecDeviceUp[j].sDB.Name, czDeviceName, strlen(czDeviceName)) == 0)
					{
						nTunnelIdx = i;
						nDeviceIdx = nIdx;
						return;
					}					
					nIdx++;					
				}
				for (int j = 0; j < m_pTunnelInfo[i].vecDeviceDown.size(); j++)
				{
					if (!m_pTunnelInfo[i].vecDeviceDown[j].sDB.IsVisible())
					{
						continue;
					}
					if (strncmp(m_pTunnelInfo[i].vecDeviceDown[j].sDB.Name, czDeviceName, strlen(czDeviceName)) == 0)
					{
						nTunnelIdx = i;
						nDeviceIdx = nIdx;
						return;
					}					
					nIdx++;	
				}
				
			}	

		}
	}
}