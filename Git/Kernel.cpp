/**
	Stock: serves as a virtual machine for the main function
	@file Kernel.cpp
	@author Hubert.Wang
	Last Modified: 12/08/21
*/
#include "Kernel.h"
#include <iomanip>

void Kernel::PullStockInfo()
{
	// read data from file
	populateVectors(raw_ticker, raw_zero_date, raw_ending, raw_estimated, raw_reported, raw_surprise, raw_percent);
	// an additional vector at the end for iwb
	raw_ticker.emplace_back("IWB");
	raw_zero_date.emplace_back("Kimi, you will not have the drink."); // a placeholder
	// pull Data
//    getPricesData(raw_ticker, raw_zero_date, raw_zero_index, raw_Prices, raw_Dates);
	getPricesDataMultiThread(raw_ticker, raw_zero_date, raw_zero_index, raw_Prices, raw_Dates, NUMT);
	// kernel now reach level 1
	kernel_state = 1;
}

Kernel::~Kernel()
{
	for (auto p : stock_map)
	{
		if (p.second != nullptr)
		{
			delete p.second;
			p.second = nullptr;
		}
	}
}

void Kernel::PickNDateStock()
{
	// First clear everything beyond level 1;
	for (auto p : stock_map)
	{
		if (p.second != nullptr)
		{
			delete p.second;
			p.second = nullptr;
		}
	}
	stock_map.clear();
	//	kernel_state = 1;

	int n_max = raw_ticker.size() - 1;
	// Prepare iwb_Returns for further use
	Vector iwb_Returns{ 0. };
	for (int di = 1; di < raw_Prices[n_max].size(); di++)
	{
		iwb_Returns.emplace_back(raw_Prices[n_max][di] / raw_Prices[n_max][di - 1] - 1.);
	}
	// initialize some intermediary vectors needed here
	vector<string> refined_ticker;
	vector<double> refined_percent;
	// Keep in mind that the last element for raw_ticker, raw_zero_date, raw_zero_index, raw_Prices, raw_Dates will be IWB
	// but not for raw_estimated, raw_reported, raw_surprise, raw_percent
	for (int si = 0; si < n_max; ++si)
	{
		// Check data integrity
		int start_index = raw_zero_index[si] - N;
		int end_index = raw_zero_index[si] + N;
		if ((start_index > 0) && (end_index < raw_Dates[si].size()))
		{
			// Valid ticker
			string current_ticker = raw_ticker[si];
			refined_ticker.emplace_back(current_ticker);
			refined_percent.emplace_back(raw_percent[si]);
			// Prepare vectors for Stock objects
			vector<string> current_date_vec;
			Vector current_price_vec;
			Vector current_iwb_vec;
			for (int di = start_index; di <= end_index; ++di)
			{
				current_date_vec.emplace_back(raw_Dates[si][di]);
				current_price_vec.emplace_back(raw_Prices[si][di]);
				current_iwb_vec.emplace_back(iwb_Returns[di]);
			}
			// Populate stock map
			stock_map[current_ticker] = new Stock(raw_zero_date[si], raw_ending[si],
				raw_estimated[si], raw_reported[si], raw_surprise[si], raw_percent[si]);
			stock_map[current_ticker]->setDate(current_date_vec);
			stock_map[current_ticker]->setPrice(current_price_vec);
			stock_map[current_ticker]->setBenchmarkR(current_iwb_vec);
			stock_map[current_ticker]->calculateXR();
		}
	}
	stock_group = Group(refined_ticker, refined_percent, N, stock_map);
	// kernel now reach level 2
	kernel_state = 2;
}


void Kernel::ShowStockInfo(string ticker)
{
	if (kernel_state == 0)
	{
		PullStockInfo();
		PickNDateStock();
	}
	auto itr = stock_map.find(ticker);
	if (itr == stock_map.end())
	{
		cout << "Could not find the stock" << endl;
		return;
	}
	Stock* stock = itr->second;
	Vector current_Prices = stock->getPrice();
	Vector current_CR = stock->getCR();
	cout << "Daily Prices" << endl;
	cout << current_Prices << endl;
	cout << "Cumulative Daily Returns" << endl;
	cout << current_CR << endl;
	string group_name[3] = { "Beat", "Meet", "Miss" };
	cout << "Group: " << group_name[stock->getGroup()] << endl;
	cout << "Announcement Date: " << stock->getAnnouncementDate() << endl;
	cout << "Period Ending: " << stock->getPeriodEnding() << endl;
	cout << "Estimated EPS: " << stock->getEstimatedEPS() << endl;
	cout << "Reported EPS: " << stock->getReportedEPS() << endl;
	cout << "Surprise: " << stock->getSurprise() << endl;
	cout << "Percent Surprise: " << stock->getPercentSurprise() << endl;
}


