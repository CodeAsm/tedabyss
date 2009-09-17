// winsock2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "winsock2.h"

#define SIO_RCVALL _WSAIOW(IOC_VENDOR,1)    //this removes the need of mstcpip.h

void StartSniffing   (SOCKET Sock);             //This will sniff here and there
void ProcessPacket   (unsigned char* , int);    //This will decide how to digest
void PrintIpHeader   (unsigned char* , int);    
void PrintIcmpPacket (unsigned char* , int);
void PrintUdpPacket  (unsigned char* , int);
void PrintTcpPacket  (unsigned char* , int);
void ConvertToHex    (unsigned char* , unsigned int);
void PrintData       (unsigned char* , int);

//IP Header Structure
typedef struct ip_hdr
{
    unsigned char  ip_header_len:4;  // 4-bit header length (in 32-bit words) normally=5 (Means 20 Bytes may be 24 also)
    unsigned char  ip_version   :4;  // 4-bit IPv4 version
    unsigned char  ip_tos;           // IP type of service
    unsigned short ip_total_length;  // Total length
    unsigned short ip_id;            // Unique identifier 
    
	unsigned char  ip_frag_offset   :5;        // Fragment offset field
	
	unsigned char  ip_more_fragment :1;
	unsigned char  ip_dont_fragment :1;
	unsigned char  ip_reserved_zero :1;
	
    unsigned char  ip_frag_offset1;    //fragment offset
	
	unsigned char  ip_ttl;           // Time to live
    unsigned char  ip_protocol;      // Protocol(TCP,UDP etc)
    unsigned short ip_checksum;      // IP checksum
    unsigned int   ip_srcaddr;       // Source address
    unsigned int   ip_destaddr;      // Source address
}   IPV4_HDR;

//UDP Header Structure
typedef struct udp_hdr
{
    unsigned short source_port;     // Source port no.
    unsigned short dest_port;       // Dest. port no.
    unsigned short udp_length;      // Udp packet length
    unsigned short udp_checksum;    // Udp checksum (optional)
}   UDP_HDR;

// TCP Header Structure
typedef struct tcp_header 
{ 
	unsigned short source_port;  // source port 
	unsigned short dest_port;    // destination port 
	unsigned int   sequence;     // sequence number - 32 bits 
	unsigned int   acknowledge;  // acknowledgement number - 32 bits 
		
	unsigned char  ns   :1;          //Nonce Sum Flag Added in RFC 3540.
	unsigned char  reserved_part1:3; //according to rfc
	unsigned char  data_offset:4;    /*The number of 32-bit words in the TCP header. 
									   This indicates where the data begins. 
									   The length of the TCP header is always a multiple 
									   of 32 bits.*/
    		
	unsigned char  fin  :1;      //Finish Flag
	unsigned char  syn  :1;      //Synchronise Flag
	unsigned char  rst  :1;      //Reset Flag
	unsigned char  psh  :1;      //Push Flag 
	unsigned char  ack  :1;      //Acknowledgement Flag 
	unsigned char  urg  :1;      //Urgent Flag
    
	unsigned char  ecn  :1;      //ECN-Echo Flag
    unsigned char  cwr  :1;      //Congestion Window Reduced Flag
	
	////////////////////////////////
	
	unsigned short window;          // window 
	unsigned short checksum;        // checksum 
	unsigned short urgent_pointer;  // urgent pointer 
}   TCP_HDR;

//ICMP Header Structure
typedef struct icmp_hdr    
{
    BYTE type;          // ICMP Error type
    BYTE code;          // Type sub code
    USHORT checksum;
    USHORT id;
    USHORT seq;
}   ICMP_HDR;


FILE *logfile;
int tcp=0,udp=0,icmp=0,others=0,igmp=0,total=0,i,j;
struct sockaddr_in source,dest;
char hex[2];

//Its free!
IPV4_HDR *iphdr;
TCP_HDR  *tcpheader;
UDP_HDR  *udpheader;
ICMP_HDR *icmpheader;

#define TEST 1

#define DEFAULT_PORT 9010
#define TEST_SERVER "203.110.170.155"

