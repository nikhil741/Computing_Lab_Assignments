//Defining the semaphore Variable
#define S_MUTEX "/mutex"
#define S_READCOUNT "/readcount"
#define S_WRITE "/write"
#define S_SUMMARY "/summary"

//Header Files
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <cstring>
#include <cstdlib>
#include<sys/types.h>
#include<sys/wait.h>
#include<signal.h>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <vector>
#include <stdlib.h>
#include <iterator>
#include <set>
#include <sys/stat.h>

using namespace std;

//Check Wheather the input file for python has been created or not
inline bool exists_test3(const std::string &name) {
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}


//Calcultes Number of Lines in TextFile
int calculateLineNo() {
    string line;
    int lineNumber = 0;
    ifstream input("data.txt");

    if (input.is_open()) {
        while (!input.eof()) {
            getline(input, line);
            lineNumber++;
        }
    }
    input.close();
    return lineNumber;
}

//DEFINED FUNCTION
//Calcultes From Where Each Crawler Has to Write Data And Writes data to central.txt file
//IF LAST CRAWLER POOL WHOLE LEFT DATA
//At the End kill the child process
void writeOwnContent(int crawlerProcessId, int noOfLinesEachProcessToWrite, int numOfCrawlerProcess, int lineNumber) {
    sem_t *semWrite = sem_open(S_WRITE, 0);   //OPEN WRITEER SEMAPHORE

    string line;
    int start = crawlerProcessId * noOfLinesEachProcessToWrite, last;
    if (crawlerProcessId != numOfCrawlerProcess - 1)
        last = start + noOfLinesEachProcessToWrite;
    else
        last = lineNumber - 1;
    int i = 0;

    ifstream input("data.txt");


    //LOOP TILL POOL WRITE FINISHED
    while (i < last) {

        //SKIPPING CONTENT TILL CRAWLER REACHES POSITION FROM WHERE TO START WRITING
        while (i < start) {
            getline(input, line);
            i++;
        }

        //REACHED TO POOL WHICH CRAWLER WILL WRITE
        getline(input, line);


        //WRITER'S PROBLEM
        sem_wait(semWrite);
        ofstream output("central.txt", ios::app);  //CRITICAL SECTION
        output << line << endl;
        output.close();
        sem_post(semWrite);


        i++;
        sleep(2);           //PROCESS SLEEP FUNCTION

    }
    input.close();
    kill(getpid(), SIGKILL);
}

//DEFINED FUNCTION
//Creates the Crawler Process
//CALLS TO CALCULTE POOL TO WRITE IN CENTRAL FILE BY SYNCRONIZATION WITH READ PROCESS
//WRITEOWNCONTENT WILL KILL ITSELF AFTER FINISH
void storeToCentralDataBase(int linesEachCrawler, int numOfCrawlerProcess, int lineNumber) {
    __pid_t child;
    for (int i = 0; i < numOfCrawlerProcess; i++) {
        child = fork();
        if (child == 0) {

            //FUNCTION
            //FIND THE PORTION OF CONTENT THIS CHILD PROCESS WILL WRITE
            //WRITE TO CENTRAL FILE BY SYNCRONIZING WITH READ PROCESS
            writeOwnContent(i, linesEachCrawler, numOfCrawlerProcess, lineNumber);
        }

    }
    return;
}


//DEFINED FUNCTION
//READ CENTRAL FILE WHEN CALLING PROCESS ACQUIRE WAIT ON WRITE SEMAPHORE
//STORE IN SET
//RETURNS SET
set <string> readCentralFile() {
    //Data Collection from text file and storing in vector data line by line
    set <string> data;
    ifstream input("central.txt");
    if (input.is_open()) {
        while (!input.eof()) {
            string line;
            getline(input, line);
            data.insert(line);
        }
    }
    return data;
}


//DEFINED FUNCTION
//STORE HASHTAG IN SET
//USER TYPES STOP TO END
//RETURN SET OF HASHTAGS
set <string> enterHashTags() {
    string hashTag;
    set <string> hashtags;
    while (hashTag.compare("STOP") != 0) {
        cout << "Enter hashtags (one at every line, put STOP to stop):";           //Enter HashTags line by line
        cin >> hashTag;
        if (hashTag.compare("STOP") == 0) {
            break;
        }
        hashtags.insert(hashTag);                                                              //Username
    }
    return hashtags;
}


//DEFINED FUNCTION
//FIND LINE CONTAINING HASHTAGS
//STORE TO MATCHED SET
//RETURN SET OF MATCHED DATA
set <string> findMatchedData(set <string> data, set <string> hashtags) {
    set <string> matched_data;
    for (set<string>::iterator it = hashtags.begin(); it != hashtags.end(); ++it) {
        //Check wheather string exists in data set if true then add line in set related_text
        for (set<string>::iterator it1 = data.begin(); it1 != data.end(); ++it1) {
            if ((*it1).find((*it)) !=
                string::npos) {                                //Check wheather hashtag exists in LINE
                //Adding item to related_text if found hashtag in a line of data
                string match = (*it1);
                matched_data.insert(match);
            }
        }
    }
    return matched_data;
}


//Defined Function
//Global Summary Generation
void globalSummaryGeneration(string user){
    string path = "";
    const char *c = NULL;
    string line;
    path.append("summary/summary_");
    path.append(user);
    path.append(".txt");
    c = path.c_str();
	ifstream input(c);
	getline(input, line);

    ofstream output("userGlobalSummary.txt", ios::app);
    output<<"----------------------------------------------------------------------------"<<endl;
    output<<"UserName="<<user<<"\t\t"<<line<<endl;
    output<<"----------------------------------------------------------------------------"<<endl;
    //Reading user Summary
    
    if (input.is_open()) {
        while (!input.eof()) {
            getline(input, line);
            output<<line<<endl;
        }
    }

    output<<endl<<endl<<endl;
    input.close();
    output.close();

}

