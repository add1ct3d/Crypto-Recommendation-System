
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
#include "utility.h"
#include <map>
#include <algorithm>

using namespace std;

int long_modulo(int x,long long y){
	return ( (x%y)+y)%y;

}

int modulo(int x,int y){
	return ( (x%y)+y)%y;
}

int CalculateDimensions(string& s){
	boost::char_separator<char> sep(", \t");
	boost::tokenizer<boost::char_separator<char>> tok(s, sep);

	int d = 0;
	for (boost::tokenizer<boost::char_separator<char>>::iterator it=tok.begin(); it!=tok.end();it++) {
		d++;
	}
	return d;
}


double EuclideanDistance(Point& p1,Point& p2){
	string s1;
	s1.clear();
	s1.assign(p1.get_all_fields());
	boost::char_separator<char> sep(", \t");
	boost::tokenizer<boost::char_separator<char>> tok(s1, sep);
	vector<data_type> v1;
	v1.clear();

	for (boost::tokenizer<boost::char_separator<char>>::iterator it=tok.begin(); it!=tok.end();it++) {
		string s;
		s.assign(*it);
		data_type item = atof(s.c_str());
		v1.push_back(item);
	}

	string s2;
	s2.clear();
	s2.assign(p2.get_all_fields());
	boost::tokenizer<boost::char_separator<char>> tok2(s2, sep);

	vector<data_type> v2;
	v2.clear();
	for (boost::tokenizer<boost::char_separator<char>>::iterator it=tok2.begin(); it!=tok2.end();it++) {;
		string s;
		s.assign(*it);
		data_type item = atof(s.c_str());
		v2.push_back(item);

	}
	data_type dist = 0 ;

	assert(v1.size() == v2.size());

	for(int i = 0 ; i < v1.size(); i++){
		data_type coord_dist = v1.at(i)-v2.at(i);
		data_type result = coord_dist*coord_dist;
		if(result<0)result=-result;
		if(result<0)return 0;
		dist = dist + result;
	}
	if(dist != 0)
		dist = sqrt(dist);
	if(dist < 0)dist = -dist;
	return dist;
}



double CosineDistance(Point& p1,Point& p2){

	string s1 = p1.get_all_fields();
	boost::char_separator<char> sep(", \t");
	boost::tokenizer<boost::char_separator<char>> tok(s1,sep);
	vector<data_type> v1;
	v1.clear();
	for (boost::tokenizer<boost::char_separator<char>>::iterator it=tok.begin(); it!=tok.end();it++) {
		data_type item = atof(it->c_str());
		v1.push_back(item);
	}
	string s2 = p2.get_all_fields();
	boost::tokenizer<boost::char_separator<char>> tok2(s2,sep);
	vector<data_type> v2;
	v2.clear();
	for (boost::tokenizer<boost::char_separator<char>>::iterator it=tok2.begin(); it!=tok2.end();it++) {;
		data_type item = atof(it->c_str());
		v2.push_back(item);
	}
	data_type init_value = 0;
	assert(v1.size() == v2.size());
	data_type xy = (data_type)inner_product(v1.begin(),v1.begin()+v1.size(),v2.begin(),init_value);
	double sum_v1 = 0.0;
	double sum_v2 = 0.0;
	for(int i = 0 ; i < v1.size() ; i++){
		sum_v1 = sum_v1 + (v1.at(i)*v1.at(i));
		sum_v2 = sum_v2 + (v2.at(i)*v2.at(i));
	}
	data_type similarity = (xy)/(sqrt(sum_v1) * sqrt(sum_v2));
	data_type dist = 1 - similarity;
	if(dist < 0) dist = -dist;
	//assert(dist >= 0);
	return dist;
}




