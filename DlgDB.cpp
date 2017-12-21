// DlgDB.cpp : implementation file
//

#include "stdafx.h"
#include "SIPCall.h"
#include "DlgDB.h"
#include "afxdialogex.h"
#include "SIPCallDlg.h"

// CDlgDB dialog

IMPLEMENT_DYNAMIC(CDlgDB, CDialogEx)

CDlgDB::CDlgDB(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgDB::IDD, pParent)
{	
	m_dbNum = 0;
	m_pDBInfo = NULL;
	m_maxID = 0;

	memset(m_pEdit, 0, sizeof(int) * 20);
	m_nEditNum = 0;

	m_strArray.Add(L"上");
	m_strArray.Add(L"下");
}

CDlgDB::~CDlgDB()
{
	m_dbNum = 0;
	m_pDBInfo = NULL;

	//m_pAPP->UpdateData();
}

void CDlgDB::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_DB, m_ReportCtrl);
}


BEGIN_MESSAGE_MAP(CDlgDB, CDialogEx)
	ON_BN_CLICKED(ID_BTN_SAVE, &CDlgDB::OnBnClickedBtnSave)
	ON_WM_LBUTTONDBLCLK()
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_DB, &CDlgDB::OnDblclkListDb)
	ON_BN_CLICKED(IDC_BUTTON_ADD, &CDlgDB::OnBnClickedButtonAdd)
	ON_WM_DESTROY()
	ON_BN_CLICKED(ID_BTN_SUB, &CDlgDB::OnBnClickedBtnSub)	
	ON_BN_CLICKED(IDC_BTN_REMOVEALL, &CDlgDB::OnBnClickedBtnRemoveall)
END_MESSAGE_MAP()


// CDlgDB message handlers


