// Mp3MusicManager.h: interface for the CMp3MusicManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MP3MUSICMANAGER_H__2FB87B47_95FF_49BC_AE8C_E1BB14E65D08__INCLUDED_)
#define AFX_MP3MUSICMANAGER_H__2FB87B47_95FF_49BC_AE8C_E1BB14E65D08__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "Strmiids.lib")


#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <basetsd.h>
#include <math.h>
#include <stdio.h>
#include <d3dx9.h>
#include <dxerr9.h>
#include <dshow.h>
#include <tchar.h>
#include "DXUtil.h"
#include "D3DEnumeration.h"
#include "D3DSettings.h"
#include "D3DApp.h"
#include "D3DFont.h"
#include "D3DFile.h"
#include "D3DUtil.h"
#include "DMUtil.h"


#include "log.h"


//-------------------------------------------------------------------
// Name: CMp3MusicManager
// Desc: Simple class for playing mp3
//-------------------------------------------------------------------
class CMp3MusicManager  
{
protected:
	IGraphBuilder	*m_pGraph;
	IMediaControl	*m_pMediaControl;
	IMediaEvent		*m_pEvent;
	IMediaSeeking	*m_pSeeking;



public:
	CMp3MusicManager();
	virtual ~CMp3MusicManager();
	
	HRESULT Init();
	HRESULT Release();

	//that's it
	HRESULT PlayFile(char* path, BOOL bStopCurrent);
	BOOL IsPlaying();

	//get functions
	IGraphBuilder*		GetGraphBuilder() { return m_pGraph; }
	IMediaControl*		GetMediaControl() { return m_pMediaControl; }
	IMediaEvent*		GetMediaEvent() { return m_pEvent; }
	IMediaSeeking*		GetMediaSeeking() { return m_pSeeking; }

};





#endif // !defined(AFX_MP3MUSICMANAGER_H__2FB87B47_95FF_49BC_AE8C_E1BB14E65D08__INCLUDED_)
