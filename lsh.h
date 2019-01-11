#ifndef LSH_H
#define LSH_H
using namespace std;



void LSH_Build_Euclidean(vector<HashTable>&  HashTables,int L,int d,vector<Point>& dataset);
void LSH_Hash_Euclidean(vector<HashTable>&  HashTables,int L,int d,vector<Point>& dataset);
void LSH_Init_Euclidean(vector<HashTable>&  HashTables,int L,int d);

void LSH_Range_Euclidean(Cluster_Struct& CLUSTERS,std::vector<HashTable>&  HashTables,int L,int d,
	double R,std::vector<Point>& centroids,string& output_path);

void LSH_Build_Cosine(vector<HashTable>&  HashTables,int L,int d,vector<Point>& dataset);
void LSH_Init_Cosine(vector<HashTable>&  HashTables,int L,int d);

void LSH_Range_Cosine(Cluster_Struct& CLUSTERS,vector<HashTable>&  HashTables,int L,int d,double R,
	vector<Point>& centroids,string& output_path);



#endif