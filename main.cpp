/*
*********************************
Project Recommendation System
@Author Nikolas Gialitsis
AM:1115201400027
*********************************

*/

#include "NN_classes.h"

#include <iostream>
#include <string>
#include <cstdlib> // for exit
#include <fstream>
#include <stdlib.h>
#include <vector> 
#include <random>
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
#include <numeric>//inner_product
#include <math.h>
#include <complex> // complex, norm
#include <ctime>
#include <map>
#include <algorithm>


#include "recommend_classes.h"
#include "cosine.h"
#include "utility.h"
#include "lsh.h"
#include "cluster.h"
#include "euclidean.h"
#include "cluster.h"

using namespace std;



void InitCryptoMap(string coins_path){
	ifstream coins_dict;
	coins_dict.open(coins_path.c_str());
	if (coins_dict.is_open()){
		string line;
		while ( getline (coins_dict,line)){
			std::vector<string> v;
			string repr = "";
			boost::char_separator<char> sep("\t");
			boost::tokenizer<boost::char_separator<char>> tok(line, sep);
			for (boost::tokenizer<boost::char_separator<char>>::iterator it=tok.begin(); it!=tok.end();it++) {
				string coin = *it;
				if(coin.at(0) >= 'A' && coin.at(0) <= 'z'){
					repr = coin;
				}
				v.push_back(coin);
			}
			//cout<<repr<<endl;
			for(int i = 0 ; i < v.size() ; i++){
				//cout<<"["<<v.at(i)<<"] = "<<repr<<endl;
				CryptoMap[v.at(i)] = repr;
			}
		}
		coins_dict.close();
	}
	else cout << "Unable to open file"<<endl; 
	//exit(0);
}


string getCryptoName(int ident){
	int i = 0;
	for(map<string,string>::iterator it = CryptoMap.begin(); it != CryptoMap.end(); ++it) {	
		if(i == ident)return it->first;
		i++;
	}
}



string CryptoDictionary(string coins_path,string coin){
	ifstream coins_dict;
	coins_dict.open(coins_path.c_str());
	if (coins_dict.is_open()){
		string line;
		while ( getline (coins_dict,line)){
			boost::char_separator<char> sep("\t");
			boost::tokenizer<boost::char_separator<char>> tok(line, sep);
			for (boost::tokenizer<boost::char_separator<char>>::iterator it=tok.begin(); it!=tok.end();it++) {
				//cout<<coin<<"!="<<*it<<endl;
				if(coin == *it){
					coins_dict.close();
					return coin;
				}
			}
		}
		coins_dict.close();
	}
	else cout << "Unable to open file"<<endl; 
	return "";
}




double SemanticDictionary(string semantic_path,string term){
	ifstream semantic_dict;
	semantic_dict.open(semantic_path.c_str());
	if (semantic_dict.is_open()){
		string line;
		while ( getline (semantic_dict,line)){
			boost::char_separator<char> sep("\t");
			boost::tokenizer<boost::char_separator<char>> tok(line, sep);
			for (boost::tokenizer<boost::char_separator<char>>::iterator it=tok.begin(); it!=tok.end();it++) {				
				if(term == *it){
					it++;
					semantic_dict.close();
					//cout<<"term = "<<term<<endl;
					cout<<"\tsemantic term = "<<term<<endl;
					return stod(*it);
				}
			}
		}
		semantic_dict.close();
	}
	else cout << "Unable to open file"<<endl; 
	return 0.0;
}


typedef struct CoinScore{
	string coin;
	double score;
} CoinScore;

int main(int argc , char* argv[]){


	string dataset_path;
	string semantic_path;
	string coins_path;
	ifstream semantic_dict;
	ifstream coins_dict;

	//Initialization
	if(argc == 1){
		dataset_path.assign("./test_dataset");
		coins_path.assign("./coins_queries.csv");
		semantic_path.assign("./vader_lexicon.csv");
	}


	for(int i = 1 ; i < argc ; i++){
		string s(argv[i]);
		if((s == "-i")and(i != argc-1)){
			string p = argv[i+1];
			dataset_path.assign(p);
			coins_path.assign("./coins_queries.csv");
			semantic_path.assign("./vader_lexicon.csv");
		}
		else;
	}




	vector<User> Users;
	InitCryptoMap(coins_path);
	ifstream dataset;
	int n  = -1;
	int old_user_id = -1;
	dataset.open(dataset_path.c_str());
	if (dataset.is_open()){

		string line;
		while ( getline (dataset,line)){
			boost::char_separator<char> sep("\t");
			boost::tokenizer<boost::char_separator<char>> tok(line, sep);
			string user_id;
			user_id.clear();
			int tweet_id;
			for (boost::tokenizer<boost::char_separator<char>>::iterator it=tok.begin(); it!=tok.end();it++) {
				user_id.append(*it);
				it++;
				tweet_id = stol(*it);
				break;
			}
			if(old_user_id == stol(user_id)){
				Users.back().AddTweet(Tweet(tweet_id,line),semantic_path,coins_path);
			}
			else{
				cout<<"new user "<<endl;
				Users.push_back(User(stol(user_id)));
				Users.back().AddTweet(Tweet(tweet_id,line),semantic_path,coins_path);
				old_user_id = stol(user_id);
				n++;
			}
		}

		dataset.close();
	}
	else cout << "Unable to open file"<<endl; 

	vector<Point> Points;

	vector<map<string,double>> UserVectors;
	for(int i = 0 ; i < Users.size() ;i++){
		Users.at(i).Normalize();
		UserVectors.push_back(Users.at(i).GetCoinsVector());
		cout << "User "<< Users.at(i).getID() << " < ";
		Users.at(i).PrintScores();
		cout<<">"<<endl;
		std::map<string,double> v_map = Users.at(i).GetCoinsVector();
		Points.push_back(Point(Users.at(i).getID(),v_map));
	}

	int L = 5;
	int d = 100;
	int K = 3;
	int W = 350;
	int TableSize = pow(2,K);
	vector<HashTable> HashTables;
	for(int l = 0 ; l < L ; l++){
		HashTables.push_back(HashTable(K,TableSize,W));	
	}


	LSH_Build_Cosine(HashTables,L,d,Points);
	
	PrintAll(HashTables,L);
	vector<vector<double>> recommendations = LSH_Cosine_Recommend(HashTables,L,Points);
	assert(recommendations.size() == Users.size());
	for(int i = 0 ; i < Users.size() ; i++){
		User& u = Users.at(i);
		vector<bool> assigned = u.getAssigned();
		vector<double> RU = recommendations.at(i);
		sort(RU.begin(),RU.end());
		vector<CoinScore> best;
		int coin_num = 0;
		cout << "UserID "<<u.getID()<<endl;
		cout<<"Recommend :"<<endl;
		int max_c = 0;
		for(vector<double>::iterator it = RU.begin() ; it != RU.end() ; it++){
			if(assigned.at(coin_num) == false){
				;//cout<<"\t"<<getCryptoName(coin_num)<< " assigned"<<endl;
			}
			else {
				cout<<"\t BUY "<<getCryptoName(coin_num)<< endl;
				max_c++;
				if(max_c == 5)break;
			}
			coin_num++;
			
		}
	}


	coins_dict.close();
	semantic_dict.close();


	return 0;
}