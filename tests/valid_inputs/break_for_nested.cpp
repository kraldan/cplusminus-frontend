
int main() {
	int sum = 0;
	for(int i = 0; i < 5; i++) {
		for(int j = 0; j < 5; j++) {
			if (j == 2) break;
			sum += j;
		}
	}
	return sum;
}
