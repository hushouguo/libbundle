/*
 * \file: dijkstra.cpp
 * \brief: Created by hushouguo at 11:45:03 Oct 25 2018
 */

/**
 * graph implementation part
 */

graph_t* graph_create(int node_num) {
	int i, j;
	graph_t* g = NULL;
	CHECK_RETURN(node_num >= NODE_MIN && node_num <= NODE_MAX, NULL);
	g = (graph_t*) malloc(sizeof(graph_t));
	g->node_num = node_num;
	g->cost_matrix = (int*) malloc(sizeof(int) * g->node_num * g->node_num);
	g->cost_lowest = (int*) malloc(sizeof(int) * g->node_num);
	g->flag_lowest = (int*) malloc(sizeof(int) * g->node_num);
	for (j = 0; j < g->node_num; ++j) {
		for (i = 0; i < g->node_num; ++i) {
			g->cost_matrix[j * g->node_num + i] = i == j ? COST_SELF : COST_UNREACHABLE;
		}
		g->cost_lowest[j] = COST_UNREACHABLE;
		g->flag_lowest[j] = 0;
	}
	return g;
}

void graph_destroy(graph_t* g) {
	assert(g);
	free(g->cost_matrix);
	free(g->cost_lowest);
	free(g->flag_lowest);
	free(g);
}

void graph_set_edge(graph_t* g, int from_id, int to_id, int cost) {
	assert(g);
	CHECK_RETURN(from_id >= 0 && from_id < g->node_num, (void) (0));
	CHECK_RETURN(to_id >= 0 && to_id < g->node_num, (void) (0));
	CHECK_RETURN(cost >= 0 || cost == COST_UNREACHABLE, (void) (0));
	g->cost_matrix[from_id * g->node_num + to_id] = cost;
	g->cost_matrix[to_id * g->node_num + from_id] = cost;
}

int graph_spread_message(graph_t* g, int entry_id) {
	int i, j, cost_largest = COST_UNREACHABLE;

	assert(g);
	CHECK_RETURN(entry_id >= 0 && entry_id < g->node_num, -1);

	/* reset cost_lowest and flag_lowest */
	for (i = 0; i < g->node_num; ++i) {
		g->cost_lowest[i] = g->cost_matrix[i * g->node_num + entry_id];
		g->flag_lowest[i] = 0;
	}

	g->cost_lowest[entry_id] = 0;
	g->flag_lowest[entry_id] = 1;

	for (j = 0; j < g->node_num; ++j) {
		/* Gets the lowest cost node in the entry node */
		int node_lowest = -1;
		int cost_lowest = COST_UNREACHABLE;
		for (i = 0; i < g->node_num; ++i) {
			CHECK_CONTINUE(g->flag_lowest[i] == 0);
			if (cost_lowest == COST_UNREACHABLE || 
					(g->cost_lowest[i] != COST_UNREACHABLE && g->cost_lowest[i] < cost_lowest)) {
				node_lowest = i;
				cost_lowest = g->cost_lowest[i];
			}
		}

		CHECK_BREAK(node_lowest != -1 && cost_lowest != COST_UNREACHABLE);/* the lowest node is not exist */

		g->flag_lowest[node_lowest] = 1;

		/* update the lowest cost of all other nodes passing through the node_lowest */
		for (i = 0; i < g->node_num; ++i) {
			int cost_node = g->cost_matrix[i * g->node_num + node_lowest];/* the cost of node passing through the node_lowest */
			CHECK_CONTINUE(g->flag_lowest[i] == 0);
			CHECK_CONTINUE(cost_node != COST_UNREACHABLE);
			if ((cost_lowest + cost_node) < g->cost_lowest[i] || g->cost_lowest[i] == COST_UNREACHABLE) {
				g->cost_lowest[i] = cost_lowest + cost_node;
			}
		}
	}

	/* find all the lowest cost nodes and find the largest one */
	for (i = 0; i < g->node_num; ++i) {
		/* printf("head node: %d -> entry node: %d, cost lowest: %d\n", i, entry_id, g->cost_lowest[i]); */
		CHECK_RETURN(g->cost_lowest[i] != COST_UNREACHABLE, -1);/* the node is unreachable */
		if (cost_largest == COST_UNREACHABLE || g->cost_lowest[i] > cost_largest) {
			cost_largest = g->cost_lowest[i];
		}
	}

	return cost_largest;
}


/* unittest */ 

void unittest_graph() {
	int id, node_num = 5, time_minimal = -1;
	graph_t* g = graph_create(node_num);
	graph_set_edge(g, 0, 1, 50);
	graph_set_edge(g, 0, 2, 30);
	graph_set_edge(g, 0, 3, 100);
	graph_set_edge(g, 0, 4, 10);
	graph_set_edge(g, 1, 2, 5);
	graph_set_edge(g, 1, 3, 20);
	graph_set_edge(g, 2, 3, 50);
	graph_set_edge(g, 3, 4, 10);
	for (id = 0; id < node_num; ++id) {
		time_minimal = graph_spread_message(g, id);
		printf("minimal time: %d from entry: %d\n", time_minimal, id);
	}
	graph_destroy(g);
}

#if 0
int main() {
	int id, node_num, time_minimal;
	graph_t* g = NULL;
	printf("i need to know the number of cities: ");
	scanf("%d", &node_num);
	g = graph_create(node_num);
	if (!g) {
		fprintf(stderr, "error: invalid input, the number of nodes is in the range of [%d,%d]\n", NODE_MIN, NODE_MAX);
		return -1;
	}

	for (id = 1; id < node_num; ++id) {
		int i = 0;
		fprintf(stderr, "adjacency matrix of line #%d: ", id);
		while (i < id) {
			int cost;
			int rc = scanf("%d", &cost);
			if (rc != 1) { /* FIXME: Need to determine whether input is `x` */
				cost = COST_UNREACHABLE;
			}

			/*printf("A(%d,%d), cost:%d\n", id, i, cost);*/
			graph_set_edge(g, id, i++, cost);

			if (getchar() == '\n') {
				break;
			}
		}
	}

	time_minimal = graph_spread_message(g, 0);
	if (time_minimal == -1) {
		fprintf(stderr, "error: Some cities are not reachable\n");
	}
	else {
		printf("%d\n", time_minimal);
	}

	graph_destroy(g);
	return 0;
}
#endif

