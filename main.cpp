#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <utility>
#include <chrono>

#include <time.h>

#include "solvers/glucose/glucose.h"
#include "solvers/gecode/gecode.h"
#include "mem_schedule/utils.h"

using namespace std;

mem_schedule::Input load_file(string fname) {
	if (fname == "")
		throw -1;
	ifstream f;
	f.open(fname);//, ios::in);
	if (!f.is_open())
		throw -1;
	std::string line;
	getline(f, line);
	std::istringstream iss(line);
	int N;
	if (!(iss >> N) || N < 1)
		throw -1;

	std::vector<int> weights(N);
	for (int i = 0; i < N; i++) {
		getline(f, line);
		std:: istringstream iss(line);
		iss >> weights[i];
	}

	std::vector<std::pair<int, int>> edges;
	while (getline(f, line)) {
		std:: istringstream iss(line);
		int x, y;
		iss >> x >> y;
		edges.push_back(std::make_pair(x, y));
	}
	f.close();
	return mem_schedule::Input(edges, weights);
}

mem_schedule::Input load_input(int n, std::string filename) {
	if (n == -1) {
		n = 5;
	}

	if (filename == "")
		return mem_schedule::Input(n);		

	try {
		return load_file(filename);
		//Ins = load_file(filename);
		// TODO add file loading
	} catch (...) {
		cout << "failed to load " << filename << endl;
	}
	return mem_schedule::Input(n);		
}

int parseInt(std::string s, int bydefault = -1) {
	int x = bydefault;
	try {
		x = std::stoi(s);
	} catch (...) {
		return bydefault;
	}
	return x;
}

int main(int argc, char** argv) {
	vector<string> args(argv + 1, argv + argc); // skip first arg = prog name
	string filename = "";
	int n = -1;
	int Isolver = 0;
	int useIncre = 0;
	int vv = 0;
	argc = args.size();
	for (int i = 0; i < argc; i++) {
		if (args[i] == "-h") {
			std::cout << "usage : (input file) (-n [random input size])" << std::endl;
			std::cout << "-I (0/1) use incremental (GLUCOSE)" << std::endl;
			std::cout << " -g use gecode" << std::endl;
			std::cout << "-d debug\n"
				<< "-p progress show\n";
			return 0;
		}
		std::string cmd = args[i];
		// no param commands
		if (cmd == "-g") {
			//std::cout << "Using Gecode" << std::endl;
			Isolver = 1;
			continue;
		}
		if (cmd == "-p") {
			vv = 2;
			continue;
		}
		if (cmd == "-d") {
			vv = 1;
			continue;
		}
		
		// one param commands
		i++;
		if (i >= argc) {
			std::cout << "Bad command probably, use -h for help" << std::endl;
			return 0;
		}
		std::string x = args[i];
		if (cmd == "-n") {
			n = parseInt(x, -1);
		} else if (cmd == "-f") {
			filename = x;
		} else if (cmd == "-I") {
			useIncre = parseInt(x, 0);
		}
	}

	//srand(111);
	srand(time(NULL));
	mem_schedule::Input in = load_input(n, filename);
	if (vv == 1) {
		in.adjprint();
		in.dprint();
		in.statprint();
	}
	auto order = in.computeOrder();
	
	if (vv == 1) {
		std::cout << "Order = ";
		for (auto i : order)
			std::cout << i << " ";
		std::cout << std::endl;
	}

	mem_schedule::Instance ins(in, order);
	if (vv == 1)
		ins.dprint();

	if (vv == 1)
		std::cout << std::endl << std::endl << "Solving : " << std::endl;
	mem_schedule::Solution sol;

	//auto starttime = std::chrono::high_resolution_clock::now(); // usertime
	std::clock_t c_starttime = std::clock();
	if (Isolver) {
		sol = gecode_memSchedule::solve(in, ins, vv, c_starttime);
	} else {
		if (useIncre == 1) {
			if (vv == 1)
				std::cout << "Incremental glucose" << std::endl;
			sol = glucose_memSchedule::solve_inc(in, ins, vv, c_starttime);
		} else if (useIncre > 1) {
			sol = glucose_memSchedule::solve(ins, vv, c_starttime, useIncre);
		} else {
			sol = glucose_memSchedule::solve(ins, vv, c_starttime, 0);
		}
	}
	if (useIncre > 1) { //big special mode
		std::clock_t c_endtime = std::clock();
		auto c_elapsed = 1000.0 * (c_endtime-c_starttime) / CLOCKS_PER_SEC;
		std::cout << std::fixed << std::setprecision(2);
		if (sol.n == ins.n)
			std::cout << "1," << c_elapsed << std::endl;
		else
			std::cout << "0," << c_elapsed << std::endl;
		return 0;
	}
	if (sol.n == ins.n) {
		if (vv == 1) {
			std::cout <<std::endl<< "Solution:" << std::endl;
			sol.dprint();
			std::cout << std::endl << std::endl << "Checking: " << std::endl;
		}

		int score = sol.check(in, order, vv);
		//auto endtime = std::chrono::high_resolution_clock::now();
		std::clock_t c_endtime = std::clock();
		auto c_elapsed = 1000.0 * (c_endtime-c_starttime) / CLOCKS_PER_SEC;
    //auto elapsed = std::chrono::duration<double, std::milli>(endtime -starttime).count();
		if (score > 0) {
			if (vv == 1) {
				std::cout << "score = " << score << std::endl;
				std::cout << "score/cputime/wallclocktime" << std::endl;
			}
			std::cout << std::fixed << std::setprecision(2);
			std::cout << score << "," << c_elapsed << std::endl; //<< "," << elapsed  << std::endl;
		} else {
			std::cout << "Failed check = " << score << std::endl;
		}
	} else {
		if (vv == 1) {
			std::cout << "No solution" << std::endl;
			std::cout << sol.n << " =/= " << ins.n << std::endl;
		}
	}
	// TODO use exception to tell why the check failed?

	return 0;
}

