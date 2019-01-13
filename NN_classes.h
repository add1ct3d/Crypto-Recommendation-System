#ifndef NN_CLASSES_H
#define NN_CLASSES_H


#include <boost/lexical_cast.hpp>
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
#include <math.h>
#include <complex> // complex, norm
#include <ctime>
#include <map>
#include <algorithm>

typedef double data_type ;
typedef std::string bitstring;


using namespace std;
static int id = 0;
static int space = 0;

class Point{
public:

	std::vector<int> g;
	string cosine_g;
	Point(){
		std::cout << "Construct a default point"<<std::endl;
		exit(0);
		this->item_id = ++id;
		this->fields.clear();		
		(this->g).clear();
		(this->cosine_g).clear();
		this->centroid_id = -1;
		this->centroid_data = "";

	}
	Point(string s){
		this->item_id = ++id;
		(this->g).clear();
		(this->cosine_g).clear();
		boost::char_separator<char> sep(", \t");
		boost::tokenizer<boost::char_separator<char>> tok(s,sep);
		string data ="";


		for (boost::tokenizer<boost::char_separator<char>>::iterator it=tok.begin(); it!=tok.end();it++) {

			data_type item = atof(it->c_str());
			if(it == tok.begin())
				this->item_id = (int)item;
			else{
				data += boost::lexical_cast<string>(item);
				data += " ";
			}


		}
		//this->item_id = id++;	
		this->fields.assign(data);
		space += sizeof(int);
		space += s.size();
		this->assigned = false;
		this->centroid_id = -1;
		std::cout << "Init Point "<<this->item_id<<std::endl;

	}

	Point(const Point& p){
		this->g = p.g;
		this->cosine_g = p.cosine_g;
		this->item_id = p.get_id();
		this->fields.assign(p.get_all_fields());
		this->centroid_id = p.get_centroid_id();
		this->centroid_data.assign(p.get_centroid_data());
		this->assigned = p.is_assigned();
	}

	Point(int item_id ,string s){
		this->g.clear();
		this->cosine_g.clear();
		this->item_id = item_id;	
		this->fields.assign(s);
		space += sizeof(int);
		space += s.size();
		this->centroid_id = -1;
		this->assigned = false;
	}

	int get_id() const{
		return this->item_id;
	}
	
	string get_all_fields() const {
		return this->fields;
	}


	int get_centroid_id() const {
		return this->centroid_id;
	}

	string get_centroid_data() const {
		return this->centroid_data;
	}

	void set_centroid(int item_id,string data = ""){
		this->centroid_id = item_id;
		this->centroid_data.assign(data);
	}

	bool is_assigned() const{
		return this->assigned;
	}

	void assign(){
		this->assigned = true;
	}

	void unassign(){
		this->assigned = false;
	}

	Point(int item_id,map<string,double>& v_map){
		this->g.clear();
		this->cosine_g.clear();
		this->item_id = item_id;
		this->fields.clear();
		int i = 0;
		for(std::map<std::string,double>::iterator it = v_map.begin(); it != v_map.end(); ++it) {
			this->fields.append(boost::lexical_cast<string>(it->second));
			i++;
			this->fields.append(",");
		}
	}

	Point(string s,int centroid_id){
		this->g.clear();
		this->cosine_g.clear();
		this->item_id = id++;	
		this->fields.assign(s);
		space += sizeof(int);
		space += s.size();
		this->centroid_id = centroid_id;
		this->assigned = false; 
	}

	Point(string s,int centroid_id,string centroid_data){
		this->g.clear();
		this->cosine_g.clear();
		this->item_id = id++;	
		this->fields.assign(s);
		space += sizeof(int);
		space += s.size();
		this->centroid_id = centroid_id; 
		this->centroid_data.assign(centroid_data);
		this->assigned = false;
	}

private:
	int item_id;
	string fields;
	int centroid_id;
	string centroid_data;
	bool assigned;

};


class DistanceInfo{
public:
	DistanceInfo(int id,int centroid,data_type dist){
		this->dist = dist;
		this->centroid = centroid;
		this->id = id;
	}

	data_type get_dist() const{
		return this->dist;
	}
	int get_centroid() const{
		return this->centroid;
	}
	int get_id() const{
		return this->id;
	}

	void normalize_dist(data_type max_dist){
		dist = dist/max_dist;
	}
private:
	data_type dist;
	int centroid;
	int id;
};

class Probability{
public:
	Probability(Point* ptr,data_type value){
		this->value = value;
		this->point_ptr= ptr;
	}

	data_type get_value() const{
		return this->value;
	}

	Point* get_point() const{
		return this->point_ptr;
	}

private:
	data_type value;
	Point* point_ptr;
};


typedef vector<vector<data_type>*> v_struct; 


typedef vector<data_type> t_struct;
typedef vector<int> r_struct;
typedef vector<int> shuffle_struct;
typedef vector<vector<Point*>> data_struct;
typedef vector<vector<data_type>*> cosine_r_struct; 



class HashTable{



public:

	HashTable(int K,int TableSize,double W){
		this->v.clear();
		this->v.reserve(K);
		this->r.clear();
		this->r.reserve(K);
		this->t.clear();
		this->t.reserve(K);
		this->cosine_r.clear();
		this->cosine_r.reserve(K);
		this->cosine_g.clear();
		this->shuffle_idents.clear();
		this->shuffle_idents.reserve(K);
		this->K = K;
		this->TableSize = TableSize;
		this->data.reserve(TableSize);
		this->W = W;
		this->data.clear();
		this->data.reserve(TableSize);
		for(int i = 0 ; i < TableSize ; i++){
			vector<Point*> v;
			this->data.push_back(v);
			this->data.at(i).clear();
		}
		assert(this->data.size() == TableSize);



	}


	int K;
	int TableSize;
	double W;
	string cosine_g;
	v_struct v;
	t_struct t;
	r_struct r;
	cosine_r_struct cosine_r;
	shuffle_struct shuffle_idents;
	data_struct data;
	
};



typedef HashTable CubeStruct;
typedef vector<vector<Point>*> Cluster_Struct;
typedef vector<vector<DistanceInfo>*> DistancesArray;



#endif