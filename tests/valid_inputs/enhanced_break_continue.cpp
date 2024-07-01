int main() {
    int i = 0, res = 0;

    while(i < 5) {
        i += 1;
        res += 10;

        while(true) {
            break 2;
        }

        res += 42314; // this line will never be reached
    }
    return res; // return 10 for break 2, 50 for continue 2
}