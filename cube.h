#ifndef CUBE_H
#define CUBE_H

void Cube_Build_Cosine(CubeStruct& Cube,int d,vector<Point>& dataset);
void Cube_Init_Cosine(CubeStruct& Cube,int d);
void Cube_Hash_Cosine(CubeStruct& Cube,vector<Point>& dataset);
void Cube_Range_Cosine(Cluster_Struct& CLUSTERS,CubeStruct& Cube,int probes,int d,double R,
	vector<Point>& centroids,string& output_path);


void Cube_Build_Euclidean(CubeStruct& Cube,int d,vector<Point>& dataset);
void Cube_Init_Euclidean(CubeStruct& Cube,int d);
void Cube_Hash_Euclidean(CubeStruct& Cube,vector<Point>& dataset,int d);
void Cube_Range_Euclidean(Cluster_Struct& CLUSTERS,CubeStruct& Cube,int probes,int d,double R,
	vector<Point>& centroids,string& output_path);


#endif