#pragma once


#include "ReportCtrl.h"
#include "afxcmn.h"
#include "DataM.h"
// CDlgDB dialog
#include "XListCtrl.h"

class CDlgDB : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgDB)

public:
	CDlgDB(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgDB();

	int m_dbNum;
	SDBInfo* m_pDBInfo;
	int m_pEdit[20];
	int m_nEditNum;
	UINT m_maxID;
	CXListCtrl m_ReportCtrl;
	CStringArray m_strArray;
	void SetCtrlStatus(int row, int col);
	void SetCtrlVisul();
	void SetCtrlCombox(int row, int col, int flag);
// Dialog Data
	enum { IDD = IDD_DIALOG_DB };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	
	afx_msg void OnBnClickedBtnSave();
	afx_msg void OnDblclkListDb(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonAdd();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedBtnSub();	
	afx_msg void OnBnClickedBtnRemoveall();
};
