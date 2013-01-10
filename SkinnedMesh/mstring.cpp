//------------------------------------------------------------------------
//------------------------------------------------------------------------
//     MSTRING
//------------------------------------------------------------------------
//------------------------------------------------------------------------


#include "mstring.h"






D3DXVECTOR3 ConstructVec3(char* str)
{
	int memory[3];
	char string[150];
	int i=1,j=0;
	D3DXVECTOR3 retvec;

	for(int a=0; a<3; a++)
	{
		for(; str[i] != ',' && str[i] != ')';i++, j++)  string[j]=str[i];
		string[j]='\0';
		memory[a]=atoi(string);
		j=0;
		i++;
	}

	retvec = D3DXVECTOR3((float)memory[0],(float)memory[1],(float)memory[2]);

	return retvec;
}

D3DXVECTOR2 ConstructVec2(char* str)
{
	int memory[2];
	char string[150];
	int i=1,j=0;
	D3DXVECTOR2 retvec;

	for(int a=0; a<2; a++)
	{
		for(; str[i] != ',' && str[i] != ')';i++, j++)  string[j]=str[i];
		string[j]='\0';
		memory[a]=atoi(string);
		j=0;
		i++;
	}

	retvec = D3DXVECTOR2((float)memory[0],(float)memory[1]);

	return retvec;
}




char* GetWord(FILE* file)
{
	char* str = new char[256];
	char c;
	int i;

	for( c=fgetc(file), i=0;  c != ' ' && c != '\n' && c != EOF ; c=fgetc(file), i++ ) str[i]=c;
	str[i]='\0';

	return str;
}


char* GetString(FILE* file)
{
	char* str = new char[256];
	char c;
	int i;

	for( c=fgetc(file), i=0;  c != '\n' && c != EOF ; c=fgetc(file), i++ ) str[i]=c;
	str[i]='\0';

	return str;
}


#define NOSUCHWORD -1

long FindWord(FILE* file, char* string)
{
	char* str;

	fseek(file,SEEK_SET,0);
	while( str=GetWord(file) ) if( strcmp(str,string)==0 || str[0]=='\0' ) break;

	if(str[0] == '\0') return NOSUCHWORD;
	else return ftell(file);
}

//-----------------------
//------------------------------------------------------
