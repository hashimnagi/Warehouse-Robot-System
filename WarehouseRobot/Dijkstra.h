#pragma once
#include <string>
#include "Graph.h"
#include "Route.h"
using namespace std;

class Dijkstra
{
public:
    static Route* findShortestRoute(Graph& graph, string fromID, string toID);
};