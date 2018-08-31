#include <iostream>
#include <stdio.h>
using namespace std;
int arr[50][100], num[8][7], M, N;
int put(int startM, int startN) {
	int check, cnt, ret, n = -1, code = 0, pt = 0;
	for (int i = 0; i < 8; i++) {
		cnt = 0; ret = 0; check = 0; 
		for (int j = 0; j < 7; j++) {
			num[i][j] = arr[startN][startM + j];
			if (num[i][j] == check)
				cnt++;
			else {
				ret = ret * 10 + cnt;
				cnt = 1;
				check = !check;
			}
		}
		ret = ret * 10 + cnt;
		switch (ret) {
		case 3211:
			n = 0;
			break;
		case 2221:
			n = 1;
			break;
		case 2122:
			n = 2;
			break;
		case 1411:
			n = 3;
			break;
		case 1132:
			n = 4;
			break;
		case 1231:
			n = 5;
			break;
		case 1114:
			n = 6;
			break;
		case 1312:
			n = 7;
			break;
		case 1213:
			n = 8;
			break;
		case 3112:
			n = 9;
			break;
		}
		if (i % 2 == 0 || i == 0) {
			code += n * 3;
		}
		else {
			code += n;
		}
		pt += n;
		startM += 7;
	}
	if (code % 10 != 0) {
		pt = 0;
	}
	return pt;
}
void find(int t) {
	for (int i = N - 1; i >= 0; i--) {
		for (int j = M - 1; j >= 0; j--) {
			if (arr[i][j] == 1) {
				printf("#%d %d\n", t, put(j - 55, i));
				return;
			}
		}
	}
}
int main()
{
	int tc;
	cin >> tc;
	for (int t = 1; t <= tc; t++) {
		cin >> N >> M;
		for (int i = 0; i < N; i++) {
			for (int j = 0; j < M; j++) {
				scanf("%1d", &arr[i][j]);
			}
		}
		find(t);
	}
}