class MovePacket
{
public:
	MovePacket(DWORD* locationArray, WORD stepNum, DWORD timeStamp, DWORD serverInfo, DWORD area, WORD faceDir = 0)
	{
		WORD packet_len = 18+stepNum*4;
		Size = packet_len+10;
		Data = new BYTE[Size];

		INT index = 0;
		DWORD fixed = 0x4d5a0000;
		WORD action = 0xf0c2;
		memcpy(&Data[index],&fixed,4);
		index += 4;
		memcpy(&Data[index],&timeStamp,4);
		index += 4;
		memcpy(&Data[index],&packet_len,2);
		index += 2;
		memcpy(&Data[index],&action,2);
		index += 2;
		memcpy(&Data[index],&serverInfo,4);
		index += 4;
		memcpy(&Data[index],&stepNum,2);
		index += 2;

		for(INT i=0;i<stepNum;i++)
		{
			memcpy(&Data[index],&locationArray[i],4);
			index += 4;
		}

		memcpy(&Data[index],&faceDir,2);
		index += 2;
		memcpy(&Data[index],&timeStamp,4);
		index += 4;
	}

	virtual ~MovePacket()
	{
		if(Data)
		{
			delete [] Data;
		}
	}

	BYTE* GetData()
	{
		return &Data[0];
	}

	WORD GetSize()
	{
		return Size;
	}

private:
	BYTE* Data;
	WORD Size;
};


int _tmain(int argc, _TCHAR* argv[])
{

		
#if defined(TEST)

	WSADATA wsa;	

	//Initialise Winsock
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2,2), &wsa) != 0)
	{
		printf("WSAStartup() failed.\n");
		return 1;
	}
    printf("Initialised");

	//----------------------
    // Create a SOCKET for connecting to server
	SOCKET ConnectSocket;
    struct sockaddr_in clientService; 

	DWORD locations[3] = {0x00300025,0x002f0025,0x002e0025};
	DWORD timestamp = 0x00fb3000;
	DWORD serverinfo = 0x0002ae05;
	DWORD area = 0x0000138c;
	MovePacket packet(locations,3,timestamp,serverinfo,area);

    ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ConnectSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    //----------------------
    // The sockaddr_in structure specifies the address family,
    // IP address, and port of the server to be connected to.
    clientService.sin_family = AF_INET;
    clientService.sin_addr.s_addr = inet_addr( TEST_SERVER );
    clientService.sin_port = htons( DEFAULT_PORT );

    //----------------------
    // Connect to server.
	int iResult;
    iResult = connect( ConnectSocket, (SOCKADDR*) &clientService, sizeof(clientService) );
    if (iResult == SOCKET_ERROR) {
        printf( "connect failed with error: %d\n", WSAGetLastError() );
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
  }

    //----------------------
    // Send an initial buffer
	iResult = send( ConnectSocket, (const char*)packet.GetData(), packet.GetSize(), 0 );
    if (iResult == SOCKET_ERROR) {
        printf("send() failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    printf("Bytes Sent: %d\n", iResult);

    // shutdown the connection since no more data will be sent
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

	while(1);

	return 1;
#else

	SOCKET sniffer;
	struct in_addr addr;
	DWORD in;	
	
	char hostname[100];
	struct hostent *local;
	WSADATA wsa;	

	fopen_s(&logfile,"log.txt","w");
	if(logfile==NULL) printf("Unable to create file.");

	//Initialise Winsock
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2,2), &wsa) != 0)
	{
		printf("WSAStartup() failed.\n");
		return 1;
	}
    printf("Initialised");

		//Create a RAW Socket
	printf("\nCreating RAW Socket...");
	sniffer = socket(AF_INET, SOCK_RAW, IPPROTO_IP);
    if (sniffer == INVALID_SOCKET)
	{
		printf("Failed to create raw socket.\n");
		return 1;
	}
    printf("Created.");
		
	//Retrive the local hostname
	if (gethostname(hostname, sizeof(hostname)) == SOCKET_ERROR) 
	{
        printf("Error : %d",WSAGetLastError());
		return 1;
    }
	printf("\nHost name : %s \n",hostname);
	
	//Retrive the available IPs of the local host
	local = gethostbyname(hostname);
    printf("\nAvailable Network Interfaces : \n");
	if (local == NULL) 
	{
        printf("Error : %d.\n",WSAGetLastError());
        return 1;
    }

    for (i = 0; local->h_addr_list[i] != 0; ++i) 
	{
		memcpy(&addr, local->h_addr_list[i], sizeof(struct in_addr));
        printf("Interface Number : %d Address : %s\n",i,inet_ntoa(addr));
    }

	printf("Enter the interface number you would like to sniff : ");
	scanf_s("%d",&in);
	
	memset(&dest, 0, sizeof(dest));
	memcpy(&dest.sin_addr.s_addr,local->h_addr_list[in],sizeof(dest.sin_addr.s_addr));
	dest.sin_family      = AF_INET;
	dest.sin_port        = 0;
    
	printf("\nBinding socket to local system and port 0 ...");
	if (bind(sniffer,(struct sockaddr *)&dest,sizeof(dest)) == SOCKET_ERROR)
	{
		printf("bind(%s) failed.\n", inet_ntoa(addr));
		return 1;
	}
    printf("Binding successful"); 
		
	//Enable this socket with the power to sniff : SIO_RCVALL is the key Receive ALL ;)
	
	j=1;
	printf("\nSetting socket to sniff...");
	if (WSAIoctl(sniffer, SIO_RCVALL, &j, sizeof(j), 0, 0, &in,0, 0) == SOCKET_ERROR)
	{
		printf("WSAIoctl() failed.\n");
		return 1;
	}
    printf("Socket set.");
	
	//Begin
	printf("\nStarted Sniffing\n");
	printf("Packet Capture Statistics...\n");
	StartSniffing(sniffer);   //Happy Sniffing
	
	//End
	closesocket(sniffer);
	WSACleanup();

	return 0;
