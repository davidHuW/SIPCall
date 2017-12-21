// SoundBase.cpp: implementation of the CSoundBase class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SoundBase.h"

#include "SIPCallDlg.h"

//------------------------------------------------------------------------------
//Macro define
#define  INP_BUFFER_SIZE		16*1024 //The input buffer size

#define RIFF_FILE       mmioFOURCC('R','I','F','F')
#define RIFF_WAVE       mmioFOURCC('W','A','V','E')
#define RIFF_FORMAT     mmioFOURCC('f','m','t',' ')
#define RIFF_CHANNEL    mmioFOURCC('d','a','t','a')

//Default values
#define DEFAULT_CHANNEL		CHANNEL_SINGLE
#define DEFAULT_SAMPLES		SAMPLES_11025
#define DEFAULT_BITS		BITS_8

//------------------------------------------------------------------------------
//The struct

//FileHeader
typedef struct
{
   DWORD   dwRiff;     // Type of file header.
   DWORD   dwSize;     // Size of file header.
   DWORD   dwWave;     // Type of wave.
} RIFF_FILEHEADER, *PRIFF_FILEHEADER;



//ChunkHeader
typedef struct
{
   DWORD   dwCKID;        // Type Identification for current chunk header.
   DWORD   dwSize;        // Size of current chunk header.
} RIFF_CHUNKHEADER, *PRIFF_CHUNKHEADER;
//---------------------------------------------------------------------------------

//-------------------------------------------------------------------
//Static member initialize
CSoundBase *CSoundBase::m_pInstance = NULL; //If you don't initialize,the GetInstance() will link erro.


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSoundBase::CSoundBase()
{
	memset(m_szSavePath,0,sizeof(m_szSavePath));
	memset(&m_WaveFormatEx,0,sizeof(m_WaveFormatEx));
	m_pBuffer1 = NULL;
	m_pBuffer2 = NULL;
	m_pWaveHdr1 = NULL;
	m_pWaveHdr2 = NULL;
	m_hWaveIn = NULL;
	m_hSaveFile = NULL;
	m_bRecording = FALSE;
	m_dwDataLength = 0;


}

CSoundBase::~CSoundBase()
{

	if(m_pWaveHdr1 != NULL)
	{
		free(m_pWaveHdr1);
		m_pWaveHdr1 = NULL;
	}
	
	if(m_pWaveHdr2 != NULL)
	{
		free(m_pWaveHdr2);
		m_pWaveHdr2 = NULL;
	}
	
	if(m_pBuffer1 != NULL)
	{
		free(m_pBuffer1);
		m_pBuffer1 = NULL;
	}
	
	if(m_pBuffer2 != NULL)
	{
		free(m_pBuffer2);
		m_pBuffer2 = NULL;
	}

}

