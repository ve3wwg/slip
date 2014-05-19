#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <setjmp.h>
#include <stdlib.h>

#include "slip.hpp"

static jmp_buf env;

static uint8_t 
readb() {
	uint8_t b;

	if ( fread(&b,1,1,stdin) == 1 )
		return b;
	longjmp(env,1);
}

static void
writeb(uint8_t b) {
	fwrite(&b,1,1,stdout);
}

int main(int argc,char **argv) {
	SLIP slip(readb,writeb);

	slip.enable_crc8(true);

	if ( argv[1] && argv[1][0] == 'w' ) {
		// Read text on stdin, write SLIP encoded on stdout
		char buf[600];

		while ( fgets(buf,sizeof buf,stdin) )
			slip.write(buf,strlen(buf));
		fflush(stdout);

	} else	{
		// Read SLIP encoded on stdin, write text on stdout

		if ( setjmp(env) != 0 ) {
			exit(0);
		}

		char buf[600];
		unsigned len;
		SLIP::Status s;

		for (;;) {
			s = slip.read(buf,sizeof buf,len);
			if ( s != SLIP::ES_Ok )
				printf("ERROR: s = %d, len = %u\n",s,len);
			else	{
				buf[len] = 0;
				fputs(buf,stdout);
			}
		}
	}

	return 0;
}
