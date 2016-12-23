#include "stdafx.h"
#include <winsock2.h>
#include <windows.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#define usermetaOffset 65536
#define categorymetaOffset 65664

struct node
{
	char msg[128];
	int msg_id;
	node *next;
}*flist,*alist,*printid;

struct bufserv{
	
		int userId;
		int forumId;
		int msgId;
		int commentId;
		int choice;
		char *forumname;
		char msg[128];
}buf1;

struct userMetaData
{
	char userName[32];
	char fileName[64];
	int live;
	int positions[2];
};

struct userMetaData um;

struct USER
{
	char user_name[30];
	char regd_date[11];
	int next_user_offset;
};
struct TECHNICIAN
{
	char tech_name[28];
	char specialization[28];
	char phone[11];
	int user_offset;
	int next_tech_offset;
};
struct CATEGORY
{
	char catg_name[24];
	int tech_offset;
};

struct bitVector
{
	char bit[65536];
};

struct user_metadata
{
	int user[31];
	int user_count;
};

struct category_metadata
{
	int category[127];
	int category_count;
};

struct user
{
	char username[32];
	int user_category_count;
	int user_categories[5];
	int unused[72];
};

struct category
{
	char category_name[36];
	int cat_message_count;
	int next_page_messages;
	int cat_messages[117];
};

struct message
{
	char mesage[128];
	int reply_count;
	int next_page_replies;
	int mes_reply[94];
};

struct reply
{
	char rply[128];
};

struct bitVector b;
struct user_metadata UM;
struct category_metadata cm;
struct user u;
struct category c;
struct message m;
struct reply r;

struct USER U;
struct TECHNICIAN T;
struct CATEGORY C;

bool flag=true;
int mid = 0;
int count1 =0;
char *Data[100];
int count=1;
int values[100];
DWORD WINAPI SocketHandler(void*);
void replyto_client(char *buf, int *csock);
int CendOffset,BendOffset;

void socket_server() {

	//The port you want the server to listen on
	int host_port= 1101;

	unsigned short wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD( 2, 2 );
 	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 || ( LOBYTE( wsaData.wVersion ) != 2 ||
		    HIBYTE( wsaData.wVersion ) != 2 )) {
	    fprintf(stderr, "No sock dll %d\n",WSAGetLastError());
		goto FINISH;
	}

	//Initialize sockets and set options
	int hsock;
	int * p_int ;
	hsock = socket(AF_INET, SOCK_STREAM, 0);
	if(hsock == -1){
		printf("Error initializing socket %d\n",WSAGetLastError());
		goto FINISH;
	}
	
	p_int = (int*)malloc(sizeof(int));
	*p_int = 1;
	if( (setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1 )||
		(setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1 ) ){
		printf("Error setting options %d\n", WSAGetLastError());
		free(p_int);
		goto FINISH;
	}
	free(p_int);

	//Bind and listen
	struct sockaddr_in my_addr;
	my_addr.sin_family = AF_INET ;
	my_addr.sin_port = htons(host_port);
	
	memset(&(my_addr.sin_zero), 0, 8);
	my_addr.sin_addr.s_addr = INADDR_ANY ;
	
	/* if you get error in bind 
	make sure nothing else is listening on that port */
	if( bind( hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1 ){
		fprintf(stderr,"Error binding to socket %d\n",WSAGetLastError());
		goto FINISH;
	}
	if(listen( hsock, 10) == -1 ){
		fprintf(stderr, "Error listening %d\n",WSAGetLastError());
		goto FINISH;
	}
	
	//Now lets do the actual server stuff

	int* csock;
	sockaddr_in sadr;
	int	addr_size = sizeof(SOCKADDR);
	
	while(true){
		printf("waiting for a connection\n");
		csock = (int*)malloc(sizeof(int));
		
		if((*csock = accept( hsock, (SOCKADDR*)&sadr, &addr_size))!= INVALID_SOCKET ){
			//printf("Received connection from %s",inet_ntoa(sadr.sin_addr));
			CreateThread(0,0,&SocketHandler, (void*)csock , 0,0);
		}
		else{
			fprintf(stderr, "Error accepting %d\n",WSAGetLastError());
		}
	}

FINISH:
;
}

void replyto_client(char *buf, int *csock) {
	
	if(send(*csock, buf, 1024, 0)==SOCKET_ERROR){
		fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
		free (csock);
	}
}

void blobAddFile(char *recvbuf,int *csock)
{
	FILE *fp;
	fp = fopen("gbfile.bin", "rb+");
	int buffer_index = 3,brek=0,size,start;
	int recvbuf_len = 1024;
	int recv_byte_cnt,temp=0;
	int len = recvbuf[2];
	char sizebuffer[15];
	while(temp<len)
		um.userName[temp++] = recvbuf[buffer_index++];
	um.userName[temp] = '\0';
	len=recvbuf[buffer_index++];
	temp = 0;
	while (temp<len)
		um.fileName[temp++]= recvbuf[buffer_index++];
	um.fileName[temp] = '\0';
	len = recvbuf[buffer_index++];
	temp = 0;
	while (temp<len)
		sizebuffer[temp++] = recvbuf[buffer_index++];
	sizebuffer[temp] = '\0';
	size=atoi(sizebuffer);
	fseek(fp, 0, SEEK_SET);
	fread(&BendOffset, sizeof(int), 1, fp);
	fseek(fp,BendOffset,SEEK_SET);
	start = sizeof(struct userMetaData);
	um.positions[0] = ftell(fp)+start;
	um.positions[1] = um.positions[0] + size;
	um.live =1;
	fwrite(&um, sizeof(struct userMetaData), 1, fp);
	temp = 0;
	while (temp<(size/1024))
	{
		if ((recv_byte_cnt = recv(*csock, recvbuf, 1024, 0)) == SOCKET_ERROR) {
			fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
			free(csock);
			return;
		}
		fwrite(recvbuf, sizeof(char), 1024, fp);
		temp++;
	}
	if (size % 1024 != 0)
	{
		if ((recv_byte_cnt = recv(*csock, recvbuf, 1024, 0)) == SOCKET_ERROR) {
			fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
			free(csock);
			return;
		}
		fwrite(recvbuf, sizeof(char), (size%1024), fp);
		temp++;
	}
	printf("%d\n", temp);
	BendOffset = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	fwrite(&BendOffset, sizeof(int), 1, fp);
	fclose(fp);
}

