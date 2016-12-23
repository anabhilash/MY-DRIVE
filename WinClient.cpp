#include <winsock2.h>
#include "StdAfx.h"
#include <windows.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <conio.h>

int getsocket()
{
	int hsock;
	int * p_int ;
	hsock = socket(AF_INET, SOCK_STREAM, 0);
	if(hsock == -1){
		printf("Error initializing socket %d\n",WSAGetLastError());
		return -1;
	}
	
	p_int = (int*)malloc(sizeof(int));
	*p_int = 1;
	if( (setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1 )||
		(setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1 ) ){
		printf("Error setting options %d\n", WSAGetLastError());
		free(p_int);
		return -1;
	}
	free(p_int);

	return hsock;
}

void blobMenu()
{
	printf("1- add file\n");
	printf("2- view list of files\n");
	printf("3- download a paricular file\n");
	printf("4- delete a file\n");
	printf("0- go back\n");
}

void blobAddFile(char *username, int hsock)
{
	char buffer[1024], fileName[50],sizebuffer[15];
	int buffer_len = 1024, bytecount, bufferIndex = 0,size;
	memset(buffer, '\0', buffer_len);
	buffer[bufferIndex++] = '1';
	buffer[bufferIndex++] = '1';
	buffer[bufferIndex++] = strlen(username);
	for (int i = 0; username[i]; i++)
	{
		buffer[bufferIndex++] = username[i];
	}
	printf("enter the file name to add along with its extension\n");
	while (getchar() != '\n');
	scanf("%[^\n]s", fileName);
	FILE *fp;
	fp = fopen(fileName, "rb");
	if (fp == NULL)
	{
		printf("file doesnot exist\n");
		return;
	}
	buffer[bufferIndex++] = strlen(fileName);
	for (int i = 0; fileName[i]; i++)
	{
		buffer[bufferIndex++] = fileName[i];
	}
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	itoa(size, sizebuffer, 10);
	buffer[bufferIndex++] = strlen(sizebuffer);
	for (int i = 0; sizebuffer[i]; i++)
	{
		buffer[bufferIndex++] = sizebuffer[i];
	}
	buffer[bufferIndex] = '\0';
	if ((bytecount = send(hsock, buffer, strlen(buffer), 0)) == SOCKET_ERROR) {
		fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
		return;
	}
	printf("Sent bytes %d\n", bytecount);
	memset(buffer, '\0', 1024);
	int count = 0;
	fseek(fp, 0, SEEK_SET);
	while (count<(size/1024))
	{
		fread(buffer, sizeof(char), buffer_len, fp);
		if ((bytecount = send(hsock, buffer, 1024, 0)) == SOCKET_ERROR) {
			fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
			return;
		}
		count++;
		memset(buffer, '\0', 1024);
	}
	if (size % 1024 != 0)
	{
		fread(buffer, sizeof(char), size%1024, fp);
		if ((bytecount = send(hsock, buffer, 1024, 0)) == SOCKET_ERROR) {
			fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
			return;
		}
		count++;
		memset(buffer, '\0', 1024);
	}
	printf("file upload completed\n");
	fclose(fp);
}

void blobViewFile(char *username, int hsock)
{
	char buffer[1024];
	int buffer_len = 1024, bytecount, bufferIndex = 0,flag=0;
	memset(buffer, '\0', buffer_len);
	buffer[bufferIndex++] = '1';
	buffer[bufferIndex++] = '2';
	buffer[bufferIndex++] = strlen(username);
	for (int i = 0; username[i]; i++)
	{
		buffer[bufferIndex++] = username[i];
	}
	buffer[bufferIndex] = '\0';
	if ((bytecount = send(hsock, buffer, 1024, 0)) == SOCKET_ERROR) {
		fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
		return;
	}
	while (1)
	{
		if ((bytecount = recv(hsock, buffer, buffer_len, 0)) == SOCKET_ERROR) {
			fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
			return;
		}
		if (1024 - strlen(buffer) <= 64)
		{
			if(flag==0)
				printf("files added by %s are:\n", username);
			printf("\n%s", bytecount, buffer);
			flag = 1;
		}
		else if (strcmp(buffer, "no users exists with that name in the database\n") == 0)
		{
			flag = 1;
			break;
		}
		else
			break;
	}
	if (flag == 0)
		printf("files added by %s are:\n%s", username, buffer);
	else 
		printf("\n%s\n", buffer);
}

