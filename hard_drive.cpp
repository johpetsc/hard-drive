#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <cmath>
#include <vector>

using namespace std;

int files = 0; // counts how many files are in the FAT table

// hard drive structs
typedef struct block{
    char bytes[512];
} block;

typedef struct sectorArray{
    block sector[60];
} sectorArray;

typedef struct trackArray{
    sectorArray track[5];
} trackArray;

// FAT table structs
typedef struct fatList{
    string file_name;
    unsigned int first_sector;
} fatList;

typedef struct fatTable{
    unsigned int used;
    unsigned int eof;
    unsigned int next;
} fatTable;

vector<fatList> fat_list(1);
fatTable *fat;

// receives a position and returns the cylinder, track and sector
void positionHardDrive(int initial_position, int cylinder_track_sector[]){

    cylinder_track_sector[0] = floor((initial_position%600)%60); // sector
    cylinder_track_sector[1] = floor((initial_position%600)/60); // track
    cylinder_track_sector[2] = floor(initial_position/600); // cylinder
    cout << "cylinder: " << cylinder_track_sector[2] << " track: " << cylinder_track_sector[1] << " sector: " << cylinder_track_sector[0] << endl;
}

// stores the name of a file and its first sector
void insertIntoTable(int initial_position, string file){
    
    fat_list.resize(files+1);
    fat_list[files].file_name = file;
    fat_list[files].first_sector = initial_position;
}

// searches for the first unused sector in the table
int searchesTable(){
    int i = 0;

    while(fat[i].used) i++;

    return i;
}

// shows FAT table
int showTable(){
    fstream file;
    int i = 0,j = 0, file_size;
    
    string file_name;
    
    cout << "FILE:          SIZE IN DISK         POSITION IN DISK" << endl;
    while(i < files){
		if(fat_list[i].file_name.compare("0") != 0){
            file_name = fat_list[i].file_name;
			file.open(file_name.c_str());
            file.seekg(0, ios::end);
            file_size = file.tellg();
            file.close();

			printf("\n%s%12d Bytes                  ", file_name.c_str(), file_size);
			j = fat_list[i].first_sector;
            
            
			while((fat[j].eof != 1) && j < 30001){
                cout << j << ",";
				j = fat[j].next;
			}
            cout << j << endl;
		}
		i++;
    }
    return 0;
}

// writes new file bytes in the hard drive
int writeFile(trackArray *cylinder){
    fstream file;
    string file_name;

    cout << "Name of file to be written (e.g., data/test1.txt):" << endl;
    cin >> file_name;

    file.open(file_name.c_str());
    file.seekg(0, ios::end);
    streampos file_size = file.tellg(); // file size in bytes
    file.close();
    
    int initial_position = searchesTable(); // initial free position for the file to be written
    cout << "initial_position: " << initial_position << endl;
    cout << "File name: " << file_name << " Size: " << file_size << endl;

    int cylinder_track_sector[] = {0, 0, 0}; // cylinder, track and sector position
    int clusters = floor(file_size/(512*4))+1; // amount of clusters needed to write the file
    cout << "Amount of clusters that will be used: " << clusters << endl;
    cout << "Files: " << files+1 << endl;

    while(initial_position%4 == 0 && files > 0){
        fat[initial_position].used = 1;
        fat[initial_position].next = initial_position+1;
        initial_position++;
    }
    cout << "Initial Position: " << initial_position << endl;
    insertIntoTable(initial_position, file_name); // inserts file into FAT table
    positionHardDrive(initial_position, cylinder_track_sector); // file position in the hard drive

    int i = 0;
    int sector = 0;
    ifstream file_i;
    file_i.open(file_name.c_str());

    while(sector < (clusters*4)){
        while(i<512){
            file_i.read(&cylinder[cylinder_track_sector[2]].track[cylinder_track_sector[1]].sector[cylinder_track_sector[0]].bytes[i], sizeof(char));
            if(file_i.eof()){
                fat[initial_position].eof = 1;
                file_i.close();
                fat[initial_position].used = 1;
                for(int j = 0; j < (initial_position%4); j++){
                    fat[initial_position+j].used = 1;
                    fat[initial_position+j].eof = 1;
                     
                }
                cout << "Position: " << initial_position << endl;
                return 0;
            }
            i++;
        }
        fat[initial_position].next = initial_position+1; 
        fat[initial_position].used = 1; 
        
        i = 0;
        cylinder_track_sector[0]++;
        sector++;
        initial_position++;

        // searches for the next free cluster
        if(cylinder_track_sector[0]%4 == 0){
            int free = 0;
            while(free == 0){
                if(cylinder_track_sector[0] == 60){
                    cylinder_track_sector[1]++;
                    cylinder_track_sector[0] = 0;
                    
                    if(cylinder_track_sector[1] == 10){
                        cylinder_track_sector[2]++;
                        cylinder_track_sector[1] = 0;
                    
                    }
                    if(cylinder_track_sector[2] == 10){
                        cout << "Not enough free space in disk." << endl;
                        return 0;
                    }
                    while((fat[initial_position].used == 1) && (cylinder_track_sector[0] != 60)){
                        cylinder_track_sector[0] += 1;
                        initial_position++;
                    }
                    if(fat[initial_position].used == 0) free = 1;
                } else {
                    while((fat[initial_position].used == 1) && (cylinder_track_sector[0] != 60)){
                        cylinder_track_sector[0] += 4;
                        initial_position++;
                    }
                    if(fat[initial_position].used == 0) free = 1;
                }
            }
        }
    }
    return 0;
}

