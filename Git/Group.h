/**
	Group: calculates and stores group statistics
	@file Group.h
	@author Hubert.Wang
	Last Modified: 12/09/21
*/

#pragma once
#include <iostream>
#include <cstdlib>
#include <map>
#include <vector>
#include <string>
#include "Stock.h"

using namespace std;

class Group
{
protected: // Inherited class can direct access
	int n_days;
	vector<string> Beat;
	vector<string> Meet;
	vector<string> Miss;
	vector<Matrix> Table_Matrix;  // Day-Group-XAR ((2N+1)*3*4)
public:
	// Constructor
	Group() = default;
	Group(const vector<string>& ticker_vec, const Vector& earning_vec, const int& n, map<string, Stock*>& stock_map);
	// Bootstrapping
	void calculateTable(const map<string, Stock*>& stock_map, int step_size);
	// Getters
	int getSize() const { return n_days; }
	vector<string> getBeat() const { return Beat; }
	vector<string> getMeet() const { return Meet; }
	vector<string> getMiss() const { return Miss; }
	vector<Matrix> getMatrix() const { return Table_Matrix; }
};

class Sub_Group : public Group
{
private:
	int select_size = 80;
public:
	// Constructor (One shot of bootstrapping)
	Sub_Group(const Group* major_group);
	void calculateTable(const map<string, Stock*>& stock_map);
};