void blobDownloadFile(char *username, int hsock)
{
	char filename[64];
	blobViewFile(username,hsock);
	printf("enter the file you want to download:\n");
	while (getchar() != '\n');
	scanf("%[^\n]s", filename);
	char buffer[1024];
	int buffer_len = 1024, bytecount, bufferIndex = 0, flag = 0,size,temp;
	memset(buffer, '\0', buffer_len);
	buffer[bufferIndex++] = '1';
	buffer[bufferIndex++] = '3';
	buffer[bufferIndex++] = strlen(username);
	for (int i = 0; username[i]; i++)
	{
		buffer[bufferIndex++] = username[i];
	}
	buffer[bufferIndex] = '\0';
	buffer[bufferIndex++] = strlen(filename);
	for (int i = 0; filename[i]; i++)
	{
		buffer[bufferIndex++] = filename[i];
	}
	buffer[bufferIndex] = '\0';
	if ((bytecount = send(hsock, buffer, 1024, 0)) == SOCKET_ERROR) {
		fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
		return;
	}
	memset(buffer, '\0', 1024);
	if ((bytecount = recv(hsock, buffer,1024,0)) == SOCKET_ERROR) {
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		return;
	}
	if (strcmp(buffer, "whether userName or file name wrong\n") == 0)
	{
		printf("%s\n", buffer);
		return;
	}
	else
		size=atoi(buffer);
	FILE *fp;
	fp = fopen(filename, "wb");
	temp= 0;
	memset(buffer, '\0', 1024);
	while (temp<(size / 1024))
	{
		if ((bytecount = recv(hsock, buffer, buffer_len, 0)) == SOCKET_ERROR) {
			fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
			return;
		}
		fwrite(buffer, sizeof(char), 1024, fp);
		temp++;
		memset(buffer, '\0', 1024);
	}
	if (size % 1024 != 0)
	{
		if ((bytecount = recv(hsock, buffer, buffer_len, 0)) == SOCKET_ERROR) {
			fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
			return;
		}
		fwrite(buffer, sizeof(char), (size % 1024), fp);
		temp++;
	}
	printf("your download is completed\n");
	fclose(fp);
}

void blobDeleteFile(char *username, int hsock)
{
	char filename[64];
	blobViewFile(username, hsock);
	printf("enter the file you want to download:\n");
	while (getchar() != '\n');
	scanf("%[^\n]s", filename);
	char buffer[1024];
	int buffer_len = 1024, bytecount, bufferIndex = 0, flag = 0, size, temp;
	memset(buffer, '\0', buffer_len);
	buffer[bufferIndex++] = '1';
	buffer[bufferIndex++] = '4';
	buffer[bufferIndex++] = strlen(username);
	for (int i = 0; username[i]; i++)
	{
		buffer[bufferIndex++] = username[i];
	}
	buffer[bufferIndex] = '\0';
	buffer[bufferIndex++] = strlen(filename);
	for (int i = 0; filename[i]; i++)
	{
		buffer[bufferIndex++] = filename[i];
	}
	buffer[bufferIndex] = '\0';
	if ((bytecount = send(hsock, buffer, 1024, 0)) == SOCKET_ERROR) {
		fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
		return;
	}
	if ((bytecount = recv(hsock, buffer, buffer_len, 0)) == SOCKET_ERROR) {
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		return;
	}
	printf("%s\n", buffer);
}

void blobStore(char *username,int hsock)
{
	int ch;
	do
	{
		blobMenu();
		printf("enter your choice:\n");
		scanf("%d", &ch);
		switch (ch)
		{
		case 1:
			blobAddFile(username,hsock);
			break;
		case 2:
			blobViewFile(username,hsock);
			break;
		case 3:
			blobDownloadFile(username,hsock);
			break;
		case 4:
			blobDeleteFile(username,hsock);
			break;
		case 0:
			return;
		}
	} while (ch != 0);
}

void calenderMenu1()
{
	printf("1-add technician\n");
	printf("2-view technicians details\n");
	printf("3-make appoinment\n");
	printf("4-technicians and his appoinments\n");
	printf("0-go back\n");
}

