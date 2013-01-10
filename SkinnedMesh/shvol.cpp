//--------------------------------------------------------------------
// Shadow Volume Class
// source
//--------------------------------------------------------------------



#include "shvol.h"



//-----------------------------------------------------------------------------
// Name: AddEdge()
// Desc: Adds an edge to a list of silohuette edges of a shadow volume.
//-----------------------------------------------------------------------------
VOID AddEdge( WORD* pEdges, DWORD& dwNumEdges, WORD v0, WORD v1 )
{
    // Remove interior edges (which appear in the list twice)
    for( DWORD i=0; i < dwNumEdges; i++ )
    {
        if( ( pEdges[2*i+0] == v0 && pEdges[2*i+1] == v1 ) ||
            ( pEdges[2*i+0] == v1 && pEdges[2*i+1] == v0 ) )
        {
            if( dwNumEdges > 1 )
            {
                pEdges[2*i+0] = pEdges[2*(dwNumEdges-1)+0];
                pEdges[2*i+1] = pEdges[2*(dwNumEdges-1)+1];
            }
            dwNumEdges--;
            return;
        }
    }

    pEdges[2*dwNumEdges+0] = v0;
    pEdges[2*dwNumEdges+1] = v1;
    dwNumEdges++;
}








//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CShadowVolume::Init(LPDIRECT3DDEVICE9 d3dDevice,
						   float BackBufferWidth, 
						   float BackBufferHeight)
{
	//set device
	m_pd3dDevice = d3dDevice;

	m_pBigSquareVB = NULL;

    // Create a big square for rendering the stencilbuffer contents
    if( FAILED( m_pd3dDevice->CreateVertexBuffer( 4*sizeof(SHADOWVERTEX),
                                       D3DUSAGE_WRITEONLY,
									   D3DFVF_XYZRHW | D3DFVF_DIFFUSE,
                                       D3DPOOL_MANAGED, 
									   &m_pBigSquareVB,
									   NULL ) ) )
        return E_FAIL;



	// Set the size of the big square shadow
    SHADOWVERTEX* v;
    FLOAT sx = BackBufferWidth;
    FLOAT sy = BackBufferHeight;

    m_pBigSquareVB->Lock( 0, 0, (void**)&v, 0 );
    v[0].p = D3DXVECTOR4(  0, sy, 0.0f, 1.0f );
    v[1].p = D3DXVECTOR4(  0,  0, 0.0f, 1.0f );
    v[2].p = D3DXVECTOR4( sx, sy, 0.0f, 1.0f );
    v[3].p = D3DXVECTOR4( sx,  0, 0.0f, 1.0f );
    v[0].color = 0x7f000000;
    v[1].color = 0x7f000000;
    v[2].color = 0x7f000000;
    v[3].color = 0x7f000000;
    m_pBigSquareVB->Unlock();



	return S_OK;
}
//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CShadowVolume::SetMatrix(D3DXMATRIX *matObjectMatrix)
{
	m_matObjectMatrix = matObjectMatrix;

	return S_OK;
}


//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CShadowVolume::Render()
{
	//m_pd3dDevice->SetTransform(D3DTS_WORLD, m_matObjectMatrix);
	//DrawVolume();
	RenderShadow();
    DrawShadow();

	return S_OK;
}


//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CShadowVolume::Release()
{
	SAFE_RELEASE( m_pBigSquareVB );

	return S_OK;
}



//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CShadowVolume::DrawVolume()
{
    m_pd3dDevice->SetFVF( D3DFVF_XYZ );

    return m_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLELIST,
										  m_dwNumVertices/3,
                                          m_pVertices, 
										  sizeof(D3DXVECTOR3) );
}



//-----------------------------------------------------------------------------
// Name: RenderShadow()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CShadowVolume::RenderShadow()
{

    // Disable z-buffer writes (note: z-testing still occurs), and enable the
    // stencil-buffer
    m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,  FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_STENCILENABLE, TRUE );


    // Dont bother with interpolating color
    m_pd3dDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_FLAT );


    // Set up stencil compare fuction, reference value, and masks.
    // Stencil test passes if ((ref & mask) cmpfn (stencil & mask)) is true.
    // Note: since we set up the stencil-test to always pass, the STENCILFAIL
    // renderstate is really not needed.
    m_pd3dDevice->SetRenderState( D3DRS_STENCILFUNC,  D3DCMP_ALWAYS );
    m_pd3dDevice->SetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP );
    m_pd3dDevice->SetRenderState( D3DRS_STENCILFAIL,  D3DSTENCILOP_KEEP );


    // If ztest passes, inc/decrement stencil buffer value
    m_pd3dDevice->SetRenderState( D3DRS_STENCILREF,       0x1 );
    m_pd3dDevice->SetRenderState( D3DRS_STENCILMASK,      0xffffffff );
    m_pd3dDevice->SetRenderState( D3DRS_STENCILWRITEMASK, 0xffffffff );
    m_pd3dDevice->SetRenderState( D3DRS_STENCILPASS,      D3DSTENCILOP_INCR );


    // Make sure that no pixels get drawn to the frame buffer
    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_ZERO );
    m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );

    m_pd3dDevice->SetRenderState( D3DRS_CULLMODE,   D3DCULL_CCW );

    // Draw front-side of shadow volume in stencil/z only
    m_pd3dDevice->SetTransform( D3DTS_WORLD, m_matObjectMatrix );
    DrawVolume();


    // Now reverse cull order so back sides of shadow volume are written.
    m_pd3dDevice->SetRenderState( D3DRS_CULLMODE,   D3DCULL_CW );


    // Decrement stencil buffer value
    m_pd3dDevice->SetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_DECR );


    // Draw back-side of shadow volume in stencil/z only
    m_pd3dDevice->SetTransform( D3DTS_WORLD, m_matObjectMatrix );
    DrawVolume();


    // Restore render states
    m_pd3dDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );
    m_pd3dDevice->SetRenderState( D3DRS_CULLMODE,  D3DCULL_CCW );
    m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,     TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_STENCILENABLE,    FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: DrawShadow()
