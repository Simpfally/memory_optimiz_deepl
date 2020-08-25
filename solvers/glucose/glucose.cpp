#include <errno.h>

#include <signal.h>
#include <zlib.h>
#include <sys/resource.h>

#include <time.h>

#include <iostream>
#include <vector>


#include "utils/System.h"
#include "utils/ParseUtils.h"
#include "utils/Options.h"
#include "core/Dimacs.h"
#include "simp/SimpSolver.h"


#include "glucose.h"


//void SIGINT_interrupt(int signum) { solver->interrupt(); (void) signum; } // TODO register correct solver..
namespace Glucose_s {

	// SOLVER
	// If any strange behavior : addClause expect shifted by one values.
	Solver::Solver(int bits) : bits(bits) {
		Glucose::SimpSolver SS;
		//SS.eliminate(true); // disable eliminations
		SS.verbosity = 0;
		SS.verbEveryConflicts = 0;

		varc = 0;
		s_var = 0;
		s_clo = 0;
	}

	std::pair<int, std::vector<int>> Solver::solve(std::vector<int> const& vdummy) {
		Glucose::vec<Glucose::Lit> dummy;
		//std::cout << "Dummy : ";
		for (int c : vdummy) {
			if (c < 0) {
				//std::cout << "-" << -c-1 << ", ";
				dummy.push(Glucose::mkLit(-c-1, true));
			} else {
				//std::cout << c-1 << ", ";
				dummy.push(Glucose::mkLit(c-1, false));
			}
		}
		//std::cout << std::endl;
		//std::cout << "Solving... " << SS.nVars() << " variables " << std::endl;
		Glucose::lbool ret = SS.solveLimited(dummy);
		//std::cout << "Finished solving..." << std::endl;
		bool SAT = ret == l_True; // true if satisfiable

		std::vector<int> solution(SS.nVars());
		if (SAT) { 
			for (int i = 0; i < SS.nVars(); i++) {
				if (SS.model[i] != l_Undef) {
					if (SS.model[i] != l_True) {
						solution[i] = 0;
					} else {
						solution[i] = 1;
					}
				}
			}
		}
		return std::make_pair(SAT, solution);
	}

	int Solver::newVar() {
		SS.newVar();
		varc++;
		return varc - 1;
	}

	int Solver::registerVar() {
		int ini = varc;
		for (int i = 0; i < bits; i++)
			newVar();
		//std::cout << SS.nVars() << " = " << varc << std::endl;
		return ini;
	}

	int Solver::getVal(int start, std::vector<int> const& solution) {
		std::vector<int> vals;
		for (int i = start; i < start+bits; i++) {
			int x = solution[i];
			int y = 0;
			if (x < 0)
				y = 0;
			if (x > 0)
				y = 1;
			vals.push_back(y);
		}
		/*std::cout << "Vector:: ";
		for (int x : vals)
			std::cout << x << ", ";
		std::cout << std::endl;*/

		return vecToInt(vals);
	}

	void Solver::addClause(std::vector<int> vclause) {
		s_var += vclause.size();
		s_clo++;

		Glucose::vec<Glucose::Lit> cl;
		for (int x : vclause) {
			if (x == 0) {
				std::cout << "bad clause number, 0 is not correct" << std::endl;
				continue;
			}
			if (x < 0)
				cl.push(Glucose::mkLit(-x-1, true)); 
			else
				cl.push(Glucose::mkLit(x-1, false));
		}
		SS.addClause(cl);
	}
	std::vector<int> Solver::intToVec(int x) {
		std::vector<int> v;
		for (int i = 0; i < bits; i++) {
			v.push_back(x & 1);
			x = x >> 1;
		}
		return v;
	}
	void Solver::AND(int a, int b, int c) {
		addClause({-(a+1), -(b+1), c+1});
		addClause({a+1, -(c+1)});
		addClause({b+1, -(c+1)});
	}
	void Solver::OR(int a, int b, int c) {
		addClause({(a+1), (b+1), -(c+1)});
		addClause({-(a+1), (c+1)});
		addClause({-(b+1), (c+1)});
	}
	void Solver::XOR(int a, int b, int c) {
		addClause({-(a+1), -(b+1), -(c+1)});
		addClause({(a+1), (b+1), -(c+1)});
		addClause({(a+1), -(b+1), (c+1)});
		addClause({-(a+1), (b+1), (c+1)});
	}
	
