#pragma comment(lib, "ws2_32.lib")
#include <stdio.h>
#include <winsock2.h>
#include <stdlib.h>
#include <time.h>
#define SERV_PORT 5000
#define MAXPENDING 5

int snum[3];
int currentscore = 0; // ���� �ǿ����� ����
int maxscore = 0; // Ŭ���̾�Ʈ�� �ִ� ����
int trynum = 0; // Ŭ���̾�Ʈ�� ���� �õ� Ƚ��
int curtrynum = 0; // ���� �ǿ��� ���߱⸦ �õ��� Ƚ��

void initGame(int *trynum, int* snum) { // ���� �ʱ�ȭ
	*trynum += 1; // �� �õ� Ƚ�� 1ȸ ����
	currentscore = 0; // ���� ���� ���� �ʱ�ȭ
	curtrynum = 0; // ���� �� �ȿ��� �õ� Ƚ�� �ʱ�ȭ
	srand((unsigned)time(NULL));
	for (int i = 0; i < 3; i++) { // ������ ���� �� �� ���� ���� 
		snum[i] = rand() % 9;
		for (int j = 0; j < i; j++) {
			if (snum[j] == snum[i])
				i--;
		}
	}
	printf(" ������ ����: %d %d %d\n\n", snum[0], snum[1], snum[2]);

}

void check(int *cnum, int *ball, int *strike) { // ��� Ȯ��
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			if ((snum[i] == cnum[j]) && (i == j)) // ���� ��ġ ��� ���� �� 
				*strike += 1; // strike �� ����
			if ((snum[i] == cnum[j]) && (i != j)) // ���� ������ ��ġ�� �ٸ� ��
				*ball += 1; // ball �� ����
		}
	}

	currentscore = 1000 - 50 * curtrynum; // �õ��� Ƚ���� �þ���� ������ �پ�鵵�� ��

	if (*strike == 3) {
		// �� ���� ������ ������ �� �ְ� ������ ���� ���� ��
		if (maxscore < currentscore)
			maxscore = currentscore; // ���� ������ �� ũ�ٸ� �ְ� ���� ����
	}
}

int main(void) {
	int servSock;
	int clntSock;
	struct sockaddr_in echoServAddr;
	struct sockaddr_in echoClntAddr;
	int clntLen;
	char Buffer[256];
	int maxLen = sizeof(Buffer);
	int recvMsgSize;
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		fprintf(stderr, "WSAStartup() failed\n");
		exit(1);
	}

	/* Construct local address structure */
	memset(&echoServAddr, 0, sizeof(echoServAddr));
	echoServAddr.sin_family = AF_INET;
	echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	echoServAddr.sin_port = htons(SERV_PORT);

	/* Create socket for incoming connection */
	if ((servSock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Error: socket() failed\n");
		exit(1);
	}

	/* Bind to the local address */
	if (bind(servSock, (struct sockaddr *) &echoServAddr,
		sizeof(echoServAddr)) < 0) {
		printf("Error: bind() failed\n");
		exit(1);
	}

	/* Mark the socket so it will listen for incoming connections */
	if (listen(servSock, MAXPENDING) < 0) {
		printf("Error: listen() failed\n");
		exit(1);
	}

	for (;;) { /* Run forever */
		/* Set the size of the in-out parameter */
		clntLen = sizeof(echoClntAddr);

		/* Wait for a client to connect */
		if ((clntSock = accept(servSock, (struct sockaddr *) &echoClntAddr, &clntLen)) < 0) {
			printf("Error: accept() failed\n");
			exit(1);
		}

		initGame(&trynum, snum); // ���� �ʱ�ȭ

		while (1) {
			int cnum[4]; // Ŭ���̾�Ʈ�κ��� �޾ƿ� ���� 3�� + ���ο� ���� ���� ����
			int result[5]; // Ŭ���̾�Ʈ���� ������ ���
			int ball = 0; // ball ����
			int strike = 0; // strike ����

			if ((recvMsgSize = recv(clntSock, (char *)cnum, sizeof(cnum), 0)) > 0) {
				for (int i = 0; i < 3; i++)
					cnum[i] = ntohl(cnum[i]); // ��Ʈ��ũ ����Ʈ ���ڸ� ȣ��Ʈ ����Ʈ�� �ٲ���

				if (cnum[3] == 1) { // ���ο� ���� ����
					printf("\n\n");
					initGame(&trynum, snum);
					continue;
				}
				if (cnum[3] == 2) { // ���� ���� & ���� Ŭ���̾�Ʈ�� ���� ����
					printf("\n\n ���� Ŭ���̾�Ʈ���� ���� ����\n");
					break;
				}

				curtrynum++; // ���� �ǿ����� ���߱� �õ� Ƚ�� ����

				printf(" Ŭ���̾�Ʈ�� �Է� ���� : %d  %d  %d\n", cnum[0], cnum[1], cnum[2]);
				check(cnum, &ball, &strike);
				printf(" %d strike\t%d ball\n", strike, ball);

				// Ŭ���̾�Ʈ���� �����ϱ� ���� �����͸� ��Ʈ��ũ ����Ʈ�� ��ȯ
				result[0] = htonl(strike);
				result[1] = htonl(ball);
				result[2] = htonl(currentscore);
				result[3] = htonl(maxscore);
				result[4] = htonl(trynum);
				send(clntSock, (char *)result, sizeof(result), 0); /* Send Result */
			}
			else
				printf("ERROR: Receiving error\n");

		}
		closesocket(clntSock);
	}
	WSACleanup();
	return 0;
}