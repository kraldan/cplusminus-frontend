// test bubble sort and malloc

void bubbleSort(int *const arr, const int arr_len) {
    bool flag = true;
    while(flag) {
        flag = false;
        for(int i = 0; i < arr_len - 1; i++) {
            if(arr[i] > arr[i+1]) {
                flag = true;
                int tmp = arr[i];
                arr[i] = arr[i+1];
                arr[i+1] = tmp;
            }
        }
    }
}

// get a number from scanf and save it in n
// return false on success, true on failure
bool getNumber(int *n) {
	if(scanf("%d", n) != 1) {
		printf("invalid number\n");
		return true;
	}
	return false;
}

void print_arr(const int *arr, int arr_len) {
    for(int i = 0; i < arr_len; i++)
        printf("%d ", arr[i]);
    printf("\n");
}

int main() {
	int *arr, arr_len;
	printf("Please enter number of elements: ");
	if(getNumber(&arr_len))
		return 1;
	arr = (int*) malloc(arr_len * sizeof(*arr));  // implement sizeof expr
	for(int i = 0; i < arr_len; i++) {
		if(getNumber(arr+i))
			return 1;
	}
    printf("running bubble-sort\n");
    bool flag = true;
    int iter = 0;
    while(flag) {
        flag = false;
        for(int i = 0; i < arr_len - 1; i++) {
            if(arr[i] > arr[i+1]) {
                flag = true;
                int tmp = arr[i];
                arr[i] = arr[i+1];
                arr[i+1] = tmp;
            }
        }
        iter++;
        printf("array after iteration n.%d: ", iter);
        print_arr(arr, arr_len);
    }

    printf("sorted array: ");
    print_arr(arr, arr_len);
	free(arr);
}
