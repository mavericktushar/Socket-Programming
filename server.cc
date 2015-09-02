#define MAXCONN 10
#define BUFSIZE 512

#include <stdio.h>
#include <iostream>
#include <sys/socket.h>


#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/stat.h>
#include <openssl/md5.h>
#include <signal.h>



using namespace std;

int servsockfd;
int delayresponse;
int dispflag;
int endprocflag;
int socknum;

pthread_t newthread[MAXCONN];

unsigned char* convert_inttobytes(int intval)
{	
	unsigned char *temp=new unsigned char[4];
	int inttemp;
	
	inttemp=intval;
	temp[0]=inttemp & 0xff;
//	cout<<"\ntemp[0] :: "<<temp[0];
	//cout<<"\nval 0th byte:: "<<c;
	
	inttemp=intval;
	inttemp=inttemp>>8;
	temp[1]=inttemp & 0xff;
//	cout<<"\ntemp[1] :: "<<temp[1];
	
	//cout<<"\nval 1st byte :: "<<c;
	
	inttemp=intval;
	inttemp=inttemp>>16;
	temp[2]=inttemp & 0xff;
//	cout<<"\ntemp[2] :: "<<temp[2];
	
	//cout<<"\nval 2nd byte :: "<<c;
	
	inttemp=intval;
	inttemp=inttemp>>24;
	temp[3]=inttemp & 0xff;
//	cout<<"\ntemp[3] :: "<<temp[3];
	
	//cout<<"\nval 3rd byte :: "<<c;
	
	return temp;
}

int convert_bytestoint(unsigned char* dlen)
{
	int val;
	
	val=0xffffffff;
	val=val & dlen[3];
	val=val<<8;
	
//	val=0xff;
	val=val | 0xff;
	val=val & dlen[2];
	val=val<<8;
	
//	val=0xff;
	val=val | 0xff;
	val=val & dlen[1];
	val=val<<8;
	
//	val=0xff;
	val=val | 0xff;
	val=val & dlen[0];
	
	return val;
}

uint16_t convert_bytestouint16_t(unsigned char* parsemsgtype)
{
	uint16_t val;
	
	val=0xff;
	val=val & parsemsgtype[0];
	val=val<<8;
	
	val=0xff;
	val=val & parsemsgtype[1];
	
	return val;
}

void catch_int_t(int sig)
{
//	cout<<"\ncatching pthread_cancel . . .";
}