void blobViewFiles(char *recvbuf, int *csock)
{
	char username[32];
	char replybuf[1024];
	int reply_index = 0,sizeUser,found=0,x;
	strcpy(username, recvbuf + 3);
	sizeUser = strlen(username);
	FILE *fp;
	fp = fopen("gbfile.bin", "rb");
	fread(&BendOffset, sizeof(int), 1, fp);
	memset(replybuf, '\0', 1024);
	fseek(fp, 4, SEEK_SET);
	while (ftell(fp)<BendOffset)
	{
		fread(&um, sizeof(struct userMetaData), 1, fp);
		if (strcmp(um.userName, username) == 0 && um.live==1)
		{
			found = 1;
			if (1024 - strlen(replybuf) > 64)
			{
				strcat(replybuf, um.fileName);
				strcat(replybuf, "\n");
			}
			else
			{

				replyto_client(replybuf, csock);
				memset(replybuf, '\0', 1024);
				strcat(replybuf, um.fileName);
				strcat(replybuf, "\n");
			}
		}
		fseek(fp, um.positions[1], SEEK_SET);
		memset(&um, 0, sizeof(struct userMetaData));
	}
	if (found == 0)
		strcpy(replybuf, "no users exists with that name in the database\n");
	replyto_client(replybuf, csock);
}

void blobDownloadFile(char *recvbuf, int *csock)
{
	char userName[32],fileName[32];
	char replybuf[1024],filesize[16];
	int reply_index = 0,temp,len,buffer_index=2,found=0;
	FILE *fp;
	fp = fopen("gbfile.bin", "rb");
	fread(&BendOffset, sizeof(int), 1, fp);
	memset(replybuf, '\0', 1024);
	temp = 0;
	len = recvbuf[buffer_index++];
	while (temp<len)
		userName[temp++] = recvbuf[buffer_index++];
	userName[temp] = '\0';
	len = recvbuf[buffer_index++];
	temp = 0;
	while (temp<len)
		fileName[temp++] = recvbuf[buffer_index++];
	fileName[temp] = '\0';
	fseek(fp, 4, SEEK_SET);
	while (ftell(fp)<BendOffset)
	{
		fread(&um, sizeof(struct userMetaData), 1, fp);
		if (strcmp(um.userName, userName) == 0 && strcmp(um.fileName,fileName)==0 &&um.live==1)
		{
			found = 1;
			break;
		}
		fseek(fp, um.positions[1], SEEK_SET);
		memset(&um, 0, sizeof(struct userMetaData));
	}
	if (found == 1)
	{
		fseek(fp, um.positions[0], SEEK_SET);
		int size = um.positions[1] - um.positions[0];
		itoa(size, filesize, 10);
		replyto_client(filesize, csock);
		memset(replybuf, '\0', 1024);
		temp = 0;
		while (temp < (size / 1024))
		{
			fread(replybuf, sizeof(char), 1024, fp);
			replyto_client(replybuf, csock);
			temp++;
			memset(replybuf, '\0', 1024);
		}
		if (size % 1024 != 0)
		{
			fread(replybuf, sizeof(char), size % 1024, fp);
			replyto_client(replybuf, csock);
			temp++;
			memset(replybuf, '\0', 1024);
		}
		printf("%d\n", temp);
	}
	else
	{
		strcpy(replybuf, "whether userName or file name wrong\n");
		replyto_client(replybuf, csock);
	}
}

void blobDeleteFile(char *recvbuf, int *csock)
{
	char userName[32], fileName[32];
	char replybuf[1024], filesize[16];
	int reply_index = 0, temp, len, buffer_index = 2, found = 0;
	FILE *fp;
	fp = fopen("gbfile.bin", "rb+");
	memset(replybuf, '\0', 1024);
	fread(&BendOffset, sizeof(int), 1, fp);
	temp = 0;
	len = recvbuf[buffer_index++];
	while (temp<len)
		userName[temp++] = recvbuf[buffer_index++];
	userName[temp] = '\0';
	len = recvbuf[buffer_index++];
	temp = 0;
	while (temp<len)
		fileName[temp++] = recvbuf[buffer_index++];
	fileName[temp] = '\0';
	int present = 0;
	temp = 0;
	fseek(fp, 4, SEEK_SET);
	while (ftell(fp)<BendOffset)
	{
		fread(&um, sizeof(struct userMetaData), 1, fp);
		if (strcmp(um.userName, userName) == 0 && strcmp(um.fileName, fileName) == 0 && um.live == 1)
		{
			found = 1;
			break;
		}
		present = um.positions[1];
		fseek(fp, um.positions[1], SEEK_SET);
		memset(&um, 0, sizeof(struct userMetaData));
		temp++;
	}
	memset(&um, '\0', sizeof(struct userMetaData));
	if (found == 1)
	{
		fseek(fp, present, SEEK_SET);
		fread(&um, sizeof(struct userMetaData), 1, fp);
		um.live = 0;
		fseek(fp, present, SEEK_SET);
		fwrite(&um, sizeof(struct userMetaData), 1, fp);
		strcpy(replybuf, "file deleted successfully\n");
	}
	else
		strcpy(replybuf, "whether user or file doesnot exist\n");

	replyto_client(replybuf, csock);
	fclose(fp);
}

void blobStore(char *recvbuf,int *csock)
{
	FILE *fp;
	fp = fopen("gbfile.bin", "rb+");
	if (fp == NULL)
	{
		system("cmd.exe /c \"fsutil file createnew gbfile.bin 1073741824\"");
	}
	fread(&BendOffset, sizeof(int), 1, fp);
	if (BendOffset == 0)
	{
		BendOffset = 4;
		fseek(fp, 0, SEEK_SET);
		fwrite(&BendOffset, sizeof(int), 1, fp);
	}
	fclose(fp);
	switch (recvbuf[1])
	{
	case '1':
		blobAddFile(recvbuf,csock);
		break;
	case '2':
		blobViewFiles(recvbuf,csock);
		break;
	case '3':
		blobDownloadFile(recvbuf,csock);
		break;
	case '4':
		blobDeleteFile(recvbuf,csock);
		break;
	}
}

void initialize(FILE *fp)
{
	int x;
	char categories[5][24] = { "doctor","police","lawyer","engineer","technician" };
	fseek(fp, 0, SEEK_SET);
	CendOffset=144;
	fwrite(&CendOffset, sizeof(int), 1, fp);
	for (int i = 0; i < 5; i++)
	{
		strcpy(C.catg_name, categories[i]);
		C.tech_offset = 0;
		fwrite(&C, sizeof(struct CATEGORY), 1, fp);
		x = ftell(fp);
	}
	fclose(fp);
}

