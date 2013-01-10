#include "log.h"


int InitLog(void)
{
	FILE* log=fopen("log.html", "w+");
	fprintf(log, "<html><head><title>WAXIS engine log</title></head><body><h3>WAXIS engine log started</h3>\n");
	fclose(log);

	return 0;
}


int LogPlease(char* string)
{
	FILE* log=fopen("log.html","r+");
	while(fgetc(log) != EOF) continue;

	
	for(char* c = string; *c != '\0'; c++)
	{
		if( *c == '\n' ) fputs("<br>", log);
		else if( *c == '-' && *(c+1) == '=' ) fputs("<b>-", log);
		else if( *c == '=' && *(c+1) == '-' ) fputs("=</b>", log);
		else if( *c == '\t' ) fputs("&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp;", log);
		else fputc(*c, log);
	}
    //fputs(string,log);
	
	fclose(log);
	return 0;
}


int HTMLLog(char* string, ...)
{
	char text[512];
	
	va_list arg_list;

	va_start(arg_list, string);        
	vsprintf(text, string, arg_list);
	va_end(arg_list);

	LogPlease(text);

	return 0;
}

