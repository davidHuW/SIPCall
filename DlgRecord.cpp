// DlgRecord.cpp : implementation file
//

#include "stdafx.h"
#include "SIPCall.h"
#include "DlgRecord.h"
#include "SIPCallDlg.h"

// CDlgRecord dialog

IMPLEMENT_DYNAMIC(CDlgRecord, CDialog)

CDlgRecord::CDlgRecord(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgRecord::IDD, pParent)
{
	m_idx = 1;
	m_pRecord = NULL;
	m_nNum = 0;
	m_SortType = ENUM_SORT_ALL;
}

CDlgRecord::~CDlgRecord()
{

}

void CDlgRecord::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_CALL, m_ListCtrl);
}


BEGIN_MESSAGE_MAP(CDlgRecord, CDialog)
	ON_BN_CLICKED(IDC_RADIO1, &CDlgRecord::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO2, &CDlgRecord::OnBnClickedRadio2)
	ON_BN_CLICKED(IDC_RADIO3, &CDlgRecord::OnBnClickedRadio3)
	ON_BN_CLICKED(IDC_BTN_ALL, &CDlgRecord::OnBnClickedBtnAll)
	ON_BN_CLICKED(IDC_BTN_MIC_REC, &CDlgRecord::OnBnClickedBtnMicRec)
	ON_BN_CLICKED(IDC_BTN_REC_EXPORT, &CDlgRecord::OnBnClickedBtnRecExport)
	ON_BN_CLICKED(IDC_BTN_REC_PRINT, &CDlgRecord::OnBnClickedBtnRecPrint)
	ON_BN_CLICKED(IDC_RADIO_HOUR, &CDlgRecord::OnBnClickedRadioHour)
	ON_BN_CLICKED(IDC_RADIO_DAY, &CDlgRecord::OnBnClickedRadioDay)
END_MESSAGE_MAP()