	void Solver::NOT(int a, int c) {
		addClause({-(a+1), -(c+1)});
		addClause({(a+1), (c+1)});
	}
	void Solver::EQ(int a, int c) {
		addClause({(a+1), -(c+1)});
		addClause({-(a+1), (c+1)});
	}

	void Solver::EQC(int ida, int c) {
		if (c)
			addClause({(ida+1)});
		else
			addClause({-(ida+1)});
	}

	void Solver::halfAdder(int a, int b, int c, int s) {
		XOR(a, b, s);
		AND(a, b, c);
	}

	void Solver::fullAdder(int a, int b, int cIn, int c, int s) {
		int xab = newVar();
		int andAB = newVar();
		int andXC = newVar();
		XOR(a, b, xab);
		AND(a, b, andAB);
		AND(xab, cIn, andXC);
		OR(andAB, andXC, c);
		XOR(xab, cIn, s);
	}

	void Solver::halfCAdder(int ida, int b, int idc, int ids) {
		if (b) {
			// A XOR 1 -> NOT A
			// A AND 1 -> A
			NOT(ida, ids);
			EQ(ida, idc);
		} else {
			// A XOR 0 -> A
			// A AND 0 -> 0
			EQ(ida, ids);
			EQC(idc, 0);
		}
	}
	void Solver::fullCAdder(int ida, int b, int idcIn, int idcOut, int ids) {
		if (b) {
			// a XOR 1 -> NOT a
			// a AND 1 -> a
			int andXC = newVar();
			int xab = newVar();
			NOT(ida, xab);
			AND(xab, idcIn, andXC);
			OR(ida, andXC, idcOut);
			XOR(xab, idcIn, ids);
		} else {
			// A XOR 0 -> A
			// A AND 0 -> 0
			AND(ida, idcIn, idcOut);
			XOR(ida, idcIn, ids);
		}
	}

	void Solver::addConst(int idA, int c, int idO) {
		std::vector<int> cvec = intToVec(c);
		//int a = 0, b = 0, cIn = 0, s = 0, cOut = 0;
		int cOut = 0, cIn = 0;
		for (int i = 0; i < bits; i++) {
			cOut = newVar();
			if (i == 0) {
				halfCAdder(idA+i, cvec[i], cOut, idO+i); // id0+i,cOut = idA+i + cvec[i]
			} else {
				fullCAdder(idA+i, cvec[i], cIn, cOut, idO+i);
			}
			cIn = cOut;
		}
	}

	// LT or equal
	int Solver::cmpLT(int ida, int idb) {
		int pred_mo = 0;
		for (int i = 0; i < bits; i++) {
			if (i == 0) {
				int nai = newVar();
				NOT(ida+i, nai);
				int andab = newVar();
				OR(idb+i, nai, andab); // change this to AND to have strict ineq
				pred_mo = andab;
			} else {
				int nai = newVar();
				NOT(ida+i, nai);
				int andab = newVar();
				AND(idb+i, nai, andab);
				int orab = newVar();
				OR(idb+i, nai, orab);
				int nami = newVar();
				AND(orab, pred_mo, nami);

				int mo = newVar();
				OR(andab, nami, mo);
				pred_mo = mo;
			}
		}
		return pred_mo;
	}
	//LT strict
	int Solver::cmpCLT(int ida, int c) {
		std::vector<int> cvec = intToVec(c);
		int pred_mo = 0;
		for (int i = 0; i < bits; i++) {
			int c = cvec[i];
			if (i == 0) {
				int nai = newVar();
				NOT(ida+i, nai);
				int andab = newVar();
				if (c) {
					// AND(1, nai) -> nai
					pred_mo = nai;
				} else {
					// AND(0, nai) -> 0
					EQC(andab, 0);
					pred_mo = andab;
				}
			} else {
				if (c) {
					int nai = newVar();
					NOT(ida+i, nai);
					// AND(1, nai) -> nai
					// OR(1, nai) -> 1
					// AND(1, pred_mo) -> pred_mo
					int mo = newVar();
					OR(nai, pred_mo, mo);
					pred_mo = mo;
				} else {
					int nai = newVar();
					NOT(ida+i, nai);
					// AND(0, nai) -> 0
					// OR(0, nai) -> nai
					int nami = newVar();
					AND(nai, pred_mo, nami);
					// OR(0, nami) -> nami
					pred_mo = nami;
				}
			}
		}
		return pred_mo;
	}
	//0 12 60 72
	void Solver::unary(int idastart, int idaend, int idbstart, int idbend) {
		int abefore = cmpLT(idaend, idbstart); // 12 60
		int bbefore = cmpLT(idbend, idastart); // 72 0
		int orab = newVar();
		OR(abefore, bbefore, orab);
		addClause({orab+1});
	}

