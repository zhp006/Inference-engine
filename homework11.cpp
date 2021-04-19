#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
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

		Sentence* fetchOnlyPredicateInSentence()
		{
			for(auto p : table)
			{
				for(auto innerTalbe : p.second)
				{
					for(auto sentence : innerTalbe.second)
					{
						if(sentence->allPredicates.size() == 1)
							return sentence;
					}
				}
			}

			return nullptr;
		}

		string stringify(Sentence* sentence)
		{
			string output = "";
			for(auto predicate : sentence->allPredicates)
			{
				string sign = predicate->negated ? "~" : "";
				output += sign + predicate->name + "(";
				for(auto param : predicate->parameters)
					output += param + ",";
				output.pop_back();
				output += ")";
				output += " ";
			}
			return output;
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

		Sentence* copySentence(Sentence* s)
		{
			Sentence* copy = new Sentence();

			for(int i = 0; i < s->allPredicates.size(); i++)
			{
				Predicate* copy_p = new Predicate();
				copy_p->parameters = s->allPredicates[i]->parameters;
				copy_p->negated = s->allPredicates[i]->negated;
				copy_p->name = s->allPredicates[i]->name;
				copy_p->rawFact = s->allPredicates[i]->rawFact;

				copy->allPredicates.push_back(copy_p);
			}

			return copy;
			
		}
		bool unifiable(Predicate* p1, Predicate* p2)
		{
			if(p1->parameters.size() != p2->parameters.size())
				return false;

			bool containConstant = false;
			for(auto p : p1->parameters)
				if(!isVariable(p)) containConstant = true;
			for(auto p : p2->parameters)
				if(!isVariable(p)) containConstant = true;

			if(!containConstant)
				return false;

			for(int i = 0; i < p1->parameters.size(); i++)
			{
				if(!isVariable(p1->parameters[i]) && !isVariable(p2->parameters[i]) && p1->parameters[i] != p2->parameters[i])
					return false;
			}

			return true;
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
	
		//convert to cnf form
		vector<Sentence*> cvt2CNF()
		{
			vector<Sentence*> sentences;
			for(auto p : rawTable)
				sentences.push_back(p.second);
			
			return sentences;
		}

		Sentence* unification(Sentence* s1, Sentence* s2, int index1, int index2)
		{
			unordered_map<string, string> lookup;
			auto targetParams = s2->allPredicates[index2]->parameters;
			auto currentParams = s1->allPredicates[index1]->parameters;

			//build the look up table that store the variable and its correponding value
			for(int i = 0; i < targetParams.size(); i++)
			{
				if(isVariable(currentParams[i]) && !isVariable(targetParams[i]))
					lookup[currentParams[i]] = targetParams[i];
				else if(isVariable(targetParams[i]) && !isVariable(currentParams[i]))
					lookup[targetParams[i]] = currentParams[i];
			}


			Sentence* result = s1;
			result->allPredicates.erase(result->allPredicates.begin() + index1);
			Sentence* tmp = new Sentence;
			tmp->allPredicates = s2->allPredicates;
			for(auto p : tmp->allPredicates)
			{
				for(int i = 0; i < p->parameters.size(); i++)
				{
					if(isVariable(p->parameters[i]) && lookup.count(p->parameters[i]))
						p->parameters[i] = lookup[p->parameters[i]];
				}
			}
			tmp->allPredicates.erase(tmp->allPredicates.begin() + index2);
			result->allPredicates.insert(result->allPredicates.end(), tmp->allPredicates.begin(), tmp->allPredicates.end());
			return result;
		}

		vector<Sentence*> resolve(Sentence* s1, Sentence* s2)
		{
			vector<Sentence*> resolvents;
			auto p1 = s1->allPredicates;
			auto p2 = s2->allPredicates;

			/* special case */
			bool contradication = false;
			bool flip = true;
			if(p1.size() == 1 && p2.size() == 1)
			{
				if(p1[0]->name == p2[0]->name && p1[0]->negated != p2[0]->negated)
				{
					for(int i = 0; i < p1[0]->parameters.size(); i++)
					{
						if(isVariable(p1[0]->parameters[i]) || isVariable(p2[0]->parameters[i]))
						{
							flip = false;
							break;
						}
							
							
						if(p1[0]->parameters[i] != p2[0]->parameters[i])
						{
							flip = false;
							break;
						}

						if(flip)
							contradication = true;	
					}

				}
			}
			if(contradication)
			{
				resolvents.push_back(nullptr);
				return resolvents;
			}
				
			/* end of special case */


			for(int i = 0; i < p1.size(); i++)
			{
				for(int j = 0; j < p2.size(); j++)
				{
					if(p1[i]->name == p2[j]->name && p1[i]->negated != p2[j]->negated)
					{
						if(unifiable(p1[i], p2[j]))
						{
							auto tmp1 = copySentence(s1);
							auto tmp2 = copySentence(s2);

							auto result = unification(tmp1, tmp2, i,j);
							resolvents.push_back(result);
						}
							
					}
				}
			}
			return resolvents;
		}

		bool resolution(Predicate* predicate)
		{
			vector<Sentence*> clauses = cvt2CNF();
			Sentence* append = new Sentence;
			append->allPredicates.push_back(predicate);
			clauses.push_back(append);

			unordered_set<string> clausesStr;
			for(auto c : clauses)
				clausesStr.insert(stringify(c));
				

			unordered_set<string> newSentence;

			int count = 0;
			int depth = 100;
			while(count < depth)
			{
				for(int i = 0; i < clauses.size() - 1; i++)
				{
					for(int j = i + 1; j < clauses.size(); j++)
					{
						vector<Sentence*> resolvents = resolve(clauses[i], clauses[j]);
						if(resolvents.empty())
							continue;


						for(auto r : resolvents)
						{
							if(r == nullptr)
								return true;
								
							newSentence.insert(stringify(r));
						}


					}
				}

				bool isSubset = true;
				for(auto n : newSentence)
				{
					if(!clausesStr.count(n))
					{
						isSubset = false;
						clausesStr.insert(n);
					}
				}

				if(isSubset)
				{
					cout << "hit" << endl;
					return false;
				}
					
				count++;
			}

			return true;
		}

		bool ask(Predicate* predicate, string sign)
		{
			/*-------------------BASE CASE-------------------*/
			
			//looking for direct contradiction
			cout << "called with ";
			printPredicate(predicate);
			//Same sign
			// auto posSentence = fetchSentence(predicate->name, sign);
			// for(auto result : posSentence)
			// {
			// 	if(result->allPredicates.size() == 1 && result->allPredicates[0]->name == predicate->name && result->allPredicates[0]->negated == predicate->negated &&
			// 	result->allPredicates[0]->parameters == predicate->parameters)
			// 	{
			// 		//found exact match means the original one contradicates the fact so return false
			// 		return false;
			// 	}
					
			// }

			string tmpSign = sign == "Positive" ? "Negative" : "Positive";
			auto negSentence = fetchSentence(predicate->name, tmpSign);
			for(auto result : negSentence)
			{
				if(result->allPredicates.size() == 1 && result->allPredicates[0]->name == predicate->name && result->allPredicates[0]->negated != predicate->negated &&
				result->allPredicates[0]->parameters == predicate->parameters)
				{
					//found a contradiction, that means the original statement is true
					return true;
				}
					
			}

			/*-------------------BASE CASE-------------------*/


			string opSign = sign == "Negative" ? "Positive" : "Negative";
			auto results = fetchSentence(predicate->name, opSign);

			for(auto result : results)
			{
				for(int i = 0; i < result->allPredicates.size(); i++)
				{
					if(result->allPredicates[i]->name == predicate->name)
					{
						auto origin = result->allPredicates;
						if(unify(result, predicate, i))
						{
							cout << "choice of sentence: " << result->rawFact << endl;
							cout << "choice of predicate: " << predicate->name << "(";
							for(auto param : predicate->parameters)
								cout << param << " ";
							cout << ")" << endl;
							cout << "after unification: " << endl;
							printKB();

							//start to resolve
							result->allPredicates.erase(result->allPredicates.begin() + i);
							if(result->allPredicates.size() == 1)
							{
								string subSign = result->allPredicates[0]->negated == true ? "Negative" : "Positive";
								if(ask(result->allPredicates[0], subSign))
									return true;
							}
							else
							{
								//traverse the KB and find the sentence with only one predicate
								Sentence* next = fetchOnlyPredicateInSentence();
								if(next)
								{
									cout << "next predicate: " << endl;
									printPredicate(next->allPredicates[0]);
									auto saveCopy = next->allPredicates[0];
									next->allPredicates.pop_back();

									string subSign = saveCopy->negated == true ? "Negative" : "Positive";
									if(ask(saveCopy, subSign))
										return true;

									//back track
									cout << "pushing back: " ;
									printPredicate(saveCopy);
									next->allPredicates.push_back(saveCopy);
										
								}
								return false;
							}
						}

						//Back track
						result->allPredicates = origin;
					}
				}
			}


			return false;
		}

		/*----------------------------------------------DEBUG FUNCTIONS---------------------------------------------- */
		void printSentence(Sentence* sentence)
		{
			for(auto predicate : sentence->allPredicates)
			{
				string output = "";
				string sign = predicate->negated ? "~" : "";
				output += sign + predicate->name + "(";
				for(auto param : predicate->parameters)
					output += param + ",";
				output.pop_back();
				output += ")";
				cout << output << " ";
			}
			cout << endl;
		}
		void printSentences(vector<Predicate*> results)
		{
			for(auto r : results)
				cout << r->rawFact << endl;
		}

		void printKB()
		{
			cout << "--------------------- KB ---------------------" << endl;
			for(auto p : table)
			{
				for(auto innerTalbe : p.second)
				{
					for(auto sentence : innerTalbe.second)
					{
						for(auto predicate : sentence->allPredicates)
						{
							string output = "";
							string sign = predicate->negated ? "~" : "";
							output += sign + predicate->name + "(";
							for(auto param : predicate->parameters)
								output += param + ",";
							output.pop_back();
							output += ")";
							cout << output << " ";
						}
						cout << endl;
					}
				}
			}

			cout << "--------------------- End of KB ---------------------" << endl;
		}

		void printPredicate(Predicate* predicate)
		{
			cout << "Predicate: ";
			if(predicate->negated)
				cout << "~";
			cout << predicate->name << "(";
			for(auto param : predicate->parameters)
				cout << param << " ";
			cout << ")" << endl;
		}
		/*----------------------------------------------DEBUG FUNCTIONS---------------------------------------------- */
};


int main()
{
	KB kb;

	kb.parse("input.txt");
	kb.initialize();


	Predicate* test = new Predicate();
	test->name = "Learn";
	test->negated = true;
	test->parameters.push_back("Sit");
	test->parameters.push_back("Ares");

	cout << kb.resolution(test) << endl;


	//cout << kb.unifiable(test, test2) << endl;
	//kb.resolution(negated_test);


	return 0;
}