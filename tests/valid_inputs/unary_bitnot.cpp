// test unary bitnot operator ~

int main() {
	return ~(~0 - 42);	// 42, this should be int-width independent, but relies on underflow
}
