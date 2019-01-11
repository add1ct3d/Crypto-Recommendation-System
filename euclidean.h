
#ifndef EUCLIDEAN_H
#define EUCLIDEAN_H
using namespace std;


int Bucket_Range_Euclidean(int bucket,HashTable* hash,vector<int>& gv,Point& p,
	double R,double e,vector<Point>& centroids,vector<Point*>& points_to_assign,int centroid,bool compare_gs);
int euclidean_h(Point& point,double w,int d,std::vector<data_type>* v, data_type t);
void euclidean_g(std::vector<int>& ,std::vector<int>& h,std::vector<int>& shuffle_idents,int K);
int euclidean_phi(std::vector<int>& r, std::vector<int>& g,int K,int TableSize);
string euclidean_f(std::vector<int>& h);

#endif