/*

*********************************
Project Recommend
@Author Nikolas Gialitsis
AM:1115201400027
*********************************

*/

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
#include <unordered_map>
#include <math.h>
#include <complex> // complex, norm
#include <ctime>

#include "NN_classes.h"
#include "lsh.h"
#include "cosine.h"
#include "utility.h"
#include <map>
#include <algorithm>
using namespace std;


class Distance{
public:
	Distance(double dist,Point& p ){
		this->dist = dist;
		this->p = new Point(p);
	}

	double getDist(){
		return dist;
	}
	Point* getPoint(){
		return p;
	}


private:
	double dist;
	Point* p;
};

typedef struct {
	bool operator()(Distance& a, Distance& b) const{   
		return a.getDist() < b.getDist();
	}   
} comparison;


typedef struct {
	bool operator()(Distance& a, Distance& b) const{   
		return a.getDist() == b.getDist();
	}   
} equality;







vector<double> CalculateWeightedRecommendation(vector<Distance>& neighbors){
	vector<vector<double>> scores;
	vector<double> similarities;
	
	
	double similarity_sum = 0;
	for(auto& x : neighbors){
		double similarity = 1 - x.getDist();
		if(similarity < 0 ) similarity = 0;
		similarity_sum += similarity;
		similarities.push_back(similarity);
	}
	vector<double> recommended;
	if(similarity_sum == 0)return recommended;
	if(similarity_sum < 0) similarity_sum = -similarity_sum;
	double z = 1/similarity_sum;


	for(auto& x : neighbors){
		boost::char_separator<char> sep("\t,");
		string fields = x.getPoint()->get_all_fields();
		boost::tokenizer<boost::char_separator<char>> tok(fields, sep);
		vector<double> score;
		int i = 0;
		for (boost::tokenizer<boost::char_separator<char>>::iterator it=tok.begin(); it!=tok.end();it++){
			score.push_back(stod(*it));
			i++;
			if(i == 100)break;
		}
		assert(i == 100);
		scores.push_back(score);
	}

	
	int cryptos = scores.at(0).size();

	for(int crypto = 0 ; crypto < cryptos ; crypto++){
		double crypto_sum = 0;
		for(int neighbor = 0 ; neighbor < scores.size() ; neighbor++){
			crypto_sum = crypto_sum + (scores.at(neighbor).at(crypto) * similarities.at(neighbor) );
		}
		recommended.push_back(z*crypto_sum);
	}

	return recommended;
}


vector<vector<double>> LSH_Cosine_Recommend(vector<HashTable>&  HashTables,int L,vector<Point>& users){
	int Max_neighbors = 20;
	data_type R = 1.0;//default
	int query_points = users.size();
	int found_items = 0;
	double e = 0.05;
	int K = HashTables.at(0).K;
	int TableSize = HashTables.at(0).TableSize;
	vector<vector<double>> result;
	for(int query = 0 ; query < query_points; query++){

		cout <<std::endl<<"--------------------"<<std::endl;
		cout <<std::endl<<"--------------------"<<std::endl;
		cout <<"(cosine)Query:"<< query<<std::endl;
		data_type min_dist = 10*R;
		
		int items_found = 0;		
		cout<<"R-near neighbours:"<<std::endl;
			
		Point& p = users.at(query); 	
		vector<int> gv[L];	
		vector<Distance> neighbors;

		for(int l = 0 ; l < L ; l++){
			cout<<"\t HashTable :"<<l<<endl;
			HashTable* hash = &(HashTables.at(l));
			
			cout<<"Query ID "<<p.get_id()<<endl;
			string gs;	
			for(int k = 0 ; k < K  ; k++){
				gs.push_back(cosine_h(p,hash->cosine_r.at(k),K));
			}

			int bucket = cosine_g(gs,hash->shuffle_idents,K);	
			bucket = bucket % TableSize;
			assert(bucket >= 0);

			
			for(vector<Point*>::iterator it = ((hash->data).at(bucket)).begin() 
				; it != ((hash->data).at(bucket)).end() ;it++ ){
				Point* point_ptr = *it;	
				if(point_ptr->cosine_g != gs){
					continue;
				}				
				data_type dist = CosineDistance(*point_ptr,p);
				if(dist <= (1+e)*R){			
					items_found++;
					cout<<"\t\t-- found Item ID"<<point_ptr->get_id()<<std::endl;
					if(point_ptr->get_id() != p.get_id()){
						neighbors.push_back(Distance(dist,*point_ptr));

					}
					else cout<<"\t\t\tsame id"<<endl;

				}				
			}
		}	
		comparison c;
		equality eq;
		std::sort(neighbors.begin(),neighbors.end(),c);
		auto last = std::unique(neighbors.begin(), neighbors.end(),eq);
		neighbors.erase(last, neighbors.end());
		cout<<"Printing Best "<<Max_neighbors<<" Distances"<<endl;
		int i = 0;
		for(vector<Distance>::iterator it = neighbors.begin() ; it != neighbors.end() ; it++){
			cout<<it->getPoint()->get_id()<<": "<<it->getDist()<<endl;
			i++;
			if(i == Max_neighbors)break;
		}
		result.push_back(CalculateWeightedRecommendation(neighbors));	
	}
	return result;
}






vector<vector<double>> Cluster_Euclidean_Recommend(Cluster_Struct& CLUSTERS,vector<Point>& users){
	int Max_neighbors = 20;
	int query_points = users.size();
	int found_items = 0;
	vector<vector<double>> result;
	for(int query = 0 ; query < query_points; query++){

		cout <<std::endl<<"--------------------"<<std::endl;
		cout <<std::endl<<"--------------------"<<std::endl;
		cout <<"(euclidean)Query:"<< query<<std::endl;
		int items_found = 0;		
		cout<<"Cluster-near neighbours:"<<std::endl;		
		Point& p = users.at(query); 	
		vector<Distance> neighbors;
		vector<Point>* cluster;
		for(int i = 0  ; i < CLUSTERS.size()  ;i++ ){
			if(CLUSTERS.at(i)->at(0).get_centroid_id() == p.get_centroid_id()){
				cluster = CLUSTERS.at(i);
			}
		}
		for(vector<Point>::iterator it = cluster->begin() ; it != cluster->end() ;it++ ){
			Point& point = *it;						
			data_type dist = EuclideanDistance(point,p);	
			cout<<"\t\t-- found Item ID"<<point.get_id()<<std::endl;
			if(point.get_id() != p.get_id()){
				neighbors.push_back(Distance(dist,point));
			}
			else cout<<"\t\t\tsame id"<<endl;			
		}		
		comparison c;
		equality eq;
		std::sort(neighbors.begin(),neighbors.end(),c);
		auto last = std::unique(neighbors.begin(), neighbors.end(),eq);
		neighbors.erase(last, neighbors.end());
		cout<<"Printing Best "<<Max_neighbors<<" Distances"<<endl;
		int i = 0;
		for(vector<Distance>::iterator it = neighbors.begin() ; it != neighbors.end() ; it++){
			cout<<it->getPoint()->get_id()<<": "<<it->getDist()<<endl;
			i++;
			if(i == Max_neighbors)break;
		}
		result.push_back(CalculateWeightedRecommendation(neighbors));	
	}
	return result;
}


