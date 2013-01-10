//-----------------------------------------------------------------------------
// File: play.cpp
//
// Desc: DirectMusic tutorial to show how to play a segment 
//       on the default audio path
//
// Copyright (c) 2000-2001 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#define INITGUID
#include <windows.h>
#include <dmusicc.h>
#include <dmusici.h>

#define CONSTRUCTED 2
#define DECONSTRUCTED 1


class MusicObject
{
	//-----------------------------------------------------------------------------
	// Defines, constants, and global variables
	//-----------------------------------------------------------------------------
public:
	char status;
	IDirectMusicLoader8*      g_pLoader     ;
	IDirectMusicPerformance8* g_pPerformance;
	IDirectMusicSegment8*     g_pSegment    ;

	HRESULT Construct(WCHAR wstrFileName[MAX_PATH]);
	HRESULT Deconstruct();
	BOOL IsPlaying();
	HRESULT Play(long NumberOfRepeats);


};





//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT MusicObject::Play(long NumberOfRepeats)
{
	if(S_FALSE==(g_pPerformance->IsPlaying(g_pSegment,NULL)))
	{
		g_pSegment->SetRepeats(NumberOfRepeats);//DMUS_SEG_REPEAT_INFINITE);
	    g_pPerformance->PlaySegmentEx( g_pSegment, NULL, NULL, 0, 
                                       0, NULL, NULL, NULL );
	}
  return S_OK;
}


//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
BOOL MusicObject::IsPlaying(void)
{
if(SUCCEEDED(g_pPerformance->IsPlaying(g_pSegment,NULL) ) ) return TRUE;
else return FALSE;
}


//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT MusicObject::Construct(WCHAR wstrFileName[MAX_PATH])
{
 

	g_pLoader=NULL;
	g_pPerformance=NULL;
	g_pSegment=NULL;
    
    // Create loader object
    CoCreateInstance( CLSID_DirectMusicLoader, NULL, CLSCTX_INPROC, 
                      IID_IDirectMusicLoader8, (void**)&g_pLoader );

    // Create performance object
    CoCreateInstance( CLSID_DirectMusicPerformance, NULL, CLSCTX_INPROC, 
                      IID_IDirectMusicPerformance8, (void**)&g_pPerformance );

    // Initialize the performance with the standard audio path.
    // This initializes both DirectMusic and DirectSound and 
    // sets up the synthesizer. 
    g_pPerformance->InitAudio( NULL, NULL, NULL, 
                               DMUS_APATH_SHARED_STEREOPLUSREVERB, 64,
                               DMUS_AUDIOF_ALL, NULL );


    // Load the segment from the file
       
    if( FAILED( g_pLoader->LoadObjectFromFile( CLSID_DirectMusicSegment,
                                               IID_IDirectMusicSegment8,
                                               wstrFileName,
                                               (LPVOID*) &g_pSegment ) ) )
    {
        MessageBox( NULL, "Media not found, sample will now quit", 
                          "DirectMusic Tutorial", MB_OK );
        g_pPerformance->CloseDown();
        g_pLoader->Release(); 
        g_pPerformance->Release();
        CoUninitialize();
        return 0;
    }

    // Download the segment's instruments to the synthesizer
    g_pSegment->Download( g_pPerformance );
   
	//g_pPerformance->PlaySegment( g_pSegment, DMUS_SEGF_DEFAULT, 0, NULL); 


    status=CONSTRUCTED;
    return 0;
}


//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
HRESULT MusicObject::Deconstruct(void)
{
	// Stop the music, and close down 
    g_pPerformance->Stop( g_pSegment, NULL, 0, 0 );
    g_pPerformance->CloseDown();

    // Cleanup all interfaces
    g_pLoader->Release(); 
    g_pPerformance->Release();
    g_pSegment->Release();

	status=DECONSTRUCTED;
     
    return 0;
}

