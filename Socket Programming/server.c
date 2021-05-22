#pragma comment(lib, "ws2_32.lib")
#include <stdio.h>
#include <winsock2.h>
#include <stdlib.h>
#include <time.h>
#define SERV_PORT 5000
#define MAXPENDING 5

int snum[3];
int currentscore = 0; // 현재 판에서의 점수
int maxscore = 0; // 클라이언트의 최대 점수
int trynum = 0; // 클라이언트의 게임 시도 횟수
int curtrynum = 0; // 현재 판에서 맞추기를 시도한 횟수

void initGame(int *trynum, int* snum) { // 게임 초기화
	*trynum += 1; // 총 시도 횟수 1회 증가
	currentscore = 0; // 현재 판의 점수 초기화
	curtrynum = 0; // 현재 판 안에서 시도 횟수 초기화
	srand((unsigned)time(NULL));
	for (int i = 0; i < 3; i++) { // 서버의 숫자 세 개 랜덤 생성 
		snum[i] = rand() % 9;
		for (int j = 0; j < i; j++) {
			if (snum[j] == snum[i])
				i--;
		}
	}
	printf(" 서버의 숫자: %d %d %d\n\n", snum[0], snum[1], snum[2]);

}

void check(int *cnum, int *ball, int *strike) { // 결과 확인
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			if ((snum[i] == cnum[j]) && (i == j)) // 값과 위치 모두 같을 때 
				*strike += 1; // strike 값 증가
			if ((snum[i] == cnum[j]) && (i != j)) // 값은 같으나 위치가 다를 때
				*ball += 1; // ball 값 증가
		}
	}

	currentscore = 1000 - 50 * curtrynum; // 시도한 횟수가 늘어날수록 점수가 줄어들도록 함

	if (*strike == 3) {
		// 한 판의 게임이 끝났을 때 최고 점수와 현재 점수 비교
		if (maxscore < currentscore)
			maxscore = currentscore; // 현재 점수가 더 크다면 최고 점수 갱신
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

		initGame(&trynum, snum); // 게임 초기화

		while (1) {
			int cnum[4]; // 클라이언트로부터 받아온 숫자 3개 + 새로운 게임 시작 여부
			int result[5]; // 클라이언트에게 전달할 결과
			int ball = 0; // ball 개수
			int strike = 0; // strike 개수

			if ((recvMsgSize = recv(clntSock, (char *)cnum, sizeof(cnum), 0)) > 0) {
				for (int i = 0; i < 3; i++)
					cnum[i] = ntohl(cnum[i]); // 네트워크 바이트 숫자를 호스트 바이트로 바꿔줌

				if (cnum[3] == 1) { // 새로운 게임 시작
					printf("\n\n");
					initGame(&trynum, snum);
					continue;
				}
				if (cnum[3] == 2) { // 게임 종료 & 현재 클라이언트와 접속 종료
					printf("\n\n 현재 클라이언트와의 접속 종료\n");
					break;
				}

				curtrynum++; // 현재 판에서의 맞추기 시도 횟수 증가

				printf(" 클라이언트의 입력 숫자 : %d  %d  %d\n", cnum[0], cnum[1], cnum[2]);
				check(cnum, &ball, &strike);
				printf(" %d strike\t%d ball\n", strike, ball);

				// 클라이언트에게 전송하기 위해 데이터를 네트워크 바이트로 변환
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