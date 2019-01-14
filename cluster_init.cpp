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
#include <algorithm>    // std::sort
 

#include "NN_classes.h"
#include "utility.h"

using namespace std;

std::vector<Point> Init_Random(std::vector<Point>& dataset,int num_clusters){
	
	//cout<<"Random selection of "<<num_clusters<<" points (simplest)"<<endl;
	std::vector<Point> centroids;
	centroids.clear();
	vector<int> random_points;
	for(int i = 0  ; i < dataset.size() ; i++){
		random_points.push_back(i);
	}	
	random_shuffle(random_points.begin(),random_points.end());
	for(int i = 0 ; i < num_clusters  ; i++){
		dataset.at(random_points.at(i)).set_centroid(i+1,dataset.at(random_points.at(i)).get_all_fields());
		centroids.push_back(dataset.at(random_points.at(i)));
	}
	assert(centroids.size() == num_clusters);
	return centroids;
}


std::vector<Point> Init_K_means(std::vector<Point>& dataset,int num_clusters,string metric){

	assert((metric == "euclidean")||(metric == "cosine"));
	std::vector<Point> centroids;
	std::random_device gen;
	uniform_int_distribution<int> distr(0,dataset.size()-1);
	centroids.push_back(dataset.at(distr(gen)));
	cout<<"Random centroid = "<<centroids.back().get_all_fields()<<endl;

	while(centroids.size() != num_clusters){
		cout<<"Initialize Centroid no"<<centroids.size()<<endl;
		DistancesArray Distances;
		Distances.clear();
		std::vector<Probability*> Probabilities;
		Probabilities.clear();
		Probabilities.push_back(new Probability(NULL,0));//P(0)=0	
		Probabilities.reserve(dataset.size() - centroids.size());
		for(int i = 0 ; i < dataset.size() ; i++){
			
			Distances.push_back(new std::vector<DistanceInfo>());
			Distances.back()->reserve(centroids.size());
			data_type max_dist = -1;

			bool is_a_centre = false;
			for(int c = 0 ; c < centroids.size() ; c++){
				if(dataset.at(i).get_id() == centroids.at(c).get_id()){
					is_a_centre = true;
					break;
				}
			}
			if(is_a_centre)continue;
			for(int c = 0 ; c < centroids.size() ; c++){
	
				data_type dist;				
				if(metric == "euclidean")
					dist = EuclideanDistance(dataset.at(i),centroids.at(c));
				else
					dist = CosineDistance(dataset.at(i),centroids.at(c));
				if(max_dist == -1)
					max_dist = dist;
				else if (dist > max_dist)
					max_dist = dist;
				else ;	
				assert(dist >= 0);
				assert(dist*dist >= 0);
				Distances.back()->push_back(
					DistanceInfo(dataset.at(i).get_id(),centroids.at(c).get_id(),dist*dist)
				);
			}
			//To avoid the P being very high we normalize the D by diving max D
			assert(max_dist >= 0);
			data_type sum_dists = 0;
			for(int c = 0 ; c < centroids.size() ; c++ ){
				Distances.back()->at(c).normalize_dist(max_dist);
				sum_dists += Distances.back()->at(c).get_dist();
			}
			Probabilities.push_back(new Probability(&(dataset.at(i)),sum_dists));			
		}

		//sort probabilities in acending order depending on their value
		std::sort(Probabilities.begin(),Probabilities.end(),CompareProbabilities);
		data_type max_prob = Probabilities.at(Probabilities.size()-1)->get_value();
		std::random_device gen;
		uniform_real_distribution<double> distr(0,max_prob);
		
		data_type random_float = distr(gen);
		Probability* SearchItem = new Probability(NULL,random_float);
		std::vector<Probability*>::iterator up;
		up =  std::upper_bound (Probabilities.begin(),Probabilities.end(),SearchItem,
									CompareProbabilities); // 
		int pos = up - Probabilities.begin();
		centroids.push_back(*((*up)->get_point()));
		assert((*up)->get_point()->get_id() == Probabilities.at(pos)->get_point()->get_id());
	}
	return centroids;
}