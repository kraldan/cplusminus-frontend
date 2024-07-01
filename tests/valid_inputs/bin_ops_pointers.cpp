// test that binary operators work with pointers

int main() {
	int arr[2];
	int 	*p0 = &arr[0],
		*p1 = &arr[1];
	int res = 0;
	// 6 * 7 == 42
	if(p0 == p0)
		res += 7;
	if(p0 <= p1 && p1 <= p1)
		res += 7;
	if(p0 < p1)
		res += 7;
	if(p0 != p1)
		res += 7;
	if(p1 >= p0 && p1 >= p1)
		res += 7;
	if(p1 > p0)
		res += 7;
	return res + (p1 - p0) - 1;
}
