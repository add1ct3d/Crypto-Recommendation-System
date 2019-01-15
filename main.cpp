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
#include "euclidean.h"
#include "cluster.h"

using namespace std;


vector<User> getRealUsers(string input_path,string output_path,string semantic_path,string coins_path);
vector<User> getVirtualUsers(string input_path,string output_path,int num_clusters,int L,string,string);
void LSH_Recommend_Coins(vector<HashTable>& HashTables,int L,vector<User>& Users,vector<Point>& Points,int total);
vector<vector<double>> Cluster_Euclidean_Recommend(Cluster_Struct& CLUSTERS,vector<Point>& centroids,vector<Point>& users);
void Cluster_Recommend_Coins(Cluster_Struct& Clusters,vector<Point>& centroids,vector<User>& Users,vector<Point>& Points,int total);
vector<vector<User>> GetPartitions(vector<User>& Users);
vector<User> GetPartitionTraining(vector<vector<User>>& partitions , int ident);
vector<User> GetPartitionEvaluation(vector<vector<User>>& partitions , int ident);
void SetEvaluationInfinite(vector<User>& evaluation_set);


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
					//cout<<"\tsemantic term = "<<term<<endl;
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
	string metric = "euclidean";
	int L = 5;
	int d = 100;
	int K = 3;
	int W = 350;
	int ml = 1;
	bool validation = false;
	int TableSize = pow(2,K);
	string dataset_path;
	string semantic_path;
	string coins_path;
	ifstream semantic_dict;
	ifstream coins_dict;
	int num_clusters = 5;
	string output_path = "./myout.out";
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
		else if(s == "-validate"){
			validation = true;
		}
		else if((s == "-o")and(i != argc-1)){
			output_path.assign(argv[i+1]);
			i++;
		}
	}
	
	InitCryptoMap(coins_path);


	if(validation == false){

		vector<User> Users  = getRealUsers(dataset_path,output_path,semantic_path,coins_path);
		vector<Point> Points;
		vector<map<string,double>> UserVectors;
		for(int i = 0 ; i < Users.size() ;i++){
			Users.at(i).Normalize();
			UserVectors.push_back(Users.at(i).GetCoinsVector());
			std::map<string,double> v_map = Users.at(i).GetCoinsVector();
			Points.push_back(Point(Users.at(i).getID(),v_map));
			Points.back().unassign();
		}




		//A1
		/*vector<HashTable> HashTables;
		for(int l = 0 ; l < L ; l++){
			HashTables.push_back(HashTable(K,TableSize,W));	
		}
		*/
		//LSH_Build_Cosine(HashTables,L,d,Points);
		//PrintAll(HashTables,L);
		//cout<<"A1"<<endl;
		//LSH_Recommend_Coins(HashTables,L,Users,Points,5);


		//A2
		vector<User> VirtualUsers = getVirtualUsers("./twitter_dataset_small_v2.csv",output_path,num_clusters,L,semantic_path,coins_path);
		vector<Point> VirtualPoints;
		vector<map<string,double>> VirtualVectors;
		for(int i = 0 ; i < VirtualUsers.size() ;i++){
			VirtualUsers.at(i).Normalize();
			VirtualVectors.push_back(VirtualUsers.at(i).GetCoinsVector());
			std::map<string,double> v_map = VirtualUsers.at(i).GetCoinsVector();
			VirtualPoints.push_back(Point(VirtualUsers.at(i).getID(),v_map));
		}
	

		/*vector<HashTable> VirtualHashes;
		for(int l = 0 ; l < L ; l++){
			VirtualHashes.push_back(HashTable(K,TableSize,W));	
		}
		LSH_Build_Cosine(VirtualHashes,L,d,VirtualPoints);
		//PrintAll(VirtualHashes,L);
		cout<<"A2"<<endl;
		LSH_Recommend_Coins(VirtualHashes,L,Users,Points,2);
	*/

	/*	//B1
		vector<Point> centroids;
		centroids = Init_Random(Points,num_clusters);
		assert(centroids.size() == num_clusters);
		Cluster_Struct CLUSTERS;
		CLUSTERS.reserve(num_clusters);
		for(int i = 0 ; i < num_clusters ; i ++){
			CLUSTERS.push_back(new vector<Point>());
		}
		int loops = 0;
		Assign_Lloyd(CLUSTERS,Points,centroids,metric);
		data_type obj1 = ObjectiveFunction(CLUSTERS,centroids,metric,num_clusters,true);
		while( loops < ml){
			K_means(CLUSTERS,centroids,metric,d);
			Assign_Lloyd(CLUSTERS,Points,centroids,metric);
			data_type obj2 = ObjectiveFunction(CLUSTERS,centroids,metric,num_clusters,true);
			data_type improv = (obj1 - obj2)/obj1; // if obj2 close to obj1 : improv -> 0 , else improv -> 1
			obj1 = obj2;
			//PrintClusters(CLUSTERS,centroids,true,output_path);
			if(improv <= 0.05)break;
			loops++;
		}
		//PrintCentroids(centroids);	
		cout<<"B1"<<endl;
		Cluster_Recommend_Coins(CLUSTERS,centroids,Users,Points,5);
	*/


		//B2
		cout<<"B2"<<endl;
		for(int i = 0 ; i < Points.size() ; i++){
			Point& p = Points.at(i);
			p.unassign(); 
			User& u =  Users.at(i);
			vector<Point> JointPoints = VirtualPoints;
			vector<User> JointUsers = VirtualUsers;
			JointUsers.push_back(u);
			JointPoints.push_back(p);
			vector<Point> centroids;
			cout<<"Init random"<<endl;
			centroids = Init_Random(JointPoints,num_clusters);
			assert(centroids.size() == num_clusters);
			Cluster_Struct CLUSTERS;
			CLUSTERS.reserve(num_clusters);
			for(int i = 0 ; i < num_clusters ; i ++){
				CLUSTERS.push_back(new vector<Point>());
			}
			cout<<"clusters  pushed"<<endl;
			int loops = 0;
			Assign_Lloyd(CLUSTERS,JointPoints,centroids,metric);
			cout<<"assigned"<<endl;
			data_type obj1 = ObjectiveFunction(CLUSTERS,centroids,metric,num_clusters,true);
			cout<<"objective"<<endl;
			while( loops < ml){
				cout<<"loop"<<endl;
				K_means(CLUSTERS,centroids,metric,d);
				Assign_Lloyd(CLUSTERS,JointPoints,centroids,metric);
				cout<<"assigned"<<endl;
				data_type obj2 = ObjectiveFunction(CLUSTERS,centroids,metric,num_clusters,true);
				data_type improv = (obj1 - obj2)/obj1; // if obj2 close to obj1 : improv -> 0 , else improv -> 1
				obj1 = obj2;
				//PrintClusters(CLUSTERS,centroids,true,output_path);
				if(improv <= 0.05)break;
				loops++;
			}
			cout<<"push p "<<endl;
			vector<Point> point_v;
			point_v.push_back(p);
			cout<<"cluster recommend"<<endl;
			Cluster_Recommend_Coins(CLUSTERS,centroids,JointUsers,point_v,2);
		}		
	}
	else{
		int count = 10;
		vector<User> Users  = getRealUsers(dataset_path,output_path,semantic_path,coins_path);
		vector<Point> Points;
		vector<map<string,double>> UserVectors;
		for(int i = 0 ; i < Users.size() ;i++){
			Users.at(i).Normalize();
			UserVectors.push_back(Users.at(i).GetCoinsVector());
			std::map<string,double> v_map = Users.at(i).GetCoinsVector();
			Points.push_back(Point(Users.at(i).getID(),v_map));
			Points.back().unassign();
		}

		vector<vector<User>> Partitions = GetPartitions(Users);
		double mean_error = 0;
		for(int partition = 0 ; partition < Partitions.size() ; partition++){
			vector<Point> NewPoints;
			vector<map<string,double>> NewUserVectors;
			vector<User> NewUsers =  GetPartitionTraining(Partitions,partition);
			vector<User> EvaluationSet = GetPartitionEvaluation(Partitions,partition);
			vector<User> Eval =  GetPartitionEvaluation(Partitions,partition);
			for(auto&x : Eval)
				NewUsers.push_back(x);

			SetEvaluationInfinite(Eval);
			for(int i = 0 ; i < NewUsers.size() ;i++){
				NewUsers.at(i).Normalize();
				NewUserVectors.push_back(NewUsers.at(i).GetCoinsVector());
				std::map<string,double> v_map = NewUsers.at(i).GetCoinsVector();
				NewPoints.push_back(Point(NewUsers.at(i).getID(),v_map));
				NewPoints.back().unassign();
			}
				//A1
			vector<HashTable> HashTables;
			for(int l = 0 ; l < L ; l++){
				HashTables.push_back(HashTable(K,TableSize,W));	
			}
			LSH_Build_Cosine(HashTables,L,d,NewPoints);
			//PrintAll(HashTables,L);
			cout<<"A1-vaidation"<<endl;

			vector<vector<double>> recommendations = LSH_Cosine_Recommend(HashTables,L,NewPoints);
			//cout<<"recommendation done"<<endl;
			double error = 0;
			int j = 0;
			for(int u = NewUsers.size() - EvaluationSet.size() -1 ; u < NewUsers.size() ;u++){
				map<string,double> sentiment_score = EvaluationSet.at(j).GetCoinsVector();
				int c = 0;
				for(map<string,double>::iterator it = sentiment_score.begin(); it != sentiment_score.end(); ++it) {
					if(recommendations.at(u).size() == 0)continue;
					double e = it->second - recommendations.at(u).at(c);
					if(e < 0) e = -e;
					error+=e;
					c++;
				}
			}
			//cout<<"error = "<<error<<endl;
			mean_error += (error/100);
		}
		mean_error*=count;
		cout<<"mean error ="<<mean_error/10<<endl;

		mean_error = 0;
		for(int partition = 0 ; partition < Partitions.size() ; partition++){
			vector<Point> NewPoints;
			vector<map<string,double>> NewUserVectors;
			vector<User> NewUsers =  GetPartitionTraining(Partitions,partition);
			vector<User> EvaluationSet = GetPartitionEvaluation(Partitions,partition);
			vector<User> Eval =  GetPartitionEvaluation(Partitions,partition);
			for(auto&x : Eval)
				NewUsers.push_back(x);

			SetEvaluationInfinite(Eval);
			for(int i = 0 ; i < NewUsers.size() ;i++){
				NewUsers.at(i).Normalize();
				NewUserVectors.push_back(NewUsers.at(i).GetCoinsVector());
				std::map<string,double> v_map = NewUsers.at(i).GetCoinsVector();
				NewPoints.push_back(Point(NewUsers.at(i).getID(),v_map));
				NewPoints.back().unassign();
			}

			vector<Point> centroids;
			centroids = Init_Random(NewPoints,num_clusters);
			assert(centroids.size() == num_clusters);
			Cluster_Struct CLUSTERS;
			CLUSTERS.reserve(num_clusters);
			for(int i = 0 ; i < num_clusters ; i ++){
				CLUSTERS.push_back(new vector<Point>());
			}
			int loops = 0;
			Assign_Lloyd(CLUSTERS,NewPoints,centroids,metric);
			data_type obj1 = ObjectiveFunction(CLUSTERS,centroids,metric,num_clusters,true);
			while( loops < ml){
				K_means(CLUSTERS,centroids,metric,d);
				Assign_Lloyd(CLUSTERS,NewPoints,centroids,metric);
				data_type obj2 = ObjectiveFunction(CLUSTERS,centroids,metric,num_clusters,true);
				data_type improv = (obj1 - obj2)/obj1; // if obj2 close to obj1 : improv -> 0 , else improv -> 1
				obj1 = obj2;
				if(improv <= 0.05)break;
				loops++;
			}			
			cout<<"B1-validate"<<endl;
			vector<vector<double>> recommendations = Cluster_Euclidean_Recommend(HashTables,L,NewPoints);
			double error = 0;
			int j = 0;
			for(int u = NewUsers.size() - EvaluationSet.size() -1 ; u < NewUsers.size() ;u++){
				map<string,double> sentiment_score = EvaluationSet.at(j).GetCoinsVector();
				int c = 0;
				for(map<string,double>::iterator it = sentiment_score.begin(); it != sentiment_score.end(); ++it) {
					if(recommendations.at(u).size() == 0)continue;
					double e = it->second - recommendations.at(u).at(c);
					if(e < 0) e = -e;
					error+=e;
					c++;
				}
			}
			//cout<<"error = "<<error<<endl;
			mean_error += (error/100);
		}
		mean_error*=count;
		cout<<"mean error ="<<mean_error/10<<endl;



	}
	return 0;
}



