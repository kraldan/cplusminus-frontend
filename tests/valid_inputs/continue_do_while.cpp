
int main() {
	int i = 0, sum = 0;
	do {
		i += 1;
		if(i % 2 == 1) continue;
		sum += i;
	} while(i < 5);
	return sum;
}