void addTechnician(char *recvbuf, int *csock)
{
	FILE *fp;
	fp = fopen("mb100file.bin", "rb+");
	int len, temp, bufferIndex = 3,prev,tempOffset;
	prev = (recvbuf[1]-1) * 28+4;
	fread(&CendOffset, sizeof(int), 1, fp);
	fseek(fp, (recvbuf[1]-1) * 28+4, SEEK_SET);
	fread(&C, sizeof(struct CATEGORY), 1, fp);
	if (C.tech_offset == 0)
	{
		temp = 0;
		len = recvbuf[bufferIndex++];
		while (temp < len)
			T.tech_name[temp++] = recvbuf[bufferIndex++];
		T.tech_name[temp] = '\0';
		temp = 0;
		len = recvbuf[bufferIndex++];
		while (temp < len)
			T.specialization[temp++] = recvbuf[bufferIndex++];
		T.specialization[temp] = '\0';
		temp = 0;
		len = recvbuf[bufferIndex++];
		while (temp < len)
			T.phone[temp++] = recvbuf[bufferIndex++];
		T.phone[temp] = '\0';
		T.user_offset = 0;
		T.next_tech_offset = 0;
		fseek(fp, CendOffset,SEEK_SET);
		tempOffset = ftell(fp);
		fwrite(&T, sizeof(struct TECHNICIAN), 1, fp);
		CendOffset = ftell(fp);
		fseek(fp, (recvbuf[1]-1) * 28+4, SEEK_SET);
		fread(&C, sizeof(struct CATEGORY), 1, fp);
		C.tech_offset = tempOffset;
		fseek(fp, (recvbuf[1] - 1) * 28+4, SEEK_SET);
		fwrite(&C, sizeof(struct CATEGORY), 1, fp);
	}
	else
	{
		prev = C.tech_offset;
		fseek(fp,C.tech_offset,SEEK_SET);
		fread(&T, sizeof(struct TECHNICIAN), 1, fp);
		while (T.next_tech_offset != 0)
		{
			prev = T.next_tech_offset;
			fseek(fp,T.next_tech_offset, SEEK_SET);
			fread(&T, sizeof(struct TECHNICIAN), 1, fp);
		}
		temp = 0;
		len = recvbuf[bufferIndex++];
		while (temp < len)
			T.tech_name[temp++] = recvbuf[bufferIndex++];
		T.tech_name[temp] = '\0';
		temp = 0;
		len = recvbuf[bufferIndex++];
		while (temp < len)
			T.specialization[temp++] = recvbuf[bufferIndex++];
		T.specialization[temp] = '\0';
		temp = 0;
		len = recvbuf[bufferIndex++];
		while (temp < len)
			T.phone[temp++] = recvbuf[bufferIndex++];
		T.phone[temp] = '\0';
		T.user_offset = 0;
		T.next_tech_offset = 0;
		fseek(fp, CendOffset, SEEK_SET);
		tempOffset= ftell(fp);
		fwrite(&T, sizeof(struct TECHNICIAN), 1, fp);
		CendOffset = ftell(fp);
		fseek(fp, prev, SEEK_SET);
		fread(&T, sizeof(struct TECHNICIAN), 1, fp);
		T.next_tech_offset = tempOffset;
		fseek(fp, prev, SEEK_SET);
		fwrite(&T, sizeof(struct TECHNICIAN), 1, fp);
	}
	fseek(fp, 0, SEEK_SET);
	fwrite(&CendOffset, sizeof(int), 1, fp);
	fclose(fp);
	replyto_client("Added Successfully\n", csock);
}

void viewTechnicianDetails(char *recvbuf, int *csock)
{
	char replybuf[1024] = { '\0' };
	FILE *fp;
	fp = fopen("mb100file.bin", "rb");
	fseek(fp, (recvbuf[1] - 1) * 28+4, SEEK_SET);
	fread(&C, sizeof(struct CATEGORY), 1, fp);
	if (C.tech_offset == 0)
		replyto_client("there are no technicians in this category\n", csock);
	else
	{
		fseek(fp, C.tech_offset, SEEK_SET);
		fread(&T, sizeof(struct TECHNICIAN), 1, fp);
		while (T.next_tech_offset != 0)
		{
			if (1024 - strlen(replybuf) > 67)
			{
				strcat(replybuf, T.tech_name);
				strcat(replybuf, "\t");
				strcat(replybuf, T.specialization);
				strcat(replybuf, "\t");
				strcat(replybuf, T.phone);
				strcat(replybuf, "\n");
				fseek(fp, T.next_tech_offset, SEEK_SET);
				fread(&T, sizeof(struct TECHNICIAN), 1, fp);
			}
			else
			{
				replyto_client(replybuf, csock);
				memset(replybuf, '\0', 1024);
			}
		}
		strcat(replybuf, T.tech_name);
		strcat(replybuf, "\t");
		strcat(replybuf, T.specialization);
		strcat(replybuf, "\t");
		strcat(replybuf, T.phone);
		strcat(replybuf, "\n");
		replyto_client(replybuf, csock);
	}
	fclose(fp);
}

int validate(char *dob,int *csock)
{
	int y, m, d;
	y = (dob[6] - '0') * 1000 + (dob[7] - '0') * 100 + (dob[8] - '0') * 10 + (dob[9] - '0');
	m = (dob[3] - '0') * 10 + (dob[4] - '0');
	d = (dob[0] - '0') * 10 + (dob[1] - '0');
	if (dob[2] != '/' || dob[5] != '/')
	{
		replyto_client("you entered date in a wrong format\n",csock);
		return -1;
	}
	if (d > 0 && m > 0 && y > 0)
	{
		if (m == 1 || m == 3 || m == 5 || m == 7 || m == 8 || m == 10 || m == 12)
		{
			if (d > 31)
			{
				replyto_client("date must not be greater than 31 for months 1,3,5,7,8,10,12\n",csock);
				return -1;
			}
		}
		else if (m == 4 || m == 6 || m == 9 || m == 11)
		{
			if (d > 30)
			{
				replyto_client("date must not be greater than 30 for months 4,6,9,11\n", csock);
				return -1;
			}
		}
		else if (m == 2)
		{
			if (y % 4 == 0)
			{
				if (d > 29)
				{
					replyto_client("date must not be greater than 29 for febrauary month in leap year\n", csock);
					return -1;
				}
			}
			else
			{
				if (d > 28)
				{
					replyto_client("date must not be greater than 28 for febrauary month in non-leap year\n", csock);
					return -1;
				}
			}
		}
		else
		{
			replyto_client("month doesnot exist:\n", csock);
			return -1;
		}
	}
	else
	{
		replyto_client("date or month or year never be zero\n", csock);
		return -1;
	}
	if (y >= 2017)
	{
		replyto_client("Appointments/Bookings cannot be made more than 5 months in advance \n",csock);
		return -1;
	}
	return 0;
}