// CDlgRecord message handlers
void CDlgRecord::SetRecord(int idx)
{
	m_idx = idx;
	
}
void CDlgRecord::SetLayout()
{
	 m_ListCtrl.SetExtendedStyle(m_ListCtrl.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);   
	if (m_idx == 1)
	{
		SetWindowText(_T("�绰��¼"));

		m_ListCtrl.InsertColumn(1, _T("�ֻ���"), LVCFMT_CENTER, 100);
		m_ListCtrl.InsertColumn(2, _T("���"), LVCFMT_CENTER, 100); 
		m_ListCtrl.InsertColumn(3, _T("�ֻ�SIP��"), LVCFMT_CENTER, 80);   
		m_ListCtrl.InsertColumn(4, _T("�ֻ���ַ"), LVCFMT_CENTER, 60);  
		m_ListCtrl.InsertColumn(5, _T("�ֻ�IP��ַ"), LVCFMT_CENTER, 80);
		m_ListCtrl.InsertColumn(6, _T("�Ӿ��绰"), LVCFMT_CENTER, 60); 		 
		m_ListCtrl.InsertColumn(7, _T("׮��"), LVCFMT_CENTER, 100); 
		m_ListCtrl.InsertColumn(8, _T("����"), LVCFMT_CENTER, 120); 

		//GetDlgItem(IDC_BTN_MIC_REC)->SetDlgItemText(L"¼����ѯ");
		GetDlgItem(IDC_BTN_MIC_REC)->ShowWindow(SW_SHOW);

	}
	else if (m_idx == 2)
	{
		SetWindowText(_T("�㲥��¼"));
		m_ListCtrl.InsertColumn(1, _T("�ֻ���"), LVCFMT_CENTER, 100); 
		m_ListCtrl.InsertColumn(2, _T("���"), LVCFMT_CENTER, 100);
		m_ListCtrl.InsertColumn(3, _T("�ֻ�SIP��"), LVCFMT_CENTER, 80);   
		m_ListCtrl.InsertColumn(4, _T("�ֻ���ַ"), LVCFMT_CENTER, 60); 
		m_ListCtrl.InsertColumn(5, _T("�ֻ�IP��ַ"), LVCFMT_CENTER, 80);
		m_ListCtrl.InsertColumn(6, _T("�㲥����"), LVCFMT_CENTER, 60); 		
		m_ListCtrl.InsertColumn(7, _T("׮��"), LVCFMT_CENTER, 100); 
		m_ListCtrl.InsertColumn(8, _T("����"), LVCFMT_CENTER, 120);

		GetDlgItem(IDC_BTN_MIC_REC)->ShowWindow(SW_SHOW);
	}
	else{
		SetWindowText(_T("Ѳ���¼"));
		m_ListCtrl.InsertColumn(1, _T("�ֻ���"), LVCFMT_CENTER, 80); 
		m_ListCtrl.InsertColumn(2, _T("���"), LVCFMT_CENTER, 80); 
		m_ListCtrl.InsertColumn(3, _T("�ֻ�SIP��"), LVCFMT_CENTER, 80);   
		m_ListCtrl.InsertColumn(4, _T("�ֻ���ַ"), LVCFMT_CENTER, 60); 
		m_ListCtrl.InsertColumn(5, _T("�ֻ�IP��ַ"), LVCFMT_CENTER, 100);
		m_ListCtrl.InsertColumn(6, _T("״̬"), LVCFMT_CENTER, 180); 
		m_ListCtrl.InsertColumn(7, _T("׮��"), LVCFMT_CENTER, 80); 
		m_ListCtrl.InsertColumn(8, _T("����"), LVCFMT_CENTER, 120);
	}


	// data to listctrl	
	for (int i = 0; i < m_nNum; i ++)
	{
		int nRow = m_ListCtrl.InsertItem(i, _T(""));

		TCHAR tValue[64] = {0};
		m_ListCtrl.SetItemText(nRow, 0, m_pRecord[i].noid);
		m_ListCtrl.SetItemText(nRow, 1, m_pRecord[i].tunnel);
		wsprintf(tValue, _T("%s"), m_pRecord[i].sip);
		m_ListCtrl.SetItemText(nRow, 2, tValue);
		wsprintf(tValue, _T("%d"), m_pRecord[i].addr);
		m_ListCtrl.SetItemText(nRow, 3, tValue);

		m_ListCtrl.SetItemText(nRow, 4, m_pRecord[i].IP);
		m_ListCtrl.SetItemText(nRow, 5, m_pRecord[i].desc);
		
		
		m_ListCtrl.SetItemText(nRow, 6, m_pRecord[i].kmid);
		m_ListCtrl.SetItemText(nRow, 7, m_pRecord[i].data);
	}

}

BOOL CDlgRecord::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	((CButton *)GetDlgItem(IDC_RADIO_HOUR))->SetCheck(TRUE);
	m_SortType = ENUM_SORT_HOUR;
	LoadRecord();
	
	SetLayout();
	
	if (g_dataType == ENUM_TYPE_USER)
	{
		GetDlgItem(IDC_BTN_ALL)->ShowWindow(SW_HIDE);
	}
	//ExportXLS();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
void CDlgRecord::LoadRecord()
{
	if (m_idx == 1)
	{
		g_data.ReadCall(m_SortType);
		m_pRecord = g_data.GetRecordCall(m_nNum);
	}
	else if (m_idx == 2)
	{
		g_data.ReadMsg(m_SortType);
		m_pRecord = g_data.GetRecordMsg(m_nNum);
	}
	else{
		g_data.ReadLog(m_SortType);
		m_pRecord = g_data.GetRecordLog(m_nNum);
	}

}

void CDlgRecord::OnBnClickedRadio1()
{
	// TODO: Add your control notification handler code here
	m_SortType = ENUM_SORT_ALL;
	LoadRecord();
	UpdateRecord();
}


void CDlgRecord::OnBnClickedRadio2()
{
	// TODO: Add your control notification handler code here
	m_SortType = ENUM_SORT_WEEK;
	LoadRecord();
	UpdateRecord();
}