//void tcp_connest(int clientsockfd)
void *tcp_connest(void *clientsockfd)
{
	struct hostent *he;
	int csock;
//	cout<<"\nConnection established . . .";
	
	csock=(int)clientsockfd;
	int recvmsglen;
	int bytesrecvd;
	unsigned char *buf=new unsigned char[BUFSIZE];
	unsigned char *buftosendrply;
	int md5flag=0;
	unsigned char *res1=new unsigned char[MD5_DIGEST_LENGTH];
	
	char *hname=new char[20];
	
	int i=0;
/*	while((recvmsglen=recv(csock,&buf[i],1,0))==1)
	{	
		cout<<"\nbuf["<<i<<"] :: "<<(int)buf[i];
		i++;
	}
*/

	signal(SIGINT,catch_int_t);

	while(i<10)
	{	
		if((recvmsglen=recv(csock,&buf[i],1,0))!=1)
		{
			perror("TCP recv() failed : ");
			exit(1);
		}
//		cout<<"\nbuf["<<i<<"] :: "<<(int)buf[i];
		i++;
	}
	
	unsigned char *dlen=new unsigned char[4];
	
	dlen[0]=buf[6];
	dlen[1]=buf[7];
	dlen[2]=buf[8];
	dlen[3]=buf[9];
	
 	int remlen=convert_bytestoint(dlen);
	
//	cout<<"\nData length received :: "<<remlen;
	
	while(i<(10+remlen))
	{
		if((recvmsglen=recv(csock,&buf[i],1,0))!=1)
		{
			perror("TCP recv() failed : ");
			exit(1);
		}
//		cout<<"\nbuf["<<i<<"] :: "<<(int)buf[i];
		i++;
	}
	
	bytesrecvd=i;
	
	unsigned char *parsemsgtype=new unsigned char[2];
	parsemsgtype[0]=buf[0];
	parsemsgtype[1]=buf[1];
	uint16_t us_msgtype=convert_bytestouint16_t(parsemsgtype);
	
//	cout<<"\nMsg type :: "<<us_msgtype;
	
	struct hostent *he1;
	if((he1=gethostbyname("nunki.usc.edu"))==NULL)		  	                      //Client gethostname()
	{
		herror("TCP server(target) gethostbyname() failed : ");
		exit(1);
	}
	
	char *nunkiipaddress=inet_ntoa(*((struct in_addr*)he1->h_addr));
	
	if(dispflag==1)
	{
		printf("\n\tReceived %d bytes from %s.",(10+remlen),nunkiipaddress);
		printf("\n\t  Messagetype: %#x%x",buf[0],buf[1]);
		printf("\n\t       Offset: 0x%02x%02x%02x%02x",buf[5],buf[4],buf[3],buf[2]);
		printf("\n\t   DataLength: 0x%02x%02x%02x%02x",buf[9],buf[8],buf[7],buf[6]);
	}
	
	if(us_msgtype==16)
	{
		int k=0;
		int adrfail=0;
		for(int j=10;j<bytesrecvd;j++)
		{
			hname[k]=buf[j];
			k++;
		}
		hname[k]='\0';
//		cout<<"\nHost name :: "<<hname;
		
		if((he=gethostbyname(hname))==NULL)
		{
			herror("TCP server gethostbyname() failed : ");
		//	exit(1);
			adrfail=1;	
		}
		
		unsigned char *offsetchar=new unsigned char[4];
		unsigned char *datalengthchar=new unsigned char[4];
		int ipaddrstrlen=0;
		char *ipaddress;
		
		if(adrfail!=1)
		{
			ipaddress=inet_ntoa(*((struct in_addr*)he->h_addr));
		
//			cout<<"\ngethostbyname() :: "<<ipaddress;
			
			ipaddrstrlen=strlen(ipaddress);
			
			buftosendrply=new unsigned char[10+ipaddrstrlen];
			
			buftosendrply[0]=0xff;
			buftosendrply[1]=0xff;
			
			buftosendrply[0]=buftosendrply[0] & 0xfe;
			buftosendrply[1]=buftosendrply[1] & 0x11;
		
			datalengthchar=convert_inttobytes(ipaddrstrlen);
		}
		else
		{
			buftosendrply=new unsigned char[10];
			
			buftosendrply[0]=0xff;
			buftosendrply[1]=0xff;
			
			buftosendrply[0]=buftosendrply[0] & 0xfe;
			buftosendrply[1]=buftosendrply[1] & 0x12;
			
			datalengthchar=convert_inttobytes(0);
		}
		
		offsetchar=convert_inttobytes(0);				//offset not specified for reply, so 0
		
		buftosendrply[2]=offsetchar[0];
		buftosendrply[3]=offsetchar[1];
		buftosendrply[4]=offsetchar[2];
		buftosendrply[5]=offsetchar[3];
	
		buftosendrply[6]=datalengthchar[0];
		buftosendrply[7]=datalengthchar[1];
		buftosendrply[8]=datalengthchar[2];
		buftosendrply[9]=datalengthchar[3];
		
		int buf_i=10,str_i;
		for(str_i=0;str_i<ipaddrstrlen;str_i++)
		{
			buftosendrply[buf_i]=ipaddress[str_i];
			buf_i++;
		}
	
	
/*		printf("\nbuftosendrply[0] :: %x",buftosendrply[0]);
		printf("\nbuftosendrply[1] :: %x",buftosendrply[1]);
		for(int j=2;j<buf_i;j++)
		{
			cout<<"\nbuftosendrply["<<j<<"] :: "<<(int)buftosendrply[j];
		}
*/		
		for(int j=0;j<buf_i;j++)
		{
			if(send(csock,&buftosendrply[j],1,0) != 1)
			{
				perror("TCP server send() failed : ");
				exit(1);
			}
		}		  		            
	}
	else if(us_msgtype==32)
	{
		int k=0;
		char flname[30];
		int statfail=0;
		int fsize=0;
		
		for(int j=10;j<bytesrecvd;j++)
		{
			flname[k]=buf[j];
			k++;
		}
		flname[k]='\0';
//		cout<<"\nFile name :: "<<flname;
		
		struct stat statval;
		
		unsigned char *offsetchar=new unsigned char[4];
		unsigned char *datalengthchar=new unsigned char[4];
	
		if(stat(flname,&statval)<0)
		{
			statfail=1;
		}
		
		if(statfail!=1)
		{
//			cout<<"\nUsing stat on file :: "<<statval.st_size;

			fsize=statval.st_size;
			
			buftosendrply=new unsigned char[14];
			
			buftosendrply[0]=0xff;
			buftosendrply[1]=0xff;
			
			buftosendrply[0]=buftosendrply[0] & 0xfe;
			buftosendrply[1]=buftosendrply[1] & 0x21;
		
			datalengthchar=convert_inttobytes(4);
		}
		else
		{
			buftosendrply=new unsigned char[10];
			
			buftosendrply[0]=0xff;
			buftosendrply[1]=0xff;
			
			buftosendrply[0]=buftosendrply[0] & 0xfe;
			buftosendrply[1]=buftosendrply[1] & 0x22;
			
			datalengthchar=convert_inttobytes(0);
		}
		
		offsetchar=convert_inttobytes(0);				//offset not specified for reply, so 0
		
		buftosendrply[2]=offsetchar[0];
		buftosendrply[3]=offsetchar[1];
		buftosendrply[4]=offsetchar[2];
		buftosendrply[5]=offsetchar[3];
	
		buftosendrply[6]=datalengthchar[0];
		buftosendrply[7]=datalengthchar[1];
		buftosendrply[8]=datalengthchar[2];
		buftosendrply[9]=datalengthchar[3];
		
		unsigned char *fsizeinchar=convert_inttobytes(fsize);
		
		int buf_i=10,str_i;
		for(str_i=0;str_i<4;str_i++)
		{
			buftosendrply[buf_i]=fsizeinchar[str_i];
			buf_i++;
		}
		
/*		printf("\nbuftosendrply[0] :: %x",buftosendrply[0]);
		printf("\nbuftosendrply[1] :: %x",buftosendrply[1]);
		for(int j=2;j<buf_i;j++)
		{
			cout<<"\nbuftosendrply["<<j<<"] :: "<<(int)buftosendrply[j];
		}
*/		
		sleep(delayresponse);
		
		for(int j=0;j<buf_i;j++)
		{
			if(send(csock,&buftosendrply[j],1,0) != 1)
			{
				perror("TCP server send() failed : ");
				exit(1);
			}
		}		  	
	}
	else if(us_msgtype==48)
	{
		int k=0;
		char flname[30];
		int getfail=0;
		FILE *fp;
		int numbytes;
		
		for(int j=10;j<bytesrecvd;j++)
		{
			flname[k]=buf[j];
			k++;
		}
		flname[k]='\0';
//		cout<<"\nFile name :: "<<flname;
		
		unsigned char *offsetchar=new unsigned char[4];
		unsigned char *datalengthchar=new unsigned char[4];
		unsigned char *filebuf=new unsigned char[512];
		unsigned char *tempbuf=new unsigned char[9650000];
		
		if(!(fp=fopen(flname,"r")))
		{
			perror("File open failed : ");
		//	exit(1);
			getfail=1;
		}
		
		if(getfail!=1)
		{
			md5flag=1;
			unsigned char *floffsetchar=new unsigned char[4];
			
			floffsetchar[0]=buf[2];
			floffsetchar[1]=buf[3];
			floffsetchar[2]=buf[4];
			floffsetchar[3]=buf[5];
			
			int floffset=convert_bytestoint(floffsetchar);
			
//			cout<<"\nValue of offset :: "<<floffset;
			
			unsigned char *res=new unsigned char[MD5_DIGEST_LENGTH];
			MD5_CTX md5ctx;
			
			MD5_Init(&md5ctx);
			
			int readchar,i=0,j=0,totsize=0,initflag=0;
			while  ((readchar=fgetc(fp))!=EOF)
			{
				if(i>=floffset)
				{
//					printf("%c",readchar);
					filebuf[j]=readchar;
					tempbuf[totsize]=readchar;
					j++;
					
					if(j==512)
					{
						initflag=1;
						MD5_Update(&md5ctx,(const unsigned char *)filebuf,sizeof(filebuf));
						j=0;
					}
					totsize++;
				}
				i++;
			}
			
			if(j>0)
			{
//				initflag=1;
				MD5_Update(&md5ctx,(const unsigned char *)filebuf,sizeof(filebuf));
			}
	
			numbytes=totsize;
			
/*			for(int i=0;i<numbytes;i++)
			{
				cout<<filebuf[i];
			}
*/	
			
//			MD5_Update(&md5ctx,(const unsigned char *)filebuf,sizeof(filebuf));
			MD5_Final(res,&md5ctx);
	
/*			cout<<"\n----------MD5-----------\n";
			for(int i=0;i<MD5_DIGEST_LENGTH;i++)
			{
				printf("%x",res[i]);
				res1[i]=res[i];
			}
	
			cout<<"\n-------------------------\n";
			
*/			
			if(dispflag==1)
			{
				cout<<"\nMD5 = ";
				for(int i=0;i<MD5_DIGEST_LENGTH;i++)
				{
					printf("%x",res[i]);
				}
			}
		
			buftosendrply=new unsigned char[10+numbytes];
			
			buftosendrply[0]=0xff;
			buftosendrply[1]=0xff;
			
			buftosendrply[0]=buftosendrply[0] & 0xfe;
			buftosendrply[1]=buftosendrply[1] & 0x31;
		
			datalengthchar=convert_inttobytes(numbytes);
		}
		else
		{
			buftosendrply=new unsigned char[10];
			
			buftosendrply[0]=0xff;
			buftosendrply[1]=0xff;
			
			buftosendrply[0]=buftosendrply[0] & 0xfe;
			buftosendrply[1]=buftosendrply[1] & 0x32;
			
			datalengthchar=convert_inttobytes(0);
		}
		
		offsetchar=convert_inttobytes(0);				//offset not specified for reply, so 0
		
		buftosendrply[2]=offsetchar[0];
		buftosendrply[3]=offsetchar[1];
		buftosendrply[4]=offsetchar[2];
		buftosendrply[5]=offsetchar[3];
	
		buftosendrply[6]=datalengthchar[0];
		buftosendrply[7]=datalengthchar[1];
		buftosendrply[8]=datalengthchar[2];
		buftosendrply[9]=datalengthchar[3];
		
		int buf_i=10,str_i;
		for(str_i=0;str_i<numbytes;str_i++)
		{
			buftosendrply[buf_i]=tempbuf[str_i];
			buf_i++;
		}
		
/*		printf("\nbuftosendrply[0] :: %x",buftosendrply[0]);
		printf("\nbuftosendrply[1] :: %x",buftosendrply[1]);
		for(int j=2;j<buf_i;j++)
		{
			cout<<"\nbuftosendrply["<<j<<"], buf_i :: "<<(int)buftosendrply[j]<<" , "<<buf_i;
		}
*/		
		if(md5flag==1)
		{
		/*	cout<<"\n----------MD5-----------\n";
			for(int i=0;i<MD5_DIGEST_LENGTH;i++)
			{
				printf("%x",res1[i]);
			}
		
				cout<<"\n-------------------------\n";
*/		}
		
		
		for(int j=0;j<buf_i;j++)
		{
			if(send(csock,&buftosendrply[j],1,0) != 1)
			{
				perror("TCP server send() failed : ");
				exit(1);
			}
		}	
	}
	else
	{
		cout<<"\nWrong message type. Error . . .";
		exit(1);
	}
	
	close(csock);
//	convert_asciitohex();
}

