/*
*********************************
Project Clustering
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
#include "cluster.h"
#include "lsh.h"
#include "euclidean.h"
#include "cosine.h"
#include "utility.h"

using namespace std;




int main(int argc,char* argv[]){




	if ((argc < 9)and(argc > 1)){
		cout << "Too few input arguments to function.(argc = "<<argc<<", expected > 3)"<<endl;
		exit(0);
	}

	clock_t total;
	total = clock();	

	string input_path;
	string config_path;
	string output_path;
	string metric;
	int K = 4; //number of hash functions
	int d = 0; //number of point dimensions
	int L = 5; //number of hashtables
	int num_clusters = K; 
	double W = 2.0; // window size used by  LSH
	int n; // int number of points
	long long M; // used by LSH for overflow prevention
	int max_loops = 1;
	int max_points = -1;
	int update_method = 0;
	int init_method = 0;
	int TableSize = 0;
	double R = 1;

	//Initialization
	if(argc == 1){
		input_path.assign("./input.small");
		config_path.assign("./cluster.conf");
	}

	for(int i = 1 ; i < argc ; i++){
		string s(argv[i]);
		if((s == "-i")and(argc > i+1)){
			string p = argv[i+1];
			input_path.assign(p);
		}
		else if((s == "-c")and(argc > i+1)){
			string p = argv[i+1];
			config_path.assign(p);
		}
		else if((s == "-o")and(argc > i+1)){
			string p = argv[i+1];
			output_path.assign(p);
		}
		else if((s == "-d")and(argc > i+1)){
			string p = argv[i+1];
			metric.assign(p);
		}
		else;
	}

	ifstream config;
	config.open(config_path.c_str()); // Open Configuration File to read the rest of the parameters
	if (config.is_open()){
		string line;
		while ( getline (config,line)){

			boost::char_separator<char> sep(": \t\n"); // Tokenize Line to extract the values
			boost::tokenizer<boost::char_separator<char>> tok(line, sep);

			for (boost::tokenizer<boost::char_separator<char>>::iterator it=tok.begin(); it!=tok.end();it++) {
				string s;
				s.assign(*it);

				if(s == "number_of_clusters"){  //Arguments don't have to be in a specific order
					it++;
					if(it != tok.end())
						num_clusters = atoi(it->c_str());
					else break;
				}
				else if( s == "number_of_hash_functions"){
					it++;
					if(it != tok.end())
						K = atoi(it->c_str());
					else break;
				}
				else if (s == "number_of_hash_tables"){
					it++;
					if(it != tok.end())
						L = atoi(it->c_str());
					else break;
				}
				else if (s == "number_of_points"){
					it++;
					if(it != tok.end())
						max_points = atoi(it->c_str());
					else break;
				}
				else if (s == "number_of_loops"){
					it++;
					if(it != tok.end())
						max_loops = atoi(it->c_str());
					else break;
				}
				else if (s == "update_method"){
					it++;
					if(it != tok.end())
						update_method = atoi(it->c_str());
					else break;

				}	
				else if (s == "init_method"){
					it++;
					if(it != tok.end())
						init_method = atoi(it->c_str());
					else break;

				}
				else if( s == "TableSize"){
					it++;
					if(it != tok.end())
						TableSize = atoi(it->c_str());
					else break;
				}	
				else if( s == "R"){
					it++;
					if(it != tok.end())
						R = atof(it->c_str());
					else break;
				}	

				else if( s == "W"){
					it++;
					if(it != tok.end())
						W = atof(it->c_str());
					else break;
				}	
				else ; 
			}
			
		}
		config.close();
	}

	vector<Point> dataset; 
	ifstream input;
	n  = 0;
	input.open(input_path.c_str());
	if (input.is_open()){

		string line;
		while ( getline (input,line)){
			
			if((max_points != -1)&&(n == max_points))break;
			Point* ptr = new Point(line);
			dataset.push_back(*ptr);
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

	if(TableSize == 0){
		if(metric == "euclidean")TableSize = n/8;
		else if(metric == "cosine") TableSize = pow(2,K);
	}
	else if (metric == "cosine")TableSize = pow(2,K);
	vector<Point> centroids;
	if(init_method == 0)
	 	centroids = Init_Random(dataset,num_clusters);
	else 
		centroids = Init_K_means(dataset,num_clusters,metric);
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
	clock_t clustering_time;
	clustering_time = clock();
	assert((metric == "euclidean")||(metric == "cosine"));
	if(metric == "euclidean"){
		LSH_Build_Euclidean(HashTables,L,d,dataset);
	}
	else{
		LSH_Build_Cosine(HashTables,L,d,dataset);
	}


	ofstream output;
	output.open(output_path.c_str(),ofstream::out | ofstream::app);
	if (!output.is_open()){
		cout<<"Output path not correct"<<endl;
		exit(-1);
	}
	output<<"Algorithm : "<<init_method+1<<"x2x"<<update_method+1<< " "<<metric<<endl;

	int loops = 0;
	while( loops < max_loops){
		if(loops != 0){
			if(update_method == 0)
				K_means(CLUSTERS,centroids,metric,d);
			else if(update_method == 1)
				K_medoids(CLUSTERS,centroids,metric,d);
		}
		R = MinCentroidsHalfDist(centroids,metric);
		if(metric == "euclidean")
			LSH_Range_Euclidean(CLUSTERS,HashTables,L,d,R,centroids,output_path);
		else{
			LSH_Range_Cosine(CLUSTERS,HashTables,L,d,R,centroids,output_path);
		}
		Assign_Lloyd(CLUSTERS,dataset,centroids,metric);
		for(int i = 0; i < dataset.size(); i++){
			Point& p = dataset.at(i);
			p.unassign();
			p.set_centroid(-1);
		}
		if(update_method == 0){
			PrintClusters(CLUSTERS,centroids,true,output_path);
		}
		else if(update_method == 1){
			PrintClusters(CLUSTERS,centroids,true,output_path);
		}
		loops++;
		if(loops == max_loops)break;
	}
	clustering_time = clock() - clustering_time;
	output<<"clustering_time: "<<(float)clustering_time/CLOCKS_PER_SEC<<endl;;
	Silhouette(CLUSTERS,centroids,metric,output_path);
	output.close();
	return 0;
}