#endif


}

void StartSniffing(SOCKET sniffer)
{
	BYTE *Buffer = new BYTE[65536]; //Its Big!
	int  mangobyte;
	
	if (Buffer == NULL)
	{
		printf("malloc() failed.\n");
		return;
	}

	do
	{
		mangobyte = recvfrom(sniffer,(char*)Buffer,65536,0,0,0); //Eat as much as u can
		if(mangobyte > 0)	ProcessPacket(Buffer, mangobyte);
		else printf( "recvfrom() failed.\n");
	} 
	while (mangobyte > 0);

	delete [] Buffer;
}

void ProcessPacket(unsigned char* Buffer, int Size)
{
	iphdr = (IPV4_HDR *)Buffer;
	++total;
	switch (iphdr->ip_protocol) //Check the Protocol and do accordingly...
	{
		case 1:  //ICMP Protocol
			++icmp;
			PrintIcmpPacket(Buffer,Size);
			break;
		
		case 2:  //IGMP Protocol
			++igmp;
			break;
		
		case 6:  //TCP Protocol
			++tcp;
			PrintTcpPacket(Buffer,Size);
			break;
		
		case 17: //UDP Protocol
			++udp;
			PrintUdpPacket(Buffer,Size);
			break;
		
		default: //Some Other Protocol like ARP etc.
			++others;
			break;
	}
	printf("TCP : %d   UDP : %d   ICMP : %d   IGMP : %d   Others : %d   Total : %d\r",tcp,udp,icmp,igmp,others,total);
}


void PrintIpHeader (unsigned char* Buffer, int Size)
{
	unsigned short iphdrlen;
		
	iphdr = (IPV4_HDR *)Buffer;
	iphdrlen = iphdr->ip_header_len*4;
	
	memset(&source, 0, sizeof(source));
	source.sin_addr.s_addr = iphdr->ip_srcaddr;
	
	memset(&dest, 0, sizeof(dest));
	dest.sin_addr.s_addr = iphdr->ip_destaddr;
	
	fprintf(logfile,"\n");
	fprintf(logfile,"IP Header\n");
	fprintf(logfile,"   |-IP Version        : %d\n",(unsigned int)iphdr->ip_version);
	fprintf(logfile,"   |-IP Header Length  : %d DWORDS or %d Bytes\n",(unsigned int)iphdr->ip_header_len,((unsigned int)(iphdr->ip_header_len))*4);
	fprintf(logfile,"   |-Type Of Service   : %d\n",(unsigned int)iphdr->ip_tos);
	fprintf(logfile,"   |-IP Total Length   : %d  Bytes(Size of Packet)\n",ntohs(iphdr->ip_total_length));
	fprintf(logfile,"   |-Identification    : %d\n",ntohs(iphdr->ip_id));
	fprintf(logfile,"   |-Reserved ZERO Field   : %d\n",(unsigned int)iphdr->ip_reserved_zero);
	fprintf(logfile,"   |-Dont Fragment Field   : %d\n",(unsigned int)iphdr->ip_dont_fragment);
	fprintf(logfile,"   |-More Fragment Field   : %d\n",(unsigned int)iphdr->ip_more_fragment);
	fprintf(logfile,"   |-TTL      : %d\n",(unsigned int)iphdr->ip_ttl);
	fprintf(logfile,"   |-Protocol : %d\n",(unsigned int)iphdr->ip_protocol);
	fprintf(logfile,"   |-Checksum : %d\n",ntohs(iphdr->ip_checksum));
	fprintf(logfile,"   |-Source IP        : %s\n",inet_ntoa(source.sin_addr));
	fprintf(logfile,"   |-Destination IP   : %s\n",inet_ntoa(dest.sin_addr));
}

