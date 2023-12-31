#ifndef BPlusTree_H
#define BPlusTree_H
#include <sstream>
#include <iostream>
#include <queue>
#include <fstream>
#include <string>
#include <cstring>
#include <ctime>
#include <cstdlib>
#include <vector>
#include <chrono> // for measuring running time
#include <cmath> // for ceil function
#include <unordered_set>
#include "BPlusTree/BPlusTree.h"
#include "util/recordStruct.h"
#include "util/data_utils.cpp"
#include "Disk/Disk.h"
using namespace std;
using namespace std :: chrono;


int main(){
	Disk disk = Disk(400, 300000000);

	// vector<recordStruct> record;


	int numRecords = 0;
    size_t totalRecordSize = 0;
	int recordsInCurrentBlock = 0;
	int sizeofrecord = 0;

	BPlusTree<float> BPtree = BPlusTree<float>(11);
	BPlusTree<float> Bptree = BPlusTree<float>(34);

	ifstream inputFile("Data/games.txt");
	
    if (!inputFile.is_open()) {
        cerr << "Error: Unable to open input file." << endl;
        return 1;
    }
	string line;
	bool firstLine = true;
	
	while (getline(inputFile, line)) {
		// Split the line into fields
		char* token = strtok(const_cast<char*>(line.c_str()), "\t");
		recordStruct record;
		
		if (firstLine){
			firstLine = false;
			continue;
		}
		
		// Parse and store data in the recordStruct
		for (size_t i = 0; i < recordStruct::NUM_FIELDS && token; ++i) {
			switch (i) {
			
				case 0: // GAME_DATE_EST
					record.GAME_DATE_EST = token;
					break;
				case 1: // TEAM_ID_home
					record.TEAM_ID_home = atoi(token);
					break;
				case 2: // PTS_home
					record.PTS_home = atoi(token);
					break;
				case 3: // FG_PCT_home
					record.FG_PCT_home = atof(token);
					break;
				case 4: // FT_PCT_home
					record.FT_PCT_home = atof(token);
					break;
				case 5: // FG3_PCT_home
					record.FG3_PCT_home = atof(token);
					break;
				case 6: // AST_home
					record.AST_home = atoi(token);
					break;
				case 7: // REB_home
					record.REB_home = atoi(token);
					break;
				case 8: // HOME_TEAM_WINS
					record.HOME_TEAM_WINS = atoi(token);
					break;
				default:
					break;
			}
			token = strtok(nullptr, "\t");
		}
        // Calculate the size of the record
       	sizeofrecord = sizeof(recordStruct);

        Address address = disk.saveDataToDisk(&record, sizeofrecord);

    
		BPtree.insert(record.FG_PCT_home, address);
		Bptree.insert(record.FG_PCT_home, address);



        // Update statistics
        ++numRecords;
		++recordsInCurrentBlock;

		
	}
  	inputFile.close(); // Close the input file after processing
	const size_t blockSize = 400; 
	size_t numRecordsInBlock = blockSize / sizeof(recordStruct);
  	int numBlocks = disk.BlockUsed();
    // Report the statistics for Experiment 1
    cout << "Experiment 1 Statistics:" << endl;
    cout << "Number of Records: " << numRecords << endl;
    cout << "Size of a Record (bytes): " << sizeofrecord << endl;
    cout << "Number of Records Stored in a Block: " << numRecordsInBlock << endl; 
	cout << "Number of Blocks for Storing the Data: " << numBlocks << endl;
	cout << endl;

//    Report the statistics for Experiment 2
	
	int numNodes = BPtree.countNodes(BPtree.getroot());
	int numLevels = BPtree.countLevels(BPtree.getroot());
	int numLevels_RootIsLevel0 = (BPtree.countLevels(BPtree.getroot())-1);
	Node<float>* cursor = BPtree.getroot();
	std::stringstream output;

	for (int i = 0; i < cursor->size; i++) {
		output << cursor->key[i] << " ";
	}
    cout << "Experiment 2 Statistics (B+ Tree):" << endl;
    cout << "Parameter 'n' of the B+ Tree: " << BPtree.getN() << endl;
    cout << "Number of Nodes in the B+ Tree: " << numNodes << endl;
    cout << "Number of Levels in the B+ Tree(root is counted as **LEVEL 1**): " <<numLevels << endl;
	//cout << "Number of Levels in the B+ Tree(root is  counted as **LEVEL 0**): " << (numLevels_RootIsLevel0) << endl;
	std::cout << "Content of the root node (only the keys): " << output.str() << std::endl;
	cout << endl;


	// Experiment 3: Retrieve movies with FG_PCT_home equal to 0.5

	float targetFGPCT = 0.5;
	auto start = high_resolution_clock::now();
	vector<Address> retrievalResult = BPtree.findKeyRange(targetFGPCT, targetFGPCT);
	// std::cout << "the record count is: " << retrievalResult.size();
	int numIndexNodesAccessed = BPtree.getNumAccessedNodes();
	int numDataBlocksAccessed = 0; 
	float totalFG3PCT = 0.0; // Initialize a variable to store the sum of FG3_PCT_home values
	int numRetrievedRecords = retrievalResult.size(); // Get the total number of retrieved records
	// Iterate through the retrieved addresses
	for (const Address& address : retrievalResult) {
		// Calculate the number of data blocks accessed
		int dataBlocksAccessed = (address.offset + sizeof(recordStruct) - 1) / 400 + 1;
		numDataBlocksAccessed += dataBlocksAccessed;

		// Access the data from disk for each address
		recordStruct* retrievedRecord = static_cast<recordStruct*>(disk.loadDataFromDisk(address, sizeof(recordStruct)));
		totalFG3PCT += retrievedRecord->FG3_PCT_home;
	}
	float averageFG3PCT = totalFG3PCT / numRetrievedRecords;
	auto end = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(end - start);


	// Report Experiment 3 statistics
	cout << "Experiment 3 Statistics:" << endl;
	cout << "Number of Index Nodes Accessed: " << numIndexNodesAccessed << endl;
	cout << "Number of Data Blocks Accessed: " << numDataBlocksAccessed << endl; 
	cout << "Average FG3_PCT_home of retrieved records: " << averageFG3PCT << endl;
	cout << "Time taken for retrieval: " << duration.count() << " microseconds" << endl;
	cout << endl;



    // // Experiment 3  Brute-force linear scan method
    auto start2 = high_resolution_clock::now();
    vector<recordStruct> matchingRecords;
	int numDataBlocksAccessedBruteForce=0;
    for (int i = 0; i < numBlocks; ++i) {
        Address address = {i, 0};
        for (int j = 0; j < numRecordsInBlock; ++j) {
            recordStruct* record = static_cast<recordStruct*>(disk.loadDataFromDisk(address, sizeof(recordStruct)));
            numDataBlocksAccessedBruteForce++;
			if (record && fabs(record->FG_PCT_home - targetFGPCT) < 1e-6) {
                matchingRecords.push_back(*record);
            }
            address.offset += sizeof(recordStruct);
        }
    }
	auto end2 = high_resolution_clock::now();
	auto duration2 = duration_cast<microseconds>(end2- start2);

    // Report brute-force statistics
    cout << "Experiment 3 Brute-Force Linear Scan Statistics:" << endl;
    cout << "Number of Data Blocks Accessed (Brute Force): " << numDataBlocksAccessedBruteForce << endl;
    cout << "Running Time (Brute Force): " << duration2.count() << " microseconds"<< endl;
	cout << endl;




	// Experiment 4: Retrieve records with FG_PCT_home in a specified range

	float targetFGPCT1 = 0.6;  // Updated target value 1
	float targetFGPCT2 = 1.0;  // Updated target value 2
	auto start4 = high_resolution_clock::now();  // Updated start time variable
	vector<Address> retrievalResult4 = BPtree.findKeyRange(targetFGPCT1, targetFGPCT2);
	int numIndexNodesAccessed4 = BPtree.getNumAccessedNodes();  // Updated variable
	int numDataBlocksAccessed4 = 0;  // Updated variable
	float totalFG3PCT4 = 0.0;  // Updated variable for sum of FG3_PCT_home values
	int numRetrievedRecords4 = retrievalResult4.size();  // Updated variable
	// Iterate through the retrieved addresses
	for (const Address& address4 : retrievalResult4) {  // Updated variable
		// Calculate the number of data blocks accessed
		int dataBlocksAccessed4 = (address4.offset + sizeof(recordStruct) - 1) / 400 + 1;  // Updated variable
		numDataBlocksAccessed4 += dataBlocksAccessed4;  // Updated variable

		// Access the data from disk for each address
		recordStruct* retrievedRecord4 = static_cast<recordStruct*>(disk.loadDataFromDisk(address4, sizeof(recordStruct)));  // Updated variable
		totalFG3PCT4 += retrievedRecord4->FG3_PCT_home;  // Updated variable
	}
	float averageFG3PCT4 = totalFG3PCT4 / numRetrievedRecords4;  // Updated variable
	auto end4 = high_resolution_clock::now();  // Updated end time variable
	auto duration4 = duration_cast<microseconds>(end4 - start4);  // Updated variable

	// Report Experiment 4 statistics
	cout << "Experiment 4 Statistics:" << endl;
	cout << "Number of Index Nodes Accessed: " << numIndexNodesAccessed4 << endl;
	cout << "Number of Data Blocks Accessed: " << numDataBlocksAccessed4 << endl;
	cout << "Average FG3_PCT_home of retrieved records: " << averageFG3PCT4 << endl;
	cout << "Time taken for retrieval: " << duration4.count() << " microseconds" << endl;
	cout << endl;

	// Experiment 4 Brute-force linear scan method
	auto start5 = high_resolution_clock::now();
	vector<recordStruct> matchingRecords4;
	int numDataBlocksAccessedBruteForce4 = 0;
	for (int i = 0; i < numBlocks; ++i) {
		Address address = {i, 0};
		for (int j = 0; j < numRecordsInBlock; ++j) {
			recordStruct* record = static_cast<recordStruct*>(disk.loadDataFromDisk(address, sizeof(recordStruct)));
			numDataBlocksAccessedBruteForce4++;
			if (record && fabs(record->FG_PCT_home - targetFGPCT) < 1e-6) {
				matchingRecords4.push_back(*record);
			}
			address.offset += sizeof(recordStruct);
		}
	}
	auto end5 = high_resolution_clock::now();
	auto duration5 = duration_cast<microseconds>(end5 - start5);

	//Report Experiment 4 Brute-Force Linear Scan Statistics
	cout << "Experiment 4 Brute-Force Linear Scan Statistics:" << endl;
	cout << "Number of Data Blocks Accessed (Brute Force): " << numDataBlocksAccessedBruteForce4 << endl;
	cout << "Running Time (Brute Force): " << duration5.count() << " microseconds"<< endl;
	cout << endl;



	// Experiment 5 delete:
	/*
		Delete those movies with the attribute
		“FG_PCT_home” below 0.35 inclusively (0,0.35]
	*/
	cout << "\nExperiment 5: Delete Records with attr FG_PCT_home below 0.35 inclusively (0,0.35]:: \n" << endl;

	//Before deleting movies
	int exp5_numNodes_beforeDel = Bptree.countNodes(Bptree.getroot());
	int exp5_numLevels_beforeDel = Bptree.countLevels(Bptree.getroot());
	int exp5_numLevels_RootIsLevel0_beforeDel = (Bptree.countLevels(Bptree.getroot()) - 1);
	Node<float> *cursor_beforeDel = Bptree.getroot();
	std::stringstream output_beforeDel;
	for (int i = 0; i < cursor_beforeDel->size; i++)
	{
		output_beforeDel << cursor_beforeDel->key[i] << " ";
	}
	std::cout << "::Before Deleting:: Experiment 5 Statistics (B+ Tree):" << endl;
	std::cout << "Parameter 'n' of the B+ Tree: " << Bptree.getN() << endl;
	std::cout << "Number of Nodes in the B+ Tree: " << exp5_numNodes_beforeDel << endl;
	std::cout << "Number of Levels in the B+ Tree(root is counted as **LEVEL 1**): " << exp5_numLevels_beforeDel << endl;
	std::cout << "Content of the root node (only the keys): " << output_beforeDel.str() << std::endl; 
	std::cout << endl;

	//Deletion Section
	vector<Address> resTotalBefore = Bptree.findKeyRange(-10, 10);
	cout << "TOTAL RECORD COUNT:: BEFORE DELETE ::: the record count is: " << resTotalBefore.size() << endl;
	vector<Address> resCheck = Bptree.findKeyRange(0, 0.35);
	cout << "::BEFORE DELETE:: No. of Records to Delete: " << resCheck.size() << endl;

	// Node<float> *val_temp = Bptree.findFirstMostNode();
	// cout << "LEFTMOST value:: " << val_temp->key[0]<< endl;

	auto start6 = high_resolution_clock::now();
	vector<Address> numOfDeleted = Bptree.delKeyRange(0, 0.35);
	for (int i=0; i<numOfDeleted.size();i++){
		disk.deleteRecord(numOfDeleted[i], sizeof(recordStruct));	
	}
	auto end6 = high_resolution_clock::now();
	auto duration6 = duration_cast<microseconds>(end6 - start6);

	int exp5_numNodes_afterDel = Bptree.countNodes(Bptree.getroot());
	int exp5_numLevels_afterDel = Bptree.countLevels(Bptree.getroot());
	int exp5_numLevels_RootIsLevel0_afterDel = (Bptree.countLevels(Bptree.getroot()) - 1);
	Node<float> *cursor_afterDel = Bptree.getroot();
	std::stringstream output_afterDel;
	for (int i = 0; i < cursor_afterDel->size; i++)
	{
		output_afterDel << cursor_afterDel->key[i] << " ";
	}

	vector<Address> resTotalAfter = Bptree.findKeyRange(-10, 10);
	cout << "TOTAL RECORD COUNT:: AFTER DELETE ::: the record count is: " << resTotalAfter.size() << endl;
	vector<Address> resCheck_AfterDel = Bptree.findKeyRange(0, 0.35);
	cout << "::AFTER DELETE:: No. of Records LEFT to Delete: " << resCheck_AfterDel.size() << "\n"<< endl;

	cout << "::After Deleting:: Experiment 5 Statistics (B+ Tree): " << endl;
	cout << "Parameter 'n' of the UPDATED B+ Tree: " << Bptree.getN() << endl;
	cout << "Number of Nodes in the UPDATED B+ Tree: " << exp5_numNodes_afterDel << endl;
	cout << "Number of Levels in the UPDATED B+ Tree(root is counted as **LEVEL 1**): " << exp5_numLevels_afterDel << endl;
	std::cout << "Content of the UPDATED B+ tree root node (only the keys): " << output_afterDel.str() << std::endl;
	cout << "Time taken for deletion: " << duration6.count() << " microseconds" << endl;
	cout << endl;
	

	// Experiment 5 Brute-force delete method
	auto start7 = high_resolution_clock::now();
	int numDataBlocksAccessedBruteForce5 = 0;
	for (int i = 0; i < numBlocks; ++i) {
		Address address = {i, 0};
		for (int j = 0; j < numRecordsInBlock; ++j) {
			recordStruct* record = static_cast<recordStruct*>(disk.loadDataFromDisk(address, sizeof(recordStruct)));
			if (record->FG_PCT_home <= 0.35 ){
				disk.deleteRecord(address, sizeof(recordStruct));
			}
			numDataBlocksAccessedBruteForce5++;

			address.offset += sizeof(recordStruct);
		}
	}
	auto end7 = high_resolution_clock::now();
	auto duration7 = duration_cast<microseconds>(end7 - start7);

	//Report Experiment 5 Brute-Force Linear Deletion Statistics
	cout << "Experiment 5 Brute-Force Deletion Statistics:" << endl;
	cout << "Number of Data Blocks Accessed (Brute Force): " << numDataBlocksAccessedBruteForce5 << endl;
	cout << "Running Time (Brute Force): " << duration7.count() << " microseconds"<< endl;
	cout << endl;

	return 0;

}
#endif