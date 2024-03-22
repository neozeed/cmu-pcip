/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */
#include	<cmu-note.h>
#include <stdio.h>
#include <types.h>
#include <task.h>
#include <q.h>
#include <netq.h>
#include <net.h>
#include <custom.h>
#include <netbuf.h>
#include <icmp.h>
#include <ip.h>
#include <attrib.h>
#include <colors.h>
#include <h19.h>
#include <ctype.h>
#include <match.h>
#include "watch.h"

struct nameber *lookup(table, num)
	register struct nameber *table;
	unsigned long num; {

	while(table->n_name)
		if(table->n_number == num) {
			table->n_count++;
			return table;
			}
		else table++;

	return 0;
	}

/* caselessly compare two strings */

static cslesscmp(s1, s2)
	register char *s1, *s2; {

	while(1) {
		if((islower(*s1) ? (*s1) - 'a' + 'A' : (*s1)) != 
		   (islower(*s2) ? (*s2) - 'a' + 'A' : (*s2)))
			return FALSE;
		if(!*s1) return TRUE;
		s1++; s2++;
		}
	}

/* lookup a nameber by name */

struct nameber *nlookup(table, name)
	register struct nameber *table;
	register char *name; {
	char *s1, *s2;

	while(table->n_name)
		if(cslesscmp(table->n_name, name)) return table;
		else table++;

	return 0;
	}


print_namebers(table)
	register struct nameber *table; {
	int length = 0;

	while(table->n_name) {
		if(length + strlen(table->n_name) > 79) {
			printf("\n");
			length = 0;
			}

		printf("%s\t", table->n_name);
		length += strlen(table->n_name) + 8 - strlen(table->n_name)%8;
		table++;
		}
	printf("\n");
	}

print_addr_namebers(table)
	register struct nameber *table; {
	int length = 0;

	while(table->n_name) {
		if(!(table->n_layer && table->n_layer->l_addr)) {
			table++;
			continue;
			}

		if(length + strlen(table->n_name) > 79) {
			printf("\n");
			length = 0;
			}

		printf("%s\t", table->n_name);
		length += strlen(table->n_name) + 8 - strlen(table->n_name)%8;

		table++;
		}
	printf("\n");
	}


/* dump statistics for all the namebers */

nameber_stats(root, tabs, lines)
	struct layer *root;
	int tabs;
	int lines; {
	register struct nameber *n;
	register int i;
	int c;

	for(i = 0; i<tabs; i++)
			putchar('\t');

	printf("unknown: %D\n", root->l_unknown);
	lines++;

	if(!root->l_children)
		return lines;

	for(n = root->l_children; n->n_name; n++) {
		if(lines > 21) {
			clr25();
			pr25(0, "---paused [type any character to continue] ---");
			while((c = h19key()) == -1) ;
			if(c == '\n')
				lines--;
			else lines = 0;
			}

		for(i = 0; i<tabs; i++)
			putchar('\t');

		printf("%s: %D\n", n->n_name, n->n_count);

		if(n->n_layer)
			lines = nameber_stats(n->n_layer, tabs+1, lines);
		lines++;

		}

	return lines;
	}
