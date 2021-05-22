#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "ws2_32.lib")
#include <stdio.h>
#include <winsock2.h>
#include <stdlib.h>
#define SERV_PORT 5000

int main(int argc, char *argv[]) {
	int sock;
	struct sockaddr_in echoServAddr;
	char servIP[15];
	int bytesRcvd;
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		fprintf(stderr, "WSAStartup() failed\n");
		exit(1);
	}
	printf("\n ������ IP �ּҸ� �Է��ϼ��� : ");
	scanf("%s", servIP);

	/* Construct the server address structure */
	memset(&echoServAddr, 0, sizeof(echoServAddr));
	echoServAddr.sin_family = AF_INET;
	echoServAddr.sin_addr.s_addr = inet_addr(servIP);
	echoServAddr.sin_port = htons(SERV_PORT);

	/* Create a reliable, stream socket using TCP */
	if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("ERROR: socket() failed\n");
		exit(1);
	}

	/* Establish the connection to the echo server */
	if (connect(sock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) < 0) {
		printf("ERROR: connect() failed\n");
		exit(1);
	}

	printf("\n\t***���� �߱� ����***\n");

	for (;;) { // ����ڰ� ���Ḧ ���� ������ �ݺ�
		int num[4]; // ������ ������ ���� �� ���ο� ���� ���� ���� 
		int result[5]; // �����κ��� �޾ƿ� ���
		char restart; // ���ο� ���� �밡 ����
		num[3] = 0;
		printf("\n ���� �� ���� �Է��ϼ��� : ");
		scanf("%d %d %d", &num[0], &num[1], &num[2]);

		// �������� �����ϱ� ���� �����͸� ��Ʈ��ũ ����Ʈ�� ��ȯ
		for (int i = 0; i < 3; i++)
			num[i] = htonl(num[i]);

		/* Send the string, including the null terminator, to the server */
		send(sock, (char *)num, sizeof(num), 0);
		if ((bytesRcvd = recv(sock, (char *)result, sizeof(result), 0)) > 0) {
			// �����κ��� �޾ƿ� ��Ʈ��ũ ����Ʈ �����͸� ȣ��Ʈ ����Ʈ�� �ٲ���
			int strike = ntohl(result[0]);
			int ball = ntohl(result[1]);
			int currentscore = ntohl(result[2]);
			int maxscore = ntohl(result[3]);
			int trynum = ntohl(result[4]);

			// ��� ���
			if (strike == 0 && ball == 0) printf("out\n");
			else printf(" >> %d strike\t%d ball\n", strike, ball);

			;			if (strike == 3) { // ����ڰ� ���ڸ� ��� ���� ���� �� ���� ������ ��
				printf("\n >> 3 strike!\t�õ� Ƚ�� : %d\t���� ���� : %d\n", trynum, currentscore);
				printf(" >> �ְ� ����: %d\n\n", maxscore);
				printf(" ������ ���� �Ͻðڽ��ϱ�? (y/n): ");
				scanf(" %c", &restart);
				if (restart == 'y') { // ���ο� ���� ����
					num[3] = 1;
					send(sock, (char *)num, sizeof(num), 0); // ������ ���ο� ���� ������ ����
				}
				if (restart == 'n') { // ���� ����
					num[3] = 2;
					send(sock, (char *)num, sizeof(num), 0); // ������ ���� ���Ḧ ����
					printf("������ �����մϴ�.\n\n");
					break;
				}
			}
		}
		else
			printf("ERROR: Receiving echoed string error\n");
	}

	closesocket(sock);
	WSACleanup();
	return 0;
}