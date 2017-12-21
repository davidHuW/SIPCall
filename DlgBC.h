#pragma once
#include "afxwin.h"

#include "DataM.h"
#include "MgrPlay.h"

// CDlgBC dialog

class CDlgBC : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgBC)

public:
	CDlgBC(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgBC();


	void SetLayout();
	void SetDeviceLayout();
	void SetPlaySrcLayout();
	void SetPlayModeLayout();
	void SetPlayListLayout();
	void SetCurTunnelIdx(int curIdx);
	void LoadSong();
	void LoadPlayList();
	void Song2List();
	void FileName2FileID(int id, char* szName, wchar_t* wName);
	void GetRowCol(int idx, int& row, int& col);	

	CString GetTTSFile();
private:
	void OnCreateButtonClick(UINT nID);
private:
	enPlaySrc m_enPlaySrc;
	enPlayModel m_enPlayModel;
private:
	int m_xoffset;
	int m_yoffset;
	int m_x;
	int m_y;
	SListSong* m_pListSong;
	int m_nSongNum;

	STunnel* m_pTunnel;
	int m_nTunnelNum;
	int m_nCurIdx;
	
	int m_nDeviceMax;

	CButton*  NewCheckBox(int nID,CRect rect,int nStyle, CString Caption);
	void  FindFile(CString strFoldername);

	void AutoSetPlayMode(enPlaySrc src);
public:
	void SetDlg(bool bConfigDlg);
private:
	bool m_bConfigDlg;
private:
	void UpdateConfig();
	void UpdateDeviceConfig(int extAddr);
	void UpdateAttr();
	void UpdateDeviceAttr(SDevice* pDevice);
	enConfigErrorCode Check();
private:
	void Path2Name(char* szPath, char* szName);
	void ReadPlayList();
	void SavePlayList();
	void StopPlay();
	void StopOneDevice(SDevice* pDevice, bool& bMic, bool& bAux);
	bool IsPlayListInSongList(SListSong* pPlayList);
private:
	void Ctrl2PlayGroup();
	SPlayGroup* GetPlayGroup();
	
private:
	SPlayGroup* m_pPlayGroup;
public:
// Dialog Data
	enum { IDD = IDD_DIALOG_BC };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedBtnSong();
	CListBox m_ListSong;
	CListBox m_ctrlPlay;
	afx_msg void OnBnClickedBtnAdd();
	afx_msg void OnBnClickedBtnRemove();
	CButton m_RadioFile;
	CButton m_RadioList;
	afx_msg void OnBnClickedRadioTts();
	afx_msg void OnBnClickedRadioFile();
	afx_msg void OnBnClickedRadioListfile();	
	afx_msg void OnBnClickedRadioMic();
	afx_msg void OnBnClickedRadioAux();
	afx_msg void OnBnClickedOk();
};
