#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <stdio.h>

using namespace std;

int arquivos = 0;

typedef struct block{
    char bytes_s[512];
} block;

typedef struct sector_array{
    block sector[60];
} sector_array;

typedef struct track_array{
    sector_array track[5];
} track_array;

typedef struct fatlist_s{
    string file_name;
    unsigned int first_sector;
} fatlist;

typedef struct fatend_s{
    unsigned int used;
    unsigned int eof;
    unsigned int next;
} fatend;

fatlist *fatl;
fatend *fat;

void oneToThree(int index, int cyl_trk_sec[]){
	int t;
	int s;
    int c = index/300;

    index -= 300*c;
    if((index/60) >= 5){
        t = ((index/60)/5)-1;
    }else{
        t = (index/60);
    }
    s = index%60;


    cyl_trk_sec[0] = c;
    cyl_trk_sec[1] = t;
    cyl_trk_sec[2] = s;
}

void criaFAT(int pos_inicial, string arquivo){
    fatl = new fatlist[arquivos+1];

    fatl[arquivos].file_name = arquivo;
    fatl[arquivos].first_sector = pos_inicial;

    cout << "{" << fatl[0].file_name << "}";

}
int procuraFAT(){
    int i = 0, j = 0, k = 0;

    while(fat[i].used == 1){
		i++;
		if(i % 60 == 0){
			j++;
			i = j*300;
			//total_time += T_MEDIO_LAT;
		}
		if(i % 3000 == 0){
			k++;
			i = k*60;
		}
    }
    
    return i;

}

int FAT(string entrada){
    entrada = "Teste1";
    cout << "NOME:          TAMANHO EM DISCO         LOCALIZACAO" << endl;
    cout << entrada << endl;

    return 0;
}

int escreverArquivo(track_array *cylinder){
    fstream file;
    ifstream file2;
    string arquivo = "Teste1.txt";
    streampos tamanhoArquivo;

    //cout << "Nome do Arquivo .txt:" << endl;
    //cin >> arquivo;

    file.open(arquivo.c_str());
    file.seekg(0, ios::end);
    tamanhoArquivo = file.tellg();
    file.close();

    int pos_inicial = procuraFAT();

    cout << "Nome: " << arquivo << " Tamanho: " << tamanhoArquivo << endl;
    int pos_cilindro[] = {0, 0, 0};
    int clusters = (tamanhoArquivo/(512*4));
    criaFAT(0, arquivo);
    oneToThree(pos_inicial, pos_cilindro);

    int i = 0, j;
    int aux, setor = 0, prox_setor, setor_atual;
    file2.open(arquivo.c_str());

    while(setor < tamanhoArquivo){
        setor_atual = pos_inicial;
        getchar();
        while((i<512)){
            file2.read(&cylinder[pos_cilindro[0]].track[pos_cilindro[1]].sector[pos_cilindro[2]].bytes_s[i], sizeof(char));
            cout << cylinder[pos_cilindro[0]].track[pos_cilindro[1]].sector[pos_cilindro[2]].bytes_s[i];
            i++;
            if(file2.eof() == 1){
                fat[setor_atual].eof = 1;
                file2.close();
                return 0;
            }
        }
        getchar();
        cout << "}";
        setor++;

        if(setor%4 == 0){
            j = 0;
            pos_cilindro[2] += 57;
            while(fat[pos_cilindro[2]].used != 0){
                pos_cilindro[2]++;
                j++;
                if(j == 4){
                    pos_cilindro[2] += 57;
                }
            }
            pos_inicial = pos_cilindro[2];
        } else {
            pos_inicial++;
            pos_cilindro[2] = pos_inicial;
        }

        if(pos_cilindro[0] != aux){

        }
        prox_setor = pos_inicial;
    }
    fat[setor_atual].eof = 1;
    file2.close();
    return 0;
}

void lerArquivo(track_array *cylinder){
    string arquivo = "Teste1.txt";
    int i = 0;
    
    //cout << "Nome do Arquivo .txt:" << endl;
    //cin >> arquivo;

    while((arquivo.compare(fatl[i].file_name) != 0) && (i <= arquivos)) {
        cout << fatl[i].file_name;
        i++;
    }
    cout << "{" << fatl[i].file_name << "}";
}

void apagarArquivo(){
    string arquivo;
    int pos;
    
    cout << "Nome do Arquivo .txt:" << endl;
    cin >> arquivo;

    pos = procuraFAT();

    while(fat[pos].eof != 1){
        fat[pos].used = 0;
        pos++;
    }
    fat[pos].eof = 0;

    remove(arquivo.c_str());

}


int main(){
    int entrada;
    string arquivo;
    track_array *cylinder = new track_array[10];
    fat = new fatend[3000];

    while(1){
        cout << "1- Escrever arquivo" << endl 
            << "2- Ler arquivo" << endl
            << "3- Apagar arquivo" << endl
            << "4- Mostrar tabela FAT" << endl
            << "5- Sair" << endl;
        cin >> entrada;

        switch (entrada){
            case 1: escreverArquivo(cylinder);
                    arquivos++;
                    break;
            case 2: lerArquivo(cylinder);
                    break;
            case 3: apagarArquivo();
                    break;
            case 4: FAT(arquivo);
                    break;
            default: return 0;
        }
    }
    delete[] cylinder;
    
    return 0;
}