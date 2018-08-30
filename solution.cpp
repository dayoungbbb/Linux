#include <vector>
#include <queue>
using namespace std;
vector<vector<bool>> visited;
int d[4][2] = { {-1, 0},{0, 1}, {1, 0},{0, -1} };
int M, N;
int bfs(int x, int y, vector<vector<int>> picture) {
	queue<pair<int, int> > q;
	q.push(make_pair(x, y));
	visited[x][y] = true;
	int cx, cy, tx, ty, cnt = 1;
	while (!q.empty()) {
		cx = q.front().first;
		cy = q.front().second;
		q.pop();
		for (int i = 0; i < 4; i++) {
			tx = cx + d[i][0];
			ty = cy + d[i][1];
			if (tx >= 0 && ty >= 0 && tx < M && ty < N && !visited[tx][ty] && picture[tx][ty] == picture[cx][cy]) {
				q.push(make_pair(tx, ty));
				visited[tx][ty] = true;
				cnt++;
			}
		}
	}
	return cnt;
}
// 전역 변수를 정의할 경우 함수 내에 초기화 코드를 꼭 작성해주세요.
vector<int> solution(int m, int n, vector<vector<int>> picture) {
	int number_of_area = 0;
	int max_size_of_one_area = 0;
	int temp = 0;
	visited.clear();
	visited = vector<vector<bool> >(m, vector<bool>(n));
	M = m; N = n;
	for (int i = 0; i < m; i++) {
		for (int j = 0; j < n; j++) {
			if (picture[i][j] != 0 && !visited[i][j]) {
				number_of_area++;
				temp = bfs(i, j, picture);
				max_size_of_one_area = temp > max_size_of_one_area ? temp : max_size_of_one_area;
			}
		}
	}
	vector<int> answer(2);
	answer[0] = number_of_area;
	answer[1] = max_size_of_one_area;
	return answer;
}

