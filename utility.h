#ifndef UTILITY_H
#define UTILITY_H


double EuclideanDistance(Point& p1,Point& p2);
double CosineDistance(Point& p1,Point& p2);
int hammingDistance(string& s1,string& s2);
int BinaryToDecimal(string& h);
int modulo(int x,int y);
int long_modulo(int x,long long y);
unsigned b2d(unsigned num);
int BinaryToDecimal(string& h);
int CalculateDimensions(string& s);
void PrintClusters(Cluster_Struct& CLUSTERS,vector<Point>& centroids,bool show_id,string& output_path);
void PrintCentroids(std::vector<Point>& centroids);
bool  CompareProbabilities(Probability* i,Probability* j ) ;
data_type Silhouette(Cluster_Struct& CLUSTERS,std::vector<Point>& centroids,string& metric,string& output_path);
void AssignNeighbors(std::vector<Point>& centroids,int* Neighbors,string metric);
data_type MeanClusterDist(Point& p, std::vector<Point>* D,string metric);
void PrintArray(void* Array,int size,string type);
int getNeighbor(Point& p,int p_cluster,std::vector<Point>& centroids,string metric);
void PrintAll(std::vector<HashTable>&  HashTables,int L) ;
void PrintHash(HashTable& hash) ;
void AssignPoints(Cluster_Struct& CLUSTERS,std::vector<Point*>& points_to_assign);
int SelectNeighborBucket(int bucket,int TableSize);
double MinCentroidsHalfDist(vector<Point>& centroids,string& metric);
#endif