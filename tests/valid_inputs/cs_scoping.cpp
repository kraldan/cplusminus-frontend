
int main() {
	int i = 5, a;
	{
		int i = 3;
		a = i;
	}
	return a+i;
}
