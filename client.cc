#define ADR_REQ 0xfe10
#define FSZ_REQ 0xfe20
#define GET_REQ 0xfe30
#define BUFSIZE 9550360

#include <stdio.h>
#include <iostream>

#include <sys/socket.h>
#include <openssl/md5.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>

using namespace std;

class messageclass
{
	public:
	
	uint16_t msgtype;
	int offset;
	int datalength;
	char data[30];
};

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

void catch_pipe(int sig)
{
//	cout<<"\nSIGPIPE . . .";
}

int main(int argc,char *argv[])
{
	struct sockaddr_in server_addrinfo;
	
	int clientsockfd;
	
	int offset,dispflag=0,datalength;
	char hostnmport[30],str[30];
	char *hostnm;
	int port;
	
	unsigned char buftosend[50];
	
	uint16_t msgtype;
	
	signal(SIGPIPE,catch_pipe);
	
//	cout<<"\nnew makefile . . .";
	
/*	cout<<"\nargc :: "<<argc;
	for(int i=0;i<argc;i++)
	{
		cout<<"\nargv+"<<i<<" :: "<<*(argv+i)<<"\n";
	}
*/	
	argc--,argv++;
//	cout<<"\n*argv now :: "<<*argv; 
	
	buftosend[0]=0xff;
	buftosend[1]=0xff;
	
	if(!strcmp(*argv,"adr"))
	{
		//do adr stuff
		//msgtype=ADR_REQ;
		buftosend[0]=buftosend[0] & 0xfe;
		buftosend[1]=buftosend[1] & 0x10;
	}
	else if(!strcmp(*argv,"fsz"))
	{
		//do fsz stuff
		//msgtype=FSZ_REQ;
		buftosend[0]=buftosend[0] & 0xfe;
		buftosend[1]=buftosend[1] & 0x20;
	}
	else if(!strcmp(*argv,"get"))
	{
		//do get stuff
		//msgtype=GET_REQ;
		buftosend[0]=buftosend[0] & 0xfe;
		buftosend[1]=buftosend[1] & 0x30;
	}
	else
	{
		cout<<"wrong first parameter specified should be {adr,fsz or get). Error . . . ";
		exit(1);
	}
	
//	cout<<"\nmsgtype :: "<<msgtype;
	
	argc--,argv++;
	
	while(*argv[0]=='-')
	{
//		cout<<"\n[In while]*argv now :: "<<*argv; 
		if(!strcmp(*argv,"-o"))
		{
			offset=atoi(*(argv+1));
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
//	cout<<"\n*argv now :: "<<*argv; 
	strcpy(hostnmport,*argv);
	hostnm=strtok(hostnmport,":");
//	cout<<"\nHost name :: "<<hostnm;
	
	port=atoi(strtok(NULL,":"));
//	cout<<"\nPort :: "<<port;
	
	
//	cout<<"\n*(argv+1)now :: "<<*(argv+1); 
	strcpy(str,*(argv+1));
	
	datalength=strlen(str);
	
	//Store integer as bytes
	unsigned char *intinbyte=new unsigned char[4];
	int intval=1617057880;
	intinbyte=convert_inttobytes(intval);
	
/*	cout<<"\nchar in int :: "<<(int)intinbyte[0];
	cout<<"\nchar in char :: "<<intinbyte[0];
	for(int i=0;i<4;i++)
	{
		cout<<"\nchar["<<i<<"] :: "<<intinbyte[i];
	}
*/	
	unsigned char *offsetchar=new unsigned char[4];
	offsetchar=convert_inttobytes(offset);
	
	unsigned char *datalengthchar=new unsigned char[4];
	datalengthchar=convert_inttobytes(datalength);
	
	buftosend[2]=offsetchar[0];
	buftosend[3]=offsetchar[1];
	buftosend[4]=offsetchar[2];
	buftosend[5]=offsetchar[3];
	
	buftosend[6]=datalengthchar[0];
	buftosend[7]=datalengthchar[1];
	buftosend[8]=datalengthchar[2];
	buftosend[9]=datalengthchar[3];
	
	int buf_i=10,str_i;
	for(str_i=0;str_i<datalength;str_i++)
	{
		buftosend[buf_i]=str[str_i];
		buf_i++;
	}
	
	
/*	printf("\nbuftosend[0] :: %x",buftosend[0]);
	printf("\nbuftosend[1] :: %x",buftosend[1]);
	for(int i=2;i<buf_i;i++)
	{
		cout<<"\nbuftosend["<<i<<"] :: "<<(int)buftosend[i];
	}
*/
	
	
//	cout<<"\nbuftosend :: "<<buftosend;

	messageclass msg;
	
	msg.msgtype=msgtype;
	msg.offset=offset;
	msg.datalength=datalength;
	strcpy(msg.data,str);
	
	
	
	if((clientsockfd=socket(PF_INET,SOCK_STREAM,IPPROTO_TCP)) < 1) // check if condition to be checked with 0 or 1
	{
		perror("TCP client socket() failed : ");
		exit(1);
	}
	
	int opval1=1;
	int sockopt_chk;
	if((sockopt_chk=setsockopt(clientsockfd,SOL_SOCKET,SO_REUSEADDR,&opval1,(socklen_t)sizeof(opval1)))<0)
	{
		perror("TCP client setsockopt() failed . . .\n");
	}	
	
	memset(&server_addrinfo,0,sizeof(server_addrinfo));
	
	server_addrinfo.sin_family=PF_INET;
	server_addrinfo.sin_addr.s_addr=htonl(INADDR_ANY);
	server_addrinfo.sin_port=port;
	
	if(connect(clientsockfd,(struct sockaddr*)&server_addrinfo,sizeof(server_addrinfo)) < 0)
	{
		perror("TCP client connect() failed : ");
		exit(1);
	}
	
	for(int i=0;i<buf_i;i++)
	{
		if(send(clientsockfd,&buftosend[i],1,0) != 1)
		{
			perror("TCP client send() failed : ");
			exit(1);
		}
	}
	
	unsigned char *buftorecv=new unsigned char[BUFSIZE];
	
	int j=0,recvmsglen;
	
/*	while((recvmsglen=recv(clientsockfd,&buftorecv[j],1,0))==1)
	{	
		cout<<"\nbuftorecv["<<j<<"] :: "<<(int)buftorecv[j];
		j++;
	}
*/

	for(int i=0;i<10;i++)
	{	
		if((recvmsglen=recv(clientsockfd,&buftorecv[i],1,0))!=1)
		{
			perror("TCP client recv() failed : ");
			exit(1);
		}
//		cout<<"\nbuftorecv["<<i<<"] :: "<<(int)buftorecv[i];
	}
	
	unsigned char *dlen=new unsigned char[4];
	
	dlen[0]=buftorecv[9];
	dlen[1]=buftorecv[8];
	dlen[2]=buftorecv[7];
	dlen[3]=buftorecv[6];
	
 	int remlen;//=convert_bytestoint(dlen);

	memset(&remlen,0,sizeof(remlen));
	memcpy(&remlen,dlen,4);
	
//	cout<<"\nData length received :: "<<remlen;
	
	for(int i=10;i<(10+remlen);i++)
	{
		if((recvmsglen=recv(clientsockfd,&buftorecv[i],1,0))!=1)
		{
			perror("TCP recv() failed : ");
			exit(1);
		}
//		cout<<"\nbuftorecv["<<i<<"] :: "<<(int)buftorecv[i];
	}
	
	struct hostent *he;
	if((he=gethostbyname("nunki.usc.edu"))==NULL)		  	                      //Client gethostname()
	{
		herror("TCP server(target) gethostbyname() failed : ");
		exit(1);
	}
	
	char *nunkiipaddress=inet_ntoa(*((struct in_addr*)he->h_addr));
	
	unsigned char *parsemsgtype=new unsigned char[2];
	parsemsgtype[0]=buftorecv[0];
	parsemsgtype[1]=buftorecv[1];
	uint16_t us_msgtype=convert_bytestouint16_t(parsemsgtype);
	
	unsigned char *rplyoffsetchar=new unsigned char[4];
	rplyoffsetchar[0]=buftorecv[2];
	rplyoffsetchar[1]=buftorecv[3];
	rplyoffsetchar[2]=buftorecv[4];
	rplyoffsetchar[3]=buftorecv[5];
	int rplyoffset=convert_bytestoint(rplyoffsetchar);
	
	unsigned char *rplydatalengthchar=new unsigned char[4];
	rplydatalengthchar[0]=buftorecv[6];
	rplydatalengthchar[1]=buftorecv[7];
	rplydatalengthchar[2]=buftorecv[8];
	rplydatalengthchar[3]=buftorecv[9];
	int rplydatalength=convert_bytestoint(rplydatalengthchar);
	
	unsigned char *filebuf=new unsigned char[512];
	
//	cout<<"\nMsg type :: "<<us_msgtype;
	
	if(us_msgtype==17)
	{
		unsigned char *ipaddr=new unsigned char[20];
		int j=0;
		for(int i=10;i<(10+remlen);i++)
		{
			ipaddr[j]=buftorecv[i];
			j++;
		}
		ipaddr[j]='\0';
		
		if(dispflag==1)
		{
			printf("\n\tReceived %d bytes from %s.",(10+remlen),nunkiipaddress);
			printf("\n\t  Messagetype: %#x%x",buftorecv[0],buftorecv[1]);
			printf("\n\t       Offset: 0x%02x%02x%02x%02x",buftorecv[5],buftorecv[4],buftorecv[3],buftorecv[2]);
			printf("\n\t   DataLength: 0x%02x%02x%02x%02x",buftorecv[9],buftorecv[8],buftorecv[7],buftorecv[6]);
			printf("\n\tADDR = %s\n",ipaddr);  			
		}
	}
	else if(us_msgtype==18)
	{
		if(dispflag==1)
		{
			printf("\n\tReceived %d bytes from %s.",(10+remlen),nunkiipaddress);
			printf("\n\t  Messagetype: %#x%x",buftorecv[0],buftorecv[1]);
			printf("\n\t       Offset: 0x%02x%02x%02x%02x",buftorecv[5],buftorecv[4],buftorecv[3],buftorecv[2]);
			printf("\n\t   DataLength: 0x%02x%02x%02x%02x",buftorecv[9],buftorecv[8],buftorecv[7],buftorecv[6]);
			printf("\n\tADDR request for '%s' failed\n",str);  			
		}
	}
	else if(us_msgtype==33)
	{
		unsigned char *flsz=new unsigned char[4];
		int j=3;
		for(int i=10;i<(10+remlen);i++)
		{
			flsz[j]=buftorecv[i];
			j--;
		}
	//	flsz[j]='\0';
	
		int vl;
		
		memset(&vl,0,sizeof(vl));
		memcpy(&vl,flsz,4);
		
		int flsz_inint=convert_bytestoint(flsz);
		
		if(dispflag==1)
		{
			printf("\n\tReceived %d bytes from %s.",(10+remlen),nunkiipaddress);
			printf("\n\t  Messagetype: %#x%x",buftorecv[0],buftorecv[1]);
			printf("\n\t       Offset: 0x%02x%02x%02x%02x",buftorecv[5],buftorecv[4],buftorecv[3],buftorecv[2]);
			printf("\n\t   DataLength: 0x%02x%02x%02x%02x",buftorecv[9],buftorecv[8],buftorecv[7],buftorecv[6]);
			printf("\n\tFILESIZE = %d\n",vl/*flsz_inint*/);  			
		}
	}
	else if(us_msgtype==34)
	{
		if(dispflag==1)
		{
			printf("\n\tReceived %d bytes from %s.",(10+remlen),nunkiipaddress);
			printf("\n\t  Messagetype: %#x%x",buftorecv[0],buftorecv[1]);
			printf("\n\t       Offset: 0x%02x%02x%02x%02x",buftorecv[5],buftorecv[4],buftorecv[3],buftorecv[2]);
			printf("\n\t   DataLength: 0x%02x%02x%02x%02x",buftorecv[9],buftorecv[8],buftorecv[7],buftorecv[6]);
			printf("\n\tFILESIZE request for '%s' failed\n",str);  			
		}
	}
	else if(us_msgtype==49)
	{
	/*	unsigned char *flsz=new unsigned char[4];
		int j=3;
		for(int i=10;i<(10+remlen);i++)
		{
			flsz[j]=buftorecv[i];
			j--;
		}
	//	flsz[j]='\0';
	
		int vl;
		
		memset(&vl,0,sizeof(vl));
		memcpy(&vl,flsz,4);
	*/	
		unsigned char *res=new unsigned char[MD5_DIGEST_LENGTH];
		MD5_CTX md5ctx;
	
		MD5_Init(&md5ctx);
		
		j=0;
		for(int i=0;i<remlen;i++)
		{
			filebuf[j]=buftorecv[i+10];
			j++;
			if(j==512)
			{
				MD5_Update(&md5ctx,(const unsigned char *)filebuf,sizeof(filebuf));
				j=0;
			}
		}
		
		if(j>0)
		{
			MD5_Update(&md5ctx,(const unsigned char *)filebuf,sizeof(filebuf));
		}
		
//		MD5_Update(&md5ctx,(const unsigned char *)filebuf,sizeof(filebuf));
		MD5_Final(res,&md5ctx);
	
/*		cout<<"\n----------MD5-----------\n";
		for(int i=0;i<MD5_DIGEST_LENGTH;i++)
		{
			printf("%x",res[i]);
		}
	
		cout<<"\n-------------------------\n";
*/		
		if(dispflag==1)
		{
			printf("\n\tReceived %d bytes from %s.",(10+remlen),nunkiipaddress);
			printf("\n\t  Messagetype: %#x%x",buftorecv[0],buftorecv[1]);
			printf("\n\t       Offset: 0x%02x%02x%02x%02x",buftorecv[5],buftorecv[4],buftorecv[3],buftorecv[2]);
			printf("\n\t   DataLength: 0x%02x%02x%02x%02x",buftorecv[9],buftorecv[8],buftorecv[7],buftorecv[6]);
			printf("\n\tFILESIZE = %d MD5 = ",remlen/*flsz_inint*/);  
			
			for(int i=0;i<MD5_DIGEST_LENGTH;i++)
			{
				printf("%x",res[i]);
			}
		}
	}
	else if(us_msgtype==50)
	{
		if(dispflag==1)
		{
			printf("\n\tReceived %d bytes from %s.",(10+remlen),nunkiipaddress);
			printf("\n\t  Messagetype: %#x%x",buftorecv[0],buftorecv[1]);
			printf("\n\t       Offset: 0x%02x%02x%02x%02x",buftorecv[5],buftorecv[4],buftorecv[3],buftorecv[2]);
			printf("\n\t   DataLength: 0x%02x%02x%02x%02x",buftorecv[9],buftorecv[8],buftorecv[7],buftorecv[6]);
			printf("\n\tGET request for '%s' failed\n",str);  			
		}
	}
	else
	{
		cout<<"\nWrong response msg type at client. Error . . .";
		exit(1);
	}
	
}
//g++ client.cc -I/home/scf-22/csci551b/openssl/include -L/home/scf-22/csci551b/openssl/lib -lcrypto -lsocket -lnsl -lresolv -o client

