/**
	Group: calculates and stores group statistics
	@file Group.cpp
	@author Hubert.Wang
	Last Modified: 12/09/21
*/
#include "Group.h"
#include "Matrix.h"
#include <algorithm>
#include <random>
#include <chrono>

Group::Group(const vector<string>& ticker_vec, const Vector& earning_vec, const int& n, map<string, Stock*>& stock_map)
{
	n_days = 2 * n + 1;
	vector<pair<double, string>> ranking_vec;
	for (int i = 0; i < ticker_vec.size(); i++)
	{
		ranking_vec.emplace_back(earning_vec[i], ticker_vec[i]);
	}
	sort(ranking_vec.begin(), ranking_vec.end());
	int break_a = round(ticker_vec.size() / 3);
	int break_b = round(ticker_vec.size() * 2 / 3);
    for (int i = 0; i < break_a; i++)
    { Miss.emplace_back(ranking_vec[i].second);
      stock_map[ranking_vec[i].second]->setGroup(2); }
    for (int i = break_a; i < break_b; i++)
    { Meet.emplace_back(ranking_vec[i].second);
      stock_map[ranking_vec[i].second]->setGroup(1); }
    for (int i = break_b; i < ticker_vec.size(); i++)
    { Beat.emplace_back(ranking_vec[i].second);
      stock_map[ranking_vec[i].second]->setGroup(0); }
}

Sub_Group::Sub_Group(const Group* major_group)
{	
	n_days = major_group -> getSize();
	vector<string> temp_miss = major_group->getMiss();
	vector<string> temp_meet = major_group->getMeet();
	vector<string> temp_beat = major_group->getBeat();
	// Shuffle; use a different seed for each shot
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	shuffle(temp_miss.begin(), temp_miss.end(), default_random_engine(seed));
	seed = std::chrono::system_clock::now().time_since_epoch().count();
	shuffle(temp_meet.begin(), temp_meet.end(), default_random_engine(seed));
	seed = std::chrono::system_clock::now().time_since_epoch().count();
	shuffle(temp_beat.begin(), temp_beat.end(), default_random_engine(seed));
	// Select the first select_size elements
	for (int i = 0; i < select_size; i++) Miss.emplace_back(temp_miss[i]);
	for (int i = 0; i < select_size; i++) Meet.emplace_back(temp_meet[i]);
	for (int i = 0; i < select_size; i++) Beat.emplace_back(temp_beat[i]);
	// 	Finally, initialize a Table_Matrix full of zeros for further use
	Vector zero_vec(4, 0.);
	Matrix zero_matrix;
	for (int mi = 0; mi < 3; mi++)
	{
		zero_matrix.emplace_back(zero_vec);
	}
	for (int di = 0; di < n_days; di++)
	{
		Table_Matrix.emplace_back(zero_matrix);
	}
}

void Group::calculateTable(const map<string, Stock*>& stock_map, int step_size)
{
	for (int si = 0; si < step_size; si++)
	{
		double fac = si;
		cout << "Bootstrapping: Loop " << (si+1) << "/40 \r";
		// create a new sub-group
		Sub_Group random_group(this);
		random_group.calculateTable(stock_map);
		if (si == 0) // Initialize with the first shot
		{
			Table_Matrix = random_group.getMatrix();
		}
		else
		{
			// Calculate cumulative average for each element of the table
			for (int di = 0; di < Table_Matrix.size(); di++)
			{
				for (int mi = 0; mi < Table_Matrix[0].size(); mi++)
				{
					// Using overloaded operators for vectors
					Table_Matrix[di][mi] = (fac / (fac + 1)) * Table_Matrix[di][mi] + (double)(1 / (fac + 1)) * random_group.getMatrix()[di][mi];
				}
			}
		}
	}
	cout << endl;
	// Finally, use the sum of squares to calculate std
	for (auto& mat : Table_Matrix)
	{
		for (auto & vec : mat)
		{
			vec[1] = sqrt(vec[1] - pow(vec[0], 2.));
			vec[3] = sqrt(vec[3] - pow(vec[2], 2.));
		}
	}
}

void Sub_Group::calculateTable(const map<string, Stock*>& stock_map)
{
	// First populate columns 0 and 2
	for (int si = 0; si < Miss.size(); si++)
	{
		double fac = si;
		string ticker = Miss[si];
		// find the corresponding stock
		auto itr = stock_map.find(ticker);
		if (itr == stock_map.end()) continue; // go to next loop
		Vector current_AR = itr->second->getAR();
		Vector current_CAR = itr->second->getCAR();
		// Manually desginate each position 
		for (int di = 0; di < n_days; di++)
		{
			Table_Matrix[di][2][0] = (fac / (fac + 1)) * Table_Matrix[di][2][0] + (1 / (fac+1)) * current_AR[di];
			Table_Matrix[di][2][2] = (fac / (fac + 1)) * Table_Matrix[di][2][2] + (1 / (fac + 1)) * current_CAR[di];
		}
	}
	// Do the same for Meet...
	for (int si = 0; si < Meet.size(); si++)
	{
		double fac = si;
		string ticker = Meet[si];
		auto itr = stock_map.find(ticker);
		if (itr == stock_map.end()) continue;
		Vector current_AR = itr->second->getAR();
		Vector current_CAR = itr->second->getCAR();
		for (int di = 0; di < n_days; di++)
		{
			Table_Matrix[di][1][0] = (fac / (fac + 1)) * Table_Matrix[di][1][0] + (1/(fac + 1)) * current_AR[di];
			Table_Matrix[di][1][2] = (fac / (fac + 1)) * Table_Matrix[di][1][2] +  (1 / (fac+1))* current_CAR[di];
		}
	}
	//and Beat.
	for (int si = 0; si < Beat.size(); si++)
	{
		double fac = si;
		string ticker = Beat[si];
		auto itr = stock_map.find(ticker);
		if (itr == stock_map.end()) continue;
		Vector current_AR = itr->second->getAR();
		Vector current_CAR = itr->second->getCAR();
		for (int di = 0; di < n_days; di++)
		{
			Table_Matrix[di][0][0] = (fac / (fac + 1)) * Table_Matrix[di][0][0] + (1 / (fac + 1)) * current_AR[di];
			Table_Matrix[di][0][2] = (fac / (fac + 1)) * Table_Matrix[di][0][2] + (1 / (fac + 1)) * current_CAR[di];
		}
	}
	// Finally, populate columns 1 and 3 as the square of 0 and 2, respectively
	for (auto& mat : Table_Matrix)
	{
		for (auto& vec : mat)
		{
			vec[1] = pow(vec[0], 2.);
			vec[3] = pow(vec[2], 2.);
		}
	}

}
