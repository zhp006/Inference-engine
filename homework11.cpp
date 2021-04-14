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
			{
				if(param[0] == ' ')
					param = param.substr(1);
				params.push_back(param);
			}
				

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

		bool isVariable(string s)
		{
			if(s.size() == 1)
			{
				char c = s[0];
				if(islower(c))
					return true;
			}
			return false;
		}
		bool unify(Sentence* sentence, Predicate* predicate, int index)
		{
			unordered_map<string, string> lookup;
			auto targetParams = sentence->allPredicates[index]->parameters;
			auto currentParams = predicate->parameters;

			//build the look up table that store the variable and its correponding value
			for(int i = 0; i < targetParams.size(); i++)
			{
				//if they have different constant in the same position then can't be unified
				if(!isVariable(currentParams[i]) && !isVariable(targetParams[i]) && currentParams[i] != targetParams[i])
					return false;
				
				if(isVariable(currentParams[i]) && !isVariable(targetParams[i]))
					lookup[currentParams[i]] = targetParams[i];
				else if(isVariable(targetParams[i]) && !isVariable(currentParams[i]))
					lookup[targetParams[i]] = currentParams[i];
					
			}


			for(auto p : sentence->allPredicates)
			{
				for(int i = 0; i < p->parameters.size(); i++)
				{
					if(isVariable(p->parameters[i]))
						p->parameters[i] = lookup[p->parameters[i]];
				}
			}
			return true;
		}
	
		bool ask(Predicate* predicate, string sign)
		{
			/*-------------------BASE CASE-------------------*/
			
			//Same sign
			auto posSentence = fetchSentence(predicate->name, sign);
			for(auto result : posSentence)
			{
				if(result->allPredicates.size() == 1 && result->allPredicates[0]->name == predicate->name && result->allPredicates[0]->negated == predicate->negated &&
				result->allPredicates[0]->parameters == predicate->parameters)
					return true;
			}

			string tmpSign = sign == "Positive" ? "Negative" : "Positive";
			auto negSentence = fetchSentence(predicate->name, tmpSign);
			for(auto result : negSentence)
			{
				if(result->allPredicates.size() == 1 && result->allPredicates[0]->name == predicate->name && result->allPredicates[0]->negated != predicate->negated &&
				result->allPredicates[0]->parameters == predicate->parameters)
					return false;
			}

			/*-------------------BASE CASE-------------------*/
			auto results = fetchSentence(predicate->name, sign);
			if(sign == "Positive")
				predicate->negated = true;
			else
				predicate->negated = false;

			for(auto result : results)
			{
				for(int i = 0; i < result->allPredicates.size(); i++)
				{
					if(result->allPredicates[i]->name == predicate->name)
					{
						auto origin = result->allPredicates;
						if(unify(result, predicate, i))
						{
							//start to resolve
							
						}
					}
				}
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


	Predicate* test = new Predicate();
	test->name = "Ready";
	test->negated = false;
	test->parameters.push_back("Ares");
	test->parameters.push_back("Bres");

	cout << "test direct truth that negated the query" << endl;
	cout << kb.ask(test, "Positive");
	//auto results = kb.fetch("Learn","Negative");
	// auto results = kb.fetchSentence("Learn", "Negative");
	// for(auto p : results.back()->allPredicates)
	// {
	// 	cout << p->name << " ";
	// 	for(auto param : p->parameters)
	// 		cout << param << " ";
	// 	cout << endl;
	// }

	// cout << "---------------" << endl;
	// Predicate* tmp = new Predicate;
	// tmp->name = "Ready";
	// tmp->parameters.push_back("first");
	// tmp->parameters.push_back("abc");
	// kb.unify(results.back(), tmp, 1);


	// for(auto p : results.back()->allPredicates)
	// {
	// 	cout << p->name << " ";
	// 	for(auto param : p->parameters)
	// 		cout << param << " ";
	// 	cout << endl;
	// }

	return 0;
}