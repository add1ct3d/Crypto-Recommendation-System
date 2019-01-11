
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
#include "cube.h"
#include "cosine.h"
#include "utility.h"

using namespace std;


char cosine_h(Point& point,vector<data_type>* r,int K){
	
	string s;
	s.assign(point.get_all_fields());
	boost::char_separator<char> sep(", \t");

	boost::tokenizer<boost::char_separator<char>> tok(s, sep);

	vector<data_type> p;
	p.clear();
	for (boost::tokenizer<boost::char_separator<char>>::iterator it=tok.begin(); it!=tok.end();it++) {
		string s;
		s.assign(*it);
		data_type item = atof(s.c_str());
		p.push_back(item);
	}
	assert(p.size() == r->size());

	if((data_type)inner_product(p.begin(),p.begin()+p.size(),r->begin(),0) >= 0){
		return '1';
	}
	else{
		return '0';

	}	
}

int cosine_g(string& h,vector<int>& shuffle,int K){

	string result;
	result.clear();

	for(int i = 0 ; i  < K ; i++){
		int ident = shuffle[i];
		assert(ident >= 0);
		result.push_back(h.at(shuffle[i]));
	}

	return b2d(atoi(result.c_str()));

} 



int Bucket_Range_Cosine(int bucket,HashTable* hash,string& gv,Point& p,
	double R,double e,vector<Point>& centroids,vector<Point*>& points_to_assign,int centroid,bool compare_gs){

	int points_assigned = 0;
	assert(bucket >= 0);
	for(vector<Point*>::iterator it = ((hash->data).at(bucket)).begin() ; 
		it != ((hash->data).at(bucket)).end() ;it++ ){

		Point* point_ptr = *it;
		if(compare_gs == true){	
			if(point_ptr->cosine_g != gv){
				continue;
			}
		}
	
		data_type dist = CosineDistance(*point_ptr,p);
		//cout<<"dist = "<<dist<<endl;
		assert(dist >= 0);

		if(dist <= (1+e)*R){	
			if((point_ptr->is_assigned()) == true) // the point has been locked
				continue;
			if(point_ptr->get_centroid_id() == -1){ // the point has not been assigned to a cluster
				cout<<"Push "<<point_ptr->get_id()<<" to " << centroid<<endl;
				point_ptr->set_centroid(centroid,centroids.at(centroid).get_all_fields());
				points_to_assign.push_back(point_ptr);
				assert(points_to_assign.back()->is_assigned() == false);
				points_assigned++;
			}
			else if((point_ptr->get_centroid_id()) != centroid){
				//the point has been assigned to a cluster but only temporarily
				//check if this centroid is closer than assigned centroid.
				//if it is,assign to this centroid.
				//cout<<"collision"<<endl;
				int old_id = point_ptr->get_centroid_id();
				data_type dist_old = CosineDistance(centroids.at(old_id),*point_ptr);
				data_type dist_new = CosineDistance(centroids.at(centroid),*point_ptr);
				if(dist_new <= dist_old){
					cout<<"Moved from cluster "<<old_id<< " to cluster "<< centroid << endl;
					point_ptr->set_centroid(centroid,p.get_all_fields());
				}
			}					
		}	
	}
	assert(points_assigned >= 0);
	return points_assigned;
}