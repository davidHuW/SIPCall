// DlgBC.cpp : implementation file
//

#include "stdafx.h"
#include "SIPCall.h"
#include "DlgBC.h"
#include "afxdialogex.h"
#include "SIPCallDlg.h"

static CString s_strSongPath = L""; 

static CString s_strFileMusic = L"";
static CString s_strFileTTS = L"";


static int s_songNum = 0;
#define  SONG_MAX_NUM (1000)
#define  COL_MAX_NUM (10)
#define  ROW_MAX_NUM (18)

#define  TTS_FILE_SZ (5)
// CDlgBC dialog

IMPLEMENT_DYNAMIC(CDlgBC, CDialogEx)

CDlgBC::CDlgBC(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgBC::IDD, pParent)
{
	m_xoffset = m_yoffset = 0;
	m_pListSong = NULL;	
	m_nSongNum = 0;
	m_pTunnel = NULL;
	m_nTunnelNum = 0;
	m_nCurIdx = 0;
	m_pPlayGroup = NULL;
}

CDlgBC::~CDlgBC()
{
	m_pPlayGroup = NULL;
}

void CDlgBC::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_SONG, m_ListSong);
	DDX_Control(pDX, IDC_LIST_PLAY, m_ctrlPlay);
	DDX_Control(pDX, IDC_RADIO_FILE, m_RadioFile);
	DDX_Control(pDX, IDC_RADIO_LIST, m_RadioList);
}


BEGIN_MESSAGE_MAP(CDlgBC, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_SONG, &CDlgBC::OnBnClickedBtnSong)
	ON_BN_CLICKED(IDC_BTN_ADD, &CDlgBC::OnBnClickedBtnAdd)
	ON_BN_CLICKED(IDC_BTN_REMOVE, &CDlgBC::OnBnClickedBtnRemove)
	ON_COMMAND(IDC_RADIO_TTS, &CDlgBC::OnBnClickedRadioTts)
	ON_BN_CLICKED(IDC_RADIO_FILE, &CDlgBC::OnBnClickedRadioFile)
	ON_BN_CLICKED(IDC_RADIO_LISTFILE, &CDlgBC::OnBnClickedRadioListfile)
	ON_COMMAND_RANGE(BTN_CREATE_START, BTN_CREATE_END, OnCreateButtonClick)		
	ON_BN_CLICKED(IDC_RADIO_MIC, &CDlgBC::OnBnClickedRadioMic)
	ON_BN_CLICKED(IDC_RADIO_AUX, &CDlgBC::OnBnClickedRadioAux)
	ON_BN_CLICKED(IDOK, &CDlgBC::OnBnClickedOk)
END_MESSAGE_MAP()


// CDlgBC message handlers
void CDlgBC::OnCreateButtonClick(UINT nID)
{
	if (nID >= BTN_CONFIG_BASEID && nID < BTN_CONFIG_BASEID + m_nDeviceMax)
	{
		int a = 0;
		a++;
	}
}

BOOL CDlgBC::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	m_x = GetSystemMetrics(SM_CXFULLSCREEN);
	m_y = GetSystemMetrics(SM_CYFULLSCREEN);

	CRect rt;
	SystemParametersInfo(SPI_GETWORKAREA,0,&rt,0);
	m_y = rt.bottom;
	MoveWindow(0, 0, m_x, m_y);
	SetWindowText(L"配置广播");

	m_pTunnel = g_data.GetData(m_nTunnelNum);
	if (m_pTunnel == NULL || m_nTunnelNum == 0)
	{
		return FALSE;
	}
	SetLayout();

	CButton* pModelList = (CButton*)GetDlgItem(IDC_RADIO_LIST);
	CButton* pModelListOnce = (CButton*)GetDlgItem(IDC_RADIO_LIST_ONCE);
	CButton* pModelOnce = (CButton*)GetDlgItem(IDC_RADIO_ONE_ONCE);
	CButton* pModelConti = (CButton*)GetDlgItem(IDC_RADIO_ONE);
	pModelList->EnableWindow(FALSE);
	pModelList->SetCheck(0);
	pModelListOnce->EnableWindow(FALSE);
	pModelListOnce->SetCheck(0);
	pModelOnce->EnableWindow(FALSE);
	pModelOnce->SetCheck(0);
	pModelConti->EnableWindow(FALSE);
	pModelConti->SetCheck(0);

	LoadSong();
	LoadPlayList();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