vector<vector<User>> GetPartitions(vector<User>& Users){
	int users_num = Users.size();
	int partition_size = users_num/10;
	vector<int> idents ;
	for(int i = 0 ; i < users_num; i++){
		idents.push_back(i);
	}

	vector<vector<User>> partitions;
	random_shuffle(idents.begin(),idents.end());
	int last = 0;
	int old = last;
	for(int i = 0 ; i < 10 ; i++){
		
		old = last;
		last += partition_size ; 
		cout<<"last = "<<last<<endl;
		vector<User> partition;
		for(int j = old ; j < last ; j++ ){

			partition.push_back(Users.at(idents.at(j)));
		}
		partitions.push_back(partition);
	}
	return partitions;
}


vector<User> GetPartitionTraining(vector<vector<User>>& partitions , int ident){
	
	vector<User> TrainSet;
	for(int i = 0 ; i < partitions.size() ; i++){
		if(i == ident)continue;
		for(auto& u : partitions.at(i)){
			TrainSet.push_back(u);
		}
	}
	return TrainSet;
}

vector<User> GetPartitionEvaluation(vector<vector<User>>& partitions , int ident){
	return partitions.at(ident);
}

void SetEvaluationInfinite(vector<User>& evaluation_set){
	for(auto& u : evaluation_set){
		u.SetInfinite();
	}
}