void PrintClusters(Cluster_Struct& CLUSTERS,vector<Point>& centroids,bool show_id,string& output_path){
	ofstream output;
	output.open(output_path.c_str(),ofstream::out | ofstream::app);
	if (!output.is_open()){
		cout<<"Output path not correct"<<endl;
		exit(-1);
	}

 	for(int i = 0 ; i < CLUSTERS.size()  ; i++){
		output<<"CLUSTER-" << i << "{size: " <<CLUSTERS.at(i)->size();
		if(show_id == true){
			output << ", centroid: "<<centroids.at(i).get_id()<< "}"<<endl;
		}
		else{
			output <<" , centroid: "<<centroids.at(i).get_all_fields()<<"}"<<endl;
		}
	}
	output.close();
}

void PrintCentroids(vector<Point>& centroids){
	cout<<"centroids : "<<endl;
	for(int i = 0 ; i < centroids.size()  ; i++){
		cout<<"\t"<<centroids.at(i).get_id()<<endl;
	}	
}


bool  CompareProbabilities(Probability* i,Probability* j ) { 
	return (i->get_value()<j->get_value()); 
}



void AssignNeighbors(vector<Point>& centroids,int* Neighbors,string metric){

	assert((metric == "euclidean") || (metric == "cosine"));
	int T = centroids.size();
	for(int c = 0 ; c < T ; c++){
		data_type min_dist = -1;
		int min_ident = 0;
		for(int d = 0 ; d < T ; d++){
			if(c == d)continue;
			data_type dist ;

			if(metric == "euclidean")
				 dist = EuclideanDistance(centroids.at(c),centroids.at(d));				
			else
				 dist = CosineDistance(centroids.at(c),centroids.at(d));		
			
			if( (min_dist == -1) || (dist < min_dist)){
				min_dist = dist;
				min_ident = d;
			}			
		}
		Neighbors[c] = min_ident;
	}
}



int getNeighbor(Point& p,int p_cluster,vector<Point>& centroids,string metric){

	assert((metric == "euclidean") || (metric == "cosine"));
	int T = centroids.size();
	data_type min_dist = -1;
	int closest_neighbor = p_cluster;

	for(int c = 0 ; c < T ; c++){

		if(c == p_cluster)continue;

		data_type dist ;
		if(metric == "euclidean")
			 dist = EuclideanDistance(p,centroids.at(c));				
		else
			 dist = CosineDistance(p,centroids.at(c));			
		
		if( (min_dist == -1) || (dist < min_dist)){
			min_dist = dist;
			closest_neighbor = c;
		}			
	}
	return closest_neighbor;
}


void PrintArray(void* Array,int size,string type){
	for(int i = 0 ; i < size; i++){
		if(type == "int")
			cout<<"Array["<<i<<"] = "<<((int*)Array)[i]<<endl;
	}
}


data_type MeanClusterDist(Point& p, vector<Point>* D,string metric){

	data_type mean_dist = 0;
	data_type dist = 0;
	if(D->size() == 0) return 0;
	for(int d = 0 ; d < D->size() ; d++){		
		if(metric == "euclidean"){
			dist += EuclideanDistance(p,D->at(d));				
		}	
		else{
			dist += CosineDistance(p,D->at(d));
		}	
	}
	mean_dist = dist/D->size();
 	return mean_dist;
}


data_type Silhouette(Cluster_Struct& CLUSTERS,vector<Point>& centroids,string& metric,string& output_path){

	assert((metric == "euclidean") || (metric == "cosine"));
	
	ofstream output;
	output.open(output_path.c_str(),ofstream::out | ofstream::app );
	if (!output.is_open()){
		cout<<"Output path not correct"<<endl;
		exit(-1);
	}


	int T = centroids.size();
	int points_num = 0;
	data_type Total_Silhouette = 0;


	output<<"Silhouette: [";
	for(int c = 0 ; c < T ; c++){

		int cluster_size = CLUSTERS.at(c)->size();	
		data_type cluster_sil = 0;
		for(int i = 0 ; i < cluster_size ; i++){
			Point& p = CLUSTERS.at(c)->at(i);	
			data_type a = MeanClusterDist(p,CLUSTERS.at(c),metric);
			int neighbor = getNeighbor(p,c,centroids,metric);
			data_type b = MeanClusterDist(p,CLUSTERS.at(neighbor),metric);
			data_type sil = a > b ? (b-a)/a : (b-a)/b;
			cluster_sil += sil;
			Total_Silhouette += sil;
			points_num++;

		}	
		cluster_sil = cluster_sil/cluster_size;
		if(c!=0)output<< ","<<cluster_sil;
		else output<<cluster_sil;
	}
	Total_Silhouette = Total_Silhouette/points_num;
	output<<","<<Total_Silhouette<<"]"<<endl;
	output.close();
 	return Total_Silhouette;
}