void makeAppointment(char *recvbuf, int *csock)
{
	char username[30], serviceman[28], role[28], date[11];
	int temp, len, bufferIndex = 3,valid,found=0,tempOffset,prev,offset;
	temp = 0;
	len = recvbuf[bufferIndex++];
	while (temp < len)
		username[temp++] = recvbuf[bufferIndex++];
	username[temp] = '\0';
	temp = 0;
	len = recvbuf[bufferIndex++];
	while (temp < len)
		serviceman[temp++] = recvbuf[bufferIndex++];
	serviceman[temp] = '\0';
	temp = 0;
	len = recvbuf[bufferIndex++];
	while (temp < len)
		role[temp++] = recvbuf[bufferIndex++];
	role[temp] = '\0';
	temp = 0;
	len = recvbuf[bufferIndex++];
	while (temp < len)
		date[temp++] = recvbuf[bufferIndex++];
	date[temp] = '\0';
	valid = validate(date,csock);
	if (valid != -1)
	{
		FILE *fp;
		fp = fopen("mb100file.bin", "rb+");
		fread(&CendOffset, sizeof(int), 1, fp);
		fseek(fp, (recvbuf[1] - 1) * 28+4, SEEK_SET);
		fread(&C, sizeof(struct CATEGORY), 1, fp);
		if (C.tech_offset == 0)
			replyto_client("there are no technicians/servicemen in this category\n", csock);
		else
		{
			prev = C.tech_offset;
			offset = prev;
			while (offset)
			{
				prev = offset;
				fseek(fp, offset, SEEK_SET);
				fread(&T, sizeof(struct TECHNICIAN), 1, fp);
				if (strcmp(T.tech_name, serviceman) == 0 && strcmp(T.specialization, role) == 0)
				{
					found = 1;
					break;
				}
				offset = T.next_tech_offset;
			}
			if (found == 1)
			{
				if (T.user_offset == 0)
				{
					strcpy(U.user_name, username);
					strcpy(U.regd_date, date);
					U.next_user_offset = 0;
					fseek(fp, CendOffset, SEEK_SET);
					tempOffset = ftell(fp);
					fwrite(&U, sizeof(struct USER), 1, fp);
					CendOffset = ftell(fp);
					fseek(fp, prev, SEEK_SET);
					fread(&T,sizeof(struct TECHNICIAN), 1, fp);
					T.user_offset = tempOffset;
					fseek(fp, prev, SEEK_SET);
					fwrite(&T, sizeof(struct TECHNICIAN), 1, fp);
				}
				else
				{
					prev = T.user_offset;
					offset = prev;
					while (offset)
					{
						prev = offset;
						fseek(fp, offset, SEEK_SET);
						fread(&U, sizeof(struct USER), 1, fp);
						if (strcmp(U.regd_date, date) == 0)
						{
							replyto_client("the date is already given to another user:\n", csock);
							return;
						}
						offset = U.next_user_offset;
					}
					strcpy(U.user_name, username);
					strcpy(U.regd_date, date);
					U.next_user_offset = 0;
					fseek(fp, CendOffset, SEEK_SET);
					tempOffset = ftell(fp);
					fwrite(&U, sizeof(struct USER), 1, fp);
					CendOffset = ftell(fp);
					fseek(fp, prev, SEEK_SET);
					fread(&U, sizeof(struct USER), 1, fp);
					U.next_user_offset = tempOffset;
					fseek(fp, prev, SEEK_SET);
					fwrite(&U, sizeof(struct USER), 1, fp);
				}
			}
			else
			{
				replyto_client("there is no servicemen with taht name and specialization:\n", csock);
				return;
			}
		}
		fseek(fp, 0, SEEK_SET);
		fwrite(&CendOffset, sizeof(int), 1, fp);
		fclose(fp);
		replyto_client("your appointment registered successfully.\n", csock);
	}
}

void viewTechnicianAppoinments(char *recvbuf, int *csock)
{
	char username[30], serviceman[28], role[28], replybuf[1024] = { '\0' };
	int temp, len, bufferIndex = 3,prev,offset,found=1;
	temp = 0;
	len = recvbuf[bufferIndex++];
	while (temp < len)
		serviceman[temp++] = recvbuf[bufferIndex++];
	serviceman[temp] = '\0';
	temp = 0;
	len = recvbuf[bufferIndex++];
	while (temp < len)
		role[temp++] = recvbuf[bufferIndex++];
	role[temp] = '\0';
	FILE *fp;
	fp = fopen("mb100file.bin", "rb");
	fread(&CendOffset, sizeof(int), 1, fp);
	fseek(fp, (recvbuf[1] - 1) * 28 + 4, SEEK_SET);
	fread(&C, sizeof(struct CATEGORY), 1, fp);
	if (C.tech_offset == 0)
		replyto_client("there are no technicians/servicemen in this category\n", csock);
	else
	{
		prev = C.tech_offset;
		offset = prev;
		while (offset)
		{
			prev = offset;
			fseek(fp, offset, SEEK_SET);
			fread(&T, sizeof(struct TECHNICIAN), 1, fp);
			if (strcmp(T.tech_name, serviceman) == 0 && strcmp(T.specialization, role) == 0)
			{
				found = 1;
				break;
			}
			offset = T.next_tech_offset;
		}
		if (found == 1)
		{
			if (T.user_offset == 0)
			{
				replyto_client("currently he has no appointments:\n",csock);
				return;
			}
			else
			{
				prev = T.user_offset;
				offset = prev;
				while (offset)
				{
					prev = offset;
					fseek(fp, offset, SEEK_SET);
					fread(&U, sizeof(struct USER), 1, fp);
					if (1024 - strlen(replybuf) > 41)
					{
						strcat(replybuf, U.user_name);
						strcat(replybuf, "\t");
						strcat(replybuf, U.regd_date);
						strcat(replybuf, "\n");
					}
					else
					{
						replyto_client(replybuf, csock);
						memset(replybuf, '\0', 1024);
						strcat(replybuf, U.user_name);
						strcat(replybuf, "\t");
						strcat(replybuf, U.regd_date);
						strcat(replybuf, "\n");
					}
					offset = U.next_user_offset;
				}
				replyto_client(replybuf, csock);
			}
		}
		else
		{
			replyto_client("there is no servicemen with that name and specialization:\n", csock);
			return;
		}
	}
}

void calenderStore(char *recvbuf, int *csock)
{
	FILE *fp;
	fp = fopen("mb100file.bin", "rb+");
	if (fp == NULL)
	{
		system("cmd.exe /c \"fsutil file createnew mb100file.bin 104857600\"");
		fp = fopen("mb100file.bin", "rb+");
	}
	fread(&CendOffset, sizeof(int), 1, fp);
	if(CendOffset==0)
		initialize(fp);
	fclose(fp);
	switch (recvbuf[2])
		{
		case 1:
			addTechnician(recvbuf, csock);
			break;
		case 2:
			viewTechnicianDetails(recvbuf, csock);
			break;
		case 3:
			makeAppointment(recvbuf,csock);
			break;
		case 4:
			viewTechnicianAppoinments(recvbuf,csock);
			break;
		}
}

