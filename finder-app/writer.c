#include <stdio.h>
#include <syslog.h>
int main(int argc, char* argv[]){
	if(argc < 3) {
			syslog(LOG_ERR,"Invalid number of arguments");
			return 1;
	}
	FILE *fp = fopen(argv[1], "a+");
	if(!fp) {
			syslog(LOG_ERR,"File creation failed!!");
			return 1;
	}

	syslog(LOG_DEBUG,"Writing %s to %s",argv[2],argv[1]);
	fprintf(fp,"%s", argv[2]);
	fclose(fp);

	return 0;
}