vector<User> getRealUsers(string input_path,string output_path,string semantic_path,string coins_path){	
	
	vector<User> Users;
	ifstream dataset;
	int n  = -1;
	int old_user_id = -1;
	dataset.open(input_path.c_str());
	int items = 0;
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
				//cout<<"new user "<<endl;
				Users.push_back(User(stol(user_id)));
				Users.back().AddTweet(Tweet(tweet_id,line),semantic_path,coins_path);
				old_user_id = stol(user_id);
				n++;
				//if(n > 200)break;
			}
			items++;
			if(items > 200)break;
		}
		dataset.close();
	}
	else cout << "Unable to open file"<<endl; 
	return Users;
}

vector<User> getVirtualUsers(string input_path,string output_path,int num_clusters,int L,
string semantic_path,string coins_path){	
	int n  = 0;
	int d =0 ;
	int ml=1;
	vector<Point> tweets_dataset; 
	ifstream input;
	input.open(input_path.c_str());
	if (input.is_open()){
		string line;
		while ( getline (input,line)){			
			Point* ptr = new Point(line);
			tweets_dataset.push_back(*ptr);
			if( n == 0){//count dimensions of the first point
				string s;
				s.assign(ptr->get_all_fields());
				d = CalculateDimensions(s);
				cout<<"dimensions :"<<d<<endl;
			}
			n++;
		}

		input.close();
	}
	else cout << "Unable to open file"<<endl; 	
	ofstream output;
	output.open(output_path.c_str(),ofstream::out | ofstream::app);
	if (!output.is_open()){
		cout<<"Output path not correct"<<endl;
		exit(-1);
	}
	int K = 4;
	int W = 4;
	int TableSize = n/4;
	vector<Point> centroids;
	centroids = Init_Random(tweets_dataset,num_clusters);
	assert(centroids.size() == num_clusters);
	Cluster_Struct CLUSTERS;
	CLUSTERS.reserve(num_clusters);
	for(int i = 0 ; i < num_clusters ; i ++){
		CLUSTERS.push_back(new vector<Point>());
	}
	vector<HashTable> HashTables;
	for(int l = 0 ; l < L ; l++){
		HashTables.push_back(HashTable(K,TableSize,W));	
	}
	LSH_Build_Euclidean(HashTables,L,d,tweets_dataset);
	int loops = 0;
	double R;
	string metric = "euclidean";
	Assign_Lloyd(CLUSTERS,tweets_dataset,centroids,metric);
	data_type obj1 = ObjectiveFunction(CLUSTERS,centroids,metric,num_clusters,true);
	while( loops < ml){
		if(loops != 0){
			K_means(CLUSTERS,centroids,metric,d);
		}
		Assign_Lloyd(CLUSTERS,tweets_dataset,centroids,metric);
		data_type obj2 = ObjectiveFunction(CLUSTERS,centroids,metric,num_clusters,true);
		data_type improv = (obj1 - obj2)/obj1; // if obj2 close to obj1 : improv -> 0 , else improv -> 1
		obj1 = obj2;
		//PrintClusters(CLUSTERS,centroids,true,output_path);
		if(improv <= 0.05)break;
		loops++;
	}
	vector<User> VirtualUsers;
	map<int,int> CentroidUser;
	for(int i = 0 ; i < centroids.size()  ; i++){
		CentroidUser[centroids.at(i).get_id()] = i;
		VirtualUsers.push_back(User(i));
	}	
	assert(VirtualUsers.size() == centroids.size());
	for(int i = 0 ; i < tweets_dataset.size() ; i++){
		int centroid_id = tweets_dataset.at(i).get_centroid_id();
		int message_id = tweets_dataset.at(i).get_id();
		int userID = CentroidUser[centroid_id];
		string tweet = MessageID[message_id];
		map<int,string>::iterator it ;
		it = MessageID.find(message_id);
		if(it == MessageID.end())VirtualUsers.erase(VirtualUsers.begin() + userID);
		VirtualUsers.at(userID).AddTweet(Tweet(message_id,tweet),semantic_path,coins_path);
	}
	input.close();
	output.close();
	return VirtualUsers;
}

	




