#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <fstream>

using namespace std;

struct Predicate{
	set<string> parameters;
	bool negated;

	Predicate()
	:parameters(set<string>()), negated(false)
	{
	}

	Predicate(set<string> parameters, bool negated)
	{
		parameters = parameters;
		negated    = negated;
	}
};


class KB{
	public:
		vector<string> queries;
		vector<string> rawFacts;	

		void parse(string file)
		{
			ifstream input;
			input.open(file);

			string line;

			// get the number of queries
			int numberOfQuery;
			getline(input, line);
			numberOfQuery = stoi(line);

			while(numberOfQuery--)
			{
				getline(input, line);
				queries.push_back(line);
			}

			//get the number of facts
			int numberOfFact;
			getline(input, line);
			numberOfFact = stoi(line);

			while(numberOfFact--)
			{
				getline(input, line);
				rawFacts.push_back(line);
			}
		}
		
};


int main()
{
	KB kb;

	kb.parse("input.txt");

	cout << "_queries: " << endl;
	for(auto r : kb.queries)
		cout << r << endl;

	cout << "_KB: " << endl;
	for(auto r : kb.rawFacts)
		cout << r << endl;

	return 0;
}