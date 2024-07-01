
// forward declare funcs, they'll be linked from libc by clang
void * malloc(int bytes);
void free(void * ptr);

int main() {
	// sizeof doesn't work yet, but 64 bytes should be enough
	int *ptr = (int *) malloc(64);
	ptr[0] = 42;
	int ret = ptr[0];
	free(ptr);
	return ret;
}
