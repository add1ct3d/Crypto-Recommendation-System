#include <iostream>
#include <string>
#include <cstdlib> // for exit
#include <fstream>
#include <stdlib.h>
#include <vector> 
#include <random>
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
#include <numeric>//std::inner_product
#include <unordered_map>
#include <math.h>
#include <complex> // std::complex, std::norm
#include <ctime>
#include <limits> // max double

using namespace std;


#include "NN_classes.h"
#include "utility.h"



void Assign_Lloyd(Cluster_Struct& CLUSTERS,std::vector<Point>& dataset,std::vector<Point>& centroids,std::string metric){
//Lloyd’s assignment - Assign each object to its nearest center.
	cout<<"Lloyd’s assignment"<<endl;
	assert((metric == "euclidean") or (metric == "cosine"));

	for(int i = 0 ; i < dataset.size() ; i++){
		if(dataset.at(i).is_assigned())continue;
		double min_dist = std::numeric_limits<double>::max();
		double min_centroid_ident = -1;

		cout<<"ASSIGN : Point :"<<i+1<<endl;
		for(int c = 0 ; c < centroids.size() ; c++){
			double dist;
			if(metric == "euclidean")
				dist = EuclideanDistance(dataset.at(i),centroids.at(c));
			else
				dist = CosineDistance(dataset.at(i),centroids.at(c));
			
			if(dist < min_dist){
				min_dist = dist;
				min_centroid_ident = c;
				assert(min_dist >= 0);			
			}
		}
		cout<<"assign point:"<<dataset.at(i).get_id()<<" to centroid:"<<min_centroid_ident<<endl;
		CLUSTERS.at(min_centroid_ident)->push_back(dataset.at(i));
	}

	return ;
}
