#include <time.h>

#include <iostream>
#include <iomanip>
#include <vector>
#include <map>

#include "utils/System.h"
#include "utils/ParseUtils.h"
#include "utils/Options.h"
#include "core/Dimacs.h"
#include "simp/SimpSolver.h"

#include "mem_schedule/utils.h"

typedef std::vector<std::vector<int>> graph;
namespace Glucose_s {

/* Graph coloring residues
	graph generate_randgraph(int n);

	void print_graph(graph G);
	int solve(Glucose::SimpSolver& S, Glucose::vec<Glucose::Lit>& dummy, bool verbose);
	void print_solution(Glucose::SimpSolver& S, int n, int ncol);
	void add_clause_colours(const graph& G, Glucose::SimpSolver& S, int ncol);
	int solve_naive(const graph& G);
	int solve_naive_once(const graph& G, int col);
	int solve_inc_aux(Glucose::SimpSolver& S, int n, int n_col);
	void solve_inc_one(const graph& G, int col);
	int solve_inc(const graph& G);
	void test_inc_naive(const graph& G, int n);
	void test_once(const graph& G, int n); */

	// dummy and clauses ints are shifted by one to avoid using 0 that can't be negative
	class Solver {
		public:
			//int are encoded 6 = 011 in vectors (biggest factor last)

			Glucose::SimpSolver SS;
			std::map<int, int> reg;
			int varc; // how far in memory are we
			int bits; // how many bits per int

			int s_var; // how many variable used
			int s_clo; // how many clauses

			Solver(int bits);

			// UNSAT : 0,[] SAT : 1, [1,1,-1] = 0-> true, 1-> true, 2 -> false
			// vdummy : [-1] -> variable 0 set to false
			std::pair<int, std::vector<int>> solve(std::vector<int> const& vdummy);
			int registerVar(); // map id->offset of the first bit of var.id
			int getVal(int id, std::vector<int> const& solution); // id+solution -> int
			void addClause(std::vector<int> vclause); // [-1,2] add clause -0 OR 1
			std::vector<int> intToVec(int x);
			int newVar(); // add var to solver and increment varc
			void AND(int a, int b, int c); // c true iif A&&B
			void OR(int a, int b, int c);  // c <=> a or b
			void NOT(int a, int c); // c = not a
			void EQ(int a, int c); // a = c
			void EQC(int ida, int c); // Equal to a constant (1 bit)
			void XOR(int a, int b, int c);
			void halfAdder(int a, int b, int c, int s); // a+b = s,c
			void halfCAdder(int ida, int b, int idc, int ids); // with a constant
			void fullAdder(int a, int b, int cIn, int c, int s); // a+b+c = s, c
			void fullCAdder(int ida, int b, int idcIn, int idcOut, int ids); // with a constant
			void addConst(int idA, int c, int idO); //  id0 = idA + c
			int cmpLT(int ida, int idb); // ida <= idb = TRUE je crois
			int cmpCLT(int ida, int c); // ida < c = TRUE
			void unary(int ida, int idaend, int idb, int idbend); // [ida, ida+aw] no overlap [idb,...]
			
	};
	int vecToInt(std::vector<int> const& v);
	std::vector<int> intToVec(int x);


}

namespace glucose_memSchedule {
	mem_schedule::Solution solve(mem_schedule::Instance const& ins, int vv, std::clock_t c_start, int joke);
	mem_schedule::Solution solve_inc(mem_schedule::Input const& in, mem_schedule::Instance const& ins, int vv, std::clock_t c_start);
	mem_schedule::Solution solve_uno(mem_schedule::Instance const& ins, int bits, int maxx);
	std::pair<int, std::vector<int>> add_constr_inc(Glucose_s::Solver & S, mem_schedule::Instance const& ins); //ret max pos
	mem_schedule::Solution solve_uno_inc(Glucose_s::Solver & S, mem_schedule::Instance const& ins, int maxx, int maxxc, std::vector<int> starts);
}