int readFile(trackArray *cylinder){
    int i = 0;
    string file_name;
    string output = "data/output.txt";

    fstream file;
    
    cout << "Name of file to be read (e.g., data/test1.txt):" << endl;
    cin >> file_name;

    while((file_name.compare(fat_list[i].file_name) != 0) && (i <= files)) {
        cout << fat_list[i].file_name;
        i++;
    }

    file.open(file_name.c_str());
    file.seekg(0, ios::end);
    streampos file_size = file.tellg();
    file.close();

    int sector = fat_list[i].first_sector;
    cout << "First sector: " << sector << endl;

    int cylinder_track_sector[] = {0, 0, 0};
    positionHardDrive(sector, cylinder_track_sector);
    cout << "Cylinder: " << cylinder_track_sector[2] << " track: " << cylinder_track_sector[1] << " Sector: " << cylinder_track_sector[0] << endl;
    i = 0;

    ofstream file_o;
    file_o.open(output.c_str());
    int j = 0;
    while(1){
        while((i<512)){
            file_o.write(&cylinder[cylinder_track_sector[2]].track[cylinder_track_sector[1]].sector[cylinder_track_sector[0]].bytes[i], sizeof(char));
            //cout << cylinder[cylinder_track_sector[2]].track[cylinder_track_sector[1]].sector[cylinder_track_sector[0]].bytes[i];
            if(file_size == j+2){
                file_o.close();
                cout << "The file content was copied to \"data/output.txt\"." << endl;
                return 0;
            }
            j++;
            i++;
        }
                
        i = 0;
        sector = fat[sector].next;
        cylinder_track_sector[0]++;
        if(cylinder_track_sector[0] == 60){
            cylinder_track_sector[1]++;
            cylinder_track_sector[0] = 0;
            if(cylinder_track_sector[1] == 10){
                cylinder_track_sector[2]++;
                cylinder_track_sector[1] = 0;
            }
        }
    }
}

int deleteFile(){
    string file_name;
    
    cout << "Name of file to be deleted (e.g., data/test1.txt):" << endl;
    cin >> file_name;

    int i = 0;
    while((fat_list[i].file_name != file_name)){
        if (i < files) i++;
        else return 0;
    }

    int j, k = 0;
    j = fat_list[i].first_sector;
    fat_list[i].file_name = "0";

    while(fat[j].eof != 1){
		fat[j].used = 0;
		k = fat[j].next;
        fat[j].next = 0;
        j = k;
    }
    fat[j].used = 0;
    fat[j].eof = 0;

    return 1;
}


int main(){
    int option;
    trackArray *cylinder = new trackArray[10];
    fat = new fatTable[30000];
    
    while(1){
        cout << "1- Write File" << endl 
            << "2- Read File" << endl
            << "3- Delete File" << endl
            << "4- Show FAT table" << endl
            << "5- Exit" << endl;
        cin >> option;

        switch (option){
            case 1: writeFile(cylinder);
                    files++;
                    break;
            case 2: readFile(cylinder);
                    break;
            case 3: if(deleteFile()) cout << "File deleted." << endl;
                    else cout << "File does not exist." << endl;                 
                    break;
            case 4: showTable();
                    break;
            default: return 0;
        }
    }
    delete[] cylinder;
    
    return 0;
}