void addTechnician(int category, int hsock)
{
	char buffer[1024],serviceman[28],role[28],phone[11];
	int buffer_len = 1024, bytecount, bufferIndex = 0, flag = 0;
	printf("enter the technician name\n");
	while (getchar() != '\n');
	scanf("%[^\n]s", serviceman);
	memset(buffer, '\0', buffer_len);
	buffer[bufferIndex++] = '3';
	buffer[bufferIndex++] = category;
	buffer[bufferIndex++] = 1;
	buffer[bufferIndex++] = strlen(serviceman);
	for (int i = 0; serviceman[i]; i++)
	{
		buffer[bufferIndex++] = serviceman[i];
	}
	printf("enter his specialization in this category\n");
	while (getchar() != '\n');
	scanf("%[^\n]s", role);
	buffer[bufferIndex++] = strlen(role);
	for (int i = 0; role[i]; i++)
	{
		buffer[bufferIndex++] = role[i];
	}
	printf("enter the phone number:\n");
	while (getchar() != '\n');
	scanf("%s", phone);
	buffer[bufferIndex++] = strlen(phone);
	for (int i = 0; phone[i]; i++)
	{
		buffer[bufferIndex++] = phone[i];
	}
	buffer[bufferIndex] = '\0';
	if ((bytecount = send(hsock, buffer, 1024, 0)) == SOCKET_ERROR) {
		fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
		return;
	}
	if ((bytecount = recv(hsock, buffer, buffer_len, 0)) == SOCKET_ERROR) {
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		return;
	}
	printf("%s\n", buffer);
}

void viewTechnicianDetails(int category, int hsock)
{
	char buffer[1024];
	int buffer_len = 1024, bytecount, bufferIndex = 0;
	memset(buffer, '\0', buffer_len);
	buffer[bufferIndex++] = '3';
	buffer[bufferIndex++] = category;
	buffer[bufferIndex++] = 2;
	if ((bytecount = send(hsock, buffer, 1024, 0)) == SOCKET_ERROR) {
		fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
		return;
	}
	printf("details of servicemen are:\n");
	while (1)
	{
		if ((bytecount = recv(hsock, buffer, buffer_len, 0)) == SOCKET_ERROR) {
			fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
			return;
		}
		if (strcmp(buffer, "there are no technicians in this category\n") == 0)
			break;
		else if (1024 - strlen(buffer) > 67)
			break;
		else
			printf("%s\n", buffer);
	}
	printf("%s\n", buffer);
}

void makeAppointment(char *username, int category, int hsock)
{
	char buffer[1024], serviceman[28], role[28], date[11];
	int buffer_len = 1024, bytecount, bufferIndex = 0, flag = 0;
	memset(buffer, '\0', buffer_len);
	buffer[bufferIndex++] = '3';
	buffer[bufferIndex++] = category;
	buffer[bufferIndex++] = 3;
	buffer[bufferIndex++] = strlen(username);
	for (int i = 0; username[i]; i++)
	{
		buffer[bufferIndex++] = username[i];
	}
	viewTechnicianDetails(category, hsock);
	printf("enter the servicemen name you want to make appointment:\n");
	while (getchar() != '\n');
	scanf("%[^\n]s", serviceman);
	printf("enter his specialization in his category:\n");
	while (getchar() != '\n');
	scanf("%[^\n]s", role);
	printf("enter date to make appointment in the fomat of \(dd/mm/yyyy\):\n");
	while (getchar() != '\n');
	scanf("%[^\n]s", date);
	buffer[bufferIndex++] = strlen(serviceman);
	for (int i = 0; serviceman[i]; i++)
	{
		buffer[bufferIndex++] = serviceman[i];
	}
	buffer[bufferIndex++] = strlen(role);
	for (int i = 0; role[i]; i++)
	{
		buffer[bufferIndex++] = role[i];
	}
	buffer[bufferIndex++] = strlen(date);
	for (int i = 0; date[i]; i++)
	{
		buffer[bufferIndex++] = date[i];
	}
	buffer[bufferIndex] = '\0';
	if ((bytecount = send(hsock, buffer, 1024, 0)) == SOCKET_ERROR) {
		fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
		return;
	}
	if ((bytecount = recv(hsock, buffer, buffer_len, 0)) == SOCKET_ERROR) {
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		return;
	}
	printf("%s\n", buffer);
}

