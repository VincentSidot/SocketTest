#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <vector>
#include <cstdarg>

#define MAX_LEN 1024

using cstring = const char*;
#define sFOR(string,var) for(size_t var = 0;var < strlen(string);++var)
#define FOR(len,var) for(size_t var = 0;var < len;++var)


typedef struct _data
{
	_data(char c, size_t p) { car = c; pos = p; }
	char car;
	size_t pos;
} data, *pdata;



cstring MAKE_STRING(cstring str, ...)
{
	char tmp[MAX_LEN];
	va_list va;
	va_start(va, str);
	vsprintf_s(tmp,str, va);
	va_end(va);
	return tmp;
}

int main()
{
	printf("Testing : ");
	printf(MAKE_STRING("Hello %s %s", "world", "!"));


	getchar(); 
	return 0;
}