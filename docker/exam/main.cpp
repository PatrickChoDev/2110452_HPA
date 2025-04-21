#include <glpk.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <omp.h>
#include <memory>
#include <sstream>

using namespace std;

int main(int numArgs, char* inputArgs[]) {
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr)

    ifstream inputFile(inputArgs[1], ios::binary);
    ofstream outputFile(inputArgs[2], ios::binary);

    inputFile.rdbuf()->pubsetbuf(nullptr, 0);
    outputFile.rdbuf()->pubsetbuf(nullptr, 0);

    if (!inputFile.is_open()) {
        cerr << "Failed to open input file." << endl;
        return 1;
    }

    int numVertices, numEdges;
    inputFile >> numVertices >> numEdges;

    // Use vectors with preallocated space for better cache performance
    vector<vector<int>> adjList(numVertices);
    #pragma omp parallel for
    for (int vertexId = 0; vertexId < numVertices; vertexId++) {
        adjList[vertexId].reserve(numVertices/4); // Estimate average degree
    }

    // Sequential graph construction to avoid race conditions
    for (int edgeId = 0; edgeId < numEdges; ++edgeId) {
        int vertex1, vertex2;
        inputFile >> vertex1 >> vertex2;
        adjList[vertex1].push_back(vertex2);
        adjList[vertex2].push_back(vertex1);
    }

    // ILP setup with GLPK
    unique_ptr<glp_prob, void(*)(glp_prob*)> linearProgram(glp_create_prob(), glp_delete_prob);
    glp_set_prob_name(linearProgram.get(), "minimum_dominating_set");
    glp_set_obj_dir(linearProgram.get(), GLP_MIN);

    // Setup columns in parallel
    #pragma omp parallel for
    for (int vertexId = 0; vertexId < numVertices; ++vertexId) {
        glp_set_col_kind(linearProgram.get(), vertexId + 1, GLP_BV);
        glp_set_obj_coef(linearProgram.get(), vertexId + 1, 1.0);
    }

    // Setup constraints in parallel chunks
    #pragma omp parallel for schedule(dynamic)
    for (int vertexId = 0; vertexId < numVertices; ++vertexId) {
        vector<int> indices(adjList[vertexId].size() + 2);
        vector<double> values(adjList[vertexId].size() + 2);

        #pragma omp critical
        {
            glp_add_rows(linearProgram.get(), 1);
            glp_set_row_bnds(linearProgram.get(), vertexId + 1, GLP_LO, 1.0, 0.0);
        }

        indices[1] = vertexId + 1;
        values[1] = 1.0;

        int currentIdx = 2;
        for (int adjacentVertex : adjList[vertexId]) {
            indices[currentIdx] = adjacentVertex + 1;
            values[currentIdx] = 1.0;
            currentIdx++;
        }

        #pragma omp critical
        {
            glp_set_mat_row(linearProgram.get(), vertexId + 1, currentIdx-1, indices.data(), values.data());
        }
    }

    // Optimize solver parameters
    glp_iocp solverParams;
    glp_init_iocp(&solverParams);
    solverParams.presolve = GLP_ON;
    solverParams.mip_gap = 0.0;
    solverParams.tm_lim = 1000000;
    solverParams.out_frq = 500;
    solverParams.msg_lev = GLP_MSG_OFF;
    solverParams.br_tech = GLP_BR_DTH;
    solverParams.bt_tech = GLP_BT_BLB;

    int solverResult = glp_intopt(linearProgram.get(), &solverParams);

    outputFile.tie(nullptr);
    if (solverResult == 0) {
        stringstream resultStream;
        #pragma omp parallel
        {
            stringstream threadStream;
            #pragma omp for ordered
            for (int vertexId = 0; vertexId < numVertices; ++vertexId) {
                threadStream << glp_mip_col_val(linearProgram.get(), vertexId + 1);
                #pragma omp ordered
                resultStream << threadStream.str();
                threadStream.str("");
            }
        }
        outputFile << resultStream.str();
    } else {
        cerr << "Solver failed." << endl;
        return 1;
    }

    return 0;
}