void viewTechnicianAppoinments(int category, int hsock)
{
	char buffer[1024], serviceman[28], role[28], date[11];
	int buffer_len = 1024, bytecount, bufferIndex = 0, flag = 0;
	memset(buffer, '\0', buffer_len);
	buffer[bufferIndex++] = '3';
	buffer[bufferIndex++] = category;
	buffer[bufferIndex++] = 4;
	viewTechnicianDetails(category, hsock);
	printf("enter the servicemen name to know his appointments:\n");
	while (getchar() != '\n');
	scanf("%[^\n]s", serviceman);
	printf("enter his specialization in his category:\n");
	while (getchar() != '\n');
	scanf("%[^\n]s", role);
	buffer[bufferIndex++] = strlen(serviceman);
	for (int i = 0; serviceman[i]; i++)
	{
		buffer[bufferIndex++] = serviceman[i];
	}
	buffer[bufferIndex++] = strlen(role);
	for (int i = 0; role[i]; i++)
	{
		buffer[bufferIndex++] = role[i];
	}
	buffer[bufferIndex] = '\0';
	if ((bytecount = send(hsock, buffer, 1024, 0)) == SOCKET_ERROR) {
		fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
		return;
	}
	printf("appointments are:\n");
	while (1)
	{
		if ((bytecount = recv(hsock, buffer, buffer_len, 0)) == SOCKET_ERROR) {
			fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
			return;
		}
		if (1024 - strlen(buffer) > 41)
			break;
	}
	printf("%s\n", buffer);
}

void calenderStore1(char *username,int category, int hsock)
{
	int ch;
	do
	{
		calenderMenu1();
		printf("enter your choice\n");
		scanf("%d", &ch);
		switch (ch)
		{
		case 1:
			addTechnician(category, hsock);
			break;
		case 2:
			viewTechnicianDetails(category, hsock);
			break;
		case 3:
			makeAppointment(username, category, hsock);
			break;
		case 4:
			viewTechnicianAppoinments(category, hsock);
			break;
		case 0:
			return;
		}
	} while (1);
}

void calenderStore(char *username, int hsock)
{
	int category;
	do
	{
		printf("categories are:\n");
		printf("1-doctor\n");
		printf("2-police\n");
		printf("3-lawyer\n");
		printf("4-engineer\n");
		printf("5-technician\n");
		printf("0-go back\n");
		printf("enter category number to know more about it\n");
		scanf("%d", &category);
		if (category == 0)
			return;
		else if (category >= 1 && category <= 5)
			calenderStore1(username, category, hsock);
		else
			printf("you enterde wrong choice\n");
	} while (1);
}

void userDetails(int hsock)
{
	char buffer[1024];
	int buffer_len = 1024, bytecount, bufferIndex = 0;
	memset(buffer, '\0', buffer_len);
	buffer[bufferIndex++] = '2';
	buffer[bufferIndex++] = 1;
	buffer[bufferIndex++] = '\0';
	if ((bytecount = send(hsock, buffer, 1024, 0)) == SOCKET_ERROR) {
		fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
		return;
	}
	if ((bytecount = recv(hsock, buffer, buffer_len, 0)) == SOCKET_ERROR) {
			fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
			return;
	}
	printf("%s\n", buffer);
}

void addUser(char *username, int hsock)
{
	char buffer[1024];
	int buffer_len = 1024, bytecount, bufferIndex = 0;
	memset(buffer, '\0', buffer_len);
	buffer[bufferIndex++] = '2';
	buffer[bufferIndex++] = 2;
	buffer[bufferIndex++] = strlen(username);
	for (int i = 0; username[i]; i++)
	{
		buffer[bufferIndex++] = username[i];
	}
	if ((bytecount = send(hsock, buffer, 1024, 0)) == SOCKET_ERROR) {
		fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
		return;
	}
	if ((bytecount = recv(hsock, buffer, buffer_len, 0)) == SOCKET_ERROR) {
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		return;
	}
	printf("%s\n", buffer);
}

void addCategory(char *username, int hsock)
{
	char buffer[1024],category[36];
	int buffer_len = 1024, bytecount, bufferIndex = 0;
	memset(buffer, '\0', buffer_len);
	buffer[bufferIndex++] = '2';
	buffer[bufferIndex++] = 3;
	buffer[bufferIndex++] = strlen(username);
	for (int i = 0; username[i]; i++)
	{
		buffer[bufferIndex++] = username[i];
	}
	printf("enter the category name:\n");
	while (getchar() != '\n');
	scanf("%[^\n]s", category);
	buffer[bufferIndex++] = strlen(category);
	for (int i = 0; category[i]; i++)
	{
		buffer[bufferIndex++] = category[i];
	}
	if ((bytecount = send(hsock, buffer, 1024, 0)) == SOCKET_ERROR) {
		fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
		return;
	}
	if ((bytecount = recv(hsock, buffer, buffer_len, 0)) == SOCKET_ERROR) {
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		return;
	}
	printf("%s\n", buffer);
}

