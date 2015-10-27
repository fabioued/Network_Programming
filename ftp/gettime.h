#include <sys/time.h>

char* get_time()
{
	struct timeval tv;
	static char str[30];
	char *ptr;

	if(gettimeofday(&tv, NULL) < 0)
		fprintf(stderr, "gettimeofday error");
	
//	ptr = ctime(&tv.tv_sec);
//	strcpy(str, &ptr[11]);

//	snprintf(str + 8, sizeof(str) - 8, ".%06ld", tv.tv_usec);
	sprintf(str, " ");

	return str;
}