BOOL CDlgDB::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	g_Logic.m_UpdateDB = true;
	SetWindowPos(GetDlgItem(IDD_DIALOG_DB), 0, 0, 1000, 600, 0);
	CenterWindow();
	SetWindowText(_T("设备信息"));

	m_ReportCtrl.SetWindowPos(NULL, 0, 0, 1000, 500, 0);
	m_ReportCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_ReportCtrl.SetBkColor(RGB(247,247,255));
	m_ReportCtrl.SetTextColor(RGB(0,0,255));
	m_ReportCtrl.SetTextBkColor(RGB(247,247,255));
	if (m_ReportCtrl.GetItemCount() <= 0) {
		int w = 900 / 9;
		if(g_Config.m_SystemConfig.nMainSystem)
		{
			w = 900 / 10;
		}
		int tt = m_ReportCtrl.InsertColumn(1, _T("编号"), LVCFMT_CENTER, w);
		tt = m_ReportCtrl.InsertColumn(2, L"分机号", LVCFMT_CENTER, w);
		m_pEdit[m_nEditNum++] = tt;
		tt = m_ReportCtrl.InsertColumn(3, L"隧道名称", LVCFMT_CENTER, w);
		m_pEdit[m_nEditNum++] = tt;
		tt = m_ReportCtrl.InsertColumn(4, L"上下标志", LVCFMT_CENTER, w);
		tt = m_ReportCtrl.InsertColumn(5, L"IP地址", LVCFMT_CENTER, w);
		tt = m_ReportCtrl.InsertColumn(6, L"视频IP", LVCFMT_CENTER, w);
		tt = m_ReportCtrl.InsertColumn(7, L"分机地址", LVCFMT_CENTER, w);
		tt = m_ReportCtrl.InsertColumn(8, L"分机号码", LVCFMT_CENTER, w);		
		tt = m_ReportCtrl.InsertColumn(9, L"桩号", LVCFMT_CENTER, w);
		m_pEdit[m_nEditNum++] = tt;
		tt = m_ReportCtrl.InsertColumn(10, L"描述", LVCFMT_CENTER, w);
		m_pEdit[m_nEditNum++] = tt;
		if(g_Config.m_SystemConfig.nMainSystem)
			tt = m_ReportCtrl.InsertColumn(11, L"主/分控IP",LVCFMT_CENTER,w);
	}
	
	m_pDBInfo = g_data.GetDB(m_dbNum);

	for (int i = 0; i < m_dbNum; i++)
	{
		
		int nRow = m_ReportCtrl.InsertItem(i, _T(""));
		
		wchar_t wValue[128] = {0};
		wsprintf(wValue, L"%d", m_pDBInfo[i].id);
		int col = 0;
		m_ReportCtrl.SetItemText(nRow, col, wValue/*m_pDBInfo[i].id*/);
		//SetCtrlStatus(nRow, col);
		col++;
		if (m_pDBInfo[i].id > m_maxID)
		{
			m_maxID = m_pDBInfo[i].id;
		}
		
		wchar_t pValue[256] = {0};
		memset(pValue, 0, 256 * sizeof(wchar_t));
		MultiByteToWideChar(CP_ACP, 0, m_pDBInfo[i].Name, -1, pValue, 256);
		m_ReportCtrl.SetItemText(nRow, col, pValue);
		SetCtrlStatus(nRow, col);
		col++;

		memset(pValue, 0, 256 * sizeof(wchar_t));
		MultiByteToWideChar(CP_ACP, 0, m_pDBInfo[i].tunnelName, -1, pValue, 256);
		m_ReportCtrl.SetItemText(nRow, col, pValue);
		SetCtrlStatus(nRow, col);
		col++;
		
		SetCtrlCombox(nRow, col, m_pDBInfo[i].flag);
		col++;

		memset(pValue, 0, 256 * sizeof(wchar_t));
		MultiByteToWideChar(CP_ACP, 0, m_pDBInfo[i].ip, -1, pValue, 256);	
		m_ReportCtrl.SetItemText(nRow, col, pValue);
		SetCtrlStatus(nRow, col);
		col++;

		memset(pValue, 0, 256 * sizeof(wchar_t));
		MultiByteToWideChar(CP_ACP, 0, m_pDBInfo[i].ip_cam, -1, pValue, 256);	
		m_ReportCtrl.SetItemText(nRow, col, pValue);
		SetCtrlStatus(nRow, col);
		col++;

		memset(pValue, 0, 256 * sizeof(wchar_t));
		wsprintf(pValue, L"%d", m_pDBInfo[i].extAddr);
		m_ReportCtrl.SetItemText(nRow, col, pValue/*m_pDBInfo[i].extAddr*/);
		SetCtrlStatus(nRow, col);
		col++;
		memset(pValue, 0, 256 * sizeof(wchar_t));
		MultiByteToWideChar(CP_ACP, 0, m_pDBInfo[i].extNo, -1, pValue, 256);
		m_ReportCtrl.SetItemText(nRow, col, pValue);
		SetCtrlStatus(nRow, col);
		col++;
		
		memset(pValue, 0, 256 * sizeof(wchar_t));
		MultiByteToWideChar(CP_ACP, 0, m_pDBInfo[i].kmID, -1, pValue, 256);
		m_ReportCtrl.SetItemText(nRow, col, pValue);
		SetCtrlStatus(nRow, col);
		col++;
		memset(pValue, 0, 256 * sizeof(wchar_t));
		MultiByteToWideChar(CP_ACP, 0, m_pDBInfo[i].desc, -1, pValue, 128);
		m_ReportCtrl.SetItemText(nRow, col, pValue);
		SetCtrlStatus(nRow, col);
		col++;
		
		
		/*struct sockaddr_in local;
		local.sin_family = AF_INET;
		local.sin_addr.s_addr = m_pDBInfo[i].controllerip;
		char * pIP = inet_ntoa(local.sin_addr);*/
		if(g_Config.m_SystemConfig.nMainSystem){
			memset(pValue, 0, 256 * sizeof(wchar_t));
			MultiByteToWideChar(CP_ACP, 0, m_pDBInfo[i].controllerip, -1, pValue, 128);
			m_ReportCtrl.SetItemText(nRow, col, pValue);

			SetCtrlStatus(nRow, col);
			col++;
		}
	}
	
	GetDlgItem(IDC_BUTTON_ADD)->SetWindowPos(NULL, 50, 510, 50, 50, 0);	
	GetDlgItem(ID_BTN_SAVE)->SetWindowPos(NULL, 150, 510, 100, 50, 0);	
	GetDlgItem(ID_BTN_SUB)->SetWindowPos(NULL, 450, 510, 100, 50, 0);
	GetDlgItem(IDC_BTN_REMOVEALL)->SetWindowPos(NULL, 650, 510, 100, 50, 0);
	SetCtrlVisul();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CDlgDB::OnBnClickedBtnSave()
{
	// TODO: Add your control notification handler code here		
	g_data.ClearDB();
	for (int k = 0; k < m_dbNum; k++)
	{
		int idx = 0;
		SDBInfo info;
		CString strText = m_ReportCtrl.GetItemText(k, idx++);
		char strValue[256] = {0};
		memset(strValue, 0, 256);
		WideCharToMultiByte(CP_ACP, 0, strText, -1, strValue, 256, NULL, NULL);
		info.id = atoi(strValue);

		strText = m_ReportCtrl.GetItemText(k, idx++);		
		WideCharToMultiByte(CP_ACP, 0, strText, -1, info.Name, 256, NULL, NULL);

		strText = m_ReportCtrl.GetItemText(k, idx++);
		WideCharToMultiByte(CP_ACP, 0, strText, -1, info.tunnelName, 256, NULL, NULL);

		strText = m_ReportCtrl.GetItemText(k, idx++);		
		if (strText == L"上")
		{
			info.flag = 0;
		}
		else{
			info.flag = 1;
		}

		strText = m_ReportCtrl.GetItemText(k, idx++);
		WideCharToMultiByte(CP_ACP, 0, strText, -1, info.ip, 256, NULL, NULL);

		strText = m_ReportCtrl.GetItemText(k, idx++);
		WideCharToMultiByte(CP_ACP, 0, strText, -1, info.ip_cam, 256, NULL, NULL);
		
		strText = m_ReportCtrl.GetItemText(k, idx++);
		memset(strValue, 0, 256);
		WideCharToMultiByte(CP_ACP, 0, strText, -1, strValue, 256, NULL, NULL);
		info.extAddr = atoi(strValue);

		
		strText = m_ReportCtrl.GetItemText(k, idx++);
		WideCharToMultiByte(CP_ACP, 0, strText, -1, info.extNo, 256, NULL, NULL);



		strText = m_ReportCtrl.GetItemText(k, idx++);
		WideCharToMultiByte(CP_ACP, 0, strText, -1, info.kmID, 256, NULL, NULL);

		strText = m_ReportCtrl.GetItemText(k, idx++);
		WideCharToMultiByte(CP_ACP, 0, strText, -1, info.desc, 128, NULL, NULL);

		strText = m_ReportCtrl.GetItemText(k,idx++);
		//memset(strValue, 0, 256);
		WideCharToMultiByte(CP_ACP, 0, strText, -1, info.controllerip, 256, NULL, NULL);
		//info.controllerip = inet_addr(strValue);
		
		if ((info.id > 0 && info.id < 0xFFFFFF) && strlen(info.ip) && (info.extAddr > 0))
		{
			g_data.AddOneDB(info);
		}		
	}
	g_data.SaveDB();
	m_pAPP->UpdateData();
	MessageBox(L"保存数据成功！", L"提醒");
	//CDlgDB::OnOK();
}