void CDlgRecord::OnBnClickedRadio3()
{
	// TODO: Add your control notification handler code here
	m_SortType = ENUM_SORT_MONTH;
	LoadRecord();
	UpdateRecord();
}
void CDlgRecord::UpdateRecord()
{
	m_ListCtrl.DeleteAllItems();
	// data to listctrl	
	for (int i = 0; i < m_nNum; i ++)
	{
		int nRow = m_ListCtrl.InsertItem(i, _T(""));

		m_ListCtrl.SetItemText(nRow, 0, m_pRecord[i].noid);
		m_ListCtrl.SetItemText(nRow, 1, m_pRecord[i].tunnel);
		TCHAR tValue[64] = {0};
		
		wsprintf(tValue, _T("%s"), m_pRecord[i].sip);
		m_ListCtrl.SetItemText(nRow, 2, tValue);
		wsprintf(tValue, _T("%d"), m_pRecord[i].addr);
		m_ListCtrl.SetItemText(nRow, 3, tValue);

		m_ListCtrl.SetItemText(nRow, 4, m_pRecord[i].IP);
		m_ListCtrl.SetItemText(nRow, 5, m_pRecord[i].desc);		
		
		m_ListCtrl.SetItemText(nRow, 6, m_pRecord[i].kmid);
		m_ListCtrl.SetItemText(nRow, 7, m_pRecord[i].data);
	}
}

void CDlgRecord::OnBnClickedBtnAll()
{
	// TODO: Add your control notification handler code here
	if (MessageBox(L"ȷ�����������", L"����", MB_OKCANCEL | MB_ICONQUESTION) == IDOK)
	{
		m_ListCtrl.DeleteAllItems();

		if (m_idx == 1)
		{
			g_data.deleteCall();
		}
		else if (m_idx == 2)
		{			
			g_data.deleteMsg();
		}
		else{			
			g_data.deleteLog();
		}	
		CDlgRecord::OnCancel();
	}	
}

BOOL FileExist(CString FileName)
{
	CFileFind fFind;
	return fFind.FindFile(FileName);
}