void userDetails(char *recvbuf, int *csock)
{
	FILE *fp;
	fp = fopen("fileSystem.bin", "rb");
	int i, ch, id;
	char replybuf[1024];
	memset(replybuf, '\0', 1024);
	fseek(fp, usermetaOffset, SEEK_SET);
	fread(&UM, sizeof(struct user_metadata), 1, fp);
	if (UM.user_count == 0)
	{
		replyto_client("there are no users to display\n",csock);
	}
	else
	{
		for (i = 0; i < UM.user_count; i++)
		{
			fseek(fp, UM.user[i], SEEK_SET);
			fread(&u, sizeof(struct user), 1, fp);
			strcat(replybuf, u.username);
			strcat(replybuf, "\n");
		}
		replyto_client(replybuf, csock);
	}
	fclose(fp);
}

void addUser(char *recvbuf, int *csock)
{
	FILE *fp;
	fp = fopen("fileSystem.bin", "rb+");
	char replybuf[1024], username[32];
	int temp, len, bufferIndex = 2,i,brek=0;
	temp = 0;
	len = recvbuf[bufferIndex++];
	while (temp < len)
		username[temp++] = recvbuf[bufferIndex++];
	username[temp] = '\0';
	u.user_category_count = 0;
	fseek(fp, 65536, SEEK_SET);
	fread(&UM, sizeof(struct user_metadata), 1, fp);
	if (UM.user_count >= 31)
	{
		replyto_client("maximum users count reached\n",csock);
	}
	else
	{
		temp = 0;
		while (temp < UM.user_count)
		{
			fseek(fp, UM.user[temp++], SEEK_SET);
			fread(&u, sizeof(struct user), 1, fp);
			if (strcmp(u.username, username) == 0)
			{
				replyto_client("username already exists:\n", csock);
				brek = 1;
				break;
			}
		}
		if (brek == 0)
		{
			fseek(fp, 0, SEEK_SET);
			fread(&b, sizeof(struct bitVector), 1, fp);
			for (i = 517; i < 65536; i++)
			{
				if (b.bit[i] == 0)
					break;
			}
			fseek(fp, i * 128, SEEK_SET);
			UM.user[UM.user_count++] = ftell(fp);
			memset(&u, 0, sizeof(user));
			strcpy(u.username, username);
			u.user_category_count = 0;
			fwrite(&u, sizeof(struct user), 1, fp);
			fseek(fp, usermetaOffset, SEEK_SET);
			fwrite(&UM, sizeof(struct user_metadata), 1, fp);
			b.bit[i] = '1';
			fseek(fp, 0, SEEK_SET);
			fwrite(&b, sizeof(struct bitVector), 1, fp);
			replyto_client("username is successfully added\n", csock);
		}
	}
	fclose(fp);
}

int getUser(char *username,FILE *fp)
{
	int temp, id ,found=0;
	temp = 0;
	fseek(fp, usermetaOffset, SEEK_SET);
	fread(&UM, sizeof(struct user_metadata), 1, fp);
	while (temp<UM.user_count)
	{
		fseek(fp, UM.user[temp++], SEEK_SET);
		fread(&u, sizeof(struct user), 1, fp);
		if (strcmp(u.username, username) == 0)
		{
			id = temp - 1;
			found = 1;
			break;
		}
	}
	if (found == 0)
		return -1;
	return id;
}

void addCategory(char *recvbuf, int *csock)
{
	FILE *fp;
	fp = fopen("fileSystem.bin", "rb+");
	char replybuf[1024], username[32],category[36];
	int temp, len, bufferIndex = 2, i, brek = 0,x,id;
	memset(replybuf, '\0', 1024);
	temp = 0;
	len = recvbuf[bufferIndex++];
	while (temp < len)
		username[temp++] = recvbuf[bufferIndex++];
	username[temp] = '\0';
	temp = 0;
	len = recvbuf[bufferIndex++];
	while (temp < len)
		category[temp++] = recvbuf[bufferIndex++];
	category[temp] = '\0';
	fseek(fp, categorymetaOffset, SEEK_SET);
	fread(&cm, sizeof(struct category_metadata), 1, fp);
	memset(&u, 0, sizeof(struct user));
	if (cm.category_count >= 127)
	{
		replyto_client("categories reached their maximum limits\n",csock);
	}
	else
	{
		id = getUser(username, fp);
		if (id == -1)
		{
			replyto_client("user is not registerd please add user:\n",csock);
		}
		else
		{
			temp = 0;
			fseek(fp, usermetaOffset, SEEK_SET);
			fread(&UM, sizeof(struct user_metadata), 1, fp);
			fseek(fp, UM.user[id], SEEK_SET);
			fread(&u, sizeof(struct user), 1, fp);
			while (temp < u.user_category_count)
			{
				fseek(fp, u.user_categories[temp++], SEEK_SET);
				fread(&c, sizeof(struct category), 1, fp);
				if (strcmp(c.category_name, category) == 0)
				{
					replyto_client("category already exist:\n", csock);
					brek = 1;
					break;
				}
			}
			if (brek == 0)
			{
				fseek(fp, 0, SEEK_SET);
				fread(&b, sizeof(struct bitVector), 1, fp);
				for (i = 517; i < 65536; i++)
				{
					if (b.bit[i] == 0 && b.bit[i + 1] == 0 && b.bit[i + 2] == 0 && b.bit[i + 3] == 0)
						break;
				}
				fseek(fp, i * 128, SEEK_SET);
				x = cm.category[cm.category_count++] = ftell(fp);
				fseek(fp, usermetaOffset, SEEK_SET);
				fread(&UM, sizeof(struct user_metadata), 1, fp);
				fseek(fp, UM.user[id], SEEK_SET);
				fread(&u, sizeof(struct user), 1, fp);
				if (u.user_category_count >= 5)
				{
					replyto_client("user categories reached maximum limit:\n", csock);
				}
				else
				{
					u.user_categories[u.user_category_count++] = x;
					memset(&c, 0, sizeof(struct category));
					strcpy(c.category_name, category);
					c.cat_message_count = 0;
					c.next_page_messages = 0;
					fseek(fp, UM.user[id], SEEK_SET);
					fwrite(&u, sizeof(struct user), 1, fp);
					fseek(fp, categorymetaOffset, SEEK_SET);
					fwrite(&cm, sizeof(struct category_metadata), 1, fp);
					fseek(fp, x, SEEK_SET);
					fwrite(&c, sizeof(struct category), 1, fp);
					b.bit[i] = b.bit[i + 1] = '1';
					b.bit[i + 2] = b.bit[i + 3] = '1';
					fseek(fp, 0, SEEK_SET);
					fwrite(&b, sizeof(struct bitVector), 1, fp);
					replyto_client("category adde successfully:\n", csock);
				}
			}
		}
	}
	fclose(fp);
}

