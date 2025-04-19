#include <glpk.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <omp.h>
#include <memory>
#include <sstream>

using namespace std;

int main(int argc, char* argv[]) {
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);

    ifstream in(argv[1], ios::binary);
    ofstream out(argv[2], ios::binary);

    in.rdbuf()->pubsetbuf(nullptr, 0);
    out.rdbuf()->pubsetbuf(nullptr, 0);

    if (!in.is_open()) {
        cerr << "Failed to open input file." << endl;
        return 1;
    }

    int n, m;
    in >> n >> m;

    // Use vectors with preallocated space for better cache performance
    vector<vector<int>> graph(n);
    for (auto& v : graph) {
        v.reserve(n/4); // Estimate average degree
    }

    // Sequential graph construction to avoid race conditions
    for (int i = 0; i < m; ++i) {
        int u, v;
        in >> u >> v;
        graph[u].push_back(v);
        graph[v].push_back(u);
    }

    // ILP setup with GLPK
    unique_ptr<glp_prob, void(*)(glp_prob*)> lp(glp_create_prob(), glp_delete_prob);
    glp_set_prob_name(lp.get(), "minimum_dominating_set");
    glp_set_obj_dir(lp.get(), GLP_MIN);

    // Setup columns
    glp_add_cols(lp.get(), n);
    for (int i = 0; i < n; ++i) {
        glp_set_col_kind(lp.get(), i + 1, GLP_BV);
        glp_set_obj_coef(lp.get(), i + 1, 1.0);
    }

    // Setup constraints
    for (int i = 0; i < n; ++i) {
        vector<int> ind(graph[i].size() + 2);
        vector<double> val(graph[i].size() + 2);

        glp_add_rows(lp.get(), 1);
        glp_set_row_bnds(lp.get(), i + 1, GLP_LO, 1.0, 0.0);

        ind[1] = i + 1;
        val[1] = 1.0;

        int idx = 2;
        for (int neighbor : graph[i]) {
            ind[idx] = neighbor + 1;
            val[idx] = 1.0;
            idx++;
        }

        glp_set_mat_row(lp.get(), i + 1, idx-1, ind.data(), val.data());
    }

    // Optimize solver parameters
    glp_iocp parm;
    glp_init_iocp(&parm);
    parm.presolve = GLP_ON;
    parm.mip_gap = 0.0;
    parm.tm_lim = 1000000;
    parm.out_frq = 500;
    parm.msg_lev = GLP_MSG_OFF;
    parm.br_tech = GLP_BR_DTH;
    parm.bt_tech = GLP_BT_BLB;

    int ret = glp_intopt(lp.get(), &parm);

    out.tie(nullptr);
    if (ret == 0) {
        stringstream ss;
        for (int i = 0; i < n; ++i) {
            ss << glp_mip_col_val(lp.get(), i + 1);
        }
        out << ss.str();
    } else {
        cerr << "Solver failed." << endl;
        return 1;
    }

    return 0;
}