void viewUserCategories(char *username, int hsock)
{
	char buffer[1024];
	int buffer_len = 1024, bytecount, bufferIndex = 0;
	memset(buffer, '\0', buffer_len);
	buffer[bufferIndex++] = '2';
	buffer[bufferIndex++] = 4;
	buffer[bufferIndex++] = strlen(username);
	for (int i = 0; username[i]; i++)
	{
		buffer[bufferIndex++] = username[i];
	}
	if ((bytecount = send(hsock, buffer, 1024, 0)) == SOCKET_ERROR) {
		fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
		return;
	}
	printf("categories are:\n");
	if ((bytecount = recv(hsock, buffer, buffer_len, 0)) == SOCKET_ERROR) {
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		return;
	}
	printf("%s\n", buffer);
}

void addMessage(char *username, int hsock)
{
	char buffer[1024], category[36],mesage[128];
	int buffer_len = 1024, bytecount, bufferIndex = 0;
	memset(buffer, '\0', buffer_len);
	buffer[bufferIndex++] = '2';
	buffer[bufferIndex++] = 5;
	buffer[bufferIndex++] = strlen(username);
	for (int i = 0; username[i]; i++)
	{
		buffer[bufferIndex++] = username[i];
	}
	viewUserCategories(username,hsock);
	printf("enter the category name:\n");
	while (getchar() != '\n');
	scanf("%[^\n]s", category);
	printf("enter the message to add:\n");
	while (getchar() != '\n');
	scanf("%[^\n]s", mesage);
	buffer[bufferIndex++] = strlen(category);
	for (int i = 0; category[i]; i++)
	{
		buffer[bufferIndex++] = category[i];
	}
	buffer[bufferIndex++] = strlen(mesage);
	for (int i = 0; mesage[i]; i++)
	{
		buffer[bufferIndex++] = mesage[i];
	}
	if ((bytecount = send(hsock, buffer, 1024, 0)) == SOCKET_ERROR) {
		fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
		return;
	}
	if ((bytecount = recv(hsock, buffer, buffer_len, 0)) == SOCKET_ERROR) {
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		return;
	}
	printf("%s\n", buffer);
}

char *viewMessage(char *username, int hsock)
{
	char buffer[1024];
	char *category = (char *)malloc(sizeof(char) * 32);
	int buffer_len = 1024, bytecount, bufferIndex = 0;
	memset(buffer, '\0', buffer_len);
	buffer[bufferIndex++] = '2';
	buffer[bufferIndex++] = 6;
	buffer[bufferIndex++] = strlen(username);
	for (int i = 0; username[i]; i++)
	{
		buffer[bufferIndex++] = username[i];
	}
	viewUserCategories(username, hsock);
	printf("enter the category name:\n");
	while (getchar() != '\n');
	scanf("%[^\n]s", category);
	buffer[bufferIndex++] = strlen(category);
	for (int i = 0; category[i]; i++)
	{
		buffer[bufferIndex++] = category[i];
	}
	if ((bytecount = send(hsock, buffer, 1024, 0)) == SOCKET_ERROR) {
		fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
		return category;
	}
	printf("Messages are :\n");
	while (1)
	{
		if ((bytecount = recv(hsock, buffer, buffer_len, 0)) == SOCKET_ERROR) {
			fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
			return category;
		}
		if (1024 - strlen(buffer) > 128)
			break;
		printf("%s\n", buffer);
	}
	printf("%s\n", buffer);
	return category;
}

void deleteMessage(char *username, int hsock)
{
	char buffer[1024], *category, mesage[128];
	int buffer_len = 1024, bytecount, bufferIndex = 0;
	memset(buffer, '\0', buffer_len);
	buffer[bufferIndex++] = '2';
	buffer[bufferIndex++] = 7;
	buffer[bufferIndex++] = strlen(username);
	for (int i = 0; username[i]; i++)
	{
		buffer[bufferIndex++] = username[i];
	}
	category=viewMessage(username, hsock);
	printf("enter the message to delete:\n");
	while (getchar() != '\n');
	scanf("%[^\n]s", mesage);
	buffer[bufferIndex++] = strlen(category);
	for (int i = 0; category[i]; i++)
	{
		buffer[bufferIndex++] = category[i];
	}
	buffer[bufferIndex++] = strlen(mesage);
	for (int i = 0; mesage[i]; i++)
	{
		buffer[bufferIndex++] = mesage[i];
	}
	if ((bytecount = send(hsock, buffer, 1024, 0)) == SOCKET_ERROR) {
		fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
		return;
	}
	if ((bytecount = recv(hsock, buffer, buffer_len, 0)) == SOCKET_ERROR) {
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		return;
	}
	printf("%s\n", buffer);
}