CButton*  CDlgBC::NewCheckBox(int nID,CRect rect,int nStyle, CString Caption)
{
	CButton *pButton = new CButton();
	ASSERT_VALID(pButton);
	pButton->Create(Caption,WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX/* | BS_BITMAP*/,rect,this,nID);
	return pButton;

}
void CDlgBC::SetLayout()
{
	if (m_pTunnel == NULL || m_nTunnelNum == 0)
	{
		return;
	}
	SetDeviceLayout();
	SetPlaySrcLayout();
	SetPlayModeLayout();
	SetPlayListLayout();
}
void CDlgBC::SetDeviceLayout()
{
	m_xoffset = 10;
	m_yoffset = 5;

	m_nDeviceMax = 0;

	int padx = 20;
	int pady = 20;
	int btnW = 80;
	int btnH = 30;
	int inter = 5;
	int recNum = m_pTunnel[m_nCurIdx].vecDeviceUp.size() + m_pTunnel[m_nCurIdx].vecDeviceDown.size();
	int col = 0;
	int row = 0;
	if (recNum > COL_MAX_NUM)
	{
		col = COL_MAX_NUM;
	}
	else{
		col = recNum;
	}
	row =  recNum / COL_MAX_NUM + 1;
	if (row > ROW_MAX_NUM)
	{
		row = ROW_MAX_NUM;
	}
#if 1
	STunnel* pTunnel = m_pTunnel + m_nCurIdx;
	int idx = 0;
	int idx1 = 0;
	for (int i = 0; i < pTunnel->vecDeviceUp.size(); i++)
	{
		if (idx > COL_MAX_NUM * ROW_MAX_NUM / 2)
		{
			break;
		}
		int iRow, iCol;
		GetRowCol(idx1, iRow, iCol);
		int left = padx + iCol * (btnW + inter);
		int top = pady + iRow * (btnH + inter);
		CRect rect(left, top, left + btnW, top + btnH);	
		wchar_t wName[MAX_PATH] = {0};
		MultiByteToWideChar(CP_ACP, 0, pTunnel->vecDeviceUp[i].sDB.Name, -1, wName, MAX_PATH);
		
		CButton* pButton = NewCheckBox(BTN_CONFIG_BASEID + idx, rect, 0, wName);

		if (m_bConfigDlg)
		{
			if (pTunnel->vecDeviceUp[i].playFlag == false)
			{
				pButton->EnableWindow(FALSE);
			}			
		}
		else{
			if (/*pTunnel->vecDeviceUp[i].IsConfig() == false &&*/ pTunnel->vecDeviceUp[i].IsCheck())
			{
				pButton->SetCheck(1);
			}
			else{
				pButton->EnableWindow(FALSE);
			}
		}
		idx++;
		idx1++;

		if (!pTunnel->vecDeviceUp[i].sDB.IsVisible())
		{
			pButton->ShowWindow(SW_HIDE);
		}
	}
	int yOffset = ROW_MAX_NUM / 2 * (btnH + inter) + pady;
	int idx2 = 0;
	
	for (int i = 0; i < pTunnel->vecDeviceDown.size(); i++)
	{
		if (idx > COL_MAX_NUM * ROW_MAX_NUM / 2)
		{
			break;
		}
		int iRow, iCol;
		GetRowCol(idx2, iRow, iCol);
		int left = padx + iCol * (btnW + inter);
		int top = pady + iRow * (btnH + inter) + yOffset;
		CRect rect(left, top, left + btnW, top + btnH);	
		wchar_t wName[MAX_PATH] = {0};
		MultiByteToWideChar(CP_ACP, 0, pTunnel->vecDeviceDown[i].sDB.Name, -1, wName, MAX_PATH);
		CButton* pButton = NewCheckBox(BTN_CONFIG_BASEID + idx, rect, 0, wName);
		if (m_bConfigDlg)
		{
			if (pTunnel->vecDeviceDown[i].playFlag == false)
			{
				pButton->EnableWindow(FALSE);
			}
		}
		else{
			if (pTunnel->vecDeviceDown[i].IsConfig() == false && 
				pTunnel->vecDeviceDown[i].IsCheck())
			{
				pButton->SetCheck(1);
			}
			else{
				pButton->EnableWindow(FALSE);
			}
		}
		idx++;
		idx2++;
		if (!pTunnel->vecDeviceDown[i].sDB.IsVisible())
		{
			pButton->ShowWindow(SW_HIDE);
		}
	}

	m_nDeviceMax = idx;

#else
	row = ROW_MAX_NUM;
	col = COL_MAX_NUM;
	for (int i = 0; i < row; i ++)
	{
		for (int j = 0; j < col; j++)
		{
			int left = padx + j * (btnW + inter);
			int top = pady + i * (btnH + inter);
			CRect rect(left, top, left + btnW, top + btnH);	
			CString strName;
			strName.Format(L"ET%03d", i * col + j);
			CButton* pButton = NewCheckBox(i * row + j, rect, 0, strName);
// 			if (j == 5)
// 			{
// 				pButton->EnableWindow(false);
// 			}
		}
	}
#endif
	((CButton*)GetDlgItem(IDC_STATIC_DEVICE_LIST))->SetWindowPos(NULL, m_xoffset, m_yoffset, COL_MAX_NUM * (btnW + inter) + padx, ROW_MAX_NUM / 2 * (btnH + inter) + pady, 0);
	((CButton*)GetDlgItem(IDC_STATIC_DEVICE_LIST2))->SetWindowPos(NULL, m_xoffset, m_yoffset + ROW_MAX_NUM / 2 * (btnH + inter) + pady, COL_MAX_NUM * (btnW + inter) + padx, ROW_MAX_NUM / 2 * (btnH + inter) + pady, 0);
	//m_yoffset += (ROW_MAX_NUM * (btnH + inter) + pady);
}

