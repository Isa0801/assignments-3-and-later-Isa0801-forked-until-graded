#include <stdio.h>
#include <syslog.h>
#include <stdlib.h>

int main (const int argc, const char** argv)
{
	openlog(NULL, 0, LOG_USER);
	if (argc < 3)
	{
		printf("Needs args\n");
		syslog(LOG_ERR, "Need args");
		closelog();
		exit(1);
	}


	syslog(LOG_DEBUG, "Writing %s to %s", argv[2], argv[1]);

	FILE* target_file = fopen(argv[1], "w+");


	fprintf(target_file, "%s\n", argv[2]);

	fclose(target_file);


	return 0;
}
