int my_abs(int n) {
    if(n >= 0)
        return n;
    else
        return -n;
	// dead code that should not cause problems during compilation
	n = 7;
}

int main() {
	return my_abs(-42);
}