void PrintAll(vector<HashTable>&  HashTables,int L) {
	cout<<"----- Printing Hash -------"<<endl<<endl;
	for(int h = 0 ; h < L ; h++){
		cout <<"Hash Table "<< h << " : "<< endl;
		PrintHash(HashTables.at(h));
	}
}


void PrintHash(HashTable& Hash){
	for(int b = 0 ; b < Hash.TableSize; b++){
		int bucket_num = 0;
		cout <<"\tBucket: "<< b << " : " << endl;
		for(vector<Point*>::iterator it = ((Hash.data).at(b)).begin() ; 
			it != ((Hash.data).at(b)).end() ;it++ ){
			cout<<"\t\t"<<(*it)->get_id()<<endl;
		}
	}
}



void AssignPoints(Cluster_Struct& CLUSTERS,vector<Point*>& points_to_assign){
	int num_points = points_to_assign.size();

	for(int i = 0 ; i < num_points ; i++){
		Point* point_ptr = points_to_assign.at(i);
		assert((point_ptr->is_assigned()) == false);
		int assign_centroid = point_ptr->get_centroid_id();
		assert(assign_centroid != -1);
		point_ptr->assign();
		CLUSTERS.at(assign_centroid)->push_back(*point_ptr);
		cout<<"Assign point:"<<point_ptr->get_id()<<" to centroid:"<<assign_centroid<<endl;
	}
}




/* 	============== SelectNeighborBucket ============
		Randomly select a neighbor bucket(coin toss)
		return bucket
	================================================
*/
int SelectNeighborBucket(int bucket,int TableSize){
	if(bucket == TableSize-1)
		bucket = TableSize-2;
	else if(bucket == 0)
		bucket = 1;
	else{
		random_device coin_toss_generator;
		uniform_int_distribution<int> coin_distribution(0,1);
		int item = coin_distribution(coin_toss_generator);
		if(item == 0)
			bucket = bucket - 1;					
		else
			bucket = bucket + 1;
	}	
	assert(bucket >= 0);
	return bucket;
}




unsigned b2d(unsigned num)
{
    unsigned res = 0;

    for(int i = 0; num > 0; ++i)
    {
        if((num % 10) == 1)
            res += (1 << i);

        num /= 10;
    }

    return res;
}


int BinaryToDecimal(string& h){
	return b2d(atoi(h.c_str()));

} 


int hammingDistance(string& s1,string& s2){
	assert(s1.size() == s2.size());
	int off_bits = 0;
	for(int i = 0 ; i < s1.size() ; i++){
		if(s1.at(i) != s2.at(i))
			off_bits++;
	}

	return off_bits;

}




double MinCentroidsHalfDist(vector<Point>& centroids,string& metric){
	assert((metric == "euclidean")||(metric == "cosine"));
	double min_dist = -1;
	for(int i = 0 ; i < centroids.size()  ; i++){
		for(int j = 0 ; j < centroids.size() ; j++){
			if(i == j)continue;
			double dist;
			if(metric == "euclidean")
				dist = EuclideanDistance(centroids.at(i),centroids.at(j));
			else
				dist = CosineDistance(centroids.at(i),centroids.at(j));
			if((min_dist == -1)||(dist < min_dist))
				min_dist = dist;
		}
	}	
	return min_dist/2;

}