void  CDlgBC::SetPlaySrcLayout()
{
	m_xoffset = 900;
	m_yoffset = 5;
	int staticW = 450;
	int staticH = 50;
	((CButton*)GetDlgItem(IDC_STATIC_SRC))->MoveWindow(m_xoffset, m_yoffset, staticW, staticH, 0);

	int w = 70; 
	int h = 20;	
	int nInter = 10;
	int yoffset = m_yoffset + nInter * 2;
	((CButton*)GetDlgItem(IDC_RADIO_FILE))->MoveWindow(m_xoffset + nInter, yoffset, w, h, 0);
	((CButton*)GetDlgItem(IDC_RADIO_MIC))->MoveWindow(m_xoffset + nInter + (w + nInter), yoffset, w, h, 0);
	((CButton*)GetDlgItem(IDC_RADIO_TTS))->MoveWindow(m_xoffset + nInter + 2*(w + nInter), yoffset, w, h, 0);
	((CButton*)GetDlgItem(IDC_RADIO_AUX))->MoveWindow(m_xoffset + nInter + 3*(w + nInter), yoffset, w, h, 0);
	((CButton*)GetDlgItem(IDC_RADIO_LISTFILE))->MoveWindow(m_xoffset + nInter + 4*(w + nInter), yoffset, w, h, 0);

	m_yoffset += staticH; 
}
void  CDlgBC::SetPlayModeLayout()
{
	
	int staticW = 450;
	int staticH = 50;
	((CButton*)GetDlgItem(IDC_STATIC_MODE))->MoveWindow(m_xoffset, m_yoffset, staticW, staticH, 0);

	int w = 70; 
	int h = 20;	
	int nInter = 10;
	int yoffset = m_yoffset + nInter * 2;
	((CButton*)GetDlgItem(IDC_RADIO_ONE))->MoveWindow(m_xoffset + nInter + 0*(w + nInter), yoffset, w, h, 0);	
	((CButton*)GetDlgItem(IDC_RADIO_ONE_ONCE))->MoveWindow(m_xoffset + nInter + 1*(w + nInter), yoffset, w, h, 0);	
	((CButton*)GetDlgItem(IDC_RADIO_LIST_ONCE))->MoveWindow(m_xoffset+ nInter + 2*(w + nInter), yoffset, w, h, 0);
	((CButton*)GetDlgItem(IDC_RADIO_LIST))->MoveWindow(m_xoffset+ nInter + 3*(w + nInter), yoffset, w+20, h, 0);
	m_yoffset += staticH; 
}
void  CDlgBC::SetPlayListLayout()
{	
	int yoffset = m_yoffset;
	int w = 60;
	int h = 20;
	((CButton*)GetDlgItem(IDC_STATIC_PLAYLIST))->SetWindowPos(NULL, m_xoffset, yoffset, w, h, 0);
	yoffset += h;
	int ListW = 450;
	int ListH = 245;
	((CButton*)GetDlgItem(IDC_LIST_PLAY))->SetWindowPos(NULL, m_xoffset, yoffset, ListW, ListH, 0);
	yoffset += ListH;

	int btnW = 60;
	int btnH = 30;
	((CButton*)GetDlgItem(IDC_BTN_ADD))->SetWindowPos(NULL, m_xoffset + 150, yoffset, btnW, btnH, 0);
	((CButton*)GetDlgItem(IDC_BTN_REMOVE))->SetWindowPos(NULL, m_xoffset +  300, yoffset, btnW, btnH, 0);
	


	((CButton*)GetDlgItem(IDC_STATIC_SONGLIST))->SetWindowPos(NULL, m_xoffset, yoffset + btnH - h, w, h, 0);
	yoffset += btnH;
	((CButton*)GetDlgItem(IDC_LIST_SONG))->SetWindowPos(NULL, m_xoffset, yoffset, ListW, ListH, 0);
	yoffset += ListH;
// 	w = 60;
// 	h = 30;
// 	((CButton*)GetDlgItem(IDC_BTN_SONG))->SetWindowPos(NULL, m_xoffset + m_x/2 + 55, m_yoffset, w, h, 0);
// 	m_yoffset += h;


	w = 100;
	h = 40;
	((CButton*)GetDlgItem(IDOK))->SetWindowPos(NULL, m_x - w - 50,  yoffset + 10, w, h, 0);
	
}

void  CDlgBC::FindFile(CString strFoldername)
{
	CString m_cstrFileList=L"";
	CFileFind tempFind;
	BOOL bFound; //判断是否成功找到文件
	bFound = tempFind.FindFile(strFoldername + L"\\*.*");   //修改" "内内容给限定查找文件类型
	CString strTmp;   //如果找到的是文件夹 存放文件夹路径
	while(bFound)      //遍历所有文件
	{
		bFound=tempFind.FindNextFile(); //第一次执行FindNextFile是选择到第一个文件，以后执行为选择
		//到下一个文件
		if(tempFind.IsDots()){
			continue; //如果找到的是返回上层的目录 则结束本次查找
		}
		else if(tempFind.IsDirectory())   //找到的是文件夹，则遍历该文件夹下的文件
		{
			strTmp=L"";
			strTmp=tempFind.GetFilePath();
			FindFile(strTmp);
		}
		else
		{
			if (s_songNum < SONG_MAX_NUM)
			{
				strTmp=tempFind.GetFileName(); //保存文件名，包括后缀名
				if (strTmp.Right(4).MakeLower() == L".mp3" ||
					strTmp.Right(4).MakeLower() == L".mp2" ||
					strTmp.Right(4).MakeLower() == L".wav" ||
					strTmp.Right(4).MakeLower() == L".wma")
				{
					// 在此处添加对找到文件的处理
					char szPath[MAX_PATH] = {0};
					WideCharToMultiByte(CP_ACP, 0, tempFind.GetFilePath(), -1, szPath, MAX_PATH, NULL, NULL);
					char szName[MAX_PATH] = {0};
					WideCharToMultiByte(CP_ACP, 0, strTmp, -1, szName, MAX_PATH, NULL, NULL);
					g_data.AddSong(szName, szPath);
					s_songNum++;
				}
			}			
		}
	}
	tempFind.Close();
	return;
}
void CDlgBC::OnBnClickedBtnSong()
{
	// TODO: Add your control notification handler code here
	g_data.ClearSongList();

	TCHAR           szFolderPath[MAX_PATH] = {0};  
	CString         strFolderPath = TEXT("");  

	BROWSEINFO      sInfo;  
	::ZeroMemory(&sInfo, sizeof(BROWSEINFO));  
	sInfo.pidlRoot   = 0;  
	sInfo.lpszTitle   = _T("请选择歌曲文件夹：");  
	sInfo.ulFlags   = BIF_DONTGOBELOWDOMAIN | BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_EDITBOX;  
	sInfo.lpfn     = NULL;  

	// 显示文件夹选择对话框  
	LPITEMIDLIST lpidlBrowse = ::SHBrowseForFolder(&sInfo);   
	if (lpidlBrowse != NULL)  
	{  
		// 取得文件夹名  
		if (::SHGetPathFromIDList(lpidlBrowse,szFolderPath))    
		{  
			s_strSongPath = szFolderPath;  
		}  
	}  
	if(lpidlBrowse != NULL)  
	{  
		::CoTaskMemFree(lpidlBrowse);  
	}  
	LoadSong();
	
}
bool CDlgBC::IsPlayListInSongList(SListSong* pPlayList)
{
	if (pPlayList == NULL)
	{
		return false;
	}
	for (int i = 0; i < m_nSongNum; i++)
	{
		if (strncmp(pPlayList->szPath, m_pListSong[i].szPath, strlen(m_pListSong[i].szPath))==0 )
		{
			return true;
		}
	}
	return false;
}
void CDlgBC::LoadPlayList()
{
	ReadPlayList();
	m_ctrlPlay.ResetContent();
	int nRecNum ;
	SListSong* pPlayList = g_data.GetPlayList(nRecNum);
	for (int i = 0; i < nRecNum; i++)
	{
		wchar_t wzName[MAX_PATH] = {0};
		MultiByteToWideChar(CP_ACP, 0, pPlayList[i].szName, -1, wzName, MAX_PATH);
		m_ctrlPlay.AddString(wzName);
	}

}
void CDlgBC::LoadSong()
{
	s_strSongPath = g_szAPPPath + L"music";
	s_songNum = 0;
	if (s_strSongPath != L"")
	{
		g_data.ClearSongList();
		FindFile(s_strSongPath);
		m_pListSong = g_data.GetSongList(m_nSongNum);
	}
	else{
		m_pListSong = NULL;
		m_nSongNum = 0;
	}
	Song2List();
}
void CDlgBC::Song2List()
{
	if (m_pListSong == NULL || m_nSongNum == 0)
	{
		m_ListSong.ResetContent();
		return;
	}
	for (int i = 0; i < m_nSongNum; i ++)
	{
		wchar_t temp[MAX_PATH] = {0};		
		FileName2FileID(m_pListSong[i].id, m_pListSong[i].szName, temp);
		m_ListSong.AddString(temp);
	}
}