void addReply(char *username, int hsock)
{
	char buffer[1024], *category, mesage[128],rply[32];
	int buffer_len = 1024, bytecount, bufferIndex = 0;
	memset(buffer, '\0', buffer_len);
	buffer[bufferIndex++] = '2';
	buffer[bufferIndex++] = 8;
	buffer[bufferIndex++] = strlen(username);
	for (int i = 0; username[i]; i++)
	{
		buffer[bufferIndex++] = username[i];
	}
	category = viewMessage(username, hsock);
	printf("enter the message to add reply:\n");
	while (getchar() != '\n');
	scanf("%[^\n]s", mesage);
	printf("enter the reply:\n");
	while (getchar() != '\n');
	scanf("%[^\n]s", rply);
	buffer[bufferIndex++] = strlen(category);
	for (int i = 0; category[i]; i++)
	{
		buffer[bufferIndex++] = category[i];
	}
	buffer[bufferIndex++] = strlen(mesage);
	for (int i = 0; mesage[i]; i++)
	{
		buffer[bufferIndex++] = mesage[i];
	}
	buffer[bufferIndex++] = strlen(rply);
	for (int i = 0; rply[i]; i++)
	{
		buffer[bufferIndex++] = rply[i];
	}
	if ((bytecount = send(hsock, buffer, 1024, 0)) == SOCKET_ERROR) {
		fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
		return;
	}
	if ((bytecount = recv(hsock, buffer, buffer_len, 0)) == SOCKET_ERROR) {
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		return;
	}
	printf("%s\n", buffer);
}

char *viewReply(char *username, int hsock)
{
	char buffer[1024], *category;
	char *mesage = (char *)malloc(sizeof(char) * 128);
	char *mescat = (char *)malloc(sizeof(char) * 160);
	int buffer_len = 1024, bytecount, bufferIndex = 0;
	memset(buffer, '\0', buffer_len);
	buffer[bufferIndex++] = '2';
	buffer[bufferIndex++] = 9;
	buffer[bufferIndex++] = strlen(username);
	for (int i = 0; username[i]; i++)
	{
		buffer[bufferIndex++] = username[i];
	}
	category = viewMessage(username, hsock);
	printf("enter the message to get replies:\n");
	while (getchar() != '\n');
	scanf("%[^\n]s", mesage);
	buffer[bufferIndex++] = strlen(category);
	for (int i = 0; category[i]; i++)
	{
		buffer[bufferIndex++] = category[i];
	}
	buffer[bufferIndex++] = strlen(mesage);
	for (int i = 0; mesage[i]; i++)
	{
		buffer[bufferIndex++] = mesage[i];
	}
	if ((bytecount = send(hsock, buffer, 1024, 0)) == SOCKET_ERROR) {
		fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
		return mescat;
	}
	printf("replies are:\n");
	while (1)
	{
		if ((bytecount = recv(hsock, buffer, buffer_len, 0)) == SOCKET_ERROR) {
			fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
			return mescat;
		}
		if (1024 - strlen(buffer) > 128)
			break;
		printf("%s\n", buffer);
	}
	printf("%s\n", buffer);
	memset(mescat, '\0', 160);
	mescat[0] = strlen(category);
	strcat(mescat, category);
	mescat[strlen(category)+1] = strlen(mesage);
	strcat(mescat, mesage);
	return mescat;
}

