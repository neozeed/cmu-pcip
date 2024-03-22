#include <stdio.h>

#define index strchr
#define rindex strrchr

char *malloc(),*strcpy(),*getenv();
char *index(),*rindex();
char *extname();

char serverhost[]="128.103.1.56";

#define NGROUPS 1024
#define STRING 512

char newsrc[STRING]="red.ini";
char fnewsrc[STRING],foldnewsrc[STRING];


struct servent *ser;
struct hostent *hp;

FILE *server;
int serverfd;

int reply,reparticle;

char cur_gname[STRING];
int cur_group;
int cur_numa,cur_first,cur_last;

int cur_article;
char cur_articleid[STRING];
char cur_path[STRING],cur_from[STRING],cur_subject[STRING],cur_replyto[STRING];


char *groups[NGROUPS];
int firsts[NGROUPS];
int ngroups;

char *servp;
int servf;
char servb[2048];

main(argc,argv)
char **argv;
{
	int n,dostat;
	static char buf[STRING];

	if(getenv("HOME")) strcpy(buf,getenv("HOME")); else *buf=0;
	if(getenv("REDREC")) strcpy(newsrc,getenv("REDREC"));
	strcpy(fnewsrc,buf); if(*buf) strcat(fnewsrc,"/"); 
	strcat(fnewsrc,newsrc);
	strcpy(foldnewsrc,buf); if(*buf) strcat(foldnewsrc,"/"); 
	strcat(newsrc,"old"); strcat(foldnewsrc,newsrc);
	if(getenv("NNTPHOST")) strcpy(serverhost,getenv("NNTPHOST"));

	argv++;
	if(*argv&&(**argv=='-')&&((*argv)[1]=='S' || (*argv)[1]=='n')) {
		argv++;
		if(*argv) {
			serverfd=netsocket();
			servf=0;
			getserv(buf,sizeof buf);
			saveactive(*argv);
			exit(0);
		}
		printf("usage: red [-S file]\n");
		exit(1);
	}

	getnewsrc();
	serverfd=netsocket(); servf=0; 
	getserv(buf,sizeof buf); buf[77]='\0'; printf("[%s]\n",buf);
	docommand();
	dumpnewsrc();
}

docommand()
{
	char buf[STRING];
	int cug,nug,pug,pua;

	cug=0; cur_article=firsts[0]; cur_group= -1; nug=1;
	while(1) {
		if(nug) {
			nug=0;
			if(cug<0||cug>=ngroups) break;
			if(cug!=cur_group) {
				printf("[%s]\n",groups[cug]);
				if(cgroup(groups[cug])) {
					printf("[invalid group]\n");
					groups[cug]=NULL; firsts[cug]=0;
					cug++; nug++; continue;
				}
			}
			cur_group=cug;
			if(firsts[cug]<cur_first) firsts[cug]=cur_first;
			if(cur_numa==0) {
				cug++; nug++; continue;
			} else if(firsts[cug]>cur_last+1) {
				printf("[group ahead]\n");
				firsts[cug]=cur_last+1;
				cug++; nug++; continue;
			} else if(firsts[cug]>cur_last) {
				cug++; nug++; continue;
			} else if(cstat(firsts[cug])&&current()&&cnext()) {
				printf("[group inaccessible]\n");
				cug++; nug++; continue;
			}
		} else {
			pug=cug; pua=cur_article;
			if(cnext()) {
				cug++; nug++; continue;
			}
		}
		if(chead()) {
			printf("[article %d: bad header]\n",cur_article);
			continue;
		}

	query:
		strncpy(buf,cur_subject,40);
		buf[40] = 0;
		printf("(%d/%d) %s (%s) ?",cur_article,
			cur_last,buf,extname(cur_from));
		fflush(stdout);
		ngets(buf);

		switch(*buf) {
		case '?':
		case 'h':
			printf("<RETURN> - skip article\n");
			printf("q - quit\n");
			printf("x - exit without changing red.ini\n");
			printf("y - read article\n");
			printf(". - read article\n");
			printf("sfilename - save in filename\n");
			printf("!command - execute command\n");
			printf("c - catch up\n");
			printf("p - previous newsgroup\n");
			printf("n - next newsgroup\n");
			printf("- - previous article\n");
			printf("g# - go to article #\n");
			break;
		case '\0':
			break;
		case 'q':
			return;
		case 'x':
			exit(0);
		case '.':
		case 'y':
			showbody();
			break;
		case 's':
			savebody(buf+1);
			break;
		case '!':
			system(buf+1);
			goto query;
		case 'c':
			firsts[cug]=cur_last+1;
			cug++; nug++;
			continue;
		case 'p':
			cug--; nug++;
			continue;
		case 'n':
			cug++; nug++;
			continue;
		case '-':
			cug=pug; firsts[cug]=pua; nug++;
			continue;
		case 'g':
			if((atoi(buf+1)<cur_last)&&(atoi(buf+1)>0)) {
				firsts[cug]=atoi(buf+1); nug++; continue;
			}
			printf("[article out of range]\n");
			goto query;
		default:
			printf("[illegal command, try ?]\n");
			goto query;
		}
		firsts[cug]=cur_article+1;
	}
}

