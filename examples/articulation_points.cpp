/**
 * This program finds articulation points in a connected graph.
 */

const int NO_PARENT = -1;
const int INF = 2000000000;
const int MAX_VERTICES = 1000;

struct Vertex {
	int neighbours[1000];
	int neighbourCount;
	int in, out, low;
	int isArticulation;
};

Vertex vertices[1000];
int T = 0;


int min(int a, int b) {
	return a < b ? a : b;
}

void DFS(int i, int parent) {
	if(vertices[i].in != INF)
		return;
	
	vertices[i].in = T++;
	int treeEdges = 0;
	for(int j = 0; j  < vertices[i].neighbourCount; j++) {
		int n = vertices[i].neighbours[j];
		if(vertices[n].in == INF) {
		    DFS(n, i);
		    if(vertices[n].low >= vertices[i].in)
			vertices[i].isArticulation = 1;
		    vertices[i].low = min(vertices[i].low, vertices[n].low);
		    treeEdges += 1;
		}
		else if(n != parent && vertices[n].in < vertices[i].in) // back edge
			vertices[i].low = min(vertices[i].low, vertices[n].in);  
	}
	vertices[i].out = T++;

	if(parent == NO_PARENT && treeEdges == 1)        // special case with root
		vertices[i].isArticulation = 0;	
}

int printf(const char *fmt, ...);
int scanf(const char *fmt, ...);

/**
 Input of the graph is expected in this format:
 n m		[ n is number of vertices, m is number of edges]
 a1 b1
 a2 b2
 ...
 am  bm	[ m edges described by their vertices (numbers from 0 to n-1)]
*/
int main() {
	
	int n, m, a, b;
	
	printf("Enter number of vertices and edges\n");
	scanf("%d %d", &n, &m);
	
	if(n > MAX_VERTICES || m > MAX_VERTICES*(MAX_VERTICES-1)/2) {
		printf("Too many vertices or edges, max vertices is 1000\n");
		return 1;
	}
	
	for(int i = 0; i < n;  i++) {
		Vertex * v = &vertices[i];
		v -> neighbourCount =  0;
		v -> in = v -> out = v -> low = INF;
		v -> isArticulation = 0;
	}
	
	printf("Please enter %d edges as pairs of numbers between 0-%d:\n", m, n-1);
	for(int i = 0; i < m;  i++) {
		if (scanf("%d %d", &a, &b) != 2 || a >= n || b >= n) {
			printf("Wrong input.\n");
			return 2;
		}
		vertices[a].neighbours[vertices[a].neighbourCount++]= b;
		vertices[b].neighbours[vertices[b].neighbourCount++]= a;
	}
		
	DFS(0, NO_PARENT);
	
	printf("Articulation points are:");
	for(int i = 0;  i < n; i++)
		if(vertices[i].isArticulation)
			printf(" %d", i);
	printf("\n");
	
	return 0;
}

