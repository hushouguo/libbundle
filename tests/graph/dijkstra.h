/*
 * \file: dijkstra.h
 * \brief: Created by hushouguo at 11:44:39 Oct 25 2018
 */
 
#ifndef __DIJKSTRA_H__
#define __DIJKSTRA_H__

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define CHECK_RETURN(COND, RETVAL) do { if (!(COND)) { return RETVAL; } } while(0)
#define CHECK_BREAK(COND) if (!(COND)) { break; }
#define CHECK_CONTINUE(COND) if (!(COND)) { continue; }


/**
 * Undirected graph
 */

#define NODE_MIN			1
#define NODE_MAX			100

#define COST_SELF			0
#define COST_UNREACHABLE	-1

struct graph_s {
	int node_num;
	int* cost_matrix;/* record cost of adjacent node */
	int* cost_lowest;/* record the lowest cost from entry node to this node */
	int* flag_lowest;/* record whether the node had obtained the minimum cost */
};

typedef struct graph_s  	graph_t;

graph_t* graph_create(int node_num);
void graph_set_edge(graph_t* g, int from_id, int to_id, int cost);
/** 
 * it returns the minimal time to spread out from the entry to the whole graph 
 * but it means that not all nodes are reachable if it returns -1
 */
int graph_spread_message(graph_t* g, int entry_id);
void graph_destroy(graph_t* g);

void unittest_graph();

#endif
