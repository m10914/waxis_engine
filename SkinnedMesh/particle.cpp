//----------------------------------------------
// PARTICLE.CPP
//----------------------------------------------

#include "particle.h"




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CParticleSystemParent::Init(LPCTSTR texPath, float grav, 
									LPDIRECT3DDEVICE9 d3dDevice,
									LPD3DXEFFECT	d3dEffect,
									float gScale, UINT LifeTime,
									D3DXVECTOR3 (*EmPosF)(D3DXVECTOR3 emitpos),
									D3DXVECTOR3 (*Vel0F)(),
									D3DXVECTOR3 (*VelTrF)(D3DXVECTOR3 vel),
									D3DXVECTOR3 (*PosTrF)(D3DXVECTOR3 emitpos, D3DXVECTOR3 oldpos, D3DXVECTOR3 vel) )
{
	m_pd3dDevice = d3dDevice;
	m_pEffect = d3dEffect;

	m_pParticleSystem = new CParticleSystem(512,
											2048,
											0.03f,
											30,
											LifeTime);

	m_pParticleTexture = NULL;

	Scale = gScale;
	m_dwNumParticlesToEmit = 1;
	m_dwParticleColor      = COLOR_WHITE;

	strTexturePath         = texPath; 
	m_pParticleSystem->fGravity   = grav;

	if(FAILED( D3DXCreateTextureFromFile( m_pd3dDevice,
										  strTexturePath,
										  &m_pParticleTexture ))) return E_FAIL;

	/*
	char ntexPath[256];
	sprintf(ntexPath, "n_%s", texPath);

	if(FAILED( D3DXCreateTextureFromFile( m_pd3dDevice,
										  strTexturePath,
										  &m_pBump ))) return E_FAIL;
	*/

	(vEmitterPosFunc) = (EmPosF);
	(Vel0Func) = (Vel0F);
	(VelTransformFunc) = (VelTrF);
	(PosTransformFunc) = (PosTrF);

	m_pParticleSystem->RestoreDeviceObjects(m_pd3dDevice);

	return S_OK;
}



//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CParticleSystemParent::Emit(int numberToEmit)
{
	m_pParticleSystem->NumToEmit = numberToEmit;


	return S_OK;
}

//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CParticleSystemParent::FrameMove(double m_fElapsedTime, DWORD numParticlesToEmit)
{
		//CurrentGravity = ParticleSystem[j].fGravity;
	m_pParticleSystem->direction = 1;

	m_pParticleSystem->Update(                      (float)m_fElapsedTime,
													numParticlesToEmit,
													m_pParticleSystem->g_clrColor[m_dwParticleColor],
													m_pParticleSystem->g_clrColorFade[m_dwParticleColor], 
													118.0f,
													vEmitterPosition,
													vEmitterPosFunc,
													Vel0Func,
													VelTransformFunc,
													PosTransformFunc
													);

	return S_OK;
}



//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CParticleSystemParent::SetEmitterPosition(D3DXVECTOR3 pos)
{
	vEmitterPosition = pos;

	return S_OK;
}
HRESULT CParticleSystemParent::SetIntensity(DWORD newInt)
{
	m_pParticleSystem->m_dwParticlesLim = newInt;

	return S_OK;
}
//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CParticleSystemParent::Render()
{
		//tex set
		//m_pd3dDevice->SetTexture(0, m_pParticleTexture);
		if(FAILED(m_pEffect->SetTexture( "tColorMap", m_pParticleTexture ))) return E_FAIL;
		//if(FAILED(m_pEffect->SetTexture( "tBumpMap",  m_pBump ))) return E_FAIL;	
		
		m_pEffect->SetFloat( "parSize", (float)Scale ); 


		m_pEffect->CommitChanges();
		m_pParticleSystem->Render( m_pd3dDevice, Scale );	


		m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);
    	m_pd3dDevice->SetRenderState( D3DRS_LIGHTING,  TRUE );


	return S_OK;
}


//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CParticleSystemParent::Release()
{

	m_pParticleTexture->Release();
	//m_pBump->Release();

	delete m_pParticleSystem;



	return S_OK;
}



