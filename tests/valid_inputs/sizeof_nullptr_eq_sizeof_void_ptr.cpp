// test that sizeof(nullptr) is the same as of void*
// https://en.cppreference.com/w/cpp/language/types

int main() {
	return sizeof(nullptr) == sizeof(void*);
}
