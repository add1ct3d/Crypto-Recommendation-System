/*

*********************************
Project LSH
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

using namespace std;


void LSH_Build_Cosine(vector<HashTable>&  HashTables,int L,int d,vector<Point>& dataset){
	cout<<"Building LSH"<<endl;
	LSH_Init_Cosine(HashTables,L,d);
	cout<<"init"<<endl;	
	LSH_Hash_Cosine(HashTables,L,d,dataset);
	cout<<"Hash"<<endl;
}


void LSH_Init_Cosine(vector<HashTable>&  HashTables,int L,int d){


	int K = HashTables.at(0).K;
	int TableSize = HashTables.at(0).TableSize;
	TableSize = pow(2,K);
	cout<<"TableSize = "<<TableSize<<endl;
	assert(TableSize > 0);
	assert(L > 0);

	random_device normal_generator;
	normal_distribution<double> normal_distribution(0.0,1.0);


	for(int l = 0 ; l < L ; l++){
		HashTable* hash = &(HashTables.at(l));
		hash->cosine_r.reserve(K);//optimization
		for(int k = 0 ; k < K; k++){ //plithos h	
			hash->cosine_r.push_back(new vector<data_type>);
			hash->cosine_r.at(k)->clear();
			hash->cosine_r.at(k)->reserve(d);

			for(int y = 0 ; y < d ; y++){
				double item = normal_distribution(normal_generator);
				hash->cosine_r.at(k)->push_back(item);
			}
			assert(hash->cosine_r.at(k)->size() == d);
		}
		assert(hash->cosine_r.size() == K);
	}
	cout<<"1"<<endl;

	
	random_device shuffle_generator;
	uniform_int_distribution<int> shuffle_distribution(0,K-1);
	bool diff=false; //flag to make sure that all g functions are unique
	while(diff != true){
		for(int i = 0 ; i < L ; i++){
			HashTable* h = &(HashTables.at(i));
			h->shuffle_idents.clear();
			h->shuffle_idents.reserve(K);
			for(int j = 0 ; j < K;j++){
				int item = shuffle_distribution(shuffle_generator);
				h->shuffle_idents.push_back(item);
			}
			diff = true; // make sure that all previously created shufflings were different
			for(int k = 0 ; k < i ; k++){
				if(h->shuffle_idents == HashTables[k].shuffle_idents){
					diff = false;
					break;
				}
			}
			if(diff == false){ //aka shufflings are not unique
				break; //repeat process from the beginning
			}
		}
	}

}


void LSH_Hash_Cosine(vector<HashTable>&  HashTables,int L,int d,vector<Point>& dataset){
	int K = HashTables.at(0).K;
	int TableSize = HashTables.at(0).TableSize;
	TableSize = pow(2,K);
	cout<<"TableSize = "<<TableSize<<endl;
	assert(TableSize > 0);
	assert(L > 0);	


	for(int i = 0 ; i < dataset.size() ; i++){
		cout<<"(cosine) point "<<i<<endl;

		for(int l = 0 ; l < L ; l++){



			HashTable* hash = &(HashTables.at(l));
			string gs;			
			for(int k = 0 ; k < K  ; k++){
				gs.push_back(cosine_h(dataset.at(i),hash->cosine_r.at(k),K));
			}	

			int bucket = cosine_g(gs,hash->shuffle_idents,K);
			bucket = bucket % TableSize;

			vector<Point*>& myv = (hash->data).at(bucket);			
			dataset.at(i).cosine_g = gs; 
			myv.push_back(&(dataset.at(i)));
		
		}
	}
}





void LSH_Range_Cosine(Cluster_Struct& CLUSTERS,vector<HashTable>&  HashTables,int L,int d,double R,
	vector<Point>& centroids,string& output_path){

	cout<<"R = |"<<R<<"|"<<endl;
	data_type e = 0.05; // acceptance to error
	data_type init_R = R; //initial radius


	float average_time = 0;
	int centroid_points = centroids.size();
	
	int K = HashTables.at(0).K;
	double W = HashTables.at(0).W;
	int TableSize = HashTables.at(0).TableSize;
	cout<<"TableSize = "<<TableSize<<endl;
	assert(TableSize > 0);
	assert(L > 0);

	int max_loops = 3; // at the end of each loop range is doubled
	R = init_R; //reset range
	for(int loop = 0 ; loop < max_loops ; loop++){

		cout<<"RANGE SEARCH R = "<<R<<endl;
		vector<Point*> points_to_assign;
		points_to_assign.clear();

		for(int centroid = 0 ; centroid < centroid_points; centroid++){
			assert(CLUSTERS.size() == centroid_points);
			cout<<" Centroid :"<<centroid<<endl;
			const clock_t begin_time = clock();
			Point& p = centroids.at(centroid); 	
			vector<int> gv[L];	

			for(int l = 0 ; l < L ; l++){
				cout<<"\t HashTable :"<<l<<endl;
				HashTable* hash = &(HashTables.at(l));
				

				string gs;	
				for(int k = 0 ; k < K  ; k++){
					gs.push_back(cosine_h(p,hash->cosine_r.at(k),K));
				}

				int bucket = cosine_g(gs,hash->shuffle_idents,K);	
				bucket = bucket % TableSize;
				assert(bucket >= 0);

				//Range search in bucket
				int assigned_points = Bucket_Range_Cosine(bucket,hash,gs,p,R,e,centroids,points_to_assign,centroid,true);
				if(assigned_points == 0){
					//if no points found , flip a coin and search in adjacent bucket
					bucket = SelectNeighborBucket(bucket,TableSize);
					Bucket_Range_Cosine(bucket,hash,gs,p,R,e,centroids,points_to_assign,centroid,true);
				}

			}
			float tLSH =  (float( clock () - begin_time ) /  CLOCKS_PER_SEC);
			average_time += tLSH;
		}
		//Assign Points to Cluster
		//if(points_to_assign.size() == 0)break;
		AssignPoints(CLUSTERS,points_to_assign);
		R*=2; // Double the radius

	}

}