void CDlgBC::OnBnClickedBtnAdd()
{
	// TODO: Add your control notification handler code here
	int idx = m_ListSong.GetCurSel();
	if (idx != -1)
	{
		wchar_t temp[MAX_PATH] = {0};		
		FileName2FileID(m_pListSong[idx].id, m_pListSong[idx].szName, temp);
		int recNum = 0;
		SListSong* pPlayList = g_data.GetPlayList(recNum);
		for (int i = 0; i < recNum; i++)
		{
			if (strcmp(m_pListSong[idx].szName, pPlayList[i].szName) == 0)
			{
				return;
			}
		}
		m_ctrlPlay.AddString(temp);
		g_data.AddPlayList(m_pListSong[idx]);
		
	}	
}

void CDlgBC::OnBnClickedBtnRemove()
{
	// TODO: Add your control notification handler code here
	int idx = m_ctrlPlay.GetCurSel();
	if (idx != -1)
	{
		m_ctrlPlay.DeleteString(idx);
		g_data.DelPlayList(idx);
	}
}
void CDlgBC::FileName2FileID(int id, char* szName, wchar_t* wName)
{
#if 0
	char szTemp[MAX_PATH] = {0}; 
	sprintf(szTemp, "%d - %s", id + 1, szName);
	MultiByteToWideChar(CP_ACP, 0, szTemp, -1, wName, MAX_PATH);
#else
	char szTemp[MAX_PATH] = {0}; 
	sprintf(szTemp, "%s", szName);
	MultiByteToWideChar(CP_ACP, 0, szTemp, -1, wName, MAX_PATH);
#endif
}
void CDlgBC::SetCurTunnelIdx(int curIdx)
{
	m_nCurIdx = curIdx;
}
void CDlgBC::GetRowCol(int idx, int& row, int& col)
{
	row = col = 0;
	row = idx / COL_MAX_NUM;
	col = idx % COL_MAX_NUM;
}

void CDlgBC::OnBnClickedRadioTts()
{
	// TODO: Add your command handler code here
	//AfxMessageBox(L"tts");
	AutoSetPlayMode(ENUM_MODEL_TTS);
	s_strFileTTS = L"";
	CFileDialog dlg(TRUE, //TRUE为OPEN对话框，FALSE为SAVE AS对话框
		NULL, 
		NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		(LPCTSTR)_TEXT("txt Files (*.txt)|*.txt|"),
		NULL);
	if(dlg.DoModal()==IDOK)
	{
		s_strFileTTS = dlg.GetPathName(); //文件名保存在了FilePathName里
		s_strFileTTS.MakeLower();
		UpdateData(FALSE);

		HANDLE hFile = CreateFile(s_strFileTTS, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == NULL || hFile == INVALID_HANDLE_VALUE)
		{
			return;
		}
		DWORD dwSz = GetFileSize(hFile, NULL);
		CloseHandle(hFile);
		if (dwSz < TTS_FILE_SZ)
		{
			s_strFileTTS = L"";
			MessageBox(L"当前文本低于5字节，无法播放，请重新选择文档，谢谢！", L"提醒");
		}
	}
	else{
		MessageBox(L"请设置语音文本路径！", L"提醒");
	}
}

void CDlgBC::OnBnClickedRadioFile()
{
	// TODO: Add your control notification handler code here
	//AfxMessageBox(L"file");
	AutoSetPlayMode(ENUM_MODEL_FILE);
	s_strFileMusic = L"";
	CFileDialog dlg(TRUE, //TRUE为OPEN对话框，FALSE为SAVE AS对话框
		NULL, 
		NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		(LPCTSTR)_TEXT("MP3 Files (*.mp3)|*.mp3|MP2 Files (*.mp2)|*.mp2|WAV Files (*.wav)|*.wav|WMA Files (*.wma)|*.wma||"),
		NULL);
	if(dlg.DoModal()==IDOK)
	{
		s_strFileMusic = dlg.GetPathName(); //文件名保存在了FilePathName里
		UpdateData(FALSE);
	}
	else{
		MessageBox(L"请设置本地文件路径！", L"提醒");
	}
}

