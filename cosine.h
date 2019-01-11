#ifndef COSINE_H
#define COSINE_H
using namespace std;

void LSH_Hash_Cosine(vector<HashTable>&  HashTables,int L,int d,vector<Point>& dataset);
int Bucket_Range_Cosine(int bucket,HashTable* hash,string& gs,Point& p,
	double R,double e,vector<Point>& centroids,vector<Point*>& points_to_assign,int centroid,bool compare_gs);
char cosine_h(Point& point,vector<data_type>* r,int K);
int cosine_g(bitstring& gv,std::vector<int>& shuffle,int K);

#endif