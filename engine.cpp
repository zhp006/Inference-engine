#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <fstream>
#include <sstream>
#include <ctime>

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

		vector<Predicate*> senQueries;
		unordered_map<string, unordered_map<string,vector<Sentence*>>> table;
		unordered_map<string, unordered_map<string,vector<Sentence*>>> origin;
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

		void cleanKB()
		{
			for(auto p : table)
			{
				for(auto innerTalbe : p.second)
				{
					for(auto sentence : innerTalbe.second)
					{
						free(sentence);
					}
				}
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

		Predicate* stringToPredicate(string s)
		{
			auto first_index = s.find('(');
			auto second_index = s.find(')');
			string rawParameters = s.substr(first_index+1, second_index - first_index - 1);

			vector<string> params;
			stringstream ss (rawParameters);
			string param;
			while(getline(ss, param, ','))
			{
				if(param[0] == ' ')
					param = param.substr(1);
				params.push_back(param);
			}

			Predicate* predicate = new Predicate();

			predicate->name = s[0] == '~' ? s.substr(1, first_index - 1) : s.substr(0, first_index);
			predicate->negated = s[0] == '~';
			predicate->parameters = params;
			predicate->rawFact = s;

			return predicate;
	}

		void initialize()
		{
			for(auto fact : rawFacts)
				rawFactToTable(fact);
			
			for(auto query : queries)
				senQueries.push_back(stringToPredicate(query));
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

		bool haveCommonPredicate(Sentence* s1, Sentence* s2)
		{
			unordered_set<string> predicates;
			for(auto p : s1->allPredicates)
				predicates.insert(p->name);
			for(auto p : s2->allPredicates)
			{
				if(predicates.count(p->name))
					return true;
			}
			return false;
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

			Sentence* result;
			Sentence* copy_s1 = copySentence(s1);
			Sentence* copy_s2 = copySentence(s2);
			copy_s1->allPredicates.erase(copy_s1->allPredicates.begin() + index1);
			copy_s2->allPredicates.erase(copy_s2->allPredicates.begin() + index2);

			for(auto p : copy_s1->allPredicates)
			{
				for(int i = 0; i <  p->parameters.size(); i++)
				{
					if(isVariable(p->parameters[i]) && lookup.count(p->parameters[i]))
						p->parameters[i] = lookup[p->parameters[i]];
				}
			}
			for(auto p : copy_s2->allPredicates)
			{
				for(int i = 0; i <  p->parameters.size(); i++)
				{
					if(isVariable(p->parameters[i]) && lookup.count(p->parameters[i]))
						p->parameters[i] = lookup[p->parameters[i]];
				}
			}

			result = copy_s1;
			result->allPredicates.insert(result->allPredicates.end(), copy_s2->allPredicates.begin(), copy_s2->allPredicates.end());


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


					}
					if(flip)
						contradication = true;	

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

		void tell(Sentence* s)
		{
			for(auto p : s->allPredicates)
			{
				string sign = p->negated ? "Negative" : "Positive";
				table[p->name][sign].push_back(s);
			}
		}

		bool loopCheck(Sentence* s1, Sentence* s2)
		{
			unordered_set<string> table;
			for(auto p1 : s1->allPredicates)
			{
				for(auto p2 : s2->allPredicates)
				{
					if(p1->name == p2->name && p1->negated != p2->negated)
						table.insert(p1->name);
					if(table.size() >= 2)
						return false;
				}
			}
			return true;
		}

		bool resolution(Predicate* predicate)
		{
			vector<Sentence*> clauses = cvt2CNF();
			Sentence* append = new Sentence;
			append->allPredicates.push_back(predicate);
			clauses.insert(clauses.begin(), append);

			unordered_set<string> clausesStr;
			for(auto c : clauses)
				clausesStr.insert(stringify(c));
				

			unordered_map<string, Sentence*> newSentence;

			set<pair<int, int>> compared;
			int count = 0;
			int depth = 1000;
			while(count < depth)
			{
				/* ORIGIN */

				// for(int i = 0; i < clauses.size() - 1; i++)
				// {
				// 	for(int j = i + 1; j < clauses.size(); j++)
				// 	{
				// 		auto cmp = make_pair(i, j);
				// 		if(compared.count(cmp) || !loopCheck(clauses[i], clauses[j]))
				// 			continue;
				// 		compared.insert(cmp);

				// 		time_t before = time(nullptr);
				// 		vector<Sentence*> resolvents = resolve(clauses[i], clauses[j]);
				// 		time_t after = time(nullptr);
				// 		int diff = after - before;
				// 		if(diff)
				// 			cout << "time for resolve: " << diff << endl;
						
				// 		if(resolvents.empty())
				// 			continue;

				// 		for(auto r : resolvents)
				// 		{
				// 			if(r == nullptr)
				// 				return true;

				// 			newSentence[stringify(r)] = r;
				// 		}

				// 	}
				// }

				/* ORIGIN */
				
				set<string> strCompared;
				set<pair<Sentence*, Sentence*>> ptrCompared;
				for(auto c : clauses)
				{

					for(auto p : c->allPredicates)
					{
						string sign = p->negated ? "Positive" : "Negative";
						auto results = fetchSentence(p->name, sign);
						for(auto r : results)
						{
							time_t before = time(nullptr);
							//string cmp  = stringify(c) + stringify(r);
							//string cmp2 = stringify(r) + stringify(c);
							auto cmp = make_pair(c, r);
							auto cmp2 = make_pair(r, c);

							time_t after = time(nullptr);
							int diff = after - before;
							if(diff)
								cout << "time for ptr make_pair: " << diff << endl;

							// if(strCompared.count(cmp) || strCompared.count(cmp2) || !loopCheck(c, r))
							// 	continue;
							if(ptrCompared.count(cmp) || ptrCompared.count(cmp2) || !loopCheck(c, r))
								continue;

							ptrCompared.insert(cmp);
							ptrCompared.insert(cmp2);
							//strCompared.insert(cmp);
							//strCompared.insert(cmp2);
							vector<Sentence*> resolvents = resolve(c, r);

							if(resolvents.empty())
								continue;
							
							for(auto r : resolvents)
							{
								if(r == nullptr)
									return true;
									

								newSentence[stringify(r)] = r;
							}


						}
					}
				}


				bool isSubset = true;
				for(auto n : newSentence)
				{
					if(!clausesStr.count(n.first))
					{
						isSubset = false;
						clausesStr.insert(n.first);
						clauses.push_back(n.second);
						tell(n.second);
					}
				}

				if(isSubset)
					return false;
					
					
					
				

				count++;
			}

			return true;
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
		
		void printAnyKB(unordered_map<string, unordered_map<string,vector<Sentence*>>> table)
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
	kb.origin = kb.table;

	vector<int> results;
	time_t before = time(nullptr);
	for(auto q : kb.senQueries)
	{
		q->negated = !q->negated;
		time_t funcBefore = time(nullptr);
		results.push_back(kb.resolution(q));
		time_t funcAfter = time(nullptr);
		cout << "----------------------------func time: " << funcAfter - funcBefore << endl;
		kb.table = kb.origin;
		// cout << kb.resolution(q) << endl;
	}
	time_t after = time(nullptr);
	cout << "total: " << after - before << endl;
	ofstream output;
	output.open("output.txt");
	for(auto r : results)
	{
		if(r)
			output << "TRUE" << endl;
		else
			output << "FALSE" << endl;
	}

	output.close();
		

	return 0;
}