void CDlgDB::OnDblclkListDb(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	int row = pNMItemActivate->iItem; //行号 
	int col = pNMItemActivate->iSubItem; //列号 
	
	//m_ReportCtrl.SetItemBkColor(row, col, RGB(255, 0, 0));
	
	*pResult = 0;
}

void CDlgDB::OnBnClickedButtonAdd()
{
	// TODO: Add your control notification handler code here
	m_maxID++;
	int nRow = m_ReportCtrl.InsertItem(m_dbNum++, _T(""));	
	wchar_t wValue[128] = {0};
	wsprintf(wValue, L"%d", m_maxID);

	int col = 0;
	m_ReportCtrl.SetItemText(nRow, col, wValue);		
	//SetCtrlStatus(nRow, col);
	col++;
	
	m_ReportCtrl.SetItemText(nRow, col, L"");
	SetCtrlStatus(nRow, col);
	col++;
	
	m_ReportCtrl.SetItemText(nRow, col, L"");
	SetCtrlStatus(nRow, col);
	col++;

	SetCtrlCombox(nRow, col, 0);
	col++;
			
	m_ReportCtrl.SetItemText(nRow, col, L"");
	SetCtrlStatus(nRow, col);
	col++;

	m_ReportCtrl.SetItemText(nRow, col, L"");
	SetCtrlStatus(nRow, col);
	col++;
	
	m_ReportCtrl.SetItemText(nRow, col, L"");
	SetCtrlStatus(nRow, col);
	col++;
	
	m_ReportCtrl.SetItemText(nRow, col, L"");
	SetCtrlStatus(nRow, col);
	col++;
	
	m_ReportCtrl.SetItemText(nRow, col, L"");
	SetCtrlStatus(nRow, col);
	col++;
	
	m_ReportCtrl.SetItemText(nRow, col, L"");
	SetCtrlStatus(nRow, col);
	col++;

	g_data.AddNullDB(m_maxID);	
	
}

