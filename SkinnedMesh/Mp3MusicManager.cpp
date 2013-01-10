// Mp3MusicManager.cpp: implementation of the CMp3MusicManager class.
//
//////////////////////////////////////////////////////////////////////

#include "Mp3MusicManager.h"




//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CMp3MusicManager::CMp3MusicManager()
{
	m_pGraph = NULL;
	m_pMediaControl = NULL;
	m_pEvent = NULL;
	m_pSeeking = NULL;
}

CMp3MusicManager::~CMp3MusicManager()
{

}



//-------------------------------------------------------------------
// Name:
// Desc:
//-------------------------------------------------------------------
HRESULT CMp3MusicManager::Init()
{

	// Initialize COM
	CoInitialize( NULL );

	// Create the graph
	CoCreateInstance(	CLSID_FilterGraph, 
						NULL, 
						CLSCTX_INPROC_SERVER, 
						IID_IGraphBuilder, 
						(void **)&m_pGraph );

	// Query for interface objects
	m_pGraph->QueryInterface( IID_IMediaControl, (void **)&m_pMediaControl );
	m_pGraph->QueryInterface( IID_IMediaEvent, (void **)&m_pEvent );
	m_pGraph->QueryInterface( IID_IMediaSeeking, (void **)&m_pSeeking );


	return S_OK;
}


//-------------------------------------------------------------------
// Name:
// Desc:
//-------------------------------------------------------------------
HRESULT CMp3MusicManager::Release()
{

	// Stop the song if playing
	m_pMediaControl->Stop();

	// Clean up
	m_pMediaControl->Release();
	m_pEvent->Release();
	m_pGraph->Release();


	return S_OK;
}


//-------------------------------------------------------------------
// Name:
// Desc:
//-------------------------------------------------------------------
HRESULT CMp3MusicManager::PlayFile(char* path, BOOL bStopCurrent)
{
	//stop current
	if(bStopCurrent)
	{
		LONGLONG newPos;
		m_pSeeking->GetDuration(&newPos);

		m_pMediaControl->Stop();

		m_pSeeking->SetPositions( &newPos, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning );
	}


	WCHAR wfname[MAX_PATH];
	MultiByteToWideChar(CP_ACP, 0, path, -1, wfname, MAX_PATH);
	if(FAILED( m_pGraph->RenderFile(wfname, NULL) ))
	{
		return E_FAIL;
	}


	// Set the playback rate
	m_pSeeking->SetRate( 1.0 );

	// Play the song
	m_pMediaControl->Run();


	return S_OK;
}


//-------------------------------------------------------------------
// Name:
// Desc:
//-------------------------------------------------------------------
BOOL CMp3MusicManager::IsPlaying()
{
	long		evCode;
	
	m_pEvent->WaitForCompletion( 0, &evCode );
	
	// If the music is done, restart it
	if( evCode == EC_COMPLETE ) return TRUE;
	else return FALSE;


	return S_OK; 
}