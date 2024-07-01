
struct LinkedList {
	int val;
	LinkedList * next;
}

int main() {
	LinkedList node1, node2;
	node1.next = &node2;
	node2.val = 22;
	return node1.next->val;
}
