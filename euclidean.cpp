
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
#include <chrono>

#include "NN_classes.h"
#include "lsh.h"
#include "utility.h"

using namespace std;

/*	================= g function ======================
	g is a vector of distinctly shuffled h-functions
	shuffling is assisted by the shuffle_idents vector.
	shuffle_idents is a vector of integers.
	The integers are selected by a uniform distribution.
	e.g   	H : h1 h2 h3 h4 h5 h6
		shuffle_idents = 2 3 1 4 6 5
		=>  g = <h2 h3 h1 h4 h6 h5>	
	==================================================
*/
void euclidean_g(vector<int>& gv,vector<int>& h,vector<int>& shuffle,int K){
	
	gv.clear();
	for(int i = 0 ; i  < K ; i++){
		int ident = shuffle.at(i);
		assert(ident >= 0);
		gv.push_back(h.at(ident));
	}

} 


/*	================= h function ======================
	implementation of  h function : calculate (p*v + t) /w
	p*v is the inner product of the given point and
	a randomly generated v vector.

	t is the random movement of the partitions and is
	determined by W.

	W is the windows size and determines the cells'size

	because the coordinates are small (< 0 ) I multiply
	the fraction by 100.
	(or else everything would turn to 0 since h converts
	result to int)
	==================================================
*/
int euclidean_h(Point& point,double w,int d,vector<data_type>* v,data_type t){


	string s;
	s.assign(point.get_all_fields()); //s = point coordinates
	boost::char_separator<char> sep(", \t"); //remove spaces , commas and tabs
	boost::tokenizer<boost::char_separator<char>> tok(s,sep);

	vector<data_type> p; //holds in a vector form
	p.clear();
	for (boost::tokenizer<boost::char_separator<char>>::iterator it=tok.begin(); it!=tok.end();it++) {
		data_type item = atof(it->c_str());
		p.push_back(item);
	}

	double init_value = 0;
	assert(v->size() == p.size());
	double pv = (data_type)inner_product(p.begin(),p.begin()+p.size(),v->begin(),init_value);

	assert(p.size() == d);
	assert(v->size() == p.size());
	assert(w > 0);
	double result = ((pv+t)*100)/(double(w));
	return (int)result;
}


/*	================= phi function ======================
	implementation of phi function : 
		calculate (r*h  mod M ) mod TableSize
	r*h is the inner product of the given h family and
	a randomly generated r vector.

	M is used to avoid overflows.
	TableSize is the number of buckets per hash table
	because the coordinates are small (< 0 ) I multiply
	resulting number resides in [0,TableSize)
	==================================================
*/

int euclidean_phi(vector<int>& r,vector<int>& g,int K,int TableSize){

	assert(g.size() == r.size());
	assert(g.size() > 0);

	long long rh = inner_product(g.begin(),g.begin()+g.size(),r.begin(),0);


	long long M = pow(2,52)-5;
	assert(TableSize > 0);
	int temp_mod = long_modulo(rh,M);
	int bucket = modulo(temp_mod,TableSize);
	assert(bucket >= 0);
	return bucket;


}




string euclidean_f(std::vector<int>& h){
	string s;
	s.clear();
	
	for(int i = 0 ; i < h.size() ; i++){

		unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
		std::default_random_engine generator(seed);
		std::uniform_real_distribution<data_type> uniform_distribution(0.0,1.0);
		
		data_type fi = uniform_distribution(generator);
 		std::uniform_int_distribution<> dis(0,1);
 		long long random_num = floor(10*fi + h.at(i));

		if((random_num % 2) == 0)
			s.push_back('0');
		else
			s.push_back('1'); 
	}
	return s;

}




/* 	====================== Bucket Range Search =========================
	Search in bucket to find points within R distance from centroid
	if points are not allocated to a cluster , assign them temporarily
	else
		if point closer to the new centroid
			assign temporarily to new centroid

	use a vector of Point* to keep track of the assignments.
	====================================================================
*/
int Bucket_Range_Euclidean(int bucket,HashTable* hash,vector<int>& gv,Point& p,
	double R,double e,vector<Point>& centroids,vector<Point*>& points_to_assign,int centroid,bool compare_gs){

	int points_assigned = 0;
	assert(bucket >= 0);
	for(vector<Point*>::iterator it = ((hash->data).at(bucket)).begin() ; 
		it != ((hash->data).at(bucket)).end() ;it++ ){

		Point* point_ptr = *it;
		if(compare_gs == true)
			if(point_ptr->g != gv)continue;

		data_type dist = EuclideanDistance(*point_ptr,p);
		assert(dist >= 0);

		if(dist <= (1+e)*R){	
			if((point_ptr->is_assigned()) == true) // the point has been locked
				continue;
			if(point_ptr->get_centroid_id() == -1){ // the point has not been assigned to a cluster
				point_ptr->set_centroid(centroid,centroids.at(centroid).get_all_fields());
				points_to_assign.push_back(point_ptr);
				assert(points_to_assign.back()->is_assigned() == false);
				points_assigned++;
			}
			else if((point_ptr->get_centroid_id()) != centroid){
				int old_id = point_ptr->get_centroid_id();
				data_type dist_old = EuclideanDistance(centroids.at(old_id),*point_ptr);
				data_type dist_new = EuclideanDistance(centroids.at(centroid),*point_ptr);
				if(dist_new <= dist_old){
					point_ptr->set_centroid(centroid,p.get_all_fields());
				}
			}					
		}	
	}
	assert(points_assigned >= 0);
	return points_assigned;
}