void LSH_Recommend_Coins(vector<HashTable>& HashTables,int L,vector<User>& Users,vector<Point>& Points,int total_coins){
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
		//cout<<"Recommend :"<<endl;
		int max_c = 0;
		for(vector<double>::iterator it = RU.begin() ; it != RU.end() ; it++){
			if(assigned.at(coin_num) == true){
				;
			}
			else {
				cout<<"\t BUY "<<CryptoMap[getCryptoName(coin_num)]<<endl;
				assigned.at(coin_num) = true;
				max_c++;
				if(max_c == total_coins)break;
			}
			coin_num++;	
		}
		if(max_c < total_coins){
			vector<string> unassigned;
			for(int j = 0 ; j < 100 ; j++){
				if(assigned.at(j) == false)
					unassigned.push_back(getCryptoName(j));
			}

			random_shuffle(unassigned.begin(),unassigned.end());
			for(int j = 0 ; j < total_coins - max_c ; j++){
				cout<<"\t BUY "<<CryptoMap[unassigned.at(j)]<<endl;
			}
		}
	}
}






void Cluster_Recommend_Coins(Cluster_Struct& Clusters,vector<Point>& centroids,
	vector<User>& Users,vector<Point>& Points,int total_coins){
	cout<<"cluster euclidean"<<endl;
	vector<vector<double>> recommendations = Cluster_Euclidean_Recommend(Clusters,centroids,Points);
	assert(recommendations.size() == Users.size());
	for(int i = 0 ; i < Users.size() ; i++){
		User& u = Users.at(i);
		vector<bool> assigned = u.getAssigned();
		vector<double> RU = recommendations.at(i);
		sort(RU.begin(),RU.end());
		vector<CoinScore> best;
		int coin_num = 0;
		cout << "UserID "<<u.getID()<<endl;
		//cout<<"Recommend :"<<endl;
		int max_c = 0;
		for(vector<double>::iterator it = RU.begin() ; it != RU.end() ; it++){
			if(assigned.at(coin_num) == true){
				;//cout<<"\t"<<getCryptoName(coin_num)<< " assigned"<<endl;
			}
			else {
				cout<<"\t BUY "<<CryptoMap[getCryptoName(coin_num)]<<endl;
				assigned.at(coin_num) = true;
				max_c++;
				if(max_c == total_coins)break;
			}
			coin_num++;	
		}
		if(max_c < total_coins){
			vector<string> unassigned;
			for(int j = 0 ; j < 100 ; j++){
				if(assigned.at(j) == false)
					unassigned.push_back(getCryptoName(j));
			}
			random_shuffle(unassigned.begin(),unassigned.end());
			for(int j = 0 ; j < total_coins - max_c ; j++){
				cout<<"\t BUY "<<CryptoMap[unassigned.at(j)]<<endl;
			}
		}
	}
}