//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
CParticleSystem::CParticleSystem( DWORD dwFlush, DWORD dwDiscard, float fRadius, DWORD Intensivity, UINT glifeTime )
{

	g_clrColor[0] =
		D3DXCOLOR( 1.0f,   1.0f,   1.0f,   1.0f );
	g_clrColor[1] =
		D3DXCOLOR( 0.0f,   0.0f,   0.0f,   1.0f );
	g_clrColor[2] =
		D3DXCOLOR( 0.0f,   0.0f,   0.0f,   1.0f );
	g_clrColor[3] =	
		D3DXCOLOR( 0.0f,   0.0f,   0.0f,   1.0f );



	g_clrColorFade[0] =
		D3DXCOLOR( 1.0f,   1.0f,   1.0f,   1.0f );
		//D3DXCOLOR( 0.5f,   0.5f,   1.0f,   1.0f ),
	g_clrColorFade[1] =
		D3DXCOLOR( 0.0f,   0.0f,   0.0f,   0.0f );
	g_clrColorFade[2] =
		D3DXCOLOR( 0.0f,   0.0f,   0.0f,   0.0f );
	g_clrColorFade[3] =
		D3DXCOLOR( 0.0f,   0.0f,   0.0f,   0.0f );


    m_fRadius        = fRadius;
	LifeTime		 = glifeTime;

    m_dwBase         = dwDiscard;
    m_dwFlush        = dwFlush;
	m_dwDiscard      = dwDiscard;

    m_dwParticles    = 0;
    m_dwParticlesLim = Intensivity;//10; //2048;
	
    m_pParticles     = NULL;
    m_pParticlesFree = NULL;
	m_pVB            = NULL;

	//particle = 0;
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
CParticleSystem::~CParticleSystem()
{
	InvalidateDeviceObjects();

    while( m_pParticles )
    {
        PARTICLE* pSpark = m_pParticles;
        m_pParticles = pSpark->m_pNext;
        delete pSpark;
    }

    while( m_pParticlesFree )
    {
        PARTICLE *pSpark = m_pParticlesFree;
        m_pParticlesFree = pSpark->m_pNext;
        delete pSpark;
    }
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CParticleSystem::RestoreDeviceObjects( LPDIRECT3DDEVICE9 pd3dDevice )
{
    HRESULT hr;

    // Create a vertex buffer for the particle system.  The size of this buffer
    // does not relate to the number of particles that exist.  Rather, the
    // buffer is used as a communication channel with the device.. we fill in 
    // a bit, and tell the device to draw.  While the device is drawing, we
    // fill in the next bit using NOOVERWRITE.  We continue doing this until 
    // we run out of vertex buffer space, and are forced to DISCARD the buffer
    // and start over at the beginning.

    if(FAILED(hr = pd3dDevice->CreateVertexBuffer( m_dwDiscard * 
		sizeof(POINTVERTEX), 
		D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY | D3DUSAGE_POINTS, 
        D3DFVF_XYZ | D3DFVF_DIFFUSE, 
		D3DPOOL_DEFAULT, 
		&m_pVB, 
		NULL )))
	{
        return E_FAIL;
	}

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CParticleSystem::InvalidateDeviceObjects()
{
    SAFE_RELEASE( m_pVB );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CParticleSystem::Update( FLOAT fSecsPerFrame,
								 DWORD dwNumParticlesToEmit,
                                 const D3DXCOLOR &clrEmitColor,
                                 const D3DXCOLOR &clrFadeColor,
								 FLOAT fEmitVel,
                                 D3DXVECTOR3 vPosition,
								 D3DXVECTOR3 (*EmPosF)(D3DXVECTOR3 emitpos),
								 D3DXVECTOR3 (*Vel0F)(),
								 D3DXVECTOR3 (*VelTrF)(D3DXVECTOR3 vel),
								 D3DXVECTOR3 (*PosTrF)(D3DXVECTOR3 emitpos, D3DXVECTOR3 oldpos, D3DXVECTOR3 vel) )
{
    PARTICLE *pParticle, **ppParticle;
    static float fTime = 0.0f;
    fTime += fSecsPerFrame;

    ppParticle = &m_pParticles;

    while( *ppParticle )
    {
        pParticle = *ppParticle;

        // Calculate new position
		//-------------------------------------------
		// Updating
		//-------------------------------------------


        float fT = fTime - pParticle->m_fTime0;

        //float fGravity = (-1)*fGravity;
        pParticle->m_fFade -= fSecsPerFrame * 0.25f;

		pParticle->m_vPos	= (*PosTrF)(vPosition, pParticle->m_vPos, pParticle->m_vVel);
		pParticle->m_vVel	= (*VelTrF)(pParticle->m_vVel);

		//pParticle->m_vPos    = pParticle->m_vVel0 * fT + pParticle->m_vPos0;
		//pParticle->m_vVel.y  = pParticle->m_vVel.y - fGravity * fT;
		//pParticle->m_vPos.y += pParticle->m_vVel.y*fT + fGravity *(fT*fT)/2;

        //pParticle->m_vPos    = pParticle->m_vVel0 * fT + pParticle->m_vPos0;
        //pParticle->m_vPos.y += ((0.5f * fGravity) * (fT * fT));
        //pParticle->m_vVel.y  = ((pParticle->m_vVel0.y + fGravity * fT));
        //pParticle->m_vPos.x += ((0.5f * fGravity) * (fT * fT));
        //pParticle->m_vVel.x  = (pParticle->m_vVel0.y + fGravity * fT)/4;

        if( pParticle->m_fFade < 0.0f )
            pParticle->m_fFade = 0.0f;



		// -------------------------------------
		// Kill old particles
		//--------------------------------------


        if( //(pParticle->m_vPos.y < m_fRadius) ||
            (pParticle->m_bSpark && pParticle->m_fFade <= 0.0f) ||
			(timeGetTime() - pParticle->CreationTime  > pParticle->LifeTime) )
        {
            // Kill particle
            *ppParticle = pParticle->m_pNext;
            pParticle->m_pNext = m_pParticlesFree;
            m_pParticlesFree = pParticle;

            if(!pParticle->m_bSpark)
                m_dwParticles--;
        }
        else
        {
            ppParticle = &pParticle->m_pNext;
        }
    }

	


	//-------------------------------------------
	// Emitting new particle
	//-------------------------------------------


	//if(particle > 1)
	//{//was added

		DWORD dwParticlesEmit = m_dwParticles + dwNumParticlesToEmit;//particle;
		while( (m_dwParticles < m_dwParticlesLim && m_dwParticles < dwParticlesEmit)
				|| NumToEmit > 0)
		{

			if(NumToEmit > 0) NumToEmit--;

			if( m_pParticlesFree )
			{
				pParticle = m_pParticlesFree;
				m_pParticlesFree = pParticle->m_pNext;
			}
			else
			{
				if( NULL == ( pParticle = new PARTICLE ) )
					return E_OUTOFMEMORY;
			}

			pParticle->m_pNext = m_pParticles;
			m_pParticles = pParticle;
			m_dwParticles++;



			pParticle->CreationTime = timeGetTime();
			pParticle->LifeTime = rand()%LifeTime;
			pParticle->m_bSpark = FALSE;


			pParticle->m_vPos0 = (*EmPosF)(vPosition);
			pParticle->m_vVel0 = (*Vel0F)();



			pParticle->m_vPos = pParticle->m_vPos0;
			pParticle->m_vVel = pParticle->m_vVel0;

			pParticle->m_clrDiffuse = clrEmitColor;
			pParticle->m_clrFade    = clrFadeColor;
			pParticle->m_fFade      = 1.0f;
			pParticle->m_fTime0     = fTime;
		

		}

    //}//was added


    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Renders the particle system using either pointsprites (if supported)
//       or using 4 vertices per particle
//-----------------------------------------------------------------------------
HRESULT CParticleSystem::Render( LPDIRECT3DDEVICE9 pd3dDevice , float Scale)
{
    HRESULT hr;

    // Set the render states for using point sprites
    pd3dDevice->SetRenderState( D3DRS_POINTSPRITEENABLE, TRUE );
    pd3dDevice->SetRenderState( D3DRS_POINTSCALEENABLE,  TRUE );

	/*
	pd3dDevice->SetRenderState( D3DRS_POINTSIZE,     FtoDW(Scale) );
    pd3dDevice->SetRenderState( D3DRS_POINTSIZE_MIN, FtoDW(0.01f) );
    pd3dDevice->SetRenderState( D3DRS_POINTSCALE_A,  FtoDW(0.00f) );
    pd3dDevice->SetRenderState( D3DRS_POINTSCALE_B,  FtoDW(0.00f) );
    pd3dDevice->SetRenderState( D3DRS_POINTSCALE_C,  FtoDW(1.00f) );
	*/


	pd3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,TRUE);        
	pd3dDevice->SetRenderState( D3DRS_LIGHTING,  FALSE );


	//pd3dDevice->SetRenderState( D3DRS_ZENABLE, FALSE ); 
	pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );


    // Set up the vertex buffer to be rendered
    pd3dDevice->SetStreamSource( 0, m_pVB, 0, sizeof(POINTVERTEX) );
    pd3dDevice->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE );


    PARTICLE*    pParticle = m_pParticles;
    POINTVERTEX* pVertices;
    DWORD        dwNumParticlesToRender = 0;



	// Lock the vertex buffer.  We fill the vertex buffer in small
	// chunks, using D3DLOCK_NOOVERWRITE.  When we are done filling
	// each chunk, we call DrawPrim, and lock the next chunk.  When
	// we run out of space in the vertex buffer, we start over at
	// the beginning, using D3DLOCK_DISCARD.

	m_dwBase += m_dwFlush;

	if(m_dwBase >= m_dwDiscard)
		m_dwBase = 0;

	if( FAILED( hr = m_pVB->Lock( m_dwBase * sizeof(POINTVERTEX),
								  m_dwFlush * sizeof(POINTVERTEX),
								  (void**) &pVertices,
								  m_dwBase ? D3DLOCK_NOOVERWRITE : D3DLOCK_DISCARD ) ) )
    {
		return hr;
    }

		
    // Render each particle
    while( pParticle )
    {
        D3DXVECTOR3 vPos(pParticle->m_vPos);
        D3DXVECTOR3 vVel(pParticle->m_vVel);
        FLOAT       fLengthSq = D3DXVec3LengthSq(&vVel);
        UINT        dwSteps;

		dwSteps = 1;
        vVel *= -0.04f / (FLOAT)dwSteps;

        D3DXCOLOR clrDiffuse = D3DXCOLOR(1,1,1,1);
        //D3DXColorLerp(&clrDiffuse, &pParticle->m_clrFade, &pParticle->m_clrDiffuse, pParticle->m_fFade);
        //clrDiffuse.r=1;clrDiffuse.g=1;clrDiffuse.b=1;clrDiffuse.a=1;
		DWORD dwDiffuse = (DWORD) clrDiffuse;
		

        // Render each particle a bunch of times to get a blurring effect

        pVertices->v     = vPos;
        pVertices->color = dwDiffuse;
        pVertices++;

        if( ++dwNumParticlesToRender == m_dwFlush )
        {
            // Done filling this chunk of the vertex buffer.  Lets unlock and
            // draw this portion so we can begin filling the next chunk.

            m_pVB->Unlock();

            if(FAILED(hr = pd3dDevice->DrawPrimitive( D3DPT_POINTLIST, m_dwBase, dwNumParticlesToRender)))
				return hr;

            // Lock the next chunk of the vertex buffer.  If we are at the 
            // end of the vertex buffer, DISCARD the vertex buffer and start
            // at the beginning.  Otherwise, specify NOOVERWRITE, so we can
            // continue filling the VB while the previous chunk is drawing.
			m_dwBase += m_dwFlush;

			if(m_dwBase >= m_dwDiscard)
				m_dwBase = 0;

			if( FAILED( hr = m_pVB->Lock( m_dwBase * sizeof(POINTVERTEX),
										  m_dwFlush * sizeof(POINTVERTEX),
										  (void**) &pVertices,
										  m_dwBase ? D3DLOCK_NOOVERWRITE : D3DLOCK_DISCARD ) ) )
            {
				return hr;
            }

            dwNumParticlesToRender = 0;
        }
    

        pParticle = pParticle->m_pNext;
    }


    // Unlock the vertex buffer
    m_pVB->Unlock();

    // Render any remaining particles
    if( dwNumParticlesToRender )
    {
        if(FAILED(hr = pd3dDevice->DrawPrimitive( D3DPT_POINTLIST, m_dwBase, dwNumParticlesToRender )))
			return hr;
    }


    // Reset render states
    pd3dDevice->SetRenderState( D3DRS_POINTSPRITEENABLE, FALSE );
    pd3dDevice->SetRenderState( D3DRS_POINTSCALEENABLE,  FALSE );


    pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
	pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,FALSE);        
	pd3dDevice->SetRenderState( D3DRS_LIGHTING,  TRUE );

    return S_OK;
}




