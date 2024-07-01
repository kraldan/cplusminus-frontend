int fact(const int n) {
    int res = 1;
    for(int i = 1; i <= n; ++i)
        res = i * res;
    return res;
}

int main() {
	return fact(4);
}
