#pragma once
#include "afxcmn.h"
#include "DataM.h"

#include "CApplication.h"
#include "CFont0.h"
#include "CRange.h"
#include "CWorkbook.h"
#include "CWorksheet.h"
#include "CWorkbooks.h"
#include "CWorksheets.h"
//#include "Cnterior.h"
// CDlgRecord dialog

struct COLATT{
	int nColIndex;
	CString strColText;
	int nPrintX;
	int nSubItemIndex;
};

class CDlgRecord : public CDialog
{
	DECLARE_DYNAMIC(CDlgRecord)

public:
	CDlgRecord(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgRecord();

	void SetRecord(int idx);
	void SetLayout();

	void LoadRecord();
	void UpdateRecord();
private:
	void ExportXLS(CString strFile);
	BOOL  PrintKq_Deal_QueryCtrl(CListCtrl   &list);
private:
	int m_idx;
	SRecord* m_pRecord;
	int m_nNum;
	enDataSortType m_SortType;

// Dialog Data
	enum { IDD = IDD_DIALOG_RECORD };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_ListCtrl;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedRadio1();
	afx_msg void OnBnClickedRadio2();
	afx_msg void OnBnClickedRadio3();
	afx_msg void OnBnClickedBtnAll();
	afx_msg void OnBnClickedBtnMicRec();
	afx_msg void OnBnClickedBtnRecExport();
	afx_msg void OnBnClickedBtnRecPrint();
	afx_msg void OnBnClickedRadioHour();
	afx_msg void OnBnClickedRadioDay();
};