void deleteReply(char *username, int hsock)
{
	char buffer[1024], *mescat,rply[128];
	int buffer_len = 1024, bytecount, bufferIndex = 0,i;
	memset(buffer, '\0', buffer_len);
	buffer[bufferIndex++] = '2';
	buffer[bufferIndex++] = 10;
	buffer[bufferIndex++] = strlen(username);
	for (i = 0; username[i]; i++)
	{
		buffer[bufferIndex++] = username[i];
	}
	mescat = viewReply(username, hsock);
	buffer[bufferIndex++] = mescat[0];
	for (i = 1; i <= mescat[0]; i++)
	{
		buffer[bufferIndex++] = mescat[i];
	}
	buffer[bufferIndex++] = mescat[i++];
	for (i; mescat[i]; i++)
	{
		buffer[bufferIndex++] = mescat[i];
	}
	printf("enter replie you want to delete:\n");
	while (getchar() != '\n');
	scanf("%[^\n]s", rply);
	buffer[bufferIndex++] = strlen(rply);
	for (i=0; rply[i]; i++)
	{
		buffer[bufferIndex++] = rply[i];
	}

	if ((bytecount = send(hsock, buffer, 1024, 0)) == SOCKET_ERROR) {
		fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
		return;
	}
	if ((bytecount = recv(hsock, buffer, buffer_len, 0)) == SOCKET_ERROR) {
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		return;
	}
	printf("%s\n", buffer);
}

void messageMenu()
{
	printf("this program is about filesystem:\n");
	printf("1-userdetails\n");
	printf("2-add user\n");
	printf("3-add category\n");
	printf("4-view user categories\n");
	printf("5-add message\n");
	printf("6-view messages:\n");
	printf("7-delete messages:\n");
	printf("8-add reply\n");
	printf("9-view replies:\n");
	printf("10-delete replies:\n");
	printf("0-go-back\n");
}

void messageStore(char *username, int hsock)
{
	int ch;
	char *temp;
	do
	{
		messageMenu();
		printf("kindly ener your choice\n");
		scanf("%d", &ch);
		switch (ch)
		{
		case 1:
			userDetails(hsock);
			break;
		case 2:
			addUser(username,hsock);
			break;
		case 3:
			addCategory(username, hsock);
			break;
		case 4:
			viewUserCategories(username, hsock);
			break;
		case 5:
			addMessage(username, hsock);
			break;
		case 6:
			viewMessage(username, hsock);
			break;
		case 7:
			deleteMessage(username, hsock);
			break;
		case 8:
			addReply(username, hsock);
			break;
		case 9:
			viewReply(username, hsock);
			break;
		case 10:
			deleteReply(username, hsock);
			break;
		}
	} while (ch != 0);
}

void storesMenu()
{
	printf("1-for blob store\n");
	printf("2-for message store\n");
	printf("3-for calender store\n");
	printf("0-go back\n");
}

void socket_client()
{

	//The port and address you want to connect to
	int host_port= 1101;
	char* host_name="127.0.0.1";

	//Initialize socket support WINDOWS ONLY!
	unsigned short wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD( 2, 2 );
 	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 || ( LOBYTE( wsaData.wVersion ) != 2 ||
		    HIBYTE( wsaData.wVersion ) != 2 )) {
	    fprintf(stderr, "Could not find sock dll %d\n",WSAGetLastError());
		goto FINISH;
	}

	//Initialize sockets and set any options

	//Connect to the server
	struct sockaddr_in my_addr;

	my_addr.sin_family = AF_INET ;
	my_addr.sin_port = htons(host_port);
	
	memset(&(my_addr.sin_zero), 0, 8);
	my_addr.sin_addr.s_addr = inet_addr(host_name);

	//if( connect( hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == SOCKET_ERROR ){
	//	fprintf(stderr, "Error connecting socket %d\n", WSAGetLastError());
	//	goto FINISH;
	//}

	//Now lets do the client related stuff
		
		int ch;
		char c;
		int hsock = getsocket();
		//add error checking on hsock...
		if( connect(hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == SOCKET_ERROR ){
			fprintf(stderr, "Error connecting socket %d\n", WSAGetLastError());
			goto FINISH;
		}
		char username[30];
		do
		{
			printf("Enter username\n");
			scanf("%s", username);
			do
			{
				storesMenu();
				printf("enter your choice\n");
				scanf("%d", &ch);
				switch (ch)
				{
				case 1:
					blobStore(username, hsock);
					break;
				case 2:
					messageStore(username, hsock);
					break;
				case 3:
					calenderStore(username, hsock);
					break;
				case 0:
					break;
				}
			} while (ch != 0);
			printf("press y or Y for continue using another username:\n");
			printf("press n or N exit from program:\n");
			flushall();
			scanf("%c", &c);
		} while (c == 'y' || c == 'Y');
		closesocket(hsock);

	//closesocket(hsock);
FINISH:
;
}