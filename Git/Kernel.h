/**
	Stock: serves as a virtual machine for the main function
	@file Kernel.h
	@author Hubert.Wang
	Last Modified: 12/08/21
*/
#pragma once
#include <vector>
#include <string>
#include <iostream>
#include "Matrix.h"
#include "Stock.h"
#include "Group.h"
#include "DataInterface.h"
#include <chrono>

using namespace std;
using namespace std::chrono;

struct Kernel
{
	//string local_eps_file = "EarningsAnnouncements.csv";
	int kernel_state = 0;
	// Raw Data (Kernel State 1)
	// a set of vectors to be generated from earnings file
	vector<string> raw_ticker;
	vector<string> raw_zero_date;
	vector<string> raw_ending;
	vector<double> raw_estimated;
	vector<double> raw_reported;
	vector<double> raw_surprise;
	vector<double> raw_percent;
	// a set of vector-vector to be generated from API
	vector<int> raw_zero_index;
	vector<Vector> raw_Prices;
	vector<vector<string>> raw_Dates;
	// Processed Data (Kernel State 2)
	int N = 60;
	int NUMT = 20; // number of threads
	map<string, Stock*> stock_map;
	Group stock_group;

	Kernel() {};
	~Kernel();
	void SetN(int N_) { N = N_; };
	void PullStockInfo();
	void PickNDateStock();
	void ShowStockInfo(string ticker);
	void ShowMatrix(int GroupID,int N);
	void ShowPlot();

};