void Kernel::ShowMatrix(int GroupID, int N)
{
	if (kernel_state == 2)
	{
		auto start = high_resolution_clock::now();
		stock_group.calculateTable(stock_map, 40);
		auto stop = high_resolution_clock::now();

		// Get duration. Substart timepoints to
		// get durarion. To cast it to proper unit
		// use duration cast method
		auto duration = duration_cast<microseconds>(stop - start);
		cout << "Time taken by function: "
			<< duration.count() / 1000000 << " seconds" << endl;
		kernel_state = 3;
	}
	else if (kernel_state < 3)
	{
		cout << "Download Data first" << endl;
		PullStockInfo();
		PickNDateStock();
		stock_group.calculateTable(stock_map, 40);
		kernel_state = 3;
		// Send user an error and bring them back to the main menu
	}
	//cout << stock_group.getMatrix().size() << endl;
	string group_name[3] = { "Beat", "Meet", "Miss" };
	cout << "Group: " << group_name[GroupID] << endl<<endl;
	cout << " Day  "<<" Avg AAR  " << " AAR-STD  " << " Avg CAAR  " << " CAAR-STD  " << endl;
	for (int i = 0; i < 2*N+1; i++)
	{
		cout <<" "<< i - N <<" " <<fixed<<setprecision(6) << stock_group.getMatrix()[i][GroupID] << endl;
	}
}

void plotResults(double* xData, double* yData, double* yData2, double* yData3, int dataSize);
void Kernel::ShowPlot()
{
	if (kernel_state == 2)
	{
		stock_group.calculateTable(stock_map, 40);
		kernel_state = 3;
	}
	else if (kernel_state < 3)
	{
		cout << "Download Data first" << endl;
		PullStockInfo();
		PickNDateStock();
		stock_group.calculateTable(stock_map, 40);
		kernel_state = 3;
		// Send user an error and bring them back to the main menu
	}
	//int i = 0;
	int nIntervals = 2 * N + 1;
	double intervalSize = 1.0;
	double stepSize = intervalSize / nIntervals;
	double* xDate = (double*)malloc((nIntervals + 1) * sizeof(double));
	double* yBeat = (double*)malloc((nIntervals + 1) * sizeof(double));
	double* yMeet = (double*)malloc((nIntervals + 1) * sizeof(double));
	double* yMiss = (double*)malloc((nIntervals + 1) * sizeof(double));
	for (int i = 0; i < nIntervals; i++)
	{
		xDate[i] = i - N;
		yBeat[i] = (stock_group.getMatrix()[i][0][2] - stock_group.getMatrix()[N][0][2]) * 100;
		yMeet[i] = (stock_group.getMatrix()[i][1][2] - stock_group.getMatrix()[N][1][2]) * 100;
		yMiss[i] = (stock_group.getMatrix()[i][2][2] - stock_group.getMatrix()[N][2][2]) * 100;
	}

	plotResults(xDate, yBeat, yMeet, yMiss, nIntervals);
}

void plotResults(double* xData, double* yData, double* yData2, double* yData3, int dataSize) {
	FILE* gnuplotPipe, * tempDataFile;
	const char* tempDataFileName1 = "CAAR_Beat";
	const char* tempDataFileName2 = "CAAR_Meet";
	const char* tempDataFileName3 = "CAAR_Miss";
	double x, y, x2, y2, x3, y3;
	int i;
	//gnuplotPipe = popen("/opt/local/bin/gnuplot", "w");       //for MAC
	gnuplotPipe = _popen("gnuplot.exe", "w");            //for Windows
	if (gnuplotPipe) {
		tempDataFile = fopen(tempDataFileName1, "w");
		for (i = 0; i < dataSize; i++) {
			x = xData[i];
			y = yData[i];
			fprintf(tempDataFile, "%lf %lf\n", x, y);
		}
		fclose(tempDataFile);
		tempDataFile = fopen(tempDataFileName2, "w");
		for (i = 0; i < dataSize; i++) {
			x2 = xData[i];
			y2 = yData2[i];
			fprintf(tempDataFile, "%lf %lf\n", x2, y2);
		}
		fclose(tempDataFile);
		tempDataFile = fopen(tempDataFileName3, "w");
		for (i = 0; i < dataSize; i++) {
			x3 = xData[i];
			y3 = yData3[i];
			fprintf(tempDataFile, "%lf %lf\n", x3, y3);
		}
		fclose(tempDataFile);
		fprintf(gnuplotPipe, "set arrow from 0,graph(0,0) to 0,graph(1,1) nohead lc rgb 'red' dt '-'\n");
		fprintf(gnuplotPipe, "set xlabel 'Announcement Date'\nset ylabel 'CAAR(%%)'\n");
		fprintf(gnuplotPipe, "plot \"%s\" with lines, \"%s\" with lines, \"%s\" with lines\n", tempDataFileName1, tempDataFileName2, tempDataFileName3);
		fflush(gnuplotPipe);
		printf("press enter to continue...");
		getchar();
		getchar();
		remove(tempDataFileName1);
		remove(tempDataFileName2);
		remove(tempDataFileName3);
		fprintf(gnuplotPipe, "exit \n");
		_pclose(gnuplotPipe);
		//pclose(gnuplotPipe);
	}
	else {
		printf("gnuplot not found...");
	}
}