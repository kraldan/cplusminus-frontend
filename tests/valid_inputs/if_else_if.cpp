// test that in a chained if-else-if, only one branch happens

int main() {
	int res = 0;
	if(false)
		res += 1;
	else if(true)
		res += 42;	// only branch taken
	else if(5 < 10)
		res += 666;
	else
		res += 10;
	return res;
}