void viewUserCategories(char *recvbuf, int *csock)
{
	FILE *fp;
	fp = fopen("fileSystem.bin", "rb");
	char replybuf[1024], username[32];
	int temp, len, bufferIndex = 2, id,i;
	memset(replybuf, '\0', 1024);
	temp = 0;
	len = recvbuf[bufferIndex++];
	while (temp < len)
		username[temp++] = recvbuf[bufferIndex++];
	username[temp] = '\0';
	fseek(fp, usermetaOffset, SEEK_SET);
	fread(&UM, sizeof(struct user_metadata), 1, fp);
	id = getUser(username, fp);
	if (id == -1)
	{
		replyto_client("either user is not added or username is wrong:\n",csock);
	}
	else
	{
		fseek(fp, UM.user[id], SEEK_SET);
		fread(&u, sizeof(struct user), 1, fp);
		if (u.user_category_count == 0)
		{
			replyto_client("there are no categories to display\n", csock);
		}
		else
		{
			for (i = 0; i < u.user_category_count; i++)
			{
				fseek(fp, u.user_categories[i], SEEK_SET);
				fread(&c, sizeof(struct category), 1, fp);
				strcat(replybuf, c.category_name);
				strcat(replybuf, "\n");
			}
			replyto_client(replybuf, csock);
		}
	}
	fclose(fp);
}

int getCategory(char *category, FILE *fp,int id)
{
	int temp,found=0;
	fseek(fp, usermetaOffset, SEEK_SET);
	fread(&UM, sizeof(struct user_metadata), 1, fp);
	fseek(fp, UM.user[id], SEEK_SET);
	fread(&u, sizeof(struct user), 1, fp);
	temp = 0;
	while (temp < u.user_category_count)
	{
		fseek(fp, u.user_categories[temp++], SEEK_SET);
		fread(&c, sizeof(struct category), 1, fp);
		if (strcmp(c.category_name, category) == 0)
		{
			id = u.user_categories[temp - 1];
			found = 1;
			break;
		}
	}
	if (found == 0)
		return -1;
	return id;
}

void addMessage(char *recvbuf, int *csock)
{
	FILE *fp;
	fp = fopen("fileSystem.bin", "rb+");
	char replybuf[1024], username[32], category[36],mesage[128];
	int temp, len, bufferIndex = 2, i, brek = 0, x, id;
	memset(replybuf, '\0', 1024);
	temp = 0;
	len = recvbuf[bufferIndex++];
	while (temp < len)
		username[temp++] = recvbuf[bufferIndex++];
	username[temp] = '\0';
	temp = 0;
	len = recvbuf[bufferIndex++];
	while (temp < len)
		category[temp++] = recvbuf[bufferIndex++];
	category[temp] = '\0';
	temp = 0;
	len = recvbuf[bufferIndex++];
	while (temp < len)
		mesage[temp++] = recvbuf[bufferIndex++];
	mesage[temp] = '\0';
	fseek(fp, usermetaOffset, SEEK_SET);
	fread(&UM, sizeof(struct user_metadata), 1, fp);
	id = getUser(username, fp);
	if (id == -1)
	{
		replyto_client("user is not registerd add user first:\n", csock);
	}
	else
	{
		id = getCategory(category, fp, id);
		if (id == -1)
		{
			replyto_client("you entered a wrong category:\n", csock);
		}
		else
		{
			fseek(fp, id, SEEK_SET);
			fread(&c, sizeof(struct category), 1, fp);
			if (c.cat_message_count >= 117)
			{
				replyto_client("messages overloaded\n", csock);
			}
			else
			{
				fseek(fp, 0, SEEK_SET);
				fread(&b, sizeof(struct bitVector), 1, fp);
				for (i = 517; i < 65536; i++)
				{
					if (b.bit[i] == 0 && b.bit[i + 1] == 0 && b.bit[i + 2] == 0 && b.bit[i + 3] == 0)
						break;
				}
				fseek(fp, i * 128, SEEK_SET);
				for (temp = 0; temp < 117; temp++)
				{
					if (c.cat_messages[temp] == 0)
						break;
				}
				c.cat_messages[temp] = ftell(fp);
				c.cat_message_count++;
				memset(&m, 0, sizeof(struct message));
				strcpy(m.mesage, mesage);
				m.next_page_replies = 0;
				m.reply_count = 0;
				fwrite(&m, sizeof(struct message), 1, fp);
				b.bit[i] = b.bit[i + 1] = '1';
				b.bit[i + 2] = b.bit[i + 3] = '1';
				fseek(fp, id, SEEK_SET);
				fwrite(&c, sizeof(struct category), 1, fp);
				fseek(fp, 0, SEEK_SET);
				fwrite(&b, sizeof(struct bitVector), 1, fp);
				replyto_client("message added successfully:\n", csock);
			}
		}
	}
	fclose(fp);
}

void viewMessage(char *recvbuf, int *csock)
{
	FILE *fp;
	fp = fopen("fileSystem.bin", "rb");
	char replybuf[1024], username[32], category[36];
	int temp, len, bufferIndex = 2, i, brek = 0, x, id;
	memset(replybuf, '\0', 1024);
	temp = 0;
	len = recvbuf[bufferIndex++];
	while (temp < len)
		username[temp++] = recvbuf[bufferIndex++];
	username[temp] = '\0';
	temp = 0;
	len = recvbuf[bufferIndex++];
	while (temp < len)
		category[temp++] = recvbuf[bufferIndex++];
	category[temp] = '\0';
	fseek(fp, usermetaOffset, SEEK_SET);
	fread(&UM, sizeof(struct user_metadata), 1, fp);
	id = getUser(username, fp);
	if (id == -1)
	{
		replyto_client("user is not registerd add user first:\n", csock);
	}
	else
	{
		id = getCategory(category, fp, id);
		if (id == -1)
		{
			replyto_client("you entered wrong category:\n", csock);
		}
		else
		{
			fseek(fp, id, SEEK_SET);
			fread(&c, sizeof(struct category), 1, fp);
			if (c.cat_message_count == 0)
			{
				replyto_client("there are no messages to show\n", csock);
			}
			else
			{
				temp = 0;
				for (int i = 0; temp < c.cat_message_count&&i<117; i++)
				{
					if (c.cat_messages[i] == 0)
						continue;
					fseek(fp, c.cat_messages[i], SEEK_SET);
					fread(&m, sizeof(struct message), 1, fp);
					if (1024 - strlen(replybuf) > 128)
					{
						strcat(replybuf, m.mesage);
						strcat(replybuf, "\n");
					}
					else
					{
						replyto_client(replybuf, csock);
						memset(replybuf, '\0', 1024);
						i--;
					}
				}
				replyto_client(replybuf, csock);
			}
		}
	}
	fclose(fp);
}