//----------------------------------------------------------------------
//Decription:
//	Start to record.
//
//Parameter:
//	pszPath: [in] The record path
//
//Retrun Values:
//	TRUE: Succeed.
//	FALSE: Failed.
//----------------------------------------------------------------------
BOOL CSoundBase::Record(TCHAR *pszPath,const PWAVEFORMAT_SETTING pWaveFormat)
{
	BOOL bResult = FALSE;
	_tcscpy(m_szSavePath,pszPath);

	SetRecordWaveFormat(pWaveFormat);
	MMRESULT ret = waveInOpen(&m_hWaveIn,WAVE_MAPPER,&m_WaveFormatEx,(DWORD)WaveInProc,NULL,CALLBACK_FUNCTION);
	if (ret != MMSYSERR_NOERROR )
	{
		g_Log.output(LOG_TYPE, "waveInOpen failed, error = %d\r\n", (int)ret);
		goto END;
	}

	m_pBuffer1=(PBYTE)malloc(INP_BUFFER_SIZE);
	m_pBuffer2=(PBYTE)malloc(INP_BUFFER_SIZE);
	if(m_pBuffer1 == NULL || m_pBuffer2 == NULL)
	{
		g_Log.output(LOG_TYPE, "m_pBuffer1 == NULL || m_pBuffer2 == NULL failed\r\n");
		goto END;
	}


	//allocate memory for wave header
	m_pWaveHdr1=reinterpret_cast<PWAVEHDR>(malloc(sizeof(WAVEHDR)));
	m_pWaveHdr2=reinterpret_cast<PWAVEHDR>(malloc(sizeof(WAVEHDR)));	

	if(m_pWaveHdr1 == NULL || m_pWaveHdr2 == NULL )
	{
		g_Log.output(LOG_TYPE, "m_pWaveHdr1 == NULL || m_pWaveHdr2 == NULL failed\r\n");
		goto END;
	}
	
	m_pWaveHdr1->lpData = (LPSTR)m_pBuffer1;
	m_pWaveHdr1->dwBufferLength = INP_BUFFER_SIZE;
	m_pWaveHdr1->dwBytesRecorded = 0;
	m_pWaveHdr1->dwUser = 0;
	m_pWaveHdr1->dwFlags = 0;
	m_pWaveHdr1->dwLoops = 1;
	m_pWaveHdr1->lpNext = NULL;
	m_pWaveHdr1->reserved = 0;	
	waveInPrepareHeader(m_hWaveIn,m_pWaveHdr1,sizeof(WAVEHDR));
	
	m_pWaveHdr2->lpData = (LPSTR)m_pBuffer2;
	m_pWaveHdr2->dwBufferLength = INP_BUFFER_SIZE;
	m_pWaveHdr2->dwBytesRecorded = 0;
	m_pWaveHdr2->dwUser = 0;
	m_pWaveHdr2->dwFlags = 0;
	m_pWaveHdr2->dwLoops = 1;
	m_pWaveHdr2->lpNext = NULL;
	m_pWaveHdr2->reserved = 0;	
	waveInPrepareHeader(m_hWaveIn,m_pWaveHdr2,sizeof(WAVEHDR));

	// Add the buffers	
	waveInAddBuffer (m_hWaveIn, m_pWaveHdr1, sizeof (WAVEHDR)) ;
	waveInAddBuffer (m_hWaveIn, m_pWaveHdr2, sizeof (WAVEHDR)) ;


	if (WriteWaveFileHeader(m_szSavePath,&m_WaveFormatEx,0,TRUE) == FALSE)
	{
		g_Log.output(LOG_TYPE, "WriteWaveFileHeader failed\r\n");
		goto END;
	}

	// Open the existing wave file incording to add wave data
	m_hSaveFile = CreateFile(m_szSavePath, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if( m_hSaveFile == INVALID_HANDLE_VALUE ) 
	{
		g_Log.output(LOG_TYPE, "CreateFile failed\r\n");
		goto END;
    }
	//Set the file pointer to the end.
	SetFilePointer(m_hSaveFile,0,NULL,FILE_END);

	//Begin recording
	waveInStart (m_hWaveIn) ;

	bResult = TRUE;
END:
	if(bResult == FALSE)
	{
		if(m_pWaveHdr1 != NULL)
		{
			free(m_pWaveHdr1);
			m_pWaveHdr1 = NULL;
		}

		if(m_pWaveHdr2 != NULL)
		{
			free(m_pWaveHdr2);
			m_pWaveHdr2 = NULL;
		}

		if(m_pBuffer1 != NULL)
		{
			free(m_pBuffer1);
			m_pBuffer1 = NULL;
		}

		
		if(m_pBuffer2 != NULL)
		{
			free(m_pBuffer2);
			m_pBuffer2 = NULL;
		}

		if(m_hWaveIn != NULL)
		{
			//Stop recording, close the device
			waveInReset(m_hWaveIn);
			waveInClose(m_hWaveIn);
			m_hWaveIn = NULL;
		}
	}
	return bResult;
}

//----------------------------------------------------------------------
//Decription:
//	Stop recording and playing
//
//Parameter:
//	NULL
//
//Retrun Values:
//	NULL
//----------------------------------------------------------------------
void CSoundBase::StopAll()
{
	StopRecording();
	StopPlaying();
}

//----------------------------------------------------------------------
//Decription:
//	Stop playing
//
//Parameter:
//	NULL
//
//Retrun Values:
//	NULL
//----------------------------------------------------------------------
void CSoundBase::StopPlaying()
{
	sndPlaySound(NULL,SND_ASYNC);
}

//----------------------------------------------------------------------
//Decription:
//	Stop recording
//
//Parameter:
//	NULL
//
//Retrun Values:
//	NULL
//----------------------------------------------------------------------
void CSoundBase::StopRecording()
{
	if(m_bRecording == FALSE)
	{
		return;
	}

	m_bRecording = FALSE;
	waveInReset(m_hWaveIn);
	waveInClose(m_hWaveIn);

	//Please don't call waveInUnprepareHeader() in the WIM_CLOSE,
	//or it will work badly !
	waveInUnprepareHeader (m_hWaveIn, m_pWaveHdr1, sizeof (WAVEHDR)) ;
	waveInUnprepareHeader (m_hWaveIn, m_pWaveHdr2, sizeof (WAVEHDR)) ;
		
	if(m_pWaveHdr1 != NULL)
	{
		free(m_pWaveHdr1);
		m_pWaveHdr1 = NULL;
	}

	if(m_pWaveHdr2 != NULL)
	{
		free(m_pWaveHdr2);
		m_pWaveHdr2 = NULL;
	}

	if(m_pBuffer1 != NULL)
	{
		free(m_pBuffer1);
		m_pBuffer1 = NULL;
	}
		
	if(m_pBuffer2 != NULL)
	{
		free(m_pBuffer2);
		m_pBuffer2 = NULL;
	}

	
}

//----------------------------------------------------------------------
//Decription:
//	Start to play.
//
//Parameter:
//	pszPath: [in] The playing path
//
//Retrun Values:
//	TRUE: Succeed.
//	FALSE: Failed.
//----------------------------------------------------------------------
BOOL CSoundBase::Play(const TCHAR *pszPath)
{	
	TCHAR szPath[MAX_SAVEPATH_LENGTH] = {0};
	if(pszPath != NULL)
	{
		_tcscpy(szPath,pszPath);
	}
	else
	{
		_tcscpy(szPath,m_szSavePath);
	}

	if(sndPlaySound(szPath,SND_ASYNC) == FALSE)
	{
		return FALSE;
	}
	return TRUE;	

}


//----------------------------------------------------------------------
//Decription:
//	Set the wave format for recording.
//
//Parameter:
//	pWaveFormat: [in] The wave format to set.
//
//Retrun Values:
//	TRUE: Succeed.
//	FALSE: Failed.
//----------------------------------------------------------------------
void CSoundBase::SetRecordWaveFormat(const PWAVEFORMAT_SETTING pWaveFormat)
{
	WAVEFORMAT_SETTING waveFormatSetting;
	if(pWaveFormat == NULL)
	{
		waveFormatSetting.channel = DEFAULT_CHANNEL;
		waveFormatSetting.samples = DEFAULT_SAMPLES;
		waveFormatSetting.bits =  DEFAULT_BITS;
	}
	else
	{
		waveFormatSetting = *pWaveFormat;
	}

	m_WaveFormatEx.wFormatTag=WAVE_FORMAT_PCM;
	m_WaveFormatEx.cbSize=0;	//When the wFormatTag is PCM, the parameter is abort.

	switch(waveFormatSetting.channel)
	{
		case CHANNEL_SINGLE:
			m_WaveFormatEx.nChannels = 1;
			break;
		case CHANNEL_DOUBLE:
			m_WaveFormatEx.nChannels = 2;
			break;
	}

	switch(waveFormatSetting.samples)
	{
		case SAMPLES_11025:
			m_WaveFormatEx.nSamplesPerSec = 11025;
			break;
		case SAMPLES_22050:
			m_WaveFormatEx.nSamplesPerSec = 22050;
			break;
		case SAMPLES_44100:
			m_WaveFormatEx.nSamplesPerSec = 44100;
			break;
	}


	switch(waveFormatSetting.bits)
	{
		case BITS_8:
			m_WaveFormatEx.wBitsPerSample = 8;
			break;
		case BITS_16:
			m_WaveFormatEx.wBitsPerSample = 16;
			break;
	}
	
	m_WaveFormatEx.nBlockAlign=m_WaveFormatEx.nChannels * m_WaveFormatEx.wBitsPerSample / 8;	
	m_WaveFormatEx.nAvgBytesPerSec=m_WaveFormatEx.nBlockAlign * m_WaveFormatEx.nSamplesPerSec;

}


//----------------------------------------------------------------------
//Decription:
//	Get instance
//
//Parameter:
//	Null
//
//Retrun Values:
//	The pointer to object
//----------------------------------------------------------------------
CSoundBase* CSoundBase::GetInstance()
{
	if(m_pInstance == NULL)
	{
		m_pInstance = new CSoundBase;
	}
	return m_pInstance;
}
void CSoundBase::Release()
{
	if(m_pInstance != NULL)
	{
		delete m_pInstance;
		m_pInstance = NULL;
	}	
}

//----------------------------------------------------------------------
//Decription:
//	WaveIn Process
//
//Parameter:
//	hwi: [in] Handle to the waveform-audio device associated with the callback function
//	uMsg: [in] Waveform-audio input message. It can be one of the messages shown :WIM_CLOSE,WIM_DATA,WIM_OPEN
//	dwInstance: [in] User instance data specified with waveInOpen. 
//	dwParam1: [in] Message parameter. 
//	dwParam2: [in] Message parameter. 
//
//Retrun Values:
//	DWORD type.
//----------------------------------------------------------------------
void CALLBACK CSoundBase::WaveInProc(HWAVEIN hWi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	switch(uMsg)
	{
		case WIM_CLOSE:
			m_pInstance->OnWIM_CLOSE(dwParam1,dwParam2);
			break;
		case WIM_DATA:
			m_pInstance->OnWIM_DATA(dwParam1,dwParam2);
			break;
		case WIM_OPEN:
			m_pInstance->OnWIM_OPEN(dwParam1,dwParam2);
			break;
	}

}



//----------------------------------------------------------------------
//Decription:
//	Write the wave file header.
//
//Parameter:
//	pszFilename: [in] The path to save
//	pWFX: [in] The information to write
//	dwBufferSize: [in] The size of wave buffer
//	bCover: [in] Cover writing or not 
//
//Retrun Values:
//	TRUE: Succeed.
//	FASLE: Failed.
//----------------------------------------------------------------------
BOOL CSoundBase::WriteWaveFileHeader(TCHAR *pszFilename, const PWAVEFORMATEX pWFX, DWORD dwBufferSize, BOOL bCover)
{

	RIFF_FILEHEADER FileHeader;
	RIFF_CHUNKHEADER WaveHeader;
	RIFF_CHUNKHEADER DataHeader;
	DWORD dwBytesWritten;
	HANDLE fh;
	BOOL bResult = FALSE;
    
	// Fill in the file, wave and data headers
	WaveHeader.dwCKID = RIFF_FORMAT;
	WaveHeader.dwSize = sizeof(WAVEFORMATEX) + pWFX->cbSize;

	// the DataHeader chunk contains the audio data
	DataHeader.dwCKID = RIFF_CHANNEL;
	DataHeader.dwSize = dwBufferSize;

	// The FileHeader
	FileHeader.dwRiff = RIFF_FILE;
	FileHeader.dwSize = sizeof(WaveHeader) + WaveHeader.dwSize + sizeof(DataHeader) + DataHeader.dwSize;
	FileHeader.dwWave = RIFF_WAVE;

    // Open wave file
	if(bCover==TRUE)
	{
		//If the file existed , cover writng
		fh = CreateFile(pszFilename, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
    }
	else
	{
		//Open the file existed
		fh = CreateFile(pszFilename, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		//Move the pointer to the begin
		SetFilePointer(fh,0,NULL,FILE_BEGIN);
	}
	if( fh == INVALID_HANDLE_VALUE ) {
//        DEBUGMSG(1, (TEXT("Error opening %s. Error code = 0x%08x\n"), pszFilename, GetLastError()));
  		goto ERROR_EXIT;
    }

    // write the riff file
    if (! WriteFile(fh, &FileHeader, sizeof(FileHeader), &dwBytesWritten, NULL)) {
//       DEBUGMSG(1, (TEXT("Error writing file header\r\n")));
		goto ERROR_EXIT;
    }

    // write the wave header
    if (! WriteFile(fh, &WaveHeader, sizeof(WaveHeader), &dwBytesWritten, NULL)) {
//        DEBUGMSG(1, (TEXT("Error writing wave header\r\n")));
		goto ERROR_EXIT;
    }

    // write the wave format
    if (! WriteFile(fh, pWFX, WaveHeader.dwSize, &dwBytesWritten, NULL)) {
//        DEBUGMSG(1, (TEXT("Error writing wave format\r\n")));
		goto ERROR_EXIT;
    }

    // write the data header
    if (! WriteFile(fh, &DataHeader, sizeof(DataHeader), &dwBytesWritten, NULL)) {
//        DEBUGMSG(1, (TEXT("Error writing PCM data header\r\n")));
		goto ERROR_EXIT;
    }
    // Success
    bResult = TRUE;

ERROR_EXIT:
	if(fh != INVALID_HANDLE_VALUE)
	{
		CloseHandle(fh);
    }
	return bResult;
}


//----------------------------------------------------------------------
//Decription:
//	On WIM_CLOSE
//------------------------------------------------------------------------
void CSoundBase::OnWIM_CLOSE(WPARAM wParam, LPARAM lParam)
{
	
	if(m_hSaveFile != NULL)
	{
		CloseHandle(m_hSaveFile);
		m_hSaveFile = NULL;
	}
	if (0 != m_dwDataLength)
	{
		//Write the data length to the file.
		WriteWaveFileHeader(m_szSavePath,&m_WaveFormatEx,m_dwDataLength,FALSE);
	}

}

//----------------------------------------------------------------------
//Decription:
//	On WIM_DATA
//------------------------------------------------------------------------
void CSoundBase::OnWIM_DATA(WPARAM wParam, LPARAM lParam)
{

	DWORD dwBytesRecorded = ((PWAVEHDR)wParam)->dwBytesRecorded;

	PBYTE pSaveBuffer;
	//allocate memory for save buffer
	pSaveBuffer = reinterpret_cast<PBYTE>(malloc(dwBytesRecorded));	
	if(pSaveBuffer == NULL)
	{
		waveInClose (m_hWaveIn) ;
		return;
	}

	//Copy the data to the save buffer.
	CopyMemory (pSaveBuffer, ((PWAVEHDR)wParam)->lpData, dwBytesRecorded) ;

	//Write the memory data to the file
	DWORD dwBytesWritten;
	if(WriteFile(m_hSaveFile, pSaveBuffer, dwBytesRecorded, &dwBytesWritten, NULL) != TRUE)
	{
		if(m_bRecording == TRUE)
		{
			waveInClose (m_hWaveIn) ;
		}
	}

	m_dwDataLength += dwBytesWritten ;

	free(pSaveBuffer);	

	//If m_bRecording is FALSE, it may call the function waveInReset().So don't add buffer.
	if(m_bRecording == TRUE)
	{
		// Send out a new buffer.The new buffer is the original full buffer, used again.
		waveInAddBuffer (m_hWaveIn, (PWAVEHDR) wParam, sizeof (WAVEHDR)) ;
	}
}

//----------------------------------------------------------------------
//Decription:
//	On WIM_OPEN
//------------------------------------------------------------------------
void CSoundBase::OnWIM_OPEN(WPARAM wParam, LPARAM lParam)
{
	m_bRecording = TRUE;
	m_dwDataLength = 0;
}

