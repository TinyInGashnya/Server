
// ServerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Server.h"
#include "ServerDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define MAX_BUFFER		2048
char		szServerName[NCBNAMSZ];
int		i, num;

HWND		hWnd_LB;	 // Для использования в потоках

int Listen(int lana, char* name);
void CALLBACK ListenCallback(PNCB pncb);
UINT ClientThread(PVOID lpParam);



// CServerDlg dialog



CServerDlg::CServerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SERVER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}
void CServerDlg::SetStarted(bool IsStarted)
{
	m_IsStarted = IsStarted;

	GetDlgItem(IDC_SERVER)->EnableWindow(!IsStarted);
	GetDlgItem(IDC_START)->EnableWindow(!IsStarted);
}

void CServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LISTBOX, m_ListBox);
}

BEGIN_MESSAGE_MAP(CServerDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_START, &CServerDlg::OnClickedStart)
END_MESSAGE_MAP()


// CServerDlg message handlers

BOOL CServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	GetDlgItem(IDC_SERVER)->SetWindowText(
		_T("TEST-SERVER"));
	SetStarted(false);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CServerDlg::OnClickedStart()
{
	LANA_ENUM	lenum;
	char		Str[200];
	hWnd_LB = m_ListBox.m_hWnd; // Для использования в потоках
	if (LanaEnum(&lenum) != NRC_GOODRET)
	{
		if (*NbCommonErrorMsg)
			m_ListBox.AddString((LPTSTR)NbCommonErrorMsg);
		sprintf_s(Str, sizeof(Str),
			"LanaEnum failed with error %d:", GetLastError());
		m_ListBox.AddString((LPTSTR)Str);
		return;
	}
	if (lenum.length == 0)
	{
		m_ListBox.AddString(
			(LPTSTR)"Sorry, no existing LANA...");
		GetDlgItem(IDC_SERVER)->EnableWindow(false);
		GetDlgItem(IDC_START)->EnableWindow(false);
		return;
	}

	if (ResetAll(&lenum, 254, 254, FALSE) != NRC_GOODRET)
	{
		if (*NbCommonErrorMsg)
			m_ListBox.AddString(NbCommonErrorMsg);
		sprintf_s(Str, sizeof(Str),
			"ResetAll failed with error %d:", GetLastError());
		m_ListBox.AddString(Str);
		return;
	}
	GetDlgItem(IDC_SERVER)->GetWindowText(szServerName,
		NCBNAMSZ);

	for (i = 0; i < lenum.length; i++)
	{
		if (*NbCommonErrorMsg)
			m_ListBox.AddString((LPTSTR)NbCommonErrorMsg);
		AddName(lenum.lana[i], szServerName, &num);
		Listen(lenum.lana[i], szServerName);
	}

	m_ListBox.AddString((LPTSTR)"Server started");
	SetStarted(true);

}
int Listen(int lana, char* name)
{
	PNCB	pncb = NULL;
	pncb = (PNCB)GlobalAlloc
	(GMEM_FIXED | GMEM_ZEROINIT, sizeof(NCB));
	pncb->ncb_command = NCBLISTEN | ASYNCH;
	pncb->ncb_lana_num = lana;
	pncb->ncb_post = ListenCallback;

	// Имя, с которым клиенты будут соединяться
	memset(pncb->ncb_name, ' ', NCBNAMSZ);
	strncpy_s((char*)pncb->ncb_name,
		sizeof(pncb->ncb_name), name, strlen(name));

	// Имя клиента,
	// '*' означает, что примем соединение от любого клиента
	memset(pncb->ncb_callname, ' ', NCBNAMSZ);
	pncb->ncb_callname[0] = '*';

	if (Netbios(pncb) != NRC_GOODRET)
	{
		char		Str[200];
		CListBox* pLB =
			(CListBox*)(CListBox::FromHandle(hWnd_LB));

		sprintf_s(Str, sizeof(Str),
			"ERROR: Netbios: NCBLISTEN: %d",
			pncb->ncb_retcode);
		pLB->AddString((LPTSTR)Str);
		return pncb->ncb_retcode;
	}
	return NRC_GOODRET;
}
void CALLBACK ListenCallback(PNCB pncb)
{
	if (pncb->ncb_retcode != NRC_GOODRET)
	{
		char	   Str[200];
		CListBox* pLB =
			(CListBox*)(CListBox::FromHandle(hWnd_LB));

		sprintf_s(Str, sizeof(Str),
			"ERROR: ListenCallback: %d", pncb->ncb_retcode);
		pLB->AddString((LPTSTR)Str);
		return;
	}

	Listen(pncb->ncb_lana_num, szServerName);
	AfxBeginThread(ClientThread, (PVOID)pncb);
	return;
}
UINT ClientThread(PVOID lpParam)
{
	PNCB		pncb = (PNCB)lpParam;
	NCB		 ncb;
	char		szRecvBuff[MAX_BUFFER];
	DWORD	   dwBufferLen = MAX_BUFFER,
		dwRetVal = NRC_GOODRET;
	char		szClientName[NCBNAMSZ + 1];
	char		Str[200];
	CListBox* pLB =
		(CListBox*)(CListBox::FromHandle(hWnd_LB));

	FormatNetbiosName((char*)pncb->ncb_callname,
		szClientName);
	while (1)
	{
		dwBufferLen = MAX_BUFFER;

		dwRetVal = Recv(pncb->ncb_lana_num, pncb->ncb_lsn,
			szRecvBuff, &dwBufferLen);
		if (dwRetVal != NRC_GOODRET)
		{
			if (*NbCommonErrorMsg)
				pLB->AddString((LPTSTR)NbCommonErrorMsg);
			sprintf_s(Str, sizeof(Str),
				"Recv failed with error %d",
				GetLastError());
			pLB->AddString((LPTSTR)Str);
			break;
		}
		szRecvBuff[dwBufferLen] = 0;
		sprintf_s(Str, sizeof(Str),
			"READ [LANA=%d]: '%s'",
			pncb->ncb_lana_num, szRecvBuff);
		pLB->AddString((LPTSTR)Str);

		dwRetVal = Send(pncb->ncb_lana_num, pncb->ncb_lsn,
			szRecvBuff, dwBufferLen);
		if (dwRetVal != NRC_GOODRET)
		{
			if (*NbCommonErrorMsg)
				pLB->AddString((LPTSTR)NbCommonErrorMsg);
			sprintf_s(Str, sizeof(Str),
				"Send failed with error %d",
				GetLastError());
			pLB->AddString((LPTSTR)Str);
			break;
		}
	}
	sprintf_s(Str, sizeof(Str),
		"Client '%s' on LANA %d disconnected",
		szClientName, pncb->ncb_lana_num);
	pLB->AddString((LPTSTR)Str);

	if (dwRetVal != NRC_SCLOSED)
	{
		// Непредусмотренная ошибка. Соединение закрывается.
		//
		ZeroMemory(&ncb, sizeof(NCB));
		ncb.ncb_command = NCBHANGUP;
		ncb.ncb_lsn = pncb->ncb_lsn;
		ncb.ncb_lana_num = pncb->ncb_lana_num;

		if (Netbios(&ncb) != NRC_GOODRET)
		{
			sprintf_s(Str, sizeof(Str),
				"ERROR: Netbios: NCBHANGUP: %d",
				ncb.ncb_retcode);
			pLB->AddString((LPTSTR)Str);
			dwRetVal = ncb.ncb_retcode;
		}
		GlobalFree(pncb);
		return dwRetVal;
	}
	GlobalFree(pncb);
	return NRC_GOODRET;
}

