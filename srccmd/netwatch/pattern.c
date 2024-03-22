/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
#include <stdio.h>
#include <match.h>
#include "watch.h"

/* clear a pattern
*/
pt_clear(p)
	register pattern *p; {
	register int i;

	for(i = 0; i < MATCH_DATA_LEN; i++)
		p->bytes[i] = 0;

	p->count = 0;
	}

pt_clrbytes(p, offset, count)
	register pattern *p;
	int offset;
	int count; {
	register int i;

	for(i = 0; i < count; i++)
		p->bytes[i+offset] = 0;

	/* need to recompute count */
	}

pt_addbytes(p, bytes, offset, count)
	register pattern *p;
	char *bytes;
	int offset;
	int count; {
	register int i;

	for(i=0; i < count; i++)
		p->bytes[i+offset] = 0xff00|(*bytes++);

	if(p->count < count+offset)
		p->count = count + offset;

	}

pt_dump(p)
	pattern *p; {
	int i;

	for(i=0; i<MATCH_DATA_LEN; i++) {
		printf("%04x ", p->bytes[i]);
		if((i+1) % 15 == 0)
			printf("\n");
		}
	return;

	for(i=0; i<MATCH_DATA_LEN; i++)
		if((p->bytes[i] & 0xff00) == 0xff00) {
			printf("offset %d: ", i);
			while((p->bytes[i] & 0xff00) == 0xff00)
				printf("%02x ", p->bytes[i++]&0xff);
			printf("\n");
			}
	}
