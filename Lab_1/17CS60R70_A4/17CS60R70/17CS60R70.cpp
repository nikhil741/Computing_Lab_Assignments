#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include <iterator>
#include <unistd.h>
#include <set>
#include <sys/stat.h>
#include <unistd.h>
using namespace std;
inline bool exists_test3 (const std::string& name) {
  struct stat buffer;   
  return (stat (name.c_str(), &buffer) == 0); 
}
std::string exec(const char* cmd) {
    char buffer[128];
    std::string result = "";
    FILE* pipe = popen(cmd, "r");
    if (!pipe) throw std::runtime_error("popen() failed!");
    try {
        while (!feof(pipe)) {
            if (fgets(buffer, 128, pipe) != NULL)
                result += buffer;
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return result;
}

int main(){
    string path;
	string user;
	string hashTag;
    set<string> data;
    set<string> hashtags;
    set<string> matched_data;
    ofstream outFile;
    string input_python;
    string python_command;
    const char * c = NULL;
    const char * outPath = NULL;
    int pid=100;

    //Data Collection from text file and storing in vector data line by line
    ifstream input("data.txt");
    
	if(input.is_open()){
        while(!input.eof()){
            string line;
            getline(input, line); 
            data.insert(line);
        }
    }
    
    while(1){
        user="";
        hashTag="";
        input_python = "";
        python_command = "";
        path="";
        hashtags.clear();
        matched_data.clear();
        c = NULL;
        outPath = NULL;
    	
		cout<<"Enter User:(STOP TO END)"<<endl;
    	cin>>user;
        if(user.compare("STOP") == 0){
            break;
        }
    	input_python.append("input_python_");
    	input_python.append(user);
    	input_python.append(".txt");

    	c = input_python.c_str();
                                                                       //UserName
    	while(hashTag.compare("STOP") != 0){
    		cout<<"Enter hashtags (one at every line, put STOP to stop):";           //Enter HashTags line by line  
    		cin>>hashTag;
            if(hashTag.compare("STOP") == 0){
                break;
            }
            hashtags.insert(hashTag);                                                              //Username    	
        	}
            pid = fork();
            if(pid == 0){
                for(set<string>::iterator it = hashtags.begin(); it!=hashtags.end(); ++it){
                    //Check wheather string exists in data set if true then add line in set related_text
                    for(set<string>::iterator it1 = data.begin(); it1 != data.end();++it1){
                        if((*it1).find((*it)) != string::npos){                                //Check wheather hashtag exists in LINE
                            //Adding item to related_text if found hashtag in a line of data
                            //string word=(*it);
                            string match= (*it1);
                            //cout<<"Word "<<word<<" Match "<<match<<endl;
                            //cout<<"Hello"<<match<<endl;
                            matched_data.insert(match);
                        }
                    }
                }
            	outFile.open(c);
                for(set<string>::iterator it = matched_data.begin(); it!=matched_data.end(); ++it){
                    outFile<<(*it)<<endl;
                    //cout<<"#mathced_data"<<(*it)<<endl;
                }
                outFile.close();
                python_command.append("python summary_maker.py ");
                //python_command.append("/home/nikhil/Computing_Lab/Assignment4/");
                python_command.append(input_python);
				python_command.append(" ");
				python_command.append(user);
    	        c = python_command.c_str();
                //system(c);
                
                while (!exists_test3(input_python));
				system(c);        
				exit(0);       
            }
    }
    return 0;
}
