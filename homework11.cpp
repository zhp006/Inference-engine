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
	//Debug purpose
	string rawFact;

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
	vector<Predicate*> premises;
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
		unordered_map<string, Sentence*> rawTable;

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

		void processSingleLiteral(string rawFact, string originSentence, bool isConclusion)
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


			/* if this literal belongs to the premises */
			if(!isConclusion)
			{
				if(!rawTable.count(originSentence))
				{
					auto sentence = new Sentence();
					rawTable[originSentence] = sentence;
				}

				rawTable[originSentence]->rawFact = originSentence;
				auto premiseLiteral = new Predicate();
				premiseLiteral->rawFact = originSentence;
				premiseLiteral->premise = true;
				premiseLiteral->parameters = params;
				premiseLiteral->negated = rawFact[0] == '~';
				rawTable[originSentence]->premises.push_back(premiseLiteral);
			}
			/* if this literal belongs to the conclusion */
			else
			{
				if(!rawTable.count(originSentence))
				{
					auto sentence = new Sentence();
					rawTable[originSentence] = sentence;
					sentence->singleLiteral = true;
				}

				rawTable[originSentence]->rawFact = originSentence;
				auto conclusionLiteral = new Predicate();
				rawTable[originSentence]->conclusion = conclusionLiteral;
				conclusionLiteral->negated = rawFact[0] == '~';
				conclusionLiteral->parameters = params;
				conclusionLiteral->conclusion = true;
				conclusionLiteral->rawFact = originSentence;
			}

			string predicate = rawFact[0] == '~' ? rawFact.substr(1, first_index - 1) : rawFact.substr(0, first_index) ;
			string sign = rawFact[0] == '~' ?  "Negative" : "Positive";
			table[predicate][sign].push_back(rawTable[originSentence]);

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
						processSingleLiteral(premise, rawFact, false);
					}
						
				}
				else
				{
					ss >> premise;
					processSingleLiteral(premise, rawFact, false);
				}
					
				/* Process conclusion */
				processSingleLiteral(conclusion, rawFact, true);
			}
			else
				processSingleLiteral(rawFact, rawFact, true);
				
		}

		void initialize()
		{
			for(auto fact : rawFacts)
				rawFactToTable(fact);
		}

		vector<Sentence*> fetch(string predicate, string sign)
		{
			return table[predicate][sign];
		}
		
};


int main()
{
	KB kb;

	kb.parse("input.txt");
	kb.initialize();

	auto results = kb.fetch("Ready","Negative");	
	// for(auto r : results)
	// 	cout << r->rawFact << endl;

	for(auto p : results.back()->premises)
	{
		for(auto param : p->parameters)
			cout << param << " ";
		cout << endl;
	}


	return 0;
}