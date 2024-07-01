// check that 2d arrays work through calculating matrix determinant of a 2x2 matrix (array)

// expects 4 integers on std in
int main() {
	int m[2][2];
	for(int i = 0; i < 4; i++) {
		// load numbers row wise
		int *loc = &m[i / 2][i % 2];
		scanf("%d", loc);
	}
	int det = m[0][0] * m[1][1] - m[0][1] * m[1][0];
	return det;
}
