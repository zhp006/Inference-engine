#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include <fstream>
#include <sstream>

using namespace std;


struct Predicate{
	vector<string> parameters;
	bool negated;
	bool premise;
	bool conclusion;

	Predicate()
	:parameters(vector<string>()), negated(false), premise(false), conclusion(false)
	{
	}

	Predicate(vector<string> parameters, bool negated)
	{
		parameters = parameters;
		negated    = negated;
	}
};

struct Sentence{
	bool singleLiteral;
	vector<Predicate*> premise;
	Predicate* conclusion;
	//Debug purpoes
	string rawFact;

	Sentence()
	:singleLiteral(false)
	{}
};


class KB{
	public:
		vector<string> queries;
		vector<string> rawFacts;	

		unordered_map<string, unordered_map<string,vector<Sentence*>>> table;

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

		void processSingleLiteral(string rawFact, string originSentence)
		{
			/* extract everything between the prentheses */
			auto first_index = rawFact.find('(');
			auto second_index = rawFact.find(')');
			string rawParameters = rawFact.substr(first_index+1, second_index - first_index - 1);

			vector<string> params;
			stringstream ss (rawParameters);
			string param;
			while(getline(ss, param, ','))
				params.push_back(param);

			/* end of parsing */

			auto sentence = new Sentence();
			sentence->singleLiteral = true;
			sentence->conclusion = new Predicate();
			sentence->conclusion->conclusion = true;
			sentence->conclusion->negated = rawFact[0] == '~';
			sentence->conclusion->parameters = params;
			sentence->rawFact = originSentence;

			string predicate = rawFact[0] == '~' ? rawFact.substr(1, first_index - 1) : rawFact.substr(0, first_index) ;
			string sign = rawFact[0] == '~' ?  "Negative" : "Positive";
			table[predicate][sign].push_back(sentence);

		}
		void rawFactToTable(string rawFact)
		{
			//first check if is a single literal by looking for the "=>"
			string implication = "=>";
			if(rawFact.find(implication) != string::npos)
			{
				//split the premise from the conclusion
				auto split = rawFact.find(implication);
				string premises = rawFact.substr(0, split - 1);
				string conclusion = rawFact.substr(split+3);

				/* Process premises */
				string premise;
				stringstream ss(premises);
				if(premises.find("&") != string::npos)
				{
					while(getline(ss, premise, '&'))
					{
						//trim off the lead whitespace
						if(premise[0] == ' ')
							premise = premise.substr(1);
						processSingleLiteral(premise, rawFact);
					}
						
				}
				else
				{
					ss >> premise;
					processSingleLiteral(premise, rawFact);
				}
					
				/* Process conclusion */
				processSingleLiteral(conclusion, rawFact);
			}
			else
				processSingleLiteral(rawFact, rawFact);
				
		}

		void initialize()
		{
			for(auto fact : rawFacts)
				rawFactToTable(fact);
		}
		
};


int main()
{
	KB kb;

	kb.parse("input.txt");
	kb.initialize();

	auto results = kb.table["Learn"]["Positive"];	
	for(auto r : results)
		cout << r->rawFact << endl;

	return 0;
}