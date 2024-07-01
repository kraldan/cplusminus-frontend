
int main() {
	int sum = 0;
	for(int i = 0; i < 5; i++) {
		if(i % 2 == 1) continue;
		sum += i;
	}
	return sum;
}