	// AUX
	int vecToInt(std::vector<int> const& v) {
		int s = 0;
		for (unsigned int i = 0; i < v.size(); i++)
			s += v[i] << i;
		return s;
	}
	std::vector<int> intToVec(int x) {
		std::vector<int> v;
		while(x > 0) {
			v.push_back(x & 1);
			x = x >> 1;
		}
		return v;
	}

}

namespace glucose_memSchedule {
	mem_schedule::Solution solve(mem_schedule::Instance const& ins, int vv, std::clock_t c_start, int joke) {
		int a = 0; // maximum weight
		int b = 0; // sum of all weights
		int m = 0;
		for (int x : ins.weights) {
			if (x > a)
				a = x;
			b += x;
		}
		int bb = b;
		int bbits = 0;
		while (bb > 0) {
			bb = bb>>1;
			bbits++;
		}
		bbits += 1;
		b += 1;
		int bits = bbits;
		mem_schedule::Solution sol;
		std::cout << std::fixed << std::setprecision(2);
		if (joke > 0) {
		  auto isol = solve_uno(ins, bits, joke); 
			return isol;
		}

		while (b - a > 1) {
			m = (a+b)/2;
		  auto isol = solve_uno(ins, bits, m); // TODO compute how many bits to use
			auto c_elapsed = 1000.0 * (std::clock() - c_start) / CLOCKS_PER_SEC;
			if (isol.n == ins.n) { // TODO get score and use it
				if (vv >= 1) {
					std::cout << a << " < X < " << b << std::endl;
				}
					//std::cout << "tested " << m << " = SAT" << std::endl;
				b = m;
				sol = isol;
			} else {
				if (vv == 1)
					std::cout << "tested " << m << " = UNSAT" << std::endl;
				a = m;
			}
			std::cout << a << "," << b << "," << c_elapsed << std::endl;
		}
		return sol;
	}
	mem_schedule::Solution solve_inc(mem_schedule::Input const& in, mem_schedule::Instance const& ins, int vv, std::clock_t c_start) {
		int bits = 11;

		int a = 0; // maximum weight
		int b = 0; // sum of all weights
		int m = 0;
		for (int x : ins.weights) {
			if (x > a)
				a = x;
			b += x;
		}
		int bb = b;
		int bbits = 0;
		while (bb > 0) {
			bb = bb>>1;
			bbits++;
		}
		if (vv == 1)
			std::cout << "bits needed : " << bbits << std::endl;
		bbits += 1;
		b += 1; // wrost case

		bits = bbits; // Auto set encoding bits


		Glucose_s::Solver S(bits);
		auto mac = add_constr_inc(S, ins);
		if (vv == 1)
			std::cout << "variables : " << S.SS.nVars() << std::endl;
		int maxxc = mac.first;
		auto starts = mac.second;
		mem_schedule::Solution sol;
		std::cout << std::fixed << std::setprecision(2);
		while (b - a > 1) {
			m = (a+b)/2;
			if (vv >= 1)
				std::cout << a << " < X < " << b << std::endl;
			if (vv == 1)
				std::cout << "solving for " << m << " maxxc =  " << maxxc << std::endl;
		  auto isol = solve_uno_inc(S, ins, m, maxxc, starts); // TODO compute how many bits to use
			auto c_elapsed = 1000.0 * (std::clock() - c_start) / CLOCKS_PER_SEC;

			if (isol.n == ins.n) { // TODO get score and use it
				int scoro = isol.check(in, in.computeOrder(), 0);
				if (vv >= 1)
					std::cout << "scoro : " << scoro << " < " << m << std::endl;
				b = m;
				if (b > scoro)
					b = scoro;
				sol = isol;
			} else {
				if (vv == 1)
					std::cout << "tested " << m << " = UNSAT" << std::endl;
				a = m;
			}
			std::cout << a << "," << b << "," << c_elapsed << std::endl;
		}
		return sol;
	}