char *extname(s)
char *s;
{
	static char buf[STRING];
	char *p;

	strcpy(buf,s);
	if((p=index(buf,'(')) && p<rindex(buf,')')) {
		*rindex(buf,')')='\0';
		p=index(buf,'(')+1;
		p[20]='\0';
		return p;
	}
	if(index(buf,' ')) *index(buf,' ')='\0';
	buf[20]='\0';
	return buf;
}


getserv(b,sb)
char *b;
{
	char c,*p=b;
	sb--;
	while(sb>0) {
		while(!servf) {
			servp=servb;
			servf=tcpread(serverfd,servb,sizeof servb);
		}
		if(*servp=='\n') {
			*b++ =0;
			servp++,servf--;
			break;
		} else if(*servp=='\r') servp++,servf--;
		else *b++ = *servp++,servf--;
	}
#ifdef DEBUG
	printf("<<< %s\n",p);
#endif
}

putserv(b)
char *b;
{
	tcpwrite(serverfd,b,strlen(b));
	tcpwrite(serverfd,"\n",1);
#ifdef DEBUG
	printf(">>> %s\n",b);
#endif
}

getnewsrc()
{
	char group[STRING];
	int first;
	int i;
	FILE *rec;

	rec=fopen(fnewsrc,"r");
	if(!rec) {
		perror(fnewsrc); exit(1);
	}
	i=0;
	while((i<NGROUPS)&&(fscanf(rec,"%s %d",group,&first)==2)) {
		groups[i]=strcpy(malloc(strlen(group)+1),group);
		firsts[i]=first;
		i++;
	}
	ngroups=i;
}

dumpnewsrc()
{
	int i;
	FILE *rec;

	unlink(foldnewsrc);
	rename(fnewsrc,foldnewsrc);
	rec=fopen(fnewsrc,"w");
	for(i=0;i<ngroups;i++)
		if(groups[i]) fprintf(rec,"%s %d\n",groups[i],firsts[i]);
	fclose(rec);
}

cgroup(s)
char *s;
{
	char buf[STRING];

	sprintf(buf,"GROUP %s",s); putserv(buf);
	getserv(buf,sizeof buf); sscanf(buf,"%d %d",&reply,&reparticle);
	if(reply==411) return -1;
	else if(reply==211) {
		sscanf(buf,"%d %d %d %d %s",
			&reply,&cur_numa,&cur_first,&cur_last,cur_gname);
		return 0;
	} else printf("cgroup: %s\n",buf);
	return -1;
}

cstat(i)
{
	char buf[STRING],*p,*q;

	sprintf(buf,"STAT %d",i); putserv(buf);
	getserv(buf,sizeof buf); sscanf(buf,"%d %d",&reply,&reparticle);
	if(reply==423) return -1;
	else if(reply==223) {
		cur_article=reparticle;
		p=buf; q=cur_articleid;
		while((p<buf+STRING-2)&&(*p!='<')) p++;
		while((p<buf+STRING-2)&&(*p!='>')) *q++ = *p++;
		*q++ ='>'; *q++='\0';
		return 0;
	} else printf("cstat: %s\n",buf);
	return -1;
}

current()
{
	char buf[STRING],*p,*q;

	sprintf(buf,"STAT"); putserv(buf);
	getserv(buf,sizeof buf); sscanf(buf,"%d %d",&reply,&reparticle);
	if(reply==423) return -1;
	else if(reply==412) return -1;
	else if(reply==223) {
		cur_article=reparticle;
		p=buf; q=cur_articleid;
		while((p<buf+STRING-2)&&(*p!='<')) p++;
		while((p<buf+STRING-2)&&(*p!='>')) *q++ = *p++;
		*q++ ='>'; *q++='\0';
		return 0;
	} else printf("current: %s\n",buf);
	return -1;
}