void catch_alarm(int sig)
{	
	endprocflag=1;
	for(int i=0;i<socknum;i++)
	{
		pthread_cancel(newthread[i]);
	}
//	shutdown(servsockfd,1);
//	exit(1);	
}

void catch_int(int sig)
{
	endprocflag=1;
	for(int i=0;i<socknum;i++)
	{
		pthread_cancel(newthread[i]);
	}
	sleep(8);
//	shutdown(servsockfd,1);
	exit(1);
}

void catch_pipe(int sig)
{
	endprocflag=1;
}

int main(int argc,char *argv[])
{
	struct sockaddr_in servsock_addrinfo;
	
	int /*servsockfd,*/clientsockfd;
	socklen_t servsockaddrinfolen;
	
//	pthread_t newthread[MAXCONN];
	pthread_attr_t attr;
	int retval;
	void *status;
	int autoshutdown=60;
//	int delayresponse=0;
	delayresponse=0;
//	int dispflag=0;
	dispflag=0;
	
	endprocflag=0;
	
	argc--,argv++;
	
//	cout<<"\nnew makefile . . .";
	
	while(*argv[0]=='-')
	{
//		cout<<"\n[In while]*argv now :: "<<*argv; 
		if(!strcmp(*argv,"-t"))
		{
			autoshutdown=atoi(*(argv+1));
			argc=argc-2;
			argv=argv+2;
		}
		else if(!strcmp(*argv,"-d"))
		{
			delayresponse=atoi(*(argv+1));
			argc=argc-2;
			argv=argv+2;
		}
		else if(!strcmp(*argv,"-m"))
		{
			dispflag=1;
			argc--;
			argv++;
		}
	}
	
	int portnum=atoi(*argv);
/*	cout<<"\nautoshutdown :: "<<autoshutdown;
	cout<<"\ndelayresponse :: "<<delayresponse;
	cout<<"\ndispflag :: "<<dispflag;
	cout<<"Port number received is :: "<<portnum<<"\n";
*/	
	signal(SIGALRM,catch_alarm);
	signal(SIGINT,catch_int);
	signal(SIGPIPE,catch_pipe);
		
	alarm(autoshutdown);
	
	if((servsockfd=socket(PF_INET,SOCK_STREAM,IPPROTO_TCP)) < 1)
	{
		perror("TCP server socket() failed : ");
		exit(1);
	}
	
	int opval1=1;
	int sockopt_chk;
	if((sockopt_chk=setsockopt(servsockfd,SOL_SOCKET,SO_REUSEADDR,&opval1,(socklen_t)sizeof(opval1)))<0)
	{
		perror("TCP server setsockopt() failed . . .\n");
	}	
	
	memset(&servsock_addrinfo,0,sizeof(servsock_addrinfo));
	
	servsock_addrinfo.sin_family=PF_INET;
	servsock_addrinfo.sin_addr.s_addr=htonl(INADDR_ANY);
	servsock_addrinfo.sin_port=htons(portnum);
	
	if(bind(servsockfd,(struct sockaddr*)&servsock_addrinfo,sizeof(servsock_addrinfo)) < 0)
	{
		perror("TCP server bind() failed : ");
		exit(1);
	}
	
	if(listen(servsockfd,MAXCONN) < 0)
	{
		perror("TCP server listen() failed : ");
		exit(1);
	}
	
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
	
	
	
	servsockaddrinfolen=sizeof(servsock_addrinfo);
	socknum=0;

	while(1)
	{
//		cout<<"\nIn while . . .";
//		cout<<"\nendprocflag :: "<<endprocflag;
		if((clientsockfd=accept(servsockfd,(struct sockaddr*)&servsock_addrinfo,&servsockaddrinfolen)) < 0)
		{
			if(endprocflag==0)
			{
				perror("TCP server accept() failed : ");
				exit(1);
			}
			else
			{
				break;
			}
		}
		
//		cout<<"\nClient socket created (socket fd) :: "<<clientsockfd;
		
		if((retval=pthread_create(&newthread[socknum],&attr,tcp_connest,(void *)clientsockfd)))
		{
			perror("\nThread not created. Error . . .");
			exit(1);
		}
//		cout<<"\nThread created . . .";		
		
	//	tcp_connest(clientsockfd);
		
		socknum++;
	//	sleep(5);
	}
	
	pthread_attr_destroy(&attr);

	for(int i=0;i<socknum;i++)
	{
		pthread_join(newthread[i],&status);
	}
	
	shutdown(servsockfd,1);
	
	return 0;
}
//g++ server.cc -I/home/scf-22/csci551b/openssl/include -L/home/scf-22/csci551b/openssl/lib -lsocket -lnsl -lresolv -lcrypto -o server

