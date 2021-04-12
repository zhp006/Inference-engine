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
	string name;
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
	vector<Predicate*> allPredicates;
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
				premiseLiteral->name = rawFact[0] == '~' ? rawFact.substr(1, first_index - 1) : rawFact.substr(0, first_index) ;
				premiseLiteral->rawFact = originSentence;
				premiseLiteral->premise = true;
				premiseLiteral->parameters = params;
				premiseLiteral->negated = rawFact[0] != '~';
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
				conclusionLiteral->name = rawFact[0] == '~' ? rawFact.substr(1, first_index - 1) : rawFact.substr(0, first_index) ;
				conclusionLiteral->negated = rawFact[0] == '~';
				conclusionLiteral->parameters = params;
				conclusionLiteral->conclusion = true;
				conclusionLiteral->rawFact = originSentence;
			}

			string predicate = rawFact[0] == '~' ? rawFact.substr(1, first_index - 1) : rawFact.substr(0, first_index) ;
			string sign = "";
			if(isConclusion)
				sign = rawFact[0] == '~' ? "Negative" : "Positive";
			else
				sign = rawFact[0] == '~' ? "Positive" : "Negative";

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


			//put premises and conclusion together
			for(auto s : rawTable)
			{
				s.second->allPredicates = s.second->premises;
				s.second->allPredicates.push_back(s.second->conclusion);
			}
				
		}

		void initialize()
		{
			for(auto fact : rawFacts)
				rawFactToTable(fact);
		}

		vector<Predicate*> fetch(string predicate, string sign)
		{
			auto sentences = table[predicate][sign];
			vector<Predicate*> result;
			for(auto sentence : sentences)
			{
				if(!sentence->premises.empty())
				{
					for(auto p : sentence->premises)
					{
						if(p->name == predicate)
							result.push_back(p);
					}
				}

				if(sentence->conclusion->name == predicate)
					result.push_back(sentence->conclusion);
			}

			return result;
		}
		
		vector<Sentence*> fetchSentence(string predicate, string sign)
		{
			return table[predicate][sign];
		}

		bool unify(Sentence* sentence, Predicate* predicate, int index)
		{
			unordered_map<string, string> lookup;
			auto targetParams = sentence->allPredicates[index]->parameters;
			auto currentParams = predicate->parameters;
			for(int i = 0; i < targetParams.size(); i++)
			{
				//if(currentParams[i].size() == 1 && is)
			}

			return true;
		}
	

		/*----------------------------------------------DEBUG FUNCTIONS---------------------------------------------- */
		void printSentences(vector<Predicate*> results)
		{
			for(auto r : results)
				cout << r->rawFact << endl;
		}
		/*----------------------------------------------DEBUG FUNCTIONS---------------------------------------------- */
};


int main()
{
	KB kb;

	kb.parse("input.txt");
	kb.initialize();

	//auto results = kb.fetch("Learn","Negative");
	auto results = kb.fetchSentence("Learn", "Negative");
	for(auto p : results.front()->allPredicates)
		cout << p->name << " ";
	cout << endl;
	//kb.printSentences(results);


	return 0;
}