void PrintTcpPacket(unsigned char* Buffer, int Size)
{
	unsigned short iphdrlen;
	
	iphdr = (IPV4_HDR *)Buffer;
	iphdrlen = iphdr->ip_header_len*4;
	
	tcpheader=(TCP_HDR*)(Buffer+iphdrlen);
			
	fprintf(logfile,"\n\n***********************TCP Packet*************************\n");	
		
	PrintIpHeader(Buffer,Size);
		
	fprintf(logfile,"\n");
	fprintf(logfile,"TCP Header\n");
	fprintf(logfile,"   |-Source Port      : %u\n",ntohs(tcpheader->source_port));
	fprintf(logfile,"   |-Destination Port : %u\n",ntohs(tcpheader->dest_port));
	fprintf(logfile,"   |-Sequence Number    : %u\n",ntohl(tcpheader->sequence));
	fprintf(logfile,"   |-Acknowledge Number : %u\n",ntohl(tcpheader->acknowledge));
	fprintf(logfile,"   |-Header Length      : %d DWORDS or %d BYTES\n"
		,(unsigned int)tcpheader->data_offset,(unsigned int)tcpheader->data_offset*4);
	fprintf(logfile,"   |-CWR Flag : %d\n",(unsigned int)tcpheader->cwr);
	fprintf(logfile,"   |-ECN Flag : %d\n",(unsigned int)tcpheader->ecn);
	fprintf(logfile,"   |-Urgent Flag          : %d\n",(unsigned int)tcpheader->urg);
	fprintf(logfile,"   |-Acknowledgement Flag : %d\n",(unsigned int)tcpheader->ack);
	fprintf(logfile,"   |-Push Flag            : %d\n",(unsigned int)tcpheader->psh);
	fprintf(logfile,"   |-Reset Flag           : %d\n",(unsigned int)tcpheader->rst);
	fprintf(logfile,"   |-Synchronise Flag     : %d\n",(unsigned int)tcpheader->syn);
	fprintf(logfile,"   |-Finish Flag          : %d\n",(unsigned int)tcpheader->fin);
	fprintf(logfile,"   |-Window         : %d\n",ntohs(tcpheader->window));
	fprintf(logfile,"   |-Checksum       : %d\n",ntohs(tcpheader->checksum));
	fprintf(logfile,"   |-Urgent Pointer : %d\n",tcpheader->urgent_pointer);
	fprintf(logfile,"\n");
	fprintf(logfile,"                        DATA Dump                         ");
	fprintf(logfile,"\n");
		
	fprintf(logfile,"IP Header\n");
	PrintData(Buffer,iphdrlen);
		
	fprintf(logfile,"TCP Header\n");
	PrintData(Buffer+iphdrlen,tcpheader->data_offset*4);
		
	fprintf(logfile,"Data Payload\n");	
	PrintData(Buffer+iphdrlen+tcpheader->data_offset*4
		,(Size-tcpheader->data_offset*4-iphdr->ip_header_len*4));
						
	fprintf(logfile,"\n###########################################################");
}

