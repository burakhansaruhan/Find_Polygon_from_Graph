#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NODES 100
#define MAX_EDGES 100

typedef struct Edge {
    int node;
    int weight;
    struct Edge* next;
} Edge;

typedef struct Graph {
    int numNodes;
    int numEdges;
    Edge* adjList[MAX_NODES];
} Graph;

typedef struct Polygon {
    int nodes[MAX_NODES];
    int numNodes;
    int perimeter;
} Polygon;

//prototipler
Graph* createGraph(int numNodes);
void addEdge(Graph* graph, int src, int dest, int weight);
void readGraphFromFile(const char* filename, Graph* graph);
void findPolygons(Graph* graph);
void DFS(Graph* graph, int startNode, int currentNode, int* visited, int* path, int pathLen, int* pathWeight, Polygon* polygons, int* polygonCount, int* levels, int level);
void printPolygons(Polygon* polygons, int polygonCount);
int isNewPolygon(Polygon* polygons, int polygonCount, int* path, int pathLen);

Graph* createGraph(int numNodes) {
	int i;
    Graph* graph = (Graph*)malloc(sizeof(Graph));
    graph->numNodes = numNodes;
    graph->numEdges = 0;
    for ( i = 0; i < numNodes; i++) {
        graph->adjList[i] = NULL;
    }
    return graph;
}

void addEdge(Graph* graph, int src, int dest, int weight) {
    Edge* newEdge = (Edge*)malloc(sizeof(Edge));
    newEdge->node = dest;
    newEdge->weight = weight;
    newEdge->next = graph->adjList[src];
    graph->adjList[src] = newEdge;

    newEdge = (Edge*)malloc(sizeof(Edge)); 		// kenari iki dugum icin de ekleme
    newEdge->node = src;
    newEdge->weight = weight;
    newEdge->next = graph->adjList[dest];
    graph->adjList[dest] = newEdge;

    graph->numEdges++;
}

void readGraphFromFile(const char* filename, Graph* graph) {
	int i;
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

	int m = graph->numEdges;
    char src[10], dest[10];
    int weight;
    for ( i = 0; i < m; i++) {
        if (fscanf(file, "%s  %s  %d", src, dest, &weight) != 3) {
            fprintf(stderr, "Error reading edge information from file.\n");
            fclose(file);
            exit(EXIT_FAILURE);
        }
        int srcNode = src[0] - 'A'; // char karakteri kolay kullanmak ivin integer cevirme 
        int destNode = dest[0] - 'A';
        addEdge(graph, srcNode, destNode, weight);
    }

    fclose(file);
}

void findPolygons(Graph* graph) {
	int i;
    int visited[MAX_NODES] = {0};
    int path[MAX_NODES];
    int pathWeight[MAX_NODES];
    int levels[MAX_NODES];
    Polygon polygons[MAX_EDGES];
    int polygonCount = 0;

    for ( i = 0; i < graph->numNodes; i++) {
        DFS(graph, i, i, visited, path, 0, pathWeight, polygons, &polygonCount, levels, 0);
        memset(visited, 0, sizeof(visited)); 		// her dfs den sonra visit dizisini sifrlama
    }

    printPolygons(polygons, polygonCount);
}

void DFS(Graph* graph, int startNode, int currentNode, int* visited, int* path, int pathLen, int* pathWeight, Polygon* polygons, int* polygonCount, int* levels, int level) {
	int i,j;
    visited[currentNode] = 1;
    path[pathLen] = currentNode;
    levels[currentNode] = level;

    Edge* edge = graph->adjList[currentNode];
    while (edge != NULL) {
        if (edge->node == startNode && pathLen >= 2) { // komsu dugum baslangic dugumune esitse ve uzunluk 2 den fazlaysa
            Polygon poly;
            poly.numNodes = pathLen + 1;
            int perimeter = 0;

            for (i = 0; i <= pathLen; i++) {
                poly.nodes[i] = path[i];
                if (i > 0) {
                    perimeter += pathWeight[i - 1];
                }
            }

            // son kenari cevreye ekleme
            perimeter += edge->weight;
            poly.perimeter = perimeter;

            if (isNewPolygon(polygons, *polygonCount, path, pathLen + 1)) {
                polygons[*polygonCount] = poly;
                (*polygonCount)++;
            }
        }

        if (!visited[edge->node]) {				// komsu dugum ziyaret edilmemisse
            pathWeight[pathLen] = edge->weight;
            DFS(graph, startNode, edge->node, visited, path, pathLen + 1, pathWeight, polygons, polygonCount, levels, level + 1);
        } else if (levels[edge->node] < levels[currentNode] && edge->node != startNode) {
            pathWeight[pathLen] = edge->weight;
        }

        edge = edge->next;
    }

    visited[currentNode] = 0;
}

int compareNodes(const void* a, const void* b) {
    return (*(int*)a - *(int*)b);
}

int isNewPolygon(Polygon* polygons, int polygonCount, int* path, int pathLen) {
	int i,j;
    // alfabetik siralama
    int sortedPath[MAX_NODES];
    memcpy(sortedPath, path, pathLen * sizeof(int));
    qsort(sortedPath, pathLen, sizeof(int), compareNodes);

    // siralanmis dugum listesini karsilastirma
    for ( i = 0; i < polygonCount; i++) {
        if (polygons[i].numNodes == pathLen) {
            // dugum dizisini siralama
            int sortedNodes[MAX_NODES];
            memcpy(sortedNodes, polygons[i].nodes, pathLen * sizeof(int));
            qsort(sortedNodes, pathLen, sizeof(int), compareNodes);

            int match = 1;
            for ( j = 0; j < pathLen; j++) {
                if (sortedNodes[j] != sortedPath[j]) {
                    match = 0;
                    break;
                }
            }
            // eslesirse 
            if (match) return 0;
        }
    }
    // daha onceki cokgenlerle eslesmezse
    return 1;
}


void printPolygons(Polygon* polygons, int polygonCount) {
	int i,j,k,sides;
    printf("\nShape Count: %d\n\n", polygonCount);

    // her bir cokgenin kenar ve indis sayisi
    int shapeCount[MAX_NODES] = {0};
    int shapeIndex[MAX_NODES][MAX_NODES] = {0};

    for ( i = 0; i < polygonCount; i++) {
        int sides = polygons[i].numNodes;
        shapeCount[sides]++;
        shapeIndex[sides][shapeCount[sides]] = i;
    }

    // kenar sayisina gore yazdirma
    for ( sides = 3; sides <= MAX_NODES; sides++) {
        if (shapeCount[sides] > 0) {
            printf("\n%d-gon Count: %d\n", sides, shapeCount[sides]);

            // o kenar sayisina ait cokgenin bilgileri
            for ( j = 1; j <= shapeCount[sides]; j++) {
                int index = shapeIndex[sides][j];
                printf("%d. %d-gon: ", j, sides);

                for ( k = 0; k < polygons[index].numNodes; k++) {
                    printf("%c ", 'A' + polygons[index].nodes[k]);
                }

                printf("Length: %d\n", polygons[index].perimeter);
            }
        }
    }
}

int main() {
    int numNodes, numEdges;
    printf("Enter the number of nodes: ");
    scanf("%d", &numNodes);
    printf("Enter the number of edges: ");
    scanf("%d", &numEdges);

    Graph* graph = createGraph(numNodes);
    graph->numEdges = numEdges;

    readGraphFromFile("graph.txt", graph);
    findPolygons(graph);

    return 0;
}