void CDlgBC::OnBnClickedRadioListfile()
{
	// TODO: Add your control notification handler code here
	AutoSetPlayMode(ENUM_MODEL_FILE_LIST);
}
void CDlgBC::Ctrl2PlayGroup()
{
	SPlayGroup group;
	// play src
	if (((CButton*)GetDlgItem(IDC_RADIO_FILE))->GetCheck())
	{
		group.attr.enSrc = ENUM_MODEL_FILE;
		memset(group.attr.pFile, 0, MAX_PATH * sizeof(char));
		WideCharToMultiByte(CP_ACP, 0, s_strFileMusic, -1, group.attr.pFile, MAX_PATH, NULL, NULL);
	}
	else if (((CButton*)GetDlgItem(IDC_RADIO_MIC))->GetCheck())
	{
		group.attr.enSrc = ENUM_MODEL_MIC;
	}
	else if (((CButton*)GetDlgItem(IDC_RADIO_TTS))->GetCheck())
	{
		group.attr.enSrc = ENUM_MODEL_TTS;
		memset(group.attr.pFile, 0, MAX_PATH * sizeof(char));
		char czFile[MAX_PATH] = {0};
		WideCharToMultiByte(CP_ACP, 0, s_strFileTTS, -1, czFile, MAX_PATH, NULL, NULL);	
		WideCharToMultiByte(CP_ACP, 0, m_pAPP->Txt2TTSPath(czFile), -1, group.attr.pFile, MAX_PATH, NULL, NULL);
	}
	else if (((CButton*)GetDlgItem(IDC_RADIO_LISTFILE))->GetCheck())
	{
		group.attr.enSrc = ENUM_MODEL_FILE_LIST;
		int curSel = m_ctrlPlay.GetCurSel();
		group.attr.curPlayIndex = 0;
		if (curSel != -1)
		{
			group.attr.curPlayIndex = curSel;
		}			

	}
	else if (((CButton*)GetDlgItem(IDC_RADIO_AUX))->GetCheck())
	{
		group.attr.enSrc = ENUM_MODEL_AUX;
	}
	else{
		group.attr.enSrc = ENUM_MODEL_NONE;
	}
	// play model
	if (((CButton*)GetDlgItem(IDC_RADIO_LIST))->GetCheck())
	{
		group.attr.enModel = ENUM_PLAY_MODEL_LIST;
	}
	else if (((CButton*)GetDlgItem(IDC_RADIO_LIST_ONCE))->GetCheck())
	{
		group.attr.enModel = ENUM_PLAY_MODEL_LIST_ONCE;
	}
	else if (((CButton*)GetDlgItem(IDC_RADIO_ONE))->GetCheck())
	{
		group.attr.enModel = ENUM_PLAY_MODEL_ONE_CONTINUE;
	}
	else if (((CButton*)GetDlgItem(IDC_RADIO_ONE_ONCE))->GetCheck())
	{
		group.attr.enModel = ENUM_PLAY_MODEL_ONE_ONCE;
	}
	else{
		group.attr.enModel = ENUM_PLAY_MODEL_NONE;			
	}
	m_pPlayGroup = g_MgrPlay.AddNew(&group, 1);
}
void CDlgBC::UpdateDeviceAttr(SDevice* pDevice)
{
	// play src
	if (((CButton*)GetDlgItem(IDC_RADIO_FILE))->GetCheck())
	{
		pDevice->playSrc = ENUM_MODEL_FILE;
		memset(pDevice->playFile, 0, MAX_PATH * sizeof(char));
		WideCharToMultiByte(CP_ACP, 0, s_strFileMusic, -1, pDevice->playFile, MAX_PATH, NULL, NULL);
	}
	else if (((CButton*)GetDlgItem(IDC_RADIO_MIC))->GetCheck())
	{
		pDevice->playSrc = ENUM_MODEL_MIC;
	}
	else if (((CButton*)GetDlgItem(IDC_RADIO_TTS))->GetCheck())
	{
		pDevice->playSrc = ENUM_MODEL_TTS;
		memset(pDevice->playFile, 0, MAX_PATH * sizeof(char));
		char czFile[MAX_PATH] = {0};
		WideCharToMultiByte(CP_ACP, 0, s_strFileTTS, -1, czFile, MAX_PATH, NULL, NULL);
		WideCharToMultiByte(CP_ACP, 0, m_pAPP->Txt2TTSPath(czFile), -1, pDevice->playFile, MAX_PATH, NULL, NULL);

	}
	else if (((CButton*)GetDlgItem(IDC_RADIO_LISTFILE))->GetCheck())
	{
		pDevice->playSrc = ENUM_MODEL_FILE_LIST;
		int curSel = m_ctrlPlay.GetCurSel();
		pDevice->curPlayIdx = 0;
		if (curSel != -1)
		{
			pDevice->curPlayIdx = curSel;
		}			
		
	}
	else if (((CButton*)GetDlgItem(IDC_RADIO_AUX))->GetCheck())
	{
		pDevice->playSrc = ENUM_MODEL_AUX;
	}
	else{
		pDevice->playSrc = ENUM_MODEL_NONE;
	}
	// play model
	if (((CButton*)GetDlgItem(IDC_RADIO_LIST))->GetCheck())
	{
		pDevice->playMode = ENUM_PLAY_MODEL_LIST;
	}
	else if (((CButton*)GetDlgItem(IDC_RADIO_LIST_ONCE))->GetCheck())
	{
		pDevice->playMode = ENUM_PLAY_MODEL_LIST_ONCE;
	}
	else if (((CButton*)GetDlgItem(IDC_RADIO_ONE))->GetCheck())
	{
		pDevice->playMode = ENUM_PLAY_MODEL_ONE_CONTINUE;
	}
	else if (((CButton*)GetDlgItem(IDC_RADIO_ONE_ONCE))->GetCheck())
	{
		pDevice->playMode = ENUM_PLAY_MODEL_ONE_ONCE;
	}
	else{
		pDevice->playMode = ENUM_PLAY_MODEL_NONE;			
	}

}

void CDlgBC::SetDlg(bool bConfigDlg)
{
	m_bConfigDlg = bConfigDlg;
}

void CDlgBC::OnBnClickedRadioMic()
{
	// TODO: Add your control notification handler code here
	AutoSetPlayMode(ENUM_MODEL_MIC);
}