	mem_schedule::Solution solve_uno_inc(Glucose_s::Solver& S, mem_schedule::Instance const& ins, int maxx, int maxxc, std::vector<int> starts) {
		std::vector<int> maxv = S.intToVec(maxx);
		std::vector<int> dummy; 
		int vv = 0;
		for (int i = 0; i < (int) maxv.size(); i++) {
			int place = maxxc + i + 1;
			if (!maxv[i])
				place = -place;
			dummy.push_back(place);
		}
		auto out = S.solve(dummy);
		std::vector<int> vecsol;
		if (out.first) {
			for (int i = 0; i < ins.n; i++) {
				int as = S.getVal(starts[i], out.second);
				vecsol.push_back(as);
			}
			if (vv == 1) {
				std::cout << "Var : " << S.varc << std::endl;
				std::cout << "Var used : " << S.s_var << std::endl;
				std::cout << "Clauses : " << S.s_clo << std::endl;
			}
		} else { 
			//std::cout << "UNSAT" << std::endl;
		}
		return mem_schedule::Solution(vecsol);
	}

	mem_schedule::Solution solve_uno(mem_schedule::Instance const& ins, int bits, int maxx) {
		Glucose_s::Solver S(bits);
		std::vector<int> dummy;

		std::vector<int> starts(ins.n);
		std::vector<int> ends(ins.n);

		for (int i = 0; i < ins.n; i++) {
			starts[i] = S.registerVar();
			S.addClause({-(starts[i]+bits)}); // disallow using last bit
			ends[i] = S.registerVar();
			S.addConst(starts[i], ins.weights[i], ends[i]); // end = start + aw
			//std::cout << "created var " << i << " of size " << ins.weights[i] << std::endl;
			int mx = S.cmpCLT(ends[i], maxx);
			S.addClause({(mx+1)});
		}

		for (auto p : ins.edges) {
			int a = p.first;
			int b = p.second;
			//std::cout << starts[a] << " " <<  ends[a] << " " <<  starts[b] << " " <<  ends[b] << std::endl;
			S.unary(starts[a], ends[a], starts[b], ends[b]);
			//std::cout << "created unary for " << a << " -/- " << b << std::endl;
		}

		auto out = S.solve(dummy);
		std::vector<int> vecsol;
		if (out.first) {
			/*std::cout << "SAT, raw solution : ";
			for (int x : out.second)
				std::cout << x << ", ";
			std::cout << std::endl;*/
			for (int i = 0; i < ins.n; i++) {
				int as = S.getVal(starts[i], out.second);
				//int es = ins.weights[i] + as;
				vecsol.push_back(as);
				//std::cout << i << " : " << as << ", " << es << std::endl;
			}
			int vv = 0;
			if (vv == 1) {
				std::cout << "Var : " << S.varc << std::endl;
				std::cout << "Var used : " << S.s_var << std::endl;
				std::cout << "Clauses : " << S.s_clo << std::endl;
			}
		} else { 
			//std::cout << "UNSAT" << std::endl;
		}
		return mem_schedule::Solution(vecsol);
	}

	std::pair<int, std::vector<int>> add_constr_inc(Glucose_s::Solver & S, mem_schedule::Instance const& ins) {
		std::vector<int> starts(ins.n);
		std::vector<int> ends(ins.n);

		int maxxs = S.registerVar();
		for (int i = 0; i < ins.n; i++) {
			starts[i] = S.registerVar();
			S.addClause({-(starts[i]+S.bits)}); // disallow using last bit
			ends[i] = S.registerVar();
			S.addConst(starts[i], ins.weights[i], ends[i]); // end = start + aw
			//std::cout << "created var " << i << " of size " << ins.weights[i] << std::endl;
			int mx = S.cmpLT(ends[i], maxxs); // end <= maxxs (set by dummy)
			S.addClause({(mx+1)});
		}

		for (auto p : ins.edges) {
			int a = p.first;
			int b = p.second;
			//std::cout << starts[a] << " " <<  ends[a] << " " <<  starts[b] << " " <<  ends[b] << std::endl;
			S.unary(starts[a], ends[a], starts[b], ends[b]);
			//std::cout << "created unary for " << a << " -/- " << b << std::endl;
		}
		return std::make_pair(maxxs, starts);
	}
}
