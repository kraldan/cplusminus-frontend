// test compound statement and name shadowing

int main() {
	int i = 42;
	{
		int i;
		i = 666;
		{
			int i;
			i = 1000;
		}
		i = -1;
	}
	return i;
}
