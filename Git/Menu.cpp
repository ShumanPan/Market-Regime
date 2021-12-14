#include <iostream>
#include "Kernel.h"
#include <string>
#include "Stock.h"
#include <map>
using namespace std;

int main(void)
{
    // TODO: Make a menu
    int selection;
    int N;
    Kernel kernel;

    while (true)
    {
        cout << endl;
        cout << "Menu" << endl;
        cout << "========" << endl;
        cout << "1 - Enter N(60<=N<=200) to retrieve 2N+1 days of historical price data" << endl;
        cout << "2 - Pull information for one stock" << endl;
        cout << "3 - Show AAR, AAR-STD, CAAR and CAAR-STD for one group" << endl;
        cout << "4 - Show CAAR graph for all three groups" << endl;
        cout << "0 - Exit" << endl << endl;
        // if the input is not an integer
        if (!(cin >> selection))
        {
            cout << "Invalid Input, Please enter number 0-4" << endl;
            cin.clear();
            while (cin.get() != '\n')
                continue;
            continue;
        }

        switch (selection)
        {
        case 1:
        {
            cout << "N(60<=N<=200): ";
            cin >> N;
            cout << endl;

            if (N < 60 || N > 200)
            {
                cout << "N<60 or N>200, error" << endl;
                cout << "Go back to Menu..." << endl;
                break;
            }

            kernel.SetN(N);
            kernel.PullStockInfo();
            kernel.PickNDateStock();
            break;
        }

        case 2:
        {
            string ticker;
            cout << "Please enter the stock name: " << endl;
            cin >> ticker;
            cout << endl;
            kernel.ShowStockInfo(ticker);
            break;
        }

        case 3:
        {
            // TODO: Beautify the format
            cout << "Enter Group : 0 - Beat, 1 - Meet, 2 - Miss" << endl;
            int GroupID;
            cin >> GroupID;
            kernel.ShowMatrix(GroupID,N);
            break;
        }

        case 4:
        {
            // TODO: gnuplot for 3 groups
            kernel.ShowPlot();
            break;
        }

        case 0:
        {
            cout << "Exited Program" << endl;
            return 0;
        }

        default:
            cout << "Invalid Input" << endl;
            break;
        }
    }




}