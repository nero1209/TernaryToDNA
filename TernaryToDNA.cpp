#define MPICH_SKIP_MPICXX  
#include <mpi.h>
#include <iostream>
#include <ctime>
#include <stdio.h>
#include <string.h>
#include <iterator>
#include <string>
#include <fstream>
#include <list>
#include <ctime>
#include <sys/stat.h>
#include <cstring>
#include <math.h>
#include <time.h>
#include <sstream>

// 4-21: Read ternary data from a file, and convert them into DNA data(ACTG);
/**
TO DO: 
  MPICH2, 
  SuprerComputing Platform;

**/

using namespace std;

#define atask 1000		// task size assign to each worker each time,depends on the size of file;
#define taskSize 1000
#define remainder 0	// last assgin's remainder;

void Linear();  // Non-parallel
string data = "G";  // The first DNA data is G;

int main(int argc, char *argv[])
{
	//Linear();
	
	int id;  // Process link
	int p;  // Number of process
	void worker(int id);
	void manager(int p);
	MPI_Init(&argc,&argv);  // Initialize MPI
	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	MPI_Comm_size(MPI_COMM_WORLD, &p);
    
	clock_t startTime,endTime;
	startTime = clock(); // Start timing

	if(p < 2)
	{
		cout << "Programs needs at least 2 processors!" << endl;
		exit(1);
	}
	else
	{
		if(!id)
		{
			manager(p);
		}
		else
			worker(id);
	}
	MPI_Finalize();  // End MPI

	endTime = clock(); // End timing
	cout << (double)(endTime - startTime) / CLOCKS_PER_SEC << endl;

	
	return 0;
}

