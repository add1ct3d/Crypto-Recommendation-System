#ifndef  CLASSES_H
#define CLASSES_H

using namespace std;

string CryptoDictionary(string coins_path,string coin);
double SemanticDictionary(string semantic_path,string term);
void LSH_Cosine_Recommend(vector<HashTable>&  HashTables,int L,vector<Point>& queries);

map<string,string> CryptoMap;
static const int alpha = 15;

class Score{
public:
	Score(string cryptocoin = "error",double value = numeric_limits<double>::infinity()){
		this->cryptocoin = cryptocoin;
		this->semantic_value = value;
	}


	string getCrypto(){
		return cryptocoin;
	}

	double getValue(){
		return semantic_value;
	}

	string setCrypto(string cryptocoin){
		this->cryptocoin = cryptocoin;
	}

	void setValue(double value){
		this->semantic_value = value;
	}

	void addValue(double value){
		this->semantic_value += value;
	}


	Score(const Score& b){
		this->cryptocoin = b.cryptocoin;
		this->semantic_value = b.semantic_value;
	}

	
private:
	string cryptocoin;
	double semantic_value;
};

class Tweet{

public:
	Tweet(int id,string text){
		this->text = text;
		this->id = id;
	}
	string getText(){
		return this->text;
	}

	int getId(){
		return this->id;
	}

	vector<Score> getScores(string semantic_path,string coins_path){
		vector<Score> scores;
		scores.clear();
		vector<string> coins;
		coins.clear();
		boost::char_separator<char> sep("\t");
		boost::tokenizer<boost::char_separator<char>> tok(this->text, sep);
		double score = 0;
		cout<<"search tweet "<<this->text<<endl;
		for (boost::tokenizer<boost::char_separator<char>>::iterator it=tok.begin(); it!=tok.end();it++) {
			string temp = *it;
			string coin = CryptoDictionary(coins_path,temp);
			if(coin != ""){
				coins.push_back(CryptoMap[temp]);
				cout<<"\t\t\tfound coin "<<temp<< " ("<<CryptoMap[temp]<<")"<<endl;
			}
			else{
				double prev = score;
				score += SemanticDictionary(semantic_path,temp);
				if(score-prev != 0)cout << "\tfound semantic "<<score-prev<<endl;

			}
		}
		score = score/(sqrt((score*score) + alpha));
		assert(score >= -1 && score <= 1);
		for(int i = 0 ; i < coins.size() ; i++){
			scores.push_back(Score(coins.at(i),score));
		}
		return scores;
	}

private:
	string text;
	int id;

};



class User{
public:
	User(int id = -1){
		this->id = id;
		tweets.clear();
		
		int i = 0;
		for(map<string,string>::iterator it = CryptoMap.begin(); it != CryptoMap.end(); ++it) {
			sentiment_score[CryptoMap[it->first]] = numeric_limits<double>::infinity();
			i++;
			artificial[CryptoMap[it->first]] = true;
		}
	}

	void AddTweet(Tweet tweet,string semantic_path,string coins_path){
		tweets.push_back(tweet);
		AddScores(tweets.back().getScores(semantic_path,coins_path));
	}

	void PrintUserTweets(){
		for(int i = 0 ; i < tweets.size() ; i++){
			cout << "User"<<this->id <<" - " << "ID "<<tweets.at(i).getId()<<":\t"<<tweets.at(i).getText() << endl;	
		}
	}

	vector<Tweet>& GetTweets(){
		return tweets;
	}

	map<string,double> GetCoinsVector(){
		return sentiment_score;
	}

	Tweet& GetTweet(int i){
		return tweets.at(i);
	}

	double GetScore(string s){
		return sentiment_score.at(s);
	}

	void AddScores(vector<Score> scores){
		for(int i = 0 ; i < scores.size() ;i++){
			AddScore(scores.at(i));
		}
	}

	void AddScore(Score score){
		if(sentiment_score[score.getCrypto()] == numeric_limits<double>::infinity())
			sentiment_score[score.getCrypto()] = score.getValue();
		else
			sentiment_score[score.getCrypto()] += score.getValue();
		artificial[score.getCrypto()] = false;
	}


	int getID(){
		return this->id;
	}


	void Normalize(){
		double avg = ReturnAverage();
		for(map<string,double>::iterator it = sentiment_score.begin(); it != sentiment_score.end(); ++it) {
			if(it->second == numeric_limits<double>::infinity())it->second = 0;
			else it->second -= avg;
		}
	}

	vector<string> getFake(){
		vector<string> v;
		for(map<string,bool>::iterator it = artificial.begin(); it != artificial.end(); ++it) {
			if(it->second == false)v.push_back(it->first);
		}
		return v;
	}


	vector<string> getTrue(){
		vector<string> v;
		for(map<string,bool>::iterator it = artificial.begin(); it != artificial.end(); ++it) {
			if(it->second == true)v.push_back(it->first);
		}
		return v;
	}	

	bool isReal(string s){
		return artificial[s];
	}

	double ReturnAverage(){
		double sum = 0.0;
		int non_inf = 0;
		for(map<string,double>::iterator it = sentiment_score.begin(); it != sentiment_score.end(); ++it) {
			if(artificial[it->first])continue;
			non_inf++;
			sum += it->second;
		}
		return sum/non_inf;
	}


	void PrintScores(){
		for(map<string,double>::iterator it = sentiment_score.begin(); it != sentiment_score.end(); ++it) {
			if(artificial[it->first]) cout << ",";
			else cout<<","<<it->second;
		}
	}


private:
	vector<Tweet> tweets;
	map<string,double> sentiment_score;
	map<string,bool> artificial;
	int id;
};


#endif