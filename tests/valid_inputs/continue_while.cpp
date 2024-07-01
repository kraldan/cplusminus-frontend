
int main() {
	int i = 0, sum = 0;
	while(i < 5) {
		i += 1;
		if(i % 2 == 1) continue;
		sum += i;
	}
	return sum;
}