void CDlgBC::OnBnClickedRadioAux()
{
	// TODO: Add your control notification handler code here
	AutoSetPlayMode(ENUM_MODEL_AUX);
}
enConfigErrorCode CDlgBC::Check()
{
	// src
	if (((CButton*)GetDlgItem(IDC_RADIO_FILE))->GetCheck() == 0 &&
		((CButton*)GetDlgItem(IDC_RADIO_MIC))->GetCheck() == 0 &&
		((CButton*)GetDlgItem(IDC_RADIO_TTS))->GetCheck() == 0 &&
		((CButton*)GetDlgItem(IDC_RADIO_LISTFILE))->GetCheck() == 0 &&
		((CButton*)GetDlgItem(IDC_RADIO_AUX))->GetCheck() == 0)
	{
		return ENUM_CONFIG_ERROR_PLAY_SRC;
	}
	
	// model
// 	if ((((CButton*)GetDlgItem(IDC_RADIO_LIST))->GetCheck() == 0) &&
// 		(((CButton*)GetDlgItem(IDC_RADIO_ONE))->GetCheck() == 0))
// 	{
// 		return false;
// 	}

	if (((CButton*)GetDlgItem(IDC_RADIO_LISTFILE))->GetCheck())
	{		
		if (g_data.GetPlayListSz() == 0)
		{
			return ENUM_CONFIG_ERROR_PLAYLIST;
		}
	}
	// checkbox
	bool bCheckBox = false;
	for(int i = 0; i < m_nDeviceMax; i ++){
		if (((CButton*)GetDlgItem(BTN_CONFIG_BASEID + i))->GetCheck())
		{
			bCheckBox = true;
			break;
		}
		
	}
	if (!bCheckBox)
	{
		return ENUM_CONFIG_ERROR_DEVICE_CHECK;
	}
	if (((CButton*)GetDlgItem(IDC_RADIO_TTS))->GetCheck() &&
		s_strFileTTS == L"")
	{
		return ENUM_CONFIG_ERROR_TTS_PATH;
	}
	if (((CButton*)GetDlgItem(IDC_RADIO_FILE))->GetCheck() &&
		s_strFileMusic == L"")
	{
		return ENUM_CONFIG_ERROR_FILE_PATH;
	}

	return ENUM_CONFIG_ERROR_NONE;
}

void CDlgBC::UpdateDeviceConfig(int extAddr)
{
	SPlayConfig config;
	config.extAddr = extAddr;

	// play src
	if (((CButton*)GetDlgItem(IDC_RADIO_FILE))->GetCheck())
	{
		config.playSrc = ENUM_MODEL_FILE;		
		WideCharToMultiByte(CP_ACP, 0, s_strFileMusic, -1, config.playFile, MAX_PATH, NULL, NULL);
	}
	else if (((CButton*)GetDlgItem(IDC_RADIO_MIC))->GetCheck())
	{
		config.playSrc = ENUM_MODEL_MIC;
	}
	else if (((CButton*)GetDlgItem(IDC_RADIO_TTS))->GetCheck())
	{
		config.playSrc = ENUM_MODEL_TTS;
		char czFile[MAX_PATH] = {0};
		WideCharToMultiByte(CP_ACP, 0, s_strFileTTS, -1, czFile, MAX_PATH, NULL, NULL);		
		WideCharToMultiByte(CP_ACP, 0, m_pAPP->Txt2TTSPath(czFile), -1, config.playFile, MAX_PATH, NULL, NULL);
	}
	else if (((CButton*)GetDlgItem(IDC_RADIO_LISTFILE))->GetCheck())
	{
		config.playSrc = ENUM_MODEL_FILE_LIST;
		int curSel = m_ctrlPlay.GetCurSel();
		config.curPlayIdx = 0;
		if (curSel != -1)
		{
			config.curPlayIdx = curSel;
		}			

	}
	else if (((CButton*)GetDlgItem(IDC_RADIO_AUX))->GetCheck())
	{
		config.playSrc = ENUM_MODEL_AUX;
	}
	else{
		config.playSrc = ENUM_MODEL_NONE;
	}
	// play model
	if (((CButton*)GetDlgItem(IDC_RADIO_LIST))->GetCheck())
	{
		config.playMode = ENUM_PLAY_MODEL_LIST;
	}
	else if (((CButton*)GetDlgItem(IDC_RADIO_LIST_ONCE))->GetCheck())
	{
		config.playMode = ENUM_PLAY_MODEL_LIST_ONCE;
	}
	else if (((CButton*)GetDlgItem(IDC_RADIO_ONE))->GetCheck())
	{
		config.playMode = ENUM_PLAY_MODEL_ONE_CONTINUE;
	}
	else if (((CButton*)GetDlgItem(IDC_RADIO_ONE_ONCE))->GetCheck())
	{
		config.playMode = ENUM_PLAY_MODEL_ONE_ONCE;
	}
	else{
		config.playMode = ENUM_PLAY_MODEL_NONE;			
	}
	g_Log.output(LOG_TYPE, "update config, ext[%d], src[%d], mode[%d], idx[%d], file[%s].......\r\n", config.extAddr, (int)config.playSrc, (int)config.playMode, config.curPlayIdx, config.playFile);
	g_data.AddPlayConfig(config);
}

