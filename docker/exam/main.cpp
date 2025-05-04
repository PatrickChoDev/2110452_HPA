#include <glpk.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_set>
#include <omp.h>
#include <algorithm>
#include <numeric>
#include <string>
#include <sstream>

using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " input_file output_file" << endl;
        return 1;
    }

    // Faster I/O operations
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);

    // Read the entire file at once for better I/O performance
    ifstream in(argv[1], ios::in | ios::binary);
    if (!in.is_open()) {
        cerr << "Failed to open input file." << endl;
        return 1;
    }

    // Read file content into memory
    in.seekg(0, ios::end);
    size_t size = in.tellg();
    in.seekg(0, ios::beg);
    string buffer(size, ' ');
    in.read(&buffer[0], size);
    in.close();

    // Parse from memory
    istringstream iss(buffer);
    int n, m;
    iss >> n >> m;

    // Use unordered_set for faster lookup
    vector<unordered_set<int>> graph(n);

    // Pre-allocate memory to avoid reallocations
    #pragma omp parallel for
    for (int i = 0; i < n; ++i) {
        graph[i].reserve(n/4); // Reserve with estimation
    }

    // Parallel edge reading using OpenMP
    vector<pair<int, int>> edges(m);
    for (int i = 0; i < m; ++i) {
        iss >> edges[i].first >> edges[i].second;
    }

    #pragma omp parallel for
    for (int i = 0; i < m; ++i) {
        int u = edges[i].first;
        int v = edges[i].second;
        #pragma omp critical
        {
            graph[u].insert(v);
            graph[v].insert(u);
        }
    }

    // Disable GLPK terminal output for performance
    glp_term_out(GLP_OFF);

    // ILP setup with GLPK
    glp_prob *lp = glp_create_prob();
    glp_set_prob_name(lp, "minimum_dominating_set");
    glp_set_obj_dir(lp, GLP_MIN);
    glp_add_cols(lp, n);

    // Vectorize column setup
    #pragma omp parallel for
    for (int i = 0; i < n; ++i) {
        glp_set_col_kind(lp, i + 1, GLP_BV);
        glp_set_obj_coef(lp, i + 1, 1.0);
    }

    // Prepare data for batch constraint addition
    vector<int> rowIndices;
    vector<vector<int>> allInds;
    vector<vector<double>> allVals;
    vector<int> rowSizes;

    rowIndices.reserve(n);
    allInds.reserve(n);
    allVals.reserve(n);
    rowSizes.reserve(n);

    // Batch process constraint data preparation
    #pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < n; ++i) {
        vector<int> ind(graph[i].size() + 2); // +2 for 1-indexed and self
        vector<double> val(graph[i].size() + 2);

        ind[1] = i + 1; // 1-indexed
        val[1] = 1.0;

        int idx = 2;
        for (int neighbor : graph[i]) {
            ind[idx] = neighbor + 1; // 1-indexed
            val[idx] = 1.0;
            idx++;
        }

        #pragma omp critical
        {
            rowIndices.push_back(i + 1);
            allInds.push_back(move(ind));
            allVals.push_back(move(val));
            rowSizes.push_back(idx - 1); // Store actual size
        }
    }

    // Add rows in batch for better performance
    glp_add_rows(lp, n);

    for (int i = 0; i < n; ++i) {
        glp_set_row_bnds(lp, i + 1, GLP_LO, 1.0, 0.0);
        glp_set_mat_row(lp, i + 1, rowSizes[i], allInds[i].data(), allVals[i].data());
    }

    // Solve with optimized parameters
    glp_iocp parm;
    glp_init_iocp(&parm);
    parm.presolve = GLP_ON;
    parm.tm_lim = 60000; // Set time limit (ms)
    parm.mip_gap = 0.01; // Allow 1% gap
    parm.fp_heur = GLP_ON; // Enable feasibility pump heuristic
    parm.sr_heur = GLP_ON; // Enable simple rounding heuristic

    int ret = glp_intopt(lp, &parm);

    // Use a string buffer for output
    stringstream output;
    if (ret == 0) {
        for (int i = 0; i < n; ++i) {
            output << glp_mip_col_val(lp, i + 1);
        }
    } else {
        cerr << "Solver failed." << endl;
        glp_delete_prob(lp);
        return 1;
    }

    // Write output all at once
    ofstream out(argv[2]);
    out << output.str();
    out.close();

    glp_delete_prob(lp);
    return 0;
}
