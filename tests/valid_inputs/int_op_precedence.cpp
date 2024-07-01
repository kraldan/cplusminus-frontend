
int main() {
	//     10   30    6     2     10    5      20     10    8     10    10
//	return 10 * 3  /  5  %  4  +  8  -  5  <<  2  >>  1  &  8  ^  2  |  10;

    /* doesn't test all the ops */
    // 24
    int x1 = 10*2 + 4;
    // 6
    int x2 = 8 - 5 << 1;
    // 1
    int x3 = 10 >> 1 & 1;

    // 31
    return x1+x2+x3;
}