void deleteMessage(char *recvbuf, int *csock)
{
	FILE *fp;
	fp = fopen("fileSystem.bin", "rb+");
	char replybuf[1024], username[32], category[36], mesage[128];
	int temp, len, bufferIndex = 2, i, brek = 0, x, id;
	memset(replybuf, '\0', 1024);
	temp = 0;
	len = recvbuf[bufferIndex++];
	while (temp < len)
		username[temp++] = recvbuf[bufferIndex++];
	username[temp] = '\0';
	temp = 0;
	len = recvbuf[bufferIndex++];
	while (temp < len)
		category[temp++] = recvbuf[bufferIndex++];
	category[temp] = '\0';
	temp = 0;
	len = recvbuf[bufferIndex++];
	while (temp < len)
		mesage[temp++] = recvbuf[bufferIndex++];
	mesage[temp] = '\0';
	fseek(fp, usermetaOffset, SEEK_SET);
	fread(&UM, sizeof(struct user_metadata), 1, fp);
	id = getUser(username, fp);
	if (id == -1)
	{
		replyto_client("user is not registerd add user first:\n", csock);
	}
	else
	{
		id = getCategory(category, fp, id);
		if (id == -1)
		{
			replyto_client("you enterd wrong category:\n", csock);
		}
		else
		{
			fseek(fp, id, SEEK_SET);
			fread(&c, sizeof(struct category), 1, fp);
			temp = 0;
			while (temp < 117)
			{
				if (c.cat_messages[temp] == 0)
				{
					temp++;
					continue;
				}
				fseek(fp, c.cat_messages[temp++], SEEK_SET);
				fread(&m, sizeof(struct message), 11, fp);
				if (strcmp(m.mesage, mesage) == 0)
				{
					brek = 1;
					break;
				}
			}
			if (brek == 0)
			{
				replyto_client("message not found:\n", csock);
			}
			else
			{
				fseek(fp, 0, SEEK_SET);
				fread(&b, sizeof(struct bitVector), 1, fp);
				x = c.cat_messages[temp - 1] / 128;
				b.bit[x] = 0;
				b.bit[x + 1] = 0;
				b.bit[x + 2] = 0;
				b.bit[x + 3] = 0;
				fseek(fp, 0, SEEK_SET);
				fwrite(&b, sizeof(struct bitVector), 1, fp);
				fseek(fp, id, SEEK_SET);
				fread(&c, sizeof(category), 1, fp);
				c.cat_message_count--;
				c.cat_messages[temp - 1] = 0;
				fseek(fp, id, SEEK_SET);
				fwrite(&c, sizeof(struct category), 1, fp);
				replyto_client("meassge deleted successfully:\n", csock);
			}
		}
	}
	fclose(fp);
}

int getMessage(char *mesage, FILE *fp,int id)
{
	int temp,brek=0;
	fseek(fp, id, SEEK_SET);
	fread(&c, sizeof(struct category), 1, fp);
	temp = 0;
	while (temp < 117)
	{
		if (c.cat_messages[temp] == 0)
		{
			temp++;
			continue;
		}
		fseek(fp, c.cat_messages[temp++], SEEK_SET);
		fread(&m, sizeof(struct message), 11, fp);
		if (strcmp(m.mesage, mesage) == 0)
		{
			id = c.cat_messages[temp - 1];
			brek = 1;
			break;
		}
	}
	if (brek == 0)
		return -1;
	return id;
}

void addReply(char *recvbuf, int *csock)
{
	FILE *fp;
	fp = fopen("fileSystem.bin", "rb+");
	char replybuf[1024], username[32], category[36], mesage[128],rply[128];
	int temp, len, bufferIndex = 2, i, brek = 0, x, id;
	memset(replybuf, '\0', 1024);
	temp = 0;
	len = recvbuf[bufferIndex++];
	while (temp < len)
		username[temp++] = recvbuf[bufferIndex++];
	username[temp] = '\0';
	temp = 0;
	len = recvbuf[bufferIndex++];
	while (temp < len)
		category[temp++] = recvbuf[bufferIndex++];
	category[temp] = '\0';
	temp = 0;
	len = recvbuf[bufferIndex++];
	while (temp < len)
		mesage[temp++] = recvbuf[bufferIndex++];
	mesage[temp] = '\0';
	temp = 0;
	len = recvbuf[bufferIndex++];
	while (temp < len)
		rply[temp++] = recvbuf[bufferIndex++];
	rply[temp] = '\0';
	id = getUser(username, fp);
	if (id == -1)
	{
		replyto_client("user is not registerd add user first:\n", csock);
	}
	else
	{
		id = getCategory(category, fp, id);
		if (id == -1)
		{
			replyto_client("you entered wrong category:\n", csock);
		}
		else
		{
			id=getMessage(mesage, fp, id);
			if (id == -1)
			{
				replyto_client("you entered wrong message:\n", csock);
			}
			else
			{
				fseek(fp, id, SEEK_SET);
				fread(&m, sizeof(struct message), 1, fp);
				if (m.reply_count >= 94)
				{
					replyto_client("replies overloaded\n",csock);
					
				}
				else
				{
					fseek(fp, 0, SEEK_SET);
					fread(&b, sizeof(struct bitVector), 1, fp);
					for (i = 517; i < 65536; i++)
					{
						if (b.bit[i] == 0)
							break;
					}
					fseek(fp, i * 128, SEEK_SET);
					for (temp = 0; temp <94; temp++)
					{
						if (m.mes_reply[temp] == 0)
							break;
					}
					m.mes_reply[temp] = ftell(fp);
					m.reply_count++;
					strcpy(r.rply, rply);
					fwrite(&r, sizeof(struct reply), 1, fp);
					b.bit[i] = '1';
					fseek(fp, id, SEEK_SET);
					fwrite(&m, sizeof(struct message), 1, fp);
					fseek(fp, 0, SEEK_SET);
					fwrite(&b, sizeof(struct bitVector), 1, fp);
					replyto_client("reply added successfully:\n", csock);
				}
			}
		}
	}
	fclose(fp);
}

