

#include <gecode/int.hh>
#include <gecode/search.hh>
#include "gecode.h"

using namespace Gecode;

namespace Gecode_s {
	// MemSched Class
	MemSched::MemSched(mem_schedule::Instance const& in) : n(in.n), weights(in.weights), edges(in.edges) {
		int MaxScore = 0;
		int minScore = 0;
		for (int x : weights) {
			MaxScore += x;
			if (x > minScore)
				minScore = x;
		}
		score = Gecode::IntVar(*this, minScore, MaxScore); // TODO minscore to be improved?
		l = Gecode::IntVarArray(*this, n, 0, MaxScore);

		// Computre score
		Gecode::IntVarArgs XX(*this, n, 0, MaxScore*2); // IntVarArgs needed by Max
		for (int i = 0; i < n ; i++) {
			rel(*this, XX[i] == l[i] + weights[i]);
		}
		max(*this, XX, score);

		for (auto p : edges) {
			Gecode::IntVarArgs startTimes;
			startTimes << l[p.first] << l[p.second];
			Gecode::IntArgs weighties; // Same here
			weighties << weights[p.first] << weights[p.second];

			Gecode::unary(*this, startTimes, weighties);
			//std::cout << p.first << " - " << p.second  << std::endl;
		}
		Gecode::branch(*this, l, Gecode::INT_VAR_SIZE_MIN(), Gecode::INT_VAL_MIN());
	}
	MemSched::MemSched(MemSched& s) : IntMinimizeSpace(s) {
		score.update(*this, s.score);
		l.update(*this, s.l);
	}
	void MemSched::print(void) const {
		std::cout << "cost:" << score << " " << l << std::endl;
	}
	Gecode::Space* MemSched::copy(void) {
		return new MemSched(*this);
	}
	Gecode::IntVar MemSched::cost(void) const {
		return score;
	}
	std::vector<int> MemSched::outL(void) const {
		std::vector<int> x;
		for (auto k : l) {
			int u = k.val();
			x.push_back(u);
		}
		return x;
	}
}

namespace gecode_memSchedule {
	mem_schedule::Solution solve(mem_schedule::Input const& in, mem_schedule::Instance const& ins, int vv, std::clock_t c_start) {
		Gecode_s::MemSched* m = new Gecode_s::MemSched(ins);
		Gecode::BAB<Gecode_s::MemSched> e(m);
		delete m;
		mem_schedule::Solution sol;
		while (Gecode_s::MemSched* s = e.next()) {
			if (vv == 1)
				s->print(); 
			sol = mem_schedule::Solution(s->outL());
			if (vv >= 0) {
				int score = sol.check(in, in.computeOrder(), 0);
				auto c_elapsed = 1000.0 * (std::clock() - c_start) / CLOCKS_PER_SEC;
				std::cout << std::fixed << std::setprecision(2);
				std::cout << score << "," << c_elapsed << std::endl;
				//std::cout << "score found : " << score << std::endl;
			}
			delete s;
		}
		return mem_schedule::Solution(sol);
	}
}
