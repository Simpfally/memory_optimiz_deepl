#include <iomanip>
#include <iostream>

#include <gecode/int.hh>
#include <gecode/minimodel.hh>
#include <gecode/search.hh>

#include "mem_schedule/utils.h"

namespace Gecode_s {
	class MemSched : public Gecode::IntMinimizeSpace {
		protected:
			int n; // number of tasks
			std::vector<int> weights;
			std::vector<std::pair<int, int>> edges;
			Gecode::IntVar score; // used by the cost function
			Gecode::IntVarArray l; // the output
		public:
			MemSched(mem_schedule::Instance const& in); // initialization
			MemSched(MemSched& s); // used to copy spaces
			void print(void) const;
			virtual Gecode::Space* copy(void); // also
			virtual Gecode::IntVar cost(void) const; // compute the cost of a space
			std::vector<int> outL(void) const;
	};
}

namespace gecode_memSchedule {
	mem_schedule::Solution solve(mem_schedule::Input const& in, mem_schedule::Instance const& ins, int vv, std::clock_t starttime);
}