//DEFINED FUNCTION
//WRITE MATCHED DATA TO TEXT FILE
//TEXT FILE GENERATED INPUT TO PYTHON FILE
//PRINT SUMMARY TO USETR_SUMMARY TEXT FILE
//RETURN VOID
void writeMatchedDataToTextFileAndRunPythonCommand(set <string> matched_data, string user) {
    string input_python;
    const char *c = NULL;
    string python_command;
    input_python.append("input_python_");
    input_python.append(user);
    input_python.append(".txt");
    c = input_python.c_str();

    //WRITE MATCHED-DATA TO TEXT FILE
    ofstream outFile;
    outFile.open(c);
    for (set<string>::iterator it = matched_data.begin(); it != matched_data.end(); ++it) {
        outFile << (*it) << endl;
    }
    outFile.close();

    //MAKING FILENAME VARIABLE FOR OUTPUT FILE
    python_command.append("python summary_maker.py ");
    python_command.append(input_python);
    python_command.append(" ");
    python_command.append(user);
    c = python_command.c_str();
    sem_t *semSummary = sem_open(S_SUMMARY, 0);

    //LOOP UNTILL INPUT PYTHON FILE CREATED
    while (!exists_test3(input_python));

    //Bourne Shell Call
    //PYTHON FILE EXECUTION
    system(c);


    //Function Call
    //Writing new user summary genrated
    //global summary.txt
    sem_wait(semSummary);
    globalSummaryGeneration(user);
    sem_post(semSummary);
}

//DEFINED FUNCTION
//RETURN SUCCESS ON SUMMARY GENERATION
int createUserSummary() {
    string user;
    set <string> data;
    set <string> hashtags;
    set <string> matched_data;
    int pid;

    //TRUNCATE GLOBALSUMMARY TEXT FILE
    std::ofstream ofs;
    ofs.open("userGlobalSummary.txt", std::ofstream::out | std::ofstream::trunc);
    ofs.close();

    while (1) {
        user = "";
        hashtags.clear();
        matched_data.clear();
        cout << "Enter User:(STOP TO END)" << endl;
        cin >> user;
        if (user.compare("STOP") == 0) {
            break;
        }
        hashtags = enterHashTags();
        pid = fork();

        //Child Process
        if (pid == 0) {
            sem_t *semMutex = sem_open(S_MUTEX, 0);
            sem_t *semReadCount = sem_open(S_READCOUNT, 0);
            sem_t *semWrite = sem_open(S_WRITE, 0);

            //Handeling the Readers Problem
            int readCount;
            sem_wait(semMutex);
            sem_post(semReadCount);                        //CRITICAL SECTION
            sem_getvalue(semReadCount, &readCount);
            if (readCount == 1) {
                sem_wait(semWrite);                 //IF READCOUNT EQUALS 1 THEN WAIT/ABORT WRITE PROCESS TO CENTRAL FILE
            }
            sem_post(semMutex);
            data = readCentralFile();        //MULTIPLE READ PROCESS CAN GO THROUGH
            sem_wait(semMutex);
            sem_wait(semReadCount);                   //CRITICAL SECTION
            sem_getvalue(semReadCount, &readCount);
            if (readCount == 0) {                   //IF READCOUNT ZERO RESUME WRITE PROCESS TO CENTRAL FILE
                sem_post(semWrite);
            }
            sem_post(semMutex);


            //FINDING THE MATCHED DATA STORED IN DATA SET COMPARING IT BY HASHTAGS
            matched_data = findMatchedData(data, hashtags);
            writeMatchedDataToTextFileAndRunPythonCommand(matched_data, user);


            //Killing the USER'S SUMMARY CREATION process as it's functioning has been done
            kill(getpid(), SIGKILL);
        }
    }
    return 0;
}


//MAIN FUNCTION
int main() {

    //Truncate Central.txt File which automatically deletes the content
    std::ofstream ofs;
    ofs.open("central.txt", std::ofstream::out | std::ofstream::trunc);
    ofs.close();

    //Variable Declaration
    __pid_t wid;
    int status, lineNumber, noOfLinesEachProcessToWrite, numOfCrawlerProcess;

    //Declare Semaphores in parent process
    sem_t *semMutex = sem_open(S_MUTEX, O_CREAT, 0644, 1);
    sem_t *semReadCount = sem_open(S_READCOUNT, O_CREAT, 0644, 0);
    sem_t *semWrite = sem_open(S_WRITE, O_CREAT, 0644, 1);
    sem_t *semSummary = sem_open(S_SUMMARY, O_CREAT, 0644, 1);

    //Initializing the semaphore variables
    sem_init(semMutex, 1, 1);
    sem_init(semReadCount, 1, 0);
    sem_init(semWrite, 1, 1);

    //FUNCTION
    lineNumber = calculateLineNo();
    cout << "Enter Number of Crawler Process" << endl;
    cin >> numOfCrawlerProcess;   //No of Crawler
    noOfLinesEachProcessToWrite = lineNumber / numOfCrawlerProcess;

    //FUNCTION
    //EVERY CRAWLER STORE IT'S CONTENT TO CENTRAL FILE
    storeToCentralDataBase(noOfLinesEachProcessToWrite, numOfCrawlerProcess, lineNumber);

    //FUNCTION
    //START CREATING SUMMARY FOR USER BY SYNCHRONIZE READ & WRITE
    createUserSummary();


    //Wait For All Child Process TO finish
    //while ((wid = wait(&status)) > 0);
    kill(-getpid(), SIGKILL);
    return 0;
}
