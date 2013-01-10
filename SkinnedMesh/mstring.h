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


#define NOSUCHWORD -1


//-------------------------------------------------------
//------------------------
D3DXVECTOR3 ConstructVec3(char* str);

D3DXVECTOR2 ConstructVec2(char* str);

char* GetWord(FILE* file);
char* GetString(FILE* file);

long FindWord(FILE* file, char* string);

//-----------------------
//------------------------------------------------------
