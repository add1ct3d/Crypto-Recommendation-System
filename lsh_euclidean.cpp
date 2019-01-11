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
#include "utility.h"
#include "euclidean.h"
using namespace std;


/* ========================   LSH   BUILD =====================
	LSH_Build stores all points from input dataset into the given HashTable using Locality Sensitive Hashing
	and euclidean metric for distances calculation.

	LSH Parameters: 
		L : number of hashtables
		d : number of dimensions
		n : number of points
		centroids : a vector of Points that represent the centres of the clusters.
		output_path : the output stream

	Additional HashTable parameters:
		K = number of h-functions
		TableSize : number of buckets per hash_table
	Method:
		Preprocessing
			Create K h-functions.
			For each hash table :
				select g , which is a vector of distinctly shuffled h-functions.
		Hashing:
			For each point in dataset:
				For each hash table :
					create g vector
					hash g vector using phi-function
					store point in bucket
 ============================================================
*/




void LSH_Build_Euclidean(vector<HashTable>&  HashTables,int L,int d,vector<Point>& dataset){
	cout<<"Building LSH"<<endl;
	LSH_Init_Euclidean(HashTables,L,d);	
	LSH_Hash_Euclidean(HashTables,L,d,dataset);
}


void LSH_Init_Euclidean(vector<HashTable>&  HashTables,int L,int d){

	int K = HashTables.at(0).K; //number of h functions
	double W = HashTables.at(0).W; // window size : determines the size of the buckets. 
	int TableSize = HashTables.at(0).TableSize; // number of buckets per hash_tabl

	/* ================ v and t initialization. ===============
		v : each hash table has it's own v
			v consists of K random d-dimensional points
			v is formed by normal distribution.
			it is used in the calculation of the 
			inner product of p*v in euclidean h
		
		t : each hash table has it's own t
			t consists of K floating point numbers
			t is formed by uniform distribution
			it is used in order to "move" the 
			inner product pv by a little bit.
			t is dependent on window size W.
			(euclidean_h)
	============================================================
	*/

  	random_device normal_generator; // seed for normal distr
	normal_distribution<double> normal_distribution(0.0,1.0);		//normal distribution

	random_device generator;		// seed for uniform distr
	uniform_real_distribution<double> uni_distribution(0.0,W-1); 	//uniform distribution

	for(int l = 0 ; l < L ; l++){

		HashTable* h = &(HashTables.at(l)); 
		h->v.clear();
		h->t.clear();
		h->v.reserve(K);//optimization
		h->t.reserve(K); 
		for(int k = 0 ; k < K ; k++){
			
			h->v.push_back(new vector<data_type>);

			h->v.at(k)->clear();
			h->v.at(k)->reserve(d);
			for(int j = 0 ; j  < d ; j++){
				double item = normal_distribution(normal_generator);
				h->v.at(k)->push_back(item);
			}
			h->t.push_back(uni_distribution(generator));
		}
	}
/* 	======= r Initialization  ========
	r is used in euclidean phi to 
	calculate the inner product of rh
	
	each hash table has it's own r vector
	r consists of K integers 
	==================================
*/

	random_device rd; //seed
	uniform_int_distribution<int> uniform_distribution(-20,20);

	for(int i = 0 ; i < L ; i++){
		HashTable* h = &(HashTables.at(i)); 
		h->r.clear();	
		h->r.reserve(K);	
		for(int j = 0 ; j < K; j++){
			int item = uniform_distribution(rd);
			h->r.push_back(item);
		}
	}

	/* 	========= g function preprocessing ========
	g is a vector of distinctly shuffled h-functions
	shuffling is assisted by the shuffle_idents vector.
	shuffle_idents is a vector of integers.
	The integers are selected by a uniform distribution.
	e.g   	H : h1 h2 h3 h4 h5 h6
			shuffle_idents = 2 3 1 4 6 5
			=>  g = <h2 h3 h1 h4 h6 h5>	
		===========================================
	*/

	random_device shuffle_generator; // seed
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



/*	==============  Hashing  ================
For each point in dataset:
	For each hash table :
		create g vector usin g_function
		hash g vector using phi-function
		store point in bucket
	========================================= 
*/
void LSH_Hash_Euclidean(vector<HashTable>&  HashTables,int L,int d,vector<Point>& dataset){

	int K = HashTables.at(0).K; //number of h functions
	double W = HashTables.at(0).W; // window size : determines the size of the buckets. 
	int TableSize = HashTables.at(0).TableSize; // number of buckets per hash_tabl
	data_type e = 0.05; // acceptance to error

	for(int i = 0 ; i < dataset.size() ; i++){

		cout<<"(euclidean)point "<<i+1<<endl;	
		int bucket = -1;
		vector<int> gv[L];
		for(int l = 0 ; l < L ; l++){
			
			HashTable* hash = &(HashTables.at(l));		
			
			vector<int> h;
			h.clear();
			h.reserve(K);

			assert(hash->v.size() == K);
			assert(hash->t.size() == K);
			for(int k = 0 ; k < K ; k++){
				int h_val = euclidean_h(dataset.at(i),W,d,hash->v.at(k),hash->t.at(k));
				h.push_back(h_val);
			}
			assert(h.size() == K);
			gv[l].clear();
			euclidean_g(gv[l],h,hash->shuffle_idents,K);
		}

		for(int l = 0 ; l < L ; l++){
			HashTable* hash = &(HashTables.at(l));
			int bucket = euclidean_phi(hash->r,gv[l],K,TableSize);
			vector<Point*>& myv = (hash->data).at(bucket);			
			dataset.at(i).g = gv[l]; 
			myv.push_back(&(dataset.at(i)));
		}
	}
}



/* =================  Euclidean LSH Range Search  ===================
	Implements clustering of points accessed by L Hash Tables.
	Input :  Initialized Hash Tables and centroids
	Output : Clusters 

	Method :
		Starting from given Range R:
		For a number of loops repeat:
			For each centroid/cluster:
				For each Hash Table:
					Hash centroid using h,g and phi functions
					Search in bucket to find points within R distance from centroid
					if points are not allocated to a cluster , assign them temporarily
					else
						if point closer to the new centroid
							assign temporarily to new centroid
					if there are no points within range in bucket
						Randomly select a neighbor bucket
						Search in neighbor bucket for points
			Assign points found within Range to centroids.
			Double Range R
*/


void LSH_Range_Euclidean(Cluster_Struct& CLUSTERS,vector<HashTable>&  HashTables,int L,int d,double R,
	vector<Point>& centroids,string& output_path){
	
	cout<<"R = |"<<R<<"|"<<endl;
	data_type e = 0.05; // acceptance to error
	data_type init_R = R; //initial radius
	float average_time = 0;
	int centroid_points = centroids.size();
	
	int K = HashTables.at(0).K;
	double W = HashTables.at(0).W;
	int TableSize = HashTables.at(0).TableSize;


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
			//hash centroid using h and g (look at LSH_Euclidean_hashing for more details)
			for(int l = 0 ; l < L ; l++){
				HashTable* hash = &(HashTables.at(l));
				vector<int> h;
				h.clear();
				for(int k = 0 ; k < K ; k++){
					int h_val = euclidean_h(p,W,d,hash->v.at(k),hash->t.at(k));
					h.push_back(h_val);
				}
				assert(h.size() == K);

				gv[l].clear();
				euclidean_g(gv[l],h,hash->shuffle_idents,K);
			}

			//hash centroid using phi (look at LSH_Euclidean_hashing for more details)
			for(int l = 0 ; l < L ; l++){
				cout<<"\t HashTable :"<<l<<endl;
				HashTable* hash = &(HashTables.at(l));
				int bucket = euclidean_phi(hash->r,gv[l],K,TableSize);
				//Range search in bucket
				int assigned_points = Bucket_Range_Euclidean(bucket,hash,gv[l],p,R,e,centroids,points_to_assign,centroid,true);
				if(assigned_points == 0){
					//if no points found , flip a coin and search in adjacent bucket
					bucket = SelectNeighborBucket(bucket,TableSize);
					Bucket_Range_Euclidean(bucket,hash,gv[l],p,R,e,centroids,points_to_assign,centroid,true);
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



