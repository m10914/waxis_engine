#include <windows.h>
#include <commdlg.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "d3dx9.h"
#include "d3dx9effect.h"

#include "DXUtil.h"
#include "D3DEnumeration.h"
#include "D3DSettings.h"
#include "D3DApp.h"
#include "D3DFont.h"
#include "D3DUtil.h"







// Helper function to stuff a FLOAT into a DWORD argument
inline DWORD FtoDW( FLOAT f ) { return *((DWORD*)&f); }


//float CurrentGravity;


//-----------------------------------------------------------------------------
// Custom vertex types
//-----------------------------------------------------------------------------
struct COLORVERTEX
{
    D3DXVECTOR3 v;
    D3DCOLOR    color;
    FLOAT       tu;
    FLOAT       tv;

    static const DWORD FVF;
};
//const DWORD COLORVERTEX::FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;



struct POINTVERTEX
{
    D3DXVECTOR3 v;
    D3DCOLOR    color;

    static const DWORD FVF;
};
//const DWORD POINTVERTEX::FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;


//-----------------------------------------------------------------------------
// Global data for the particles
//-----------------------------------------------------------------------------
struct PARTICLE
{
    BOOL        m_bSpark;     // Sparks are less energetic particles that
                              // are generated where/when the main particles
                              // hit the ground
    UINT			CreationTime;
	UINT			LifeTime;

    D3DXVECTOR3 m_vPos;       // Current position
    D3DXVECTOR3 m_vVel;       // Current velocity

    D3DXVECTOR3 m_vPos0;      // Initial position
    D3DXVECTOR3 m_vVel0;      // Initial velocity
    FLOAT       m_fTime0;     // Time of creation

    D3DXCOLOR   m_clrDiffuse; // Initial diffuse color
    D3DXCOLOR   m_clrFade;    // Faded diffuse color
    FLOAT       m_fFade;      // Fade progression

    PARTICLE*   m_pNext;      // Next particle in list
};


enum PARTICLE_COLORS { COLOR_WHITE, COLOR_RED, COLOR_GREEN, COLOR_BLUE, NUM_COLORS };








//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
class CParticleSystem
{
public:
	//DWORD      particle;
	
	D3DXCOLOR g_clrColor[NUM_COLORS];
	DWORD g_clrColorFade[NUM_COLORS];


public:
    FLOAT     m_fRadius;
    DWORD     m_dwBase;
	DWORD     m_dwFlush;
    DWORD     m_dwDiscard;

    DWORD     m_dwParticles;
    DWORD     m_dwParticlesLim;
    PARTICLE* m_pParticles;
    PARTICLE* m_pParticlesFree;

    // Geometry
    LPDIRECT3DVERTEXBUFFER9 m_pVB;

public:
    float    fGravity;
	int      direction;
	UINT     LifeTime;
	int		 NumToEmit;

    CParticleSystem( DWORD dwFlush, DWORD dwDiscard, float fRadius, DWORD Intensivity, UINT glifeTime);
   ~CParticleSystem();

    HRESULT RestoreDeviceObjects( LPDIRECT3DDEVICE9 pd3dDevice );
    HRESULT InvalidateDeviceObjects();

    HRESULT Update( FLOAT fSecsPerFrame,
					DWORD dwNumParticlesToEmit,
                    const D3DXCOLOR &dwEmitColor,
					const D3DXCOLOR &dwFadeColor,
                    FLOAT fEmitVel,
					D3DXVECTOR3 vPosition,
					D3DXVECTOR3 (*EmPosF)(D3DXVECTOR3 emitpos),
					D3DXVECTOR3 (*Vel0F)(),
					D3DXVECTOR3 (*VelTrF)(D3DXVECTOR3 vel),
					D3DXVECTOR3 (*PosTrF)(D3DXVECTOR3 emitpos, D3DXVECTOR3 oldpos, D3DXVECTOR3 vel) );

    HRESULT Render( LPDIRECT3DDEVICE9 pd3dDevice, float Scale );

};



//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
class CParticleSystemParent
{
protected:
	LPDIRECT3DTEXTURE9     m_pParticleTexture;
	LPD3DXEFFECT		   m_pEffect;
	CParticleSystem*       m_pParticleSystem;

	LPDIRECT3DDEVICE9	   m_pd3dDevice;


	float                  fGravity;
	float					Scale;
	D3DLIGHT9              plight;
	LPCTSTR                strTexturePath;

	DWORD                  m_dwNumParticlesToEmit;
	DWORD                  m_dwParticleColor;
	
	BOOL                   m_bAnimateEmitter;
	
	D3DXVECTOR3            vEmitterPosition;

	D3DXVECTOR3				(*vEmitterPosFunc)(D3DXVECTOR3 emitpos);
	D3DXVECTOR3				(*Vel0Func)(void);
	D3DXVECTOR3				(*VelTransformFunc)(D3DXVECTOR3 vel);
	D3DXVECTOR3				(*PosTransformFunc)(D3DXVECTOR3 emitpos, D3DXVECTOR3 oldpos, D3DXVECTOR3 vel);


public:
	//funcions
	//init
	HRESULT					Init( LPCTSTR,
								  float, 
								  LPDIRECT3DDEVICE9, 
								  LPD3DXEFFECT,
								  float, 
								  UINT,
									D3DXVECTOR3 (*EmPosF)(D3DXVECTOR3 emitpos),
									D3DXVECTOR3 (*Vel0F)(),
									D3DXVECTOR3 (*VelTrF)(D3DXVECTOR3 vel),
									D3DXVECTOR3 (*PosTrF)(D3DXVECTOR3 emitpos, D3DXVECTOR3 oldpos, D3DXVECTOR3 vel) );
	
	//framemove
	HRESULT					FrameMove(double m_fElapsedTime, DWORD numParticlesToEmi);
	HRESULT					SetEmitterPosition(D3DXVECTOR3);
	HRESULT					SetIntensity(DWORD);
	HRESULT					Emit(int numberToEmit);

	//render
	HRESULT					Render();
	

	//release
	HRESULT					Release();
};