BOOL DirectoryExist(CString Path)
{
	WIN32_FIND_DATA fd;
	BOOL ret = FALSE;
	HANDLE hFind = FindFirstFile(Path, &fd);
	if ((hFind != INVALID_HANDLE_VALUE) && (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
	{
		//Ŀ¼����
		ret = TRUE;

	}
	FindClose(hFind);
	return ret;
}
void CDlgRecord::OnBnClickedBtnMicRec()
{
	// TODO: Add your control notification handler code here
	if (m_idx == 1)
	{
		wchar_t wOpenPath[MAX_PATH] = {0};
		MultiByteToWideChar(CP_ACP, 0, g_Config.m_SystemConfig.path, -1, wOpenPath, MAX_PATH);
		if (DirectoryExist(wOpenPath))
		{
			ShellExecute(NULL, L"open", wOpenPath, NULL, NULL, SW_SHOW); 
		}
		else{
			MessageBox(L"���õ��ļ��в����ڣ����������ã�", L"����");
		}
	}
	else if (m_idx == 2)
	{
 		wchar_t wOpenPath[MAX_PATH] = {0};
 		MultiByteToWideChar(CP_ACP, 0, g_Config.m_SystemConfig.pathRecord, -1, wOpenPath, MAX_PATH);
		//CString strMic = g_szAPPPath + L"record";
		if (DirectoryExist(wOpenPath))
		{
			ShellExecute(NULL, L"open", wOpenPath, NULL, NULL, SW_SHOW); 
		}
		else{
			MessageBox(L"¼���ļ��в����ڣ����������ã�", L"����");
		}
	}
}

void   GetCellName(int nRow, int nCol, CString &strName)

{

	int nSeed = nCol;

	CString strRow;

	char cCell = 'A' + nCol - 1;



	strName.Format(_T("%c"), cCell);



	strRow.Format(_T( "%d "), nRow);

	strName += strRow;

}

void CDlgRecord::ExportXLS(CString strFile)
{
	// = _T("E:\\Test.xls");

	COleVariant
		covTrue((short)TRUE),
		covFalse((short)FALSE),
		covOptional((long)DISP_E_PARAMNOTFOUND,   VT_ERROR);

	CApplication   app;
	CWorkbooks   books;
	CWorkbook   book;
	CWorksheets   sheets;
	CWorksheet   sheet;
	CRange   range;
	CFont0   font;

	if (!app.CreateDispatch(_T("Excel.Application")))
	{
		MessageBox(_T("����ʧ�ܣ�"));
		return;
	}

	//Get   a   new   workbook.

	books = app.get_Workbooks();
	book = books.Add(covOptional);
	sheets = book.get_Worksheets();
	sheet = sheets.get_Item(COleVariant((short)1));
	////////////////////////////////////CListCtrl�ؼ�report���//////////////////////////////////////////////////////////

	CHeaderCtrl   *pmyHeaderCtrl;
	pmyHeaderCtrl = m_ListCtrl.GetHeaderCtrl();//�˾�ȡ��CListCtrl�ؼ����б��^
	int   iRow,iCol;
	int   m_cols   =   pmyHeaderCtrl-> GetItemCount();
	int   m_rows = m_ListCtrl.GetItemCount();
	HDITEM   hdi;
	TCHAR     lpBuffer[256];
	bool       fFound   =   false;
	hdi.mask   =   HDI_TEXT;
	hdi.pszText   =   lpBuffer;
	hdi.cchTextMax   =   256;
	CString   colname;
	CString strTemp;
	for(iCol=0;   iCol <m_cols;   iCol++)//���б�ı���ͷд��EXCEL
	{

		GetCellName(1 ,iCol + 1, colname);
		range   =   sheet.get_Range(COleVariant(colname),COleVariant(colname));
		pmyHeaderCtrl-> GetItem(iCol,   &hdi);
		range.put_Value2(COleVariant(hdi.pszText));
		int   nWidth   =   m_ListCtrl.GetColumnWidth(iCol)/6;
		//�õ���iCol+1��  
		range.AttachDispatch(range.get_Item(_variant_t((long)(iCol+1)),vtMissing).pdispVal,true); 
		//�����п� 
		range.put_ColumnWidth(_variant_t((long)nWidth));

	}

	range   =   sheet.get_Range(COleVariant( _T("A1 ")),   COleVariant(colname));
	//range.put_RowHeight(_variant_t((long)50));//�����еĸ߶�
	font = range.get_Font();
	font.put_Bold(covTrue);
	//range.put_VerticalAlignment(COleVariant((short)-4108));//xlVAlignCenter   =   -4108
	// varUnit.lVal = -4108;//ˮƽ��-4131����-4152���ҡ���ֱ��-4160���ϣ�-4107����
	range.put_HorizontalAlignment(COleVariant((short)-4108));


	COleSafeArray   saRet;
	DWORD   numElements[]={m_rows,m_cols};       //5x2   element   array
	saRet.Create(VT_BSTR,   2,   numElements);
	range   =   sheet.get_Range(COleVariant( _T("A2 ")),covOptional);
	range = range.get_Resize(COleVariant((short)m_rows),COleVariant((short)m_cols));
	long   index[2];
	range   =   sheet.get_Range(COleVariant( _T("A2 ")),covOptional);
	range   =   range.get_Resize(COleVariant((short)m_rows),COleVariant((short)m_cols));
	for(iRow = 1; iRow   <=   m_rows;   iRow++)//���б�����д��EXCEL
	{
		for (iCol = 1;   iCol   <=   m_cols;   iCol++)  
		{
			index[0]=iRow-1;
			index[1]=iCol-1;
			CString   szTemp;
			szTemp=m_ListCtrl.GetItemText(iRow-1,iCol-1);
			BSTR   bstr   =   szTemp.AllocSysString();
			saRet.PutElement(index,bstr);
			SysFreeString(bstr);
		}
	}
	range.put_Value2(COleVariant(saRet));
	saRet.Detach();
	book.SaveCopyAs(COleVariant(strFile));
	//       cellinterior.ReleaseDispatch();
	book.put_Saved(true);
	book.ReleaseDispatch();  
	books.ReleaseDispatch(); 
	app.Quit();
	app.ReleaseDispatch();
}


void CDlgRecord::OnBnClickedBtnRecExport()
{
	// TODO: Add your control notification handler code here
	CString strFile = L"";
	SYSTEMTIME tm;
	GetLocalTime(&tm);
	if (m_idx == 1)
	{
		strFile.Format(L"�绰��¼_%04d%02d%02d%02d%02d%02d.xls", tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond);
		
	}
	else if (m_idx == 2)
	{
		strFile.Format(L"�㲥��¼_%04d%02d%02d%02d%02d%02d.xls", tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond);
		
	}
	else{
		strFile.Format(L"Ѳ���¼_%04d%02d%02d%02d%02d%02d.xls", tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond);
	}

	CFileDialog dlg(FALSE, L".xls", strFile,
		OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY,
		L"(*.xls)|*.xls||", this);
	dlg.m_ofn.lpstrTitle = _T("�����ļ�");
	if (dlg.DoModal() == IDOK)
	{
		ExportXLS(dlg.GetPathName());
		
	}
	
}


BOOL  CDlgRecord::PrintKq_Deal_QueryCtrl(CListCtrl   &list)
{   

	HFONT hRecordFont = NULL;//��¼������

	HFONT hTopicFont = NULL;//���������

	HFONT hCodeFont = NULL;//�ֶε�����

	//����(������ݵ�)����

	hRecordFont=CreateFont(80,20,1,0,FW_EXTRALIGHT,0,0,0,ANSI_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		DEFAULT_PITCH,
		L"������");
	//�������������

	hTopicFont=CreateFont(260,30,10,0,FW_BOLD,0,0,0,ANSI_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		DEFAULT_PITCH,
		L"����_GB2312");
	//�����ֶε�����

	hTopicFont=CreateFont(100,20,1,0,FW_BOLD,0,0,0,ANSI_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		DEFAULT_PITCH,
		L"����_GB2312");
	//���ô�ӡ�Ի���

	PRINTDLG   pd;

	pd.lStructSize=sizeof(PRINTDLG);

	pd.Flags=PD_RETURNDC;

	pd.hDC=NULL;

	pd.hwndOwner=NULL;

	pd.hInstance=NULL;

	pd.nMaxPage=100;

	pd.nMinPage=1; 

	pd.nFromPage=1; 

	pd.nToPage=1; 

	pd.nCopies=1;

	pd.hDevMode=NULL;   

	pd.hDevNames=NULL;   

	/////////////////////////////////////////////////////////

	//��ʾ��ӡ�Ի������û����趨ֽ�Ŵ�С��.

	if(!PrintDlg(&pd))   return   FALSE;

	ASSERT(pd.hDC!=NULL);/*���Ի�ȡ�ľ����Ϊ��.*/

	int   nHorRes = GetDeviceCaps(pd.hDC,HORZRES);

	int   nVerRes = GetDeviceCaps(pd.hDC,VERTRES);

	int   nXMargin = 10;//ҳ�ߵĿհ�   

	int   nYMargin = 10;

	///////////////////////////////////////////////////////////

	TEXTMETRIC  tm;/*ӳ��ṹ��*/

	GetTextMetrics(pd.hDC, &tm);

	int   nCharWidth = tm.tmAveCharWidth;

	int   ncaps=(tm.tmPitchAndFamily &1?3:2)*nCharWidth/2;

	int   nCharHeight =tm.tmExternalLeading +tm.tmHeight+ncaps;//

	///////////////////////////////////////////////////////////

	CHeaderCtrl*   pHeader = list.GetHeaderCtrl();

	//����У��еĸ���

	int   nColCount   =   pHeader->GetItemCount();//��ȡ��ͷ�ĸ��� 

	int   nLineCount   =   list.GetItemCount(); //��ȡListCtrl�ļ�¼����

	int   ColOrderArray[100];

	COLATT   ca[100];   

	list.GetColumnOrderArray(ColOrderArray,   nColCount); //�洢��ͷ������ֵ

	int   nColX=nXMargin*nCharWidth;

	////////////////////////////////////////////////////////////

	//�������е���Ϣ��ȷ���б�������ݳ��ȡ�

	for(int i=0;i<nColCount;i++)   
	{   
		ca[i].nColIndex =ColOrderArray[i];   

		LVCOLUMN lvc;

		wchar_t text[100];

		lvc.mask = LVCF_TEXT|LVCF_SUBITEM;

		lvc.pszText = text;

		lvc.cchTextMax =100;

		list.GetColumn(ca[i].nColIndex,&lvc);

		ca[i].strColText=lvc.pszText;

		ca[i].nSubItemIndex=lvc.iSubItem;

		ca[i].nPrintX=nColX;   

		nColX+=nCharWidth *  ca[i].strColText.GetLength();

		/////////////////////////////////////////////////////////////

		if(nColX > nHorRes)   //��ʾ�������ͷ����λ���Ѿ�������  
		{
			DeleteDC(pd.hDC);

			AfxMessageBox(L"�ֶ�̫�࣬�޷���һ���ڴ�ӡ�������ýϴ��ֽ��������ӡ��");  

			return  FALSE;   
		}

	}   	  
	//���ô�ӡ�ļ��ı���Ի��� 
	DOCINFO   di;   

	di.cbSize = sizeof(DOCINFO);   

//	CString strFile = L"";
// 	SYSTEMTIME time;
// 	GetLocalTime(&time);
	if (m_idx == 1)
	{
		di.lpszDocName = L"�绰��¼";
		//strFile.Format(L"�绰��¼_%04d%02d%02d%02d%02d%02d.xls", tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond);

	}
	else if (m_idx == 2)
	{
		di.lpszDocName = L"�㲥��¼";
		//strFile.Format(L"�㲥��¼_%04d%02d%02d%02d%02d%02d.xls", tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond);

	}
	else{
		di.lpszDocName = L"Ѳ���¼";
		//strFile.Format(L"Ѳ���¼_%04d%02d%02d%02d%02d%02d.xls", tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond);
	}

	    

	di.lpszOutput = (LPTSTR)NULL;     

	di.lpszDatatype = (LPTSTR)NULL;

	di.fwType = 0;

	StartDoc(pd.hDC,&di);

	StartPage(pd.hDC);

	SelectObject(pd.hDC,hTopicFont);

	TextOut(pd.hDC,nHorRes/2,nYMargin,di.lpszDocName,wcslen(di.lpszDocName));

	////////////////////////////////////////////////

	//�������еĿ�ȣ���ʹ�����ں���Ĵ�ӡ���ʱ�����ȵĴ�ӡ��ֽ��. 

	int   space =(nHorRes-nXMargin*nCharWidth-nColX)/(nColCount);  
	int i = 0;
	for(i=1;i<nColCount;i++)   
	{   
		ca[i].nPrintX+=i*space;
	}
	SelectObject(pd.hDC,hCodeFont);

	//����б���

	for(i =0;i<nColCount;i++){

		TextOut(pd.hDC,ca[i].nPrintX,nYMargin+200,
		ca[i].strColText,ca[i].strColText.GetLength());
	}
	int   nMaxLinePerPage =nVerRes/nCharHeight -3;

	int   nCurPage=1;  

	SelectObject(pd.hDC,hRecordFont);//������ѡ���豸��������
	//������е�����   
	for(i=0;i<nLineCount;i++)
	{   
		for(int j=0;j<nColCount;j++)
		{   
			if(i+1-(nCurPage-1)*nMaxLinePerPage>nMaxLinePerPage)
			{   
				nCurPage++;   
				if (nCurPage > pd.nToPage)
				{
					break;
				}
				//�µ�һҳ   
				EndPage(pd.hDC);   

				StartPage(pd.hDC);   

				
			}   
			CString  subitem=list.GetItemText(i,ca[j].nColIndex); 

			TextOut(pd.hDC,ca[j].nPrintX,     
				nYMargin+250+(i+1-(nCurPage-1)*nMaxLinePerPage)*nCharHeight,     
				subitem,subitem.GetLength());   
		}   
	}   

	EndPage(pd.hDC);

	EndDoc(pd.hDC);

	//��ӡ����

	DeleteObject(hTopicFont);

	DeleteObject(hRecordFont);

	DeleteObject(hCodeFont);

	DeleteDC(pd.hDC);

	return  TRUE;
}


void CDlgRecord::OnBnClickedBtnRecPrint()
{
	// TODO: Add your control notification handler code here
	PrintKq_Deal_QueryCtrl(m_ListCtrl);
}


void CDlgRecord::OnBnClickedRadioHour()
{
	// TODO: Add your control notification handler code here
	m_SortType = ENUM_SORT_HOUR;
	LoadRecord();
	UpdateRecord();
}


void CDlgRecord::OnBnClickedRadioDay()
{
	// TODO: Add your control notification handler code here
	m_SortType = ENUM_SORT_DAY;
	LoadRecord();
	UpdateRecord();
}
