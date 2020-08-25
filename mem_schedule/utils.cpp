#include <vector>
#include <utility>
#include <iostream>
#include <time.h>

#include "utils.h"

namespace mem_schedule {
	// INPUT

	Input::Input(int n) : n(n), weights(n) {
		for (int i = 0; i < n; i++) {
			for (int j = 0; j < i; j++) {
				if (rand()%2) {
					if (rand()%2)
						if (rand()%2)
							edges.push_back(std::make_pair(i, j));
				}
			}
			weights[i] = rand() % 10 + 0;
		}
	}

	std::vector<int> Input::computeOrder() const { // TODO add cycle check
		auto G = adj_matrix();
		std::vector<int> L; 
		std::vector<int> S; // vertices with no incomming edge

		// Initial fill S
		for (int i = 0; i < n; i++) {
			int b = 0;
			for (int j = 0; j < n; j++) {
				if (G[j][i]) { // [j][i] = 1 => j->i
					b = 1;
					break;
				}
			}
			if (!b) 
				S.push_back(i);
		}

		while (S.size() > 0) {
			int x = S.back();
			S.pop_back();
			L.push_back(x);

			for (int j = 0; j < n; j++) {
				if (G[x][j]) {
					G[x][j] = 0;

					// check if "j" has incoming edges
					int b = 0;
					for (int k = 0; k < n; k++) {
						if (G[k][j]) {
							b = 1;
							break;
						}
					}
					if (!b) {
						S.push_back(j);
					}
				}
			}
		}
		// TODO if G has edges left : not a DAG
		return L;
	}

	void Input::dprint() const {
		for (auto e : edges) {
			std::cout << e.first << " -> " << e.second << std::endl;
		}
	}

	void Input::adjprint() const {
		auto G = adj_matrix();
		for (int i = 0; i < n; i++) {
			for (int j = 0; j < n; j++)
				std::cout << G[i][j] << " ";
			std::cout << std::endl;
		}
	}

	void Input::statprint() const {
		std::vector<int> childs(n, 0);
		std::vector<int> parents(n, 0);
		for (auto e : edges) {
			childs[e.first]++;
			parents[e.second]++;
		}
		/*
		for (int i = 0; i < n; i++) {
			std::cout << "Task " << i << " : weight = " << weights[i] << ", childs = " << childs[i] << ", parents = " << parents[i] << std::endl;
		}*/
	}

	std::vector<std::vector<int>> Input::adj_matrix() const {
		std::vector<std::vector<int>> G(n, std::vector<int>(n, 0));
		for (auto k : edges) {
			G[k.first][k.second] = 1;
		}
		return G;
	}

	// INSTANCE
	Instance::Instance(Input const& in, std::vector<int> const& order) : n(in.n), weights(in.weights) {
		auto G = in.adj_matrix();

		std::vector<int> mem; // tasks in memory
		for (int x : order) {
			// add one task in memory, it is now in conflict with every
			// task already in memory
			for (int y : mem) {
				edges.push_back(std::make_pair(y, x));
			}
			mem.push_back(x);
			//for (auto j : mem)
			//	cout << j << ", ";
			//cout << endl;
					
			for (int j = 0; j < n; j++) {
				G[j][x] = 0; // x has no parent anymore
			} 
			for (int i = mem.size() - 1; i >= 0; i--) {
				int im = mem[i];
				if (no_child(im, G))
					mem.erase(mem.begin() + i);
			}
		}
	}

	void Instance::dprint() const {
		std::vector<std::vector<int>> C(n);
		for (auto e : edges) {
			C[e.first].push_back(e.second);
			C[e.second].push_back(e.first);
		}

		for (int i = 0; i < n; i++) {
			std::cout << "task " << i << " : weight = " << weights[i];
			if (C[i].size() == 0) {
				std::cout << std::endl;
				continue;
			}

 			std::cout << ", No overlap w/ = ";
			for (auto x : C[i])
				std::cout << x << " ";
			std::cout << std::endl;
		}
	}


	// MEMORY
	int Memory::add_mem(int id, int start, int weight) {
		int end = start + weight;
		for (auto m : vmem) {
			// minsert' start is inside m
			if (!(end <= m.start || m.end <= start)) {
				std::cout << "task " << id << " : [" << start << ", " << end << "] overlaps with task " << m.id << " [" << m.start << ", " << m.end << "]" << std::endl;
				return 0;
			}
		}
		vmem.push_back(mem(id, start, end));
		return 1;
	}

	void Memory::print(int n) {
		std::vector<int> mm(n, -1);
		for (auto m : vmem)
			for (int i = m.start; i < m.end; i++)
				mm[i] = m.id;

		std::cout << "Memory : ";
		for (int i = 0; i < n; i++)
			if (mm[i] != -1)
				std::cout << mm[i];
			else
				std::cout << "_";

		std::cout << std::endl;
	}

	void Memory::remove_mem(int id) {
		for (unsigned int i = 0; i < vmem.size(); i++) {
			if (vmem[i].id == id) {
				vmem.erase(vmem.begin() + i);
				return;
			}
		}
	}

		
	// SOLUTION
	Solution::Solution(std::vector<int> const& offsets) : n(offsets.size()), offsets(offsets) {}
	Solution::Solution() : n(0), offsets({}) {}

	void Solution::dprint() const {
		for (int i = 0; i < n; i++)
			std::cout << "task " << i << " @ " << offsets[i] << std::endl;
	}
	
	int Solution::check(Input const& in, std::vector<int> const& order, int vv) const {
		std::vector<int> completed(n, 0); // 1 if task is done
		auto G = in.adj_matrix();
		Memory M; // custom class

		int sum = 0;
		for (int i = 0; i < n; i++) {
			int j = in.weights[i] + offsets[i];
			if (j > sum)
				sum = j;
		}


		for (int t : order) {
			if (vv == 1)
				std::cout << std::endl << "Doing " << t << std::endl;
			// CHECK : no uncompleted parents
			for (int i = 0; i < n; i++) {
				if (G[i][t] && !completed[i]) // i -> t
					return -1; // order error
			}
			// Add to memory
			if (!M.add_mem(t, offsets[t], in.weights[t]))
				return -2; // offset error
			completed[t] = 1;

			// for all tasks in memory, remove if all childs are completed
			std::vector<int> toremove;
			if (vv == 1) 
				std::cout << "in memory : ";
			for (auto m : M.vmem) {
				if (vv == 1) 
					std::cout << m.id << ", ";
				int needed = 0;
				for (int j = 0; j < n; j++) {
					// if m has a uncompleted child
					if (G[m.id][j] && !completed[j]) {
						needed = 1;
						break;
					}
				}
				if (!needed)
					toremove.push_back(m.id);
			}
			if (vv == 1) {
				std::cout << std::endl;
				M.print(sum);
				std::cout << "removed : ";
			}
			for (int i : toremove) {
				if (vv == 1) 
					std::cout << i << ", ";
				M.remove_mem(i);
			}
			if (vv == 1) 
				std::cout << std::endl;
		}
		for (int i = 0; i < n; i++) {
			if (!completed[i]) {
				return -3; // order error
			}
		}
		int maxmem = 0;
		for (int i = 0; i < n; i++) {
			int c = offsets[i] + in.weights[i];
			if (c > maxmem)
				maxmem = c;
		}

		return maxmem; // solution' score
	}

	
	// AUX functions
	
	int no_child(int x, std::vector<std::vector<int>> const& G) {
		int n = G.size();
		for (int i = 0; i < n; i++)
			if (G[x][i])
				return 0;
		return 1;
	}
}