void viewReply(char *recvbuf, int *csock)
{
	FILE *fp;
	fp = fopen("fileSystem.bin", "rb+");
	char replybuf[1024], username[32], category[36], mesage[128];
	int temp, len, bufferIndex = 2, i, brek = 0, x, id;
	memset(replybuf, '\0', 1024);
	temp = 0;
	len = recvbuf[bufferIndex++];
	while (temp < len)
		username[temp++] = recvbuf[bufferIndex++];
	username[temp] = '\0';
	temp = 0;
	len = recvbuf[bufferIndex++];
	while (temp < len)
		category[temp++] = recvbuf[bufferIndex++];
	category[temp] = '\0';
	temp = 0;
	len = recvbuf[bufferIndex++];
	while (temp < len)
		mesage[temp++] = recvbuf[bufferIndex++];
	mesage[temp] = '\0';
	id = getUser(username, fp);
	if (id == -1)
	{
		replyto_client("user is not registerd add user first:\n", csock);
	}
	else
	{
		id = getCategory(category, fp, id);
		if (id == -1)
		{
			replyto_client("you entered wrong category:\n", csock);
		}
		else
		{
			id = getMessage(mesage, fp, id);
			if (id == -1)
			{
				replyto_client("you entered wrong message:\n", csock);
			}
			else
			{
				fseek(fp, id, SEEK_SET);
				fread(&m, sizeof(struct message), 1, fp);
				if (m.reply_count ==0)
				{
					replyto_client("message has no replies:\n", csock);
				}
				else
				{
					temp = 0;
					for (int i = 0; temp < m.reply_count&&i<94; i++)
					{
						if (m.mes_reply[i] == 0)
							continue;
						fseek(fp, m.mes_reply[i], SEEK_SET);
						fread(&r, sizeof(struct reply), 1, fp);
						if (1024 - strlen(replybuf) > 128)
						{
							strcat(replybuf, r.rply);
							strcat(replybuf, "\n");
						}
						else
						{
							replyto_client(replybuf, csock);
							memset(replybuf, '\0', 1024);
							strcat(replybuf, r.rply);
							strcat(replybuf, "\n");
						}
					}
					replyto_client(replybuf, csock);
				}
			}
		}
	}
	fclose(fp);
}

void deleteReply(char *recvbuf, int *csock)
{
	FILE *fp;
	fp = fopen("fileSystem.bin", "rb+");
	char replybuf[1024], username[32], category[36], mesage[128], rply[128];
	int temp, len, bufferIndex = 2, i, brek = 0, x, id;
	memset(replybuf, '\0', 1024);
	temp = 0;
	len = recvbuf[bufferIndex++];
	while (temp < len)
		username[temp++] = recvbuf[bufferIndex++];
	username[temp] = '\0';
	temp = 0;
	len = recvbuf[bufferIndex++];
	while (temp < len)
		category[temp++] = recvbuf[bufferIndex++];
	category[temp] = '\0';
	temp = 0;
	len = recvbuf[bufferIndex++];
	while (temp < len)
		mesage[temp++] = recvbuf[bufferIndex++];
	mesage[temp] = '\0';
	temp = 0;
	len = recvbuf[bufferIndex++];
	while (temp < len)
		rply[temp++] = recvbuf[bufferIndex++];
	rply[temp] = '\0';
	id = getUser(username, fp);
	if (id == -1)
	{
		replyto_client("user is not registerd add user first:\n", csock);
	}
	else
	{
		id = getCategory(category, fp, id);
		if (id == -1)
		{
			replyto_client("you entered wrong category:\n", csock);
		}
		else
		{
			id = getMessage(mesage, fp, id);
			if (id == -1)
			{
				replyto_client("you entered wrong message:\n", csock);
			}
			else
			{
				fseek(fp, id, SEEK_SET);
				fread(&m, sizeof(struct message), 1, fp);
				if (m.reply_count == 0)
				{
					replyto_client("there are no replies to delete:\n",csock);
				}
				else
				{
					temp = 0;
					while (temp < 94)
					{
						if (m.mes_reply[temp] == 0)
						{
							temp++;
							continue;
						}
						fseek(fp, m.mes_reply[temp++], SEEK_SET);
						fread(&r, sizeof(struct reply), 1, fp);
						if (strcmp(r.rply, rply) == 0)
						{
							brek = 1;
							break;
						}
					}
					if (brek == 0)
					{
						replyto_client("ypu entered a wrong reply:\n", csock);
					}
					else
					{
						x = m.mes_reply[temp - 1] / 128;
						fseek(fp, 0, SEEK_SET);
						fread(&b, sizeof(struct bitVector), 1, fp);
						b.bit[x] = 0;
						fseek(fp, 0, SEEK_SET);
						fwrite(&b, sizeof(struct bitVector), 1, fp);
						m.reply_count--;
						m.mes_reply[temp - 1] = 0;
						fseek(fp, id, SEEK_SET);
						fwrite(&m, sizeof(struct message), 1, fp);
						replyto_client("reply deleted successfully:\n", csock);
					}
				}
			}
		}
	}
	fclose(fp);
}

void messageStore(char *recvbuf, int *csock)
{
	int x;
	FILE *fp;
	fp = fopen("fileSystem.bin", "rb+");
	if (fp == NULL)
	{
		system("cmd.exe /c \"fsutil file createnew fileSystem.bin 8388608 \"");
		fp = fopen("fileSystem.bin", "rb+");
	}
	fseek(fp, 0, SEEK_SET);
	int c = fgetc(fp);
	if (c == 0)
	{
		for (int i = 0; i < 517; i++)
			b.bit[i] = '1';
		fseek(fp, 0, SEEK_SET);
		fwrite(&b, sizeof(struct bitVector), 1, fp);
		x = ftell(fp);
		UM.user_count = 0;
		fwrite((&UM), sizeof(struct user_metadata), 1, fp);
		x = ftell(fp);
		cm.category_count = 0;
		fwrite(&cm, sizeof(struct category_metadata), 1, fp);
		x = ftell(fp);
	}
	fclose(fp);
	switch (recvbuf[1])
	{
		case 1:
			userDetails(recvbuf,csock);
			break;
		case 2:
			addUser(recvbuf,csock);
			break;
		case 3:
			addCategory(recvbuf, csock);
			break;
		case 4:
			viewUserCategories(recvbuf, csock);
			break;
		case 5:
			addMessage(recvbuf, csock);
			break;
		case 6:
			viewMessage(recvbuf, csock);
			break;
		case 7:
			deleteMessage(recvbuf, csock);
			break;
		case 8:
			addReply(recvbuf, csock);
			break;
		case 9:
			viewReply(recvbuf, csock);
			break;
		case 10:
			deleteReply(recvbuf, csock);
			break;
	}
}

DWORD WINAPI SocketHandler(void* lp){
    int *csock = (int*)lp;

	char recvbuf[1024];
	int recvbuf_len = 1024;
	int recv_byte_cnt;
	do
	{
		memset(recvbuf, 0, recvbuf_len);
		if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR) {
			fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
			free(csock);
			return 0;
		}
		switch (recvbuf[0])
		{
		case '1':
			blobStore(recvbuf, csock);
			break;
		case '2':
			messageStore(recvbuf,csock);
			break;
		case '3':
				calenderStore(recvbuf,csock);
				break;
		}
	} while (1);

	//printf("Received bytes %d\nReceived string \"%s\"\n", recv_byte_cnt, recvbuf);
	//process_input(recvbuf, recv_byte_cnt, csock);

    return 0;
}