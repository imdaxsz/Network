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
	printf("\n 서버의 IP 주소를 입력하세요 : ");
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

	printf("\n\t***숫자 야구 게임***\n");

	for (;;) { // 사용자가 종료를 원할 때까지 반복
		int num[4]; // 서버로 전달할 숫자 및 새로운 게임 시작 여부 
		int result[5]; // 서버로부터 받아온 결과
		char restart; // 새로운 게임 싲가 여부
		num[3] = 0;
		printf("\n 숫자 세 개를 입력하세요 : ");
		scanf("%d %d %d", &num[0], &num[1], &num[2]);

		// 서버에게 전송하기 위해 데이터를 네트워크 바이트로 변환
		for (int i = 0; i < 3; i++)
			num[i] = htonl(num[i]);

		/* Send the string, including the null terminator, to the server */
		send(sock, (char *)num, sizeof(num), 0);
		if ((bytesRcvd = recv(sock, (char *)result, sizeof(result), 0)) > 0) {
			// 서버로부터 받아온 네트워크 바이트 데이터를 호스트 바이트로 바꿔줌
			int strike = ntohl(result[0]);
			int ball = ntohl(result[1]);
			int currentscore = ntohl(result[2]);
			int maxscore = ntohl(result[3]);
			int trynum = ntohl(result[4]);

			// 결과 출력
			if (strike == 0 && ball == 0) printf("out\n");
			else printf(" >> %d strike\t%d ball\n", strike, ball);

			;			if (strike == 3) { // 사용자가 숫자를 모두 맞춰 게임 한 판이 끝났을 때
				printf("\n >> 3 strike!\t시도 횟수 : %d\t현재 점수 : %d\n", trynum, currentscore);
				printf(" >> 최고 점수: %d\n\n", maxscore);
				printf(" 게임을 새로 하시겠습니까? (y/n): ");
				scanf(" %c", &restart);
				if (restart == 'y') { // 새로운 게임 시작
					num[3] = 1;
					send(sock, (char *)num, sizeof(num), 0); // 서버에 새로운 게임 시작을 보냄
				}
				if (restart == 'n') { // 게임 종료
					num[3] = 2;
					send(sock, (char *)num, sizeof(num), 0); // 서버에 게임 종료를 보냄
					printf("게임을 종료합니다.\n\n");
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