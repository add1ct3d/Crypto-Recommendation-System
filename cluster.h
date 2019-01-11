#ifndef CLUSTER_H
#define CLUSTER_H

using namespace std;




/* INITIALIZATION */
std::vector<Point> Init_Random(std::vector<Point>& dataset,int num_clusters);
std::vector<Point> Init_K_means(std::vector<Point>& dataset,int num_clusters,string metric);

/* ASSIGNMENT */
void Assign_Lloyd(Cluster_Struct& CLUSTERS,std::vector<Point>& dataset,std::vector<Point>& centroids,std::string metric);

/* UPDATE */
Point CalculateMean(std::vector<Point>* cluster ,string metric,int d);
void K_means(Cluster_Struct& CLUSTERS,std::vector<Point>& centroids,string metric,int d);
data_type ObjectiveFunction(Cluster_Struct& CLUSTERS,std::vector<Point>& centroids,string metric,int K,bool k_means);
void K_medoids(Cluster_Struct& CLUSTERS,std::vector<Point>& centroids,string metric,int d);
Point CalculateMedoid(std::vector<Point>* cluster ,string metric,int d);
#endif