// Desc: Draws a big gray polygon over scene according to the mask in the
//       stencil buffer. (Any pixel with stencil==1 is in the shadow.)
//-----------------------------------------------------------------------------
HRESULT CShadowVolume::DrawShadow()
{
    // Set renderstates (disable z-buffering, enable stencil, disable fog, and
    // turn on alphablending)
    m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,          FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_STENCILENABLE,    TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_FOGENABLE,        FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
    m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );

    // Only write where stencil val >= 1 (count indicates # of shadows that
    // overlap that pixel)
    m_pd3dDevice->SetRenderState( D3DRS_STENCILREF,  0x1 );
    m_pd3dDevice->SetRenderState( D3DRS_STENCILFUNC, D3DCMP_LESSEQUAL );
    m_pd3dDevice->SetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_KEEP );



    // Draw a big, gray square
    m_pd3dDevice->SetFVF( D3DFVF_XYZRHW | D3DFVF_DIFFUSE );
    m_pd3dDevice->SetStreamSource( 0, m_pBigSquareVB, 0, sizeof(SHADOWVERTEX) );
    m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );



    // Restore render states
    m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,          TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_STENCILENABLE,    FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_FOGENABLE,        TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );

    return S_OK;
}


// Name: Reset
VOID    CShadowVolume::Reset() { m_dwNumVertices = 0L; }

//-----------------------------------------------------------------------------
// Name: BuildFromMesh()
// Desc: Takes a mesh as input, and uses it to build a shadowvolume. The
//       technique used considers each triangle of the mesh, and adds it's
//       edges to a temporary list. The edge list is maintained, such that
//       only silohuette edges are kept. Finally, the silohuette edges are
//       extruded to make the shadow volume vertex list.
//-----------------------------------------------------------------------------
HRESULT CShadowVolume::BuildFromMesh( LPD3DXMESH pMesh, D3DXVECTOR3 vLight )
{
	//vars
	DWORD i;

    // Note: the MESHVERTEX format depends on the FVF of the mesh
    struct MESHVERTEX { D3DXVECTOR3 p, n; FLOAT tu, tv; };

    MESHVERTEX* pVertices;
    WORD*       pIndices;

    // Lock the geometry buffers
    pMesh->LockVertexBuffer( 0L, (LPVOID*)&pVertices );
    pMesh->LockIndexBuffer( 0L, (LPVOID*)&pIndices );
    DWORD dwNumFaces    = pMesh->GetNumFaces();

    // Allocate a temporary edge list
    WORD* pEdges = new WORD[dwNumFaces*6];
    if( pEdges == NULL )
    {
        pMesh->UnlockVertexBuffer();
        pMesh->UnlockIndexBuffer();
        return E_OUTOFMEMORY;
    }
    DWORD dwNumEdges = 0;

    // For each face
    for( i=0; i<dwNumFaces; i++ )
    {
        WORD wFace0 = pIndices[3*i+0];
        WORD wFace1 = pIndices[3*i+1];
        WORD wFace2 = pIndices[3*i+2];

        D3DXVECTOR3 v0 = pVertices[wFace0].p;
        D3DXVECTOR3 v1 = pVertices[wFace1].p;
        D3DXVECTOR3 v2 = pVertices[wFace2].p;

        // Transform vertices or transform light?
        D3DXVECTOR3 vCross1(v2-v1);
        D3DXVECTOR3 vCross2(v1-v0);
        D3DXVECTOR3 vNormal;
        D3DXVec3Cross( &vNormal, &vCross1, &vCross2 );

        if( D3DXVec3Dot( &vNormal, &vLight ) >= 0.0f)
        {
            AddEdge( pEdges, dwNumEdges, wFace0, wFace1 );
            AddEdge( pEdges, dwNumEdges, wFace1, wFace2 );
            AddEdge( pEdges, dwNumEdges, wFace2, wFace0 );
        }
    }


    for( i=0; i < 2*dwNumEdges ; i+=2 )
    {

        D3DXVECTOR3 v1 = pVertices[pEdges[i+0]].p;
        D3DXVECTOR3 v2 = pVertices[pEdges[i+1]].p;
        D3DXVECTOR3 v3 = v1 - vLight*10;
        D3DXVECTOR3 v4 = v2 - vLight*10;

        // Add a quad (two triangles) to the vertex list
        m_pVertices[m_dwNumVertices++] = v1;
        m_pVertices[m_dwNumVertices++] = v2;
        m_pVertices[m_dwNumVertices++] = v3;

        m_pVertices[m_dwNumVertices++] = v2;
        m_pVertices[m_dwNumVertices++] = v4;
        m_pVertices[m_dwNumVertices++] = v3;
    }
    // Delete the temporary edge list
    delete[] pEdges;

    // Unlock the geometry buffers
    pMesh->UnlockVertexBuffer();
    pMesh->UnlockIndexBuffer();

    return S_OK;
}