void CDlgDB::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: Add your message handler code here
	g_Logic.m_UpdateDB = false;
}

void CDlgDB::OnBnClickedBtnSub()
{
	// TODO: Add your control notification handler code here
	POSITION pos =  m_ReportCtrl.GetFirstSelectedItemPosition();     
	int indexToBeDeleted = m_ReportCtrl.GetNextSelectedItem(pos);   //取得当前行号

	if (indexToBeDeleted >= 0)
	{
		int id = _ttoi(m_ReportCtrl.GetItemText(indexToBeDeleted,0));
		m_ReportCtrl.DeleteItem(indexToBeDeleted);
		g_data.DeleteOneDB(id);
	}
}

void CDlgDB::OnBnClickedBtnRemoveall()
{
	// TODO: Add your control notification handler code here
	if (MessageBox(L"确定清除所有数据吗？", L"提醒", MB_OKCANCEL | MB_ICONQUESTION) == IDOK)
	{
		m_ReportCtrl.DeleteAllItems();
		g_data.deleteDB();	
		g_data.deleteCall();
		g_data.deleteLog();
		g_data.deleteMsg();
		m_pAPP->OnResetApp();
	}
}
void CDlgDB::SetCtrlStatus(int row, int col)
{
	if (g_dataType == ENUM_TYPE_SUPER)
	{
		m_ReportCtrl.SetEdit(row, col);
	}
	else if (g_dataType == ENUM_TYPE_ADMIN)
	{
		bool bFind = false;
		for (int i = 0; i < m_nEditNum; i ++)
		{
			if (col == m_pEdit[i])
			{
				bFind = true;
				break;
			}
		}
		if (bFind)
		{
			m_ReportCtrl.SetEdit(row, col);
		}
	}
	else{

	}
}
void CDlgDB::SetCtrlVisul()
{
	if (g_dataType == ENUM_TYPE_SUPER)
	{
		
	}
	else if (g_dataType == ENUM_TYPE_ADMIN) //????????
	{
		//(GetDlgItem(ID_BTN_SAVE))->ShowWindow(SW_HIDE);
		(GetDlgItem(IDC_BUTTON_ADD))->ShowWindow(SW_HIDE);
		(GetDlgItem(ID_BTN_SUB))->ShowWindow(SW_HIDE);
		(GetDlgItem(IDC_BTN_REMOVEALL))->ShowWindow(SW_HIDE);
	}
	else{
		(GetDlgItem(ID_BTN_SAVE))->ShowWindow(SW_HIDE);
		(GetDlgItem(IDC_BUTTON_ADD))->ShowWindow(SW_HIDE);
		(GetDlgItem(ID_BTN_SUB))->ShowWindow(SW_HIDE);
		(GetDlgItem(IDC_BTN_REMOVEALL))->ShowWindow(SW_HIDE);		
			
	}
}
void CDlgDB::SetCtrlCombox(int row, int col, int flag)
{
	int idx = 0;
	if (flag == 0)
	{
		idx = 0;
	}
	else{
		idx = 1;
	}

	if (g_dataType == ENUM_TYPE_SUPER)
	{		
		m_ReportCtrl.SetComboBox(row, col, TRUE, &m_strArray, 80, idx);
	}
	else if (g_dataType == ENUM_TYPE_ADMIN)
	{
		
		m_ReportCtrl.SetItemText(row, col, m_strArray[idx]);
	}
	else{		
		m_ReportCtrl.SetItemText(row, col, m_strArray[idx]);
	}
}

