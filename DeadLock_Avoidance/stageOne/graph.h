/*
    parts of source code in graph.h borrowed from
    SRC CODE: 
        https://www.geeksforgeeks.org/detect-cycle-in-a-graph/
        https://stackoverflow.com/questions/64996845/remove-elements-from-adjacency-list-c

*/

#include <iostream>
#include <vector>
#include <string>

class graph {
private:
    std::vector<int> *adj;
    int V;
    int N, K;

    bool isCyclicUtil(int v, bool visited[], bool *recStack) {
        if(visited[v] == false) {
            // Mark the current node as visited and part of recursion stack
            visited[v] = true;
            recStack[v] = true;
    
            // Recur for all the vertices adjacent to this vertex
            std::vector<int>::iterator i;
            for(i = adj[v].begin(); i != adj[v].end(); ++i) {
                if ( !visited[*i] && isCyclicUtil(*i, visited, recStack) )
                    return true;
                else if (recStack[*i])
                    return true;
            }
    
        }
        recStack[v] = false;  // remove the vertex from recursion stack
        return false;
    }
public:
    graph(int nVertices, int nProcesses, int nResources) {
        V = nVertices + 1;
        adj = new std::vector<int> [nVertices + 1];

        N = nProcesses;
        K = nResources;
    }
    ~graph() {
        delete [] adj;
    }

    void addEdge(int u, int v) {
        adj[u].push_back(v);
    }

    void removeEdge(int u, int v) {
        auto it=std::find(adj[u].begin(), adj[u].end(), v);
        if (it!=adj[u].end())
            adj[u].erase(it);
    }

    bool validEdge(int u) {
        if(adj[u].empty()) {
            return false;
        }
        return true;
    } 

    bool isCyclic() {
        // Mark all the vertices as not visited and not part of recursion
        // stack
        bool *visited = new bool[V];
        bool *recStack = new bool[V];
        for(int i = 0; i < V; i++)
        {
            visited[i] = false;
            recStack[i] = false;
        }
    
        // Call the recursive helper function to detect cycle in different
        // DFS trees
        for(int i = 0; i < V; i++)
            if ( !visited[i] && isCyclicUtil(i, visited, recStack))
                return true;
    
        return false;
    }

    // A utility function to print the adjacency list
    // representation of graph in the format requested in doc
    void printGraph(int p = -1, int r = -1) {
        // assignment edges
        for(int i = 1; i <= K; i++) {
            bool exists = false;
            if(adj[i].empty())
                std::cout << "(-1, -1) ; ";
            else
                std::cout << "(" << i << ", " << adj[i][0] << ")" << " ; ";
        }   std::cout << std::endl;

        // // claim edges
        for(int i = K + 1; i < V; i++) {
            bool exists = false;
            if(adj[i].empty())
                std::cout << "(-1, -1) ; ";
            else {
                std::sort(adj[i].begin(), adj[i].end());
                for(int j = 0; j < adj[i].size(); j++) {
                    std::cout << "(" << i << ", " << adj[i][j] << ")" << " ; ";
                }
            }
        }   std::cout << std::endl;

        // request edges
        std::cout << "(" << p << ", " << r << ")" << std::endl;
    }

    void debugPrintGraph() {
        for (int v = 0; v < V; ++v) {
            std::cout << v << ": ";
            for (auto x : adj[v])
                std::cout << x << " ";
            printf("\n");
        }
    }
};