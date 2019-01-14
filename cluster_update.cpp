
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


/*
Update: Calculate mean of each cluster, make it new center.
*/




Point CalculateMean(std::vector<Point>* cluster ,string metric,int d){

	int T = cluster->size();

	data_type coords[T][d];
	

	for(int i = 0 ; i < T ; i++){
		string s = cluster->at(i).get_all_fields();
		boost::char_separator<char> sep(", \t");
		boost::tokenizer<boost::char_separator<char>> tok(s,sep);
		int j = 0;
		for (boost::tokenizer<boost::char_separator<char>>::iterator it=tok.begin(); it!=tok.end();it++) {
			data_type item = atof(it->c_str());
			coords[i][j] = item;
			j++;
		}
		assert(j == d);
	}


	data_type new_id = ++id;
	string new_s = "";
	new_s += boost::lexical_cast<std::string>(new_id);
	new_s += " ";
	for(int j = 0 ; j < d ; j++){
		data_type sum = 0;
		for(int i = 0 ; i < T ; i++){
			sum += coords[i][j];
		}
		data_type c = sum/T;
		new_s += boost::lexical_cast<std::string>(c);
		new_s += " ";
	}

	Point p(new_s);
	string s = p.get_all_fields();
	int p_dimensions = CalculateDimensions(s);
	assert(p_dimensions == d);
	return p;


}


/*
Point CalculateMedoid(std::vector<Point>* cluster ,string metric,int d){
	assert((metric == "euclidean") || (metric == "cosine"));
	int T = cluster->size();
	data_type min_dist = -1;
	int min_centroid = -1;

	Point mean_point = CalculateMean(cluster,metric,d);
	for(int i = 0 ; i <  T ; i++){
		data_type dist = 0;	
		if(metric == "euclidean")
			dist += EuclideanDistance(cluster->at(i),mean_point);
		else
			dist += CosineDistance(cluster->at(i),mean_point);
		assert(dist >= 0);
		
		if((min_dist == -1) || (dist < min_dist)){
			min_dist = dist;
			min_centroid = i;
		}
	}
	assert(min_centroid >= 0);
	return cluster->at(min_centroid);	
}
*/


Point CalculateMedoid(std::vector<Point>* cluster ,string metric,int d){
	assert((metric == "euclidean") || (metric == "cosine"));
	int T = cluster->size();
	data_type min_dist = -1;
	int min_centroid = -1;


	data_type Distances[T][T];
	for(int i = 0 ; i < T ; i++){
		for(int j = 0 ; j < T ; j++){
			Distances[i][j]=-1;
			Distances[j][i]=-1;
		}
	}
	for(int i = 0 ; i < T ; i++){
		data_type sum = 0;
		for(int j = 0 ; j < T ; j++){
			data_type dist;
			if(i == j)continue;
			if(Distances[i][j] != -1)
				dist = Distances[i][j];
			else{
				if(metric == "euclidean")
					dist += EuclideanDistance(cluster->at(i),cluster->at(j));
				else
					dist += CosineDistance(cluster->at(i),cluster->at(j));
				Distances[i][j] = dist;
				Distances[j][i] = dist;
			}
			sum+=dist;	
		}
		if((min_dist == -1)||(sum < min_dist)){
			min_dist = sum;
			min_centroid = i;
		}
	}
	return cluster->at(min_centroid);
}








void K_means(Cluster_Struct& CLUSTERS,std::vector<Point>& centroids,string metric,int d){

	int K = centroids.size();
	centroids.clear();
	assert(centroids.size() == 0);
 	int count = 0;
 	for(int i = 0 ; i < K ; i++){	
		cout<< "K-MEANS cluster :"<<count++<<endl;
		centroids.push_back(CalculateMean(CLUSTERS.at(i),metric,d));
	}
	assert(centroids.size() == K);

	for(int i = 0 ; i < K ; i ++){
		CLUSTERS.at(i)->clear();
	}

}



data_type ObjectiveFunction(Cluster_Struct& CLUSTERS,std::vector<Point>& centroids,string metric,int K,bool k_means){

	assert((metric == "euclidean") || (metric == "cosine"));
	data_type value = 0 ;
 	for(int i = 0 ; i < CLUSTERS.size(); i++){	
		for(int j = 0 ; j < CLUSTERS[i]->size() ; j++){			
			data_type dist;
			if(metric == "euclidean")
				dist = EuclideanDistance(CLUSTERS.at(i)->at(j),centroids.at(i));
			else
				dist = CosineDistance(CLUSTERS.at(i)->at(j),centroids.at(i));
			if(k_means)value += (dist*dist);
			else value += dist;
		}
	}
	return value;

}




void K_medoids(Cluster_Struct& CLUSTERS,std::vector<Point>& centroids,string metric,int d){

	int K = centroids.size();
	centroids.clear();
	assert(centroids.size() == 0);
 	int count = 0;
 	for(int i = 0 ; i < K ; i++){	
		cout<< "K-MEDOIDS cluster :"<<count++<<endl;
		centroids.push_back(CalculateMedoid(CLUSTERS.at(i),metric,d));
		//cout << " mean is "<<centroids.back().get_all_fields()<<endl;
	}
	assert(centroids.size() == K);

	for(int i = 0 ; i < K ; i ++){
		CLUSTERS.at(i)->clear();
	}

}