void PrintUdpPacket(unsigned char *Buffer,int Size)
{
	unsigned short iphdrlen;
		
	iphdr = (IPV4_HDR *)Buffer;
	iphdrlen = iphdr->ip_header_len*4;
	
	udpheader = (UDP_HDR *)(Buffer + iphdrlen);
	
	fprintf(logfile,"\n\n***********************UDP Packet*************************\n");
	
	PrintIpHeader(Buffer,Size);			
	
	fprintf(logfile,"\nUDP Header\n");
	fprintf(logfile,"   |-Source Port      : %d\n",ntohs(udpheader->source_port));
	fprintf(logfile,"   |-Destination Port : %d\n",ntohs(udpheader->dest_port));
	fprintf(logfile,"   |-UDP Length       : %d\n",ntohs(udpheader->udp_length));
	fprintf(logfile,"   |-UDP Checksum     : %d\n",ntohs(udpheader->udp_checksum));
	
	fprintf(logfile,"\n");
	fprintf(logfile,"IP Header\n");
	PrintData(Buffer,iphdrlen);
		
	fprintf(logfile,"UDP Header\n");
	PrintData(Buffer+iphdrlen,sizeof(UDP_HDR));
		
	fprintf(logfile,"Data Payload\n");	
	PrintData(Buffer+iphdrlen+sizeof(UDP_HDR)
		,(Size - sizeof(UDP_HDR) - iphdr->ip_header_len*4));
	
	fprintf(logfile,"\n###########################################################");
}

void PrintIcmpPacket(unsigned char* Buffer , int Size)
{
	unsigned short iphdrlen;
		
	iphdr = (IPV4_HDR *)Buffer;
	iphdrlen = iphdr->ip_header_len*4;
	
	icmpheader=(ICMP_HDR*)(Buffer+iphdrlen);
		
	fprintf(logfile,"\n\n***********************ICMP Packet*************************\n");	
	PrintIpHeader(Buffer,Size);
			
	fprintf(logfile,"\n");
		
	fprintf(logfile,"ICMP Header\n");
	fprintf(logfile,"   |-Type : %d",(unsigned int)(icmpheader->type));
			
	if((unsigned int)(icmpheader->type)==11) fprintf(logfile,"  (TTL Expired)\n");
	else if((unsigned int)(icmpheader->type)==0) fprintf(logfile,"  (ICMP Echo Reply)\n");
			
	
	fprintf(logfile,"   |-Code : %d\n",(unsigned int)(icmpheader->code));
	fprintf(logfile,"   |-Checksum : %d\n",ntohs(icmpheader->checksum));
	fprintf(logfile,"   |-ID       : %d\n",ntohs(icmpheader->id));
	fprintf(logfile,"   |-Sequence : %d\n",ntohs(icmpheader->seq));
	fprintf(logfile,"\n");

	fprintf(logfile,"IP Header\n");
	PrintData(Buffer,iphdrlen);
		
	fprintf(logfile,"UDP Header\n");
	PrintData(Buffer+iphdrlen,sizeof(ICMP_HDR));
		
	fprintf(logfile,"Data Payload\n");	
	PrintData(Buffer+iphdrlen+sizeof(ICMP_HDR)
		,(Size - sizeof(ICMP_HDR) - iphdr->ip_header_len*4));
	
	fprintf(logfile,"\n###########################################################");
}

void PrintData (unsigned char* data , int Size)
{
	
	for(i=0 ; i < Size ; i++)
	{
		if( i!=0 && i%16==0)   //if one line of hex printing is complete...
		{
			fprintf(logfile,"         ");
			for(j=i-16 ; j<i ; j++)
			{
				if(data[j]>=32 && data[j]<=128)
					fprintf(logfile,"%c",(unsigned char)data[j]); //if its a number or alphabet
				
				else fprintf(logfile,"."); //otherwise print a dot
			}
			fprintf(logfile,"\n");
		} 
		
		if(i%16==0) fprintf(logfile,"   ");
	
		fprintf(logfile," %02X",(unsigned int)data[i]);
				
		if( i==Size-1)  //print the last spaces
		{
			for(j=0;j<15-i%16;j++) fprintf(logfile,"   "); //extra spaces
			
			fprintf(logfile,"         ");
			
			for(j=i-i%16 ; j<=i ; j++)
			{
				if(data[j]>=32 && data[j]<=128) fprintf(logfile,"%c",(unsigned char)data[j]);
				else fprintf(logfile,".");
			}
			fprintf(logfile,"\n");
		}
	}
}