cnext()
{
	static char name[STRING];
	char buf[STRING],*p,*q;

	sprintf(buf,"NEXT"); putserv(buf);
	getserv(buf,sizeof buf); sscanf(buf,"%d %d",&reply,&reparticle);
	if(reply==421) return -1;
	else if(reply==223) {
		cur_article=reparticle;
		p=buf; q=cur_articleid;
		while((p<buf+STRING-2)&&(*p!='<')) p++;
		while((p<buf+STRING-2)&&(*p!='>')) *q++ = *p++;
		*q++ ='>'; *q++='\0';
		return 0;
	} else printf("cnext: %s\n",buf);
	return -1;
}

extract(to,key,from)
char *to,*key,*from;
{
	if(!strncmp(from,key,strlen(key))) {
		from+=strlen(key);
		while(*from==' ') from++;
		strcpy(to,from);
		return 1;
	} else return 0;
}

chead()
{
	char buf[STRING];

	*cur_path=0;
	*cur_from=0;
	*cur_replyto=0;
	*cur_subject=0;

	sprintf(buf,"HEAD %d",cur_article); putserv(buf);
	getserv(buf,sizeof buf); sscanf(buf,"%d %d",&reply,&reparticle);
	if(reply==423) return -1;
	else if(reply==221) while(1) {
		getserv(buf,sizeof buf);
		if(!strcmp(buf,".")) {
			if(!*cur_replyto) strcpy(cur_replyto,cur_from);
			return 0;
		}
		extract(cur_path,"Path:",buf) ||
		extract(cur_from,"From:",buf) ||
		extract(cur_subject,"Subject:",buf) ||
		extract(cur_replyto,"Reply-To:",buf);
	} else printf("chead: %s\n",buf);
	return -1;
}

showbody()
{
	char buf[STRING];
	int line=0;

	sprintf(buf,"BODY %d",cur_article); putserv(buf);
	getserv(buf,sizeof buf); sscanf(buf,"%d %d",&reply,&reparticle);
	if(reply==423) return -1;
	else if(reply==222) while(1) {
		if(line==22) {
			printf("more(<RETURN> to continue, q to quit)?");
			fflush(stdout);
			ngets(buf);
			if(*buf) {
				while(getserv(buf,sizeof buf),strcmp(buf,"."));
				return 0;
			}
			line=0;
		}
		getserv(buf,sizeof buf);
		if(strcmp(buf,".")) puts(buf); else return 0;
		line++;
	} else printf("showbody: %d %s\n",reply,buf);
	return -1;
}
savebody(s)
char *s;
{
	char buf[STRING];
	FILE *f;

	while(*s == ' ')
		s++;
	if(!(f=fopen(s,"w"))) {
		perror(s); return -1;
	}
	sprintf(buf,"BODY %d",cur_article); putserv(buf);
	getserv(buf,sizeof buf); sscanf(buf,"%d %d",&reply,&reparticle);
	if(reply==423) goto badsave;
	else if(reply==222) {
		fprintf(f,"Subject: %s\n",cur_subject);
		fprintf(f,"From: %s\n",cur_from);
		fprintf(f,"Path: %s\n",cur_path);
		fprintf(f,"Reply-To: %s\n",cur_replyto);
		fprintf(f,"\n");
		while(1) {
			getserv(buf,sizeof buf);
			if(strcmp(buf,".")) fprintf(f,"%s\n",buf); else break;
		}
		fclose(f);
		return 0;
	} else printf("savebody: %d %s\n",reply,buf);
badsave:
	fclose(f);
	return -1;
}

saveactive(s)
char *s;
{
	char buf[STRING];
	FILE *f;

	if(!(f=fopen(s,"w"))) {
		perror(s); return -1;
	}
	sprintf(buf,"LIST"); putserv(buf);
	getserv(buf,sizeof buf); sscanf(buf,"%d %d",&reply,&reparticle);
	if(reply==215) {
		while(1) {
			char group[STRING];
			int to,from;
			char act;

			getserv(buf,sizeof buf);
			if(!strcmp(buf,".")) break;
			sscanf(buf,"%s %d %d %c",group,&to,&from,&act);
			fprintf(f,"%s %d\n",group,to);
		}
		fclose(f);
		return 0;
	} else printf("saveactive: %d %s\n",reply,buf);
badsave:
	fclose(f);
	return -1;
}

netsocket()
{
	int fd;

	if((fd = tcpopen(serverhost, 119)) < 0) {
		fprintf(stderr, "tcpopen failed\n");
		exit(1);
	}
	return(fd);
}