void manager(int p)
{
	MPI_Status status;  
	int terminate = 0;  // Terminated workers;
	int prenum = 0;  // The number of times that have been processed;
	int *power;   // power[0]: judge the process is correct or not(0 = OK, 1 = Stop)
					// power[1]: denote worker's ID;
	int tagnum = 0;  // Request number from worker;
	int part = 0;  // To compute the size of each task send to worker, atask-part;
	string DNA;  // Record the final result
	string r;  // Save the result send by worker
	string Data;
	char* file;
	char* combinefinData;
	string tem_combinefinData;

	combinefinData = (char*)malloc(sizeof(char)*taskSize);
	file = (char*)malloc(sizeof(char)*taskSize);
	power = (int*)malloc(sizeof(int)*2) ;	
	memset(combinefinData,0,sizeof(char)*taskSize);  
	memset(file,0,sizeof(char)*taskSize);  
	memset(power,0,sizeof(int)*2) ;

	// Read the digits in file, save them to balance1;
	/**
	ifstream infile; 
	infile.open("TernaryData_10^5.txt");
	string balance1;
	infile >> balance1;
	infile.close();
	cout << balance1 << endl;
	**/

    FILE *fp;  
    static char ch[100000];
	fp = fopen("TernaryData_10^4.txt", "r");
	//fp = fopen("E:/TernaryToDNA/TernaryData_10^5.txt", "r");
	if(fp == NULL)
	{
		cout << "Cannot Find File" << endl;

		ofstream fout( "result.txt" ); 
		fout << "Cannot Find the file" << endl;
		fout.close();

		exit(0);
	}
	fgets(ch, 100000, (FILE*)fp);
	fclose(fp);
	Data = ch;

	// Compute the number of times will process;
	int lenofdata = Data.size();
	double count = (double) lenofdata / atask;
	int num = ceil(count);

	// Manager Process
	do
	{
		// Receive request from worker;
		MPI_Recv(power, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		if(power[0] == 0)
		{
			if(prenum < num)
			{
				
				string temstr;
				temstr = Data.substr(part, atask);				
				stringstream stream;									//convert INT to STRING
				string stringtagnum;
				stream << tagnum;  
				stringtagnum = stream.str();
				string combine;											
				combine = temstr + stringtagnum;

				const char *p = combine.data();
				file = const_cast<char*>(p);
				


				//send the sliced ternary data packaged files
				MPI_Send(file, taskSize, MPI_CHAR, power[1], 0, MPI_COMM_WORLD); 
				// cout << file << endl;
				part += atask;
				prenum++;
				tagnum++;

				// Combining result from worker;
				// receive DNA data from worker;

				// cout << "Test!!!" << endl;
				MPI_Recv(combinefinData, taskSize, MPI_CHAR, MPI_ANY_SOURCE, 99, MPI_COMM_WORLD, &status) ;
				// cout << "Test!!!" << endl;
				
				string temcombinefinData;
				temcombinefinData = combinefinData;		//CHAR* save in STRING
				//tem_combinefinData.append(combinefinData);
				
				// Deal with tagnum in data;
				string TAG;
				if (temcombinefinData.size() > atask)
					TAG = temcombinefinData.substr(atask,4);						//??tag?????4
				else
					TAG = temcombinefinData.substr(remainder,4);					//?????tag

				int numofworker = atoi(TAG.c_str()); 
	
				// Deal with Data;	
				string result;
				if(temcombinefinData.size()>atask)
					result=temcombinefinData.substr(0,atask);
				else
					result=temcombinefinData.substr(0,remainder);				//????(remainder???)???
			
				DNA.append(result);
				//const char *p1 = result.data();							// convert STRING to CONST CHAR*
				//charresult = const_cast<char*>(p1); 					// convert CONST CHAR* to CHAR*

			}
			// After the file is process completed, send terminate sign to worker;
			else
			{
				file[0] = 's';
				MPI_Send(file, taskSize, MPI_CHAR, power[1], 0, MPI_COMM_WORLD) ;  
			}	
		}
		if(power[0] == 1)
			terminate++;
	
	} while(terminate < (p - 1)); 

	//cout << DNA << endl;
}

void worker(int id)
{
	MPI_Status status;  
	string temfile;
	char *combinefinData;
	char *file;
	char* finData;
	int *power;   // power[0]: judge the process is correct or not(0 = OK, 1 = Stop)
					// power[1]: denote worker's ID;
	combinefinData = (char*)malloc(sizeof(char)*taskSize) ;
	finData = (char*)malloc(sizeof(char)*taskSize) ;
	file = (char*)malloc(sizeof(char)*taskSize) ; 
	power = (int*)malloc(sizeof(int)*2) ;
	memset(combinefinData,0,sizeof(char)*taskSize) ;   
	memset(finData,0,sizeof(char)*taskSize) ;    
	memset(file,0,sizeof(char)*taskSize) ;   
	memset(power,0,sizeof(int)*2) ; 
	power[0] = 0;
	power[1] = id;

	
	// Worker start;
	while(1)
	{
		// Send request to manager, receive ternary digits data;
		// After processing, send DNA data to manager;
		MPI_Send(power, 2, MPI_INT, 0, id, MPI_COMM_WORLD);

		//cout << "Test!!!" << endl;
		MPI_Recv(file, taskSize, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &status);  
		//cout << "Test!!!" << endl;
		
		if(file[0] != 's')
		{
			temfile = file;

			string dfile;
			if(temfile.size() < atask)
		  		dfile=temfile.substr(0,remainder);
			else
				dfile=temfile.substr(0,atask);		

			string temtag;
			if (temfile.size() > atask)
				temtag = temfile.substr(atask,4);
			else
				temtag = temfile.substr(remainder,4);									//???????tag


			// Ternary to DNA
			int length = dfile.length();
			string tem;
			char decide;  // Save each number in source file;
			int result; // To decide whethe it is ACTG;
			for(int i = 0; i < length; i++)
			{
				tem = data[data.length()-1];  // save the last characte of DNA data;
				decide = dfile.at(i);
				// from G
				result = tem.compare("G");
				if (result == 0 && decide=='0')
					data.append("A");
				else if (result == 0 && decide=='1')
					data.append("T");
				else if (result == 0 && decide=='2')
					data.append("C");
				// from T
				result = tem.compare("T");
				if (result == 0 && decide=='0')
					data.append("C");
				else if (result == 0 && decide=='1')
					data.append("A");
				else if (result == 0 && decide=='2')
					data.append("G");
				// from A
				result = tem.compare("A");
				if (result == 0 && decide=='0')
					data.append("G");
				else if (result == 0 && decide=='1')
					data.append("C");
				else if (result == 0 && decide=='2')
					data.append("T");
				// from C
				result = tem.compare("C");
				if (result == 0 && decide=='0')
					data.append("T");
				else if (result == 0 && decide=='1')
					data.append("G");
				else if (result == 0 && decide=='2')
					data.append("A");

			}
			// Send the resulting DNA data to manager;
			string combine;
			combine = data + temtag;

			const char *p2 = combine.data();							
			combinefinData = const_cast<char*>(p2); 						
			MPI_Send(combinefinData, taskSize, MPI_CHAR, 0, 99, MPI_COMM_WORLD) ;		
		}
		else
		{
			power[0] = 1;
            MPI_Send(power, 2, MPI_INT, 0, id, MPI_COMM_WORLD);
            break;
		}
	}
}


void Linear()
{
	clock_t startTime,endTime;
	startTime = clock(); //计时开始
	string balance1;

    FILE *fp;  
    static char ch[100000];
	//fp = fopen("TernaryData_10^5.txt", "r");
	fp = fopen("TernaryData_10^3.txt", "r");
	if(fp == NULL)
	{
		cout << "Cannot Find File" << endl;

		ofstream fout( "result.txt" ); 
		fout << "Cannot Find the file" << endl;
		fout.close();

		exit(0);
	}
	fgets(ch, 100000, (FILE*)fp);
	fclose(fp);
	balance1 = ch;

	/**
	ifstream infile; 
	infile.open("TernaryData_10^3.txt");
	string balance1;
	infile >> balance1;
	**/
	int length = balance1.length();
	string data = "G";  // The first DNA data is G;
	string tem;
	char decide;  // Save last digit in source file;
	int result; // To decide whethe it is ACTG;
	//MPI_Init(&argc,&argv);  // Start MPI
	for(int i = 0; i < length; i++)
	{
		tem = data[data.length()-1];  // save the last characte of DNA data;
		decide = balance1.at(i);  // Put last digit of ternary data file into decide;

		// strcmp(str1.c_str(), str2.c_str());
		// strcmp(data.back(), "G");
        // from G
		result = tem.compare("G");
		if (result == 0 && decide=='0')
		{
			//cout << balance1[i] << "\n";
			data.append("A");
		}
		else if (result == 0 && decide=='1')
			data.append("T");
		else if (result == 0 && decide=='2')
			data.append("C");
		// from T
		result = tem.compare("T");
		if (result == 0 && decide=='0')
			data.append("C");
		else if (result == 0 && decide=='1')
			data.append("A");
		else if (result == 0 && decide=='2')
			data.append("G");
		// from A
		result = tem.compare("A");
		if (result == 0 && decide=='0')
			data.append("G");
		else if (result == 0 && decide=='1')
			data.append("C");
		else if (result == 0 && decide=='2')
			data.append("T");
		// from C
		result = tem.compare("C");
		if (result == 0 && decide=='0')
			data.append("T");
		else if (result == 0 && decide=='1')
			data.append("G");
		else if (result == 0 && decide=='2')
			data.append("A");
		//else 
			//data = data;
		

	}
	//cout << data << "\n";
	//MPI_Finalize();  // End MPI
	endTime = clock(); //计时结束
	ofstream fout( "Q-result_10^4.txt" ); 
	fout << data << endl;
	fout.close();

	cout << (double)(endTime - startTime) / CLOCKS_PER_SEC << endl;		
}




	/**
	clock_t startTime,endTime;
	startTime = clock(); //计时开始

	ifstream infile; 
	infile.open("TernaryData_10^7.txt");
	string balance1;
	infile >> balance1;
	int length = balance1.length();
	string data = "G";  // The first DNA data is G;
	string tem;
	char decide;  // Save each number in source file;
	int result; // To decide whethe it is ACTG;
	MPI_Init(&argc,&argv);  // Start MPI
	for(int i = 0; i < length; i++)
	{
		tem = data[data.length()-1];  // save the last characte of DNA data;
		decide = balance1.at(i);

		// strcmp(str1.c_str(), str2.c_str());
		// strcmp(data.back(), "G");
        // from G
		result = tem.compare("G");
        if (result == 0 && decide=='0')
		{
			//cout << balance1[i] << "\n";
            data.append("A");
		}
		else if (result == 0 && decide=='1')
			data.append("T");
		else if (result == 0 && decide=='2')
			data.append("C");
		// from T
		result = tem.compare("T");
		if (result == 0 && decide=='0')
			data.append("C");
		else if (result == 0 && decide=='1')
			data.append("A");
		else if (result == 0 && decide=='2')
			data.append("G");
		// from A
		result = tem.compare("A");
		if (result == 0 && decide=='0')
			data.append("G");
		else if (result == 0 && decide=='1')
			data.append("C");
		else if (result == 0 && decide=='2')
			data.append("T");
		// from C
		result = tem.compare("C");
		if (result == 0 && decide=='0')
			data.append("T");
		else if (result == 0 && decide=='1')
			data.append("G");
		else if (result == 0 && decide=='2')
			data.append("A");
		//else 
			//data = data;
		

	}
	//cout << data << "\n";
	MPI_Finalize();  // End MPI
	endTime = clock(); //计时结束
	cout << (double)(endTime - startTime) / CLOCKS_PER_SEC << endl;
	**/