void CDlgBC::UpdateConfig()
{
	STunnel* pCurTunnel = m_pTunnel + m_nCurIdx;
	vector<SDevice>::iterator vecIter = pCurTunnel->vecDeviceUp.begin();
	SDevice* pDevice = &(*vecIter);	
	int idx = 0;
	for (int i = 0; i < pCurTunnel->vecDeviceUp.size(); i++)
	{
		if (idx > m_nDeviceMax)
		{
			break;
		}
		if (((CButton*)GetDlgItem(BTN_CONFIG_BASEID + idx))->GetCheck())
		{
			UpdateDeviceConfig(pDevice[i].sDB.extAddr);
		}
		idx++;
	}
	if (!pCurTunnel->vecDeviceDown.empty())
	{
		vecIter = pCurTunnel->vecDeviceDown.begin();
		pDevice = &(*vecIter);
		for (int i = 0; i < pCurTunnel->vecDeviceDown.size(); i++)
		{
			if (idx > m_nDeviceMax)
			{
				break;
			}		
			if (((CButton*)GetDlgItem(BTN_CONFIG_BASEID + idx))->GetCheck())
			{
				UpdateDeviceConfig(pDevice[i].sDB.extAddr);
			}
			idx++;
		}
	}
	
}
void CDlgBC::UpdateAttr()
{
	STunnel* pCurTunnel = m_pTunnel + m_nCurIdx;
	vector<SDevice>::iterator vecIter = pCurTunnel->vecDeviceUp.begin();
	SDevice* pDevice = &(*vecIter);	
	int idx = 0;
	for (int i = 0; i < pCurTunnel->vecDeviceUp.size(); i++)
	{
		if (idx > m_nDeviceMax)
		{
			break;
		}
		if (((CButton*)GetDlgItem(BTN_CONFIG_BASEID + idx))->GetCheck())
		{
			UpdateDeviceAttr(pDevice + i);
		}
		idx++;
	}
	if (!pCurTunnel->vecDeviceDown.empty())
	{
		vecIter = pCurTunnel->vecDeviceDown.begin();
		pDevice = &(*vecIter);
		for (int i = 0; i < pCurTunnel->vecDeviceDown.size(); i++)
		{
			if (idx > m_nDeviceMax)
			{
				break;
			}		
			if (((CButton*)GetDlgItem(BTN_CONFIG_BASEID + idx))->GetCheck())
			{
				UpdateDeviceAttr(pDevice + i);
			}
			idx++;
		}
	}
	
	
}
void CDlgBC::StopOneDevice(SDevice* pDevice, bool& bMic, bool& bAux)
{
	bMic = false;
	bAux = false;
	if (pDevice->IsOffline()){
		return;
	}
	enBCOptStatus opt = pDevice->IsBcPlay();
	if (opt == ENUM_BC_OPT_NONE)
	{
		return;
	}
	//else if (opt == ENUM_BC_OPT_REMOTE)
	//{
	//	//发送远程关闭指令
	//	g_Log.output(LOG_TYPE, "ENUM_SEND_BC_OPT_CLOSE: StopOneDevice dlg: %d\r\n", pDevice->sDB.extAddr);
	//	g_MSG.SendBCStatus(ENUM_SEND_BC_OPT_CLOSE, pDevice->sDB.extAddr);
	//	return;
	//}	
	else if (opt == ENUM_BC_OPT_LOCAL){// 正在播放
		pDevice->enStopFlag = ENUM_STOP_MSG_CHANGE;
		if (pDevice->playSrc == ENUM_MODEL_FILE ||
			pDevice->playSrc == ENUM_MODEL_FILE_LIST ||
			pDevice->playSrc == ENUM_MODEL_TTS){
			// 先停，发送停止消息
			if (pDevice->threadID){				
				g_Play.stop(pDevice->playParam);						
			}
		}		
		else if (pDevice->playSrc == ENUM_MODEL_MIC){
			bMic = true;
			//m_pAPP->EndPlay(pDevice);
			::SendMessage(m_pAPP->m_hWnd, RECV_MSG_STOP_MIC, NULL, (LPARAM)pDevice);
		}
		else if (pDevice->playSrc == ENUM_MODEL_AUX){
			bAux = true;
			//m_pAPP->EndPlay(pDevice);
			::SendMessage(m_pAPP->m_hWnd, RECV_MSG_STOP_AUX, NULL, (LPARAM)pDevice);
		}
	}
}
void CDlgBC::StopPlay()
{
	STunnel* pCurTunnel = m_pTunnel + m_nCurIdx;
	vector<SDevice>::iterator vecIter = pCurTunnel->vecDeviceUp.begin();
	SDevice* pDevice = &(*vecIter);
	bool bCloseMic = false;
	bool bCloseAux = false;
	int idx = 0;
	for (int i = 0; i < pCurTunnel->vecDeviceUp.size(); i++)
	{
		if (idx > m_nDeviceMax)
		{
			break;
		}
		if (((CButton*)GetDlgItem(BTN_CONFIG_BASEID + idx))->GetCheck())
		{
			bool bmic;
			bool baux;
			StopOneDevice(pDevice + i, bmic, baux); 
			if (bmic)
			{
				bCloseMic = true;
			}
			if (baux)
			{
				bCloseAux = true;
			}
		}
		idx++;
	}
	if (!pCurTunnel->vecDeviceDown.empty())
	{
		vecIter = pCurTunnel->vecDeviceDown.begin();
		pDevice = &(*vecIter);
		for (int i = 0; i < pCurTunnel->vecDeviceDown.size(); i++)
		{
			if (idx > m_nDeviceMax)
			{
				break;
			}		
			if (((CButton*)GetDlgItem(BTN_CONFIG_BASEID + idx))->GetCheck())
			{
				bool bmic;
				bool baux;
				StopOneDevice(pDevice + i, bmic, baux); 
				if (bmic)
				{
					bCloseMic = true;
				}
				if (baux)
				{
					bCloseMic = true;
				}

			}


			idx++;
		}
	}
	
	m_pAPP->AutoCloseMicAux(bCloseMic, bCloseAux);
// 	if (bCloseMic)
// 	{
// 		USHORT portAudio = g_Config.m_SystemConfig.portAudio;
// 		g_MSG.SendCloseMic(g_Config.m_SystemConfig.IP_MIC, portAudio);	
// 		g_Play.RecordStop();
// 	}
// 	if (bCloseAux)
// 	{
// 		USHORT portAudio = g_Config.m_SystemConfig.portAudio;
// 		g_MSG.SendCloseAux(g_Config.m_SystemConfig.IP_AUX, portAudio);	
// 	}
	
}
void CDlgBC::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	enConfigErrorCode errorCode = Check();
	if (errorCode == ENUM_CONFIG_ERROR_NONE)
	{		
		if (((CButton*)GetDlgItem(IDC_RADIO_TTS))->GetCheck())
		{
			char file[MAX_PATH] = {0};
			WideCharToMultiByte(CP_ACP, 0, s_strFileTTS, -1, file, MAX_PATH, NULL, NULL);
			m_pAPP->Text2Speach(file);
		}
		if (m_bConfigDlg)
		{
			// 更新至缓存
			UpdateConfig();
			// 停止播放的
			StopPlay();
			// 更新
			//UpdateAttr();
			// 判断要不要开启MIC
			
		}
		else{
			// 更新
			UpdateAttr();
		}
		SavePlayList();
		CDialogEx::OnOK();
	}	
	else{		
		switch(errorCode){
		case ENUM_CONFIG_ERROR_PLAY_SRC:
			MessageBox(L"配置广播失败，请设置广播源！", L"提醒");
			break;
		case ENUM_CONFIG_ERROR_PLAY_MODEL:
			MessageBox(L"配置广播失败，请设置播放方式！", L"提醒");
			break;
		case ENUM_CONFIG_ERROR_DEVICE_CHECK:
			MessageBox(L"配置广播失败，请选择广播设备！", L"提醒");
			break;
		case ENUM_CONFIG_ERROR_FILE_PATH:
			MessageBox(L"配置广播失败，请设置本地文件路径！", L"提醒");
			break;
		case ENUM_CONFIG_ERROR_TTS_PATH:
			MessageBox(L"配置广播失败，请设置语音文本路径！", L"提醒");
			break;
		case ENUM_CONFIG_ERROR_PLAYLIST:
			MessageBox(L"配置广播失败，请设置播放列表！", L"提醒");
			break;
		}
	}
}
void CDlgBC::AutoSetPlayMode(enPlaySrc src)
{
	CButton* pModelList = (CButton*)GetDlgItem(IDC_RADIO_LIST);
	CButton* pModelListOnce = (CButton*)GetDlgItem(IDC_RADIO_LIST_ONCE);
	CButton* pModelOnce = (CButton*)GetDlgItem(IDC_RADIO_ONE_ONCE);
	CButton* pModelConti = (CButton*)GetDlgItem(IDC_RADIO_ONE);

	switch(src){
	case ENUM_MODEL_FILE:
		pModelList->EnableWindow(FALSE);
		pModelList->SetCheck(0);
		pModelListOnce->EnableWindow(FALSE);
		pModelListOnce->SetCheck(0);
		pModelOnce->EnableWindow(TRUE);
		pModelOnce->SetCheck(0);
		pModelConti->EnableWindow(TRUE);
		pModelConti->SetCheck(1);
		break;
	case ENUM_MODEL_FILE_LIST:
		pModelList->EnableWindow(TRUE);
		pModelList->SetCheck(0);
		pModelListOnce->EnableWindow(TRUE);
		pModelListOnce->SetCheck(0);
		pModelOnce->EnableWindow(TRUE);
		pModelOnce->SetCheck(0);
		pModelConti->EnableWindow(TRUE);
		pModelConti->SetCheck(1);
		break;	
	case ENUM_MODEL_TTS:
		pModelList->EnableWindow(FALSE);
		pModelList->SetCheck(0);
		pModelListOnce->EnableWindow(FALSE);
		pModelListOnce->SetCheck(0);
		pModelOnce->EnableWindow(TRUE);
		pModelOnce->SetCheck(0);
		pModelConti->EnableWindow(TRUE);
		pModelConti->SetCheck(1);
		break;
	case ENUM_MODEL_MIC:
		pModelList->EnableWindow(FALSE);
		pModelList->SetCheck(0);
		pModelListOnce->EnableWindow(FALSE);
		pModelListOnce->SetCheck(0);
		pModelOnce->EnableWindow(FALSE);
		pModelOnce->SetCheck(0);
		pModelConti->EnableWindow(FALSE);
		pModelConti->SetCheck(0);
		break;
	case ENUM_MODEL_AUX:
		pModelList->EnableWindow(FALSE);
		pModelList->SetCheck(0);
		pModelListOnce->EnableWindow(FALSE);
		pModelListOnce->SetCheck(0);
		pModelOnce->EnableWindow(FALSE);
		pModelOnce->SetCheck(0);
		pModelConti->EnableWindow(FALSE);
		pModelConti->SetCheck(0);
		break;
	}
}
void CDlgBC::Path2Name(char* szPath, char* szName)
{
	memset(szName, 0, MAX_PATH * sizeof(char));
	int len = strlen(szPath);
	if (len > MAX_PATH)
	{
		return;
	}
	//int szDot = len - 1;
	int szTit = len - 1;
	for(int i = len - 1; i > 0; i--){
		if ((szPath[i] == '\\') && (szTit == len - 1) )
		{
			szTit = i;			
		}
// 		if ((szPath[i] == '.') && (szDot == len - 1))
// 		{
// 			szDot = i;
// 		}
		if (/*(szDot != (len -1)) &&*/ 
			(szTit != (len -1)))
		{
			break;
		}
	}
// 	if (szTit >= szDot)
// 	{
// 		return;
// 	}
// 	memcpy(szName, szPath + szTit + 1, szDot - szTit - 1);
	memcpy(szName, szPath + szTit + 1, len - szTit - 1);
}
void CDlgBC::ReadPlayList()
{
	g_data.ClearPlayList();
	HANDLE hFile = CreateFile(g_szAPPPath + L"play.lst", GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == NULL || hFile == INVALID_HANDLE_VALUE)
	{
		return;
	}
	DWORD dwSz = GetFileSize(hFile, NULL);
	int recNum = dwSz / MAX_PATH;	
	for (int i = 0; i < recNum; i ++)
	{
		DWORD dwRead = 0;
		SListSong playList;
		playList.id = i;
		ReadFile(hFile, playList.szPath, MAX_PATH, &dwRead, NULL);		
		Path2Name(playList.szPath, playList.szName);
		if (IsPlayListInSongList(&playList))
		{
			g_data.AddPlayList(playList);
		}		
	}
	if (hFile)
	{
		CloseHandle(hFile);
		hFile = NULL;
	}
}
void CDlgBC::SavePlayList()
{
	if (g_data.GetPlayListSz() == 0)
	{
		return;
	}
	HANDLE hFile = CreateFile(g_szAPPPath + L"play.lst", GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == NULL || hFile == INVALID_HANDLE_VALUE)
	{
		return;
	}
	int recNum = 0;
	SListSong* pPlayList = g_data.GetPlayList(recNum);
	for (int i = 0; i < recNum; i ++)
	{
		DWORD dwWrite = 0;
		WriteFile(hFile, pPlayList[i].szPath, MAX_PATH, &dwWrite, NULL);
	}
	if (hFile)
	{
		CloseHandle(hFile);
		hFile = NULL;
	}
}

SPlayGroup* CDlgBC::GetPlayGroup()
{
	return m_pPlayGroup;
}
CString CDlgBC::GetTTSFile()
{
	return s_strFileTTS;
}
