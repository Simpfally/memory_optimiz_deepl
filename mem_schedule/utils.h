#pragma once
#include <vector>
#include <utility>

namespace mem_schedule {
	class Input { // directed graph of dependancies (and DAG)
		public : 
			int n; // tasks/vertex count
			std::vector<int> weights; // memory taken by task
			std::vector<std::pair<int, int>> edges; // a->b means b depends on a

			Input(int n); // Random DAG of size n (density = 0.5)
			Input(std::vector<std::pair<int, int>> edges, std::vector<int> weights) : n(weights.size()), weights(weights), edges(edges) {}

			void dprint() const; // Print instance to sdtout
			void adjprint() const; // Print the adjmatrix
			void statprint() const; // Print some statistics about each tasks
			std::vector<std::vector<int>> adj_matrix() const; // return adj matrix
			std::vector<int> computeOrder() const; // Topological order of DAG
		private:
	};

	class Instance { // undirected graph of constraint
		public:
			int n; // tasks/vertex count
			std::vector<std::pair<int, int>> edges; // a-b means a and b can't overlap
			std::vector<int> weights; // memory taken by task

			Instance(const Input& in, const std::vector<int>& order);

			void dprint() const; // Print instance to sdtout

		private:
	};

	class Memory {
		public:
			struct mem {
				int id;
				int start;
				int end;

				mem(int id, int start, int end) : id(id), start(start), end(end) {}
			};

			std::vector<mem> vmem;

			void print(int n);
			int add_mem(int id, int start, int end); // return 0 if it fails to add
			void remove_mem(int id);

	};


	class Solution {
		public :
			int n;
			std::vector<int> offsets; // i->starting memory offset

			Solution(std::vector<int> const& offsets);
			Solution();

			void dprint() const;
			int check(Input const& in, std::vector<int> const& order, int vv) const;
		private:
	};


	int no_child(int x, std::vector<std::vector<int>> const& G);
}
