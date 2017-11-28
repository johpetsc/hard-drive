#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <cmath>
#include <vector>

using namespace std;

int arquivos = 0;//Quantidade de arquivos txt adicionados a memória

typedef struct block{
    char bytes_s[512];
} block;

typedef struct sector_array{
    block sector[60];
} sector_array;

typedef struct track_array{
    sector_array track[5];
} track_array;
//Structs que simulam a estrutura física de um HD

typedef struct fatlist_s{
    string file_name;
    unsigned int first_sector;
} fatlist;

typedef struct fatend_s{
    unsigned int used;
    unsigned int eof;
    unsigned int next;
} fatend;
//Structs para a tabela FAT

vector<fatlist> fatl(1);
fatend *fat;

void posicaoHD(int pos_inicial, int c_t_s[]){
    //Recebe uma posição de 0-3000 e retorna o cilindro, trilha e setor.

    c_t_s[0] = floor((pos_inicial%300)%60);
    c_t_s[1] = floor((pos_inicial%300)/60);
    c_t_s[2] = floor(pos_inicial/300);
    cout << "cilindro2: " << c_t_s[2] << " trilha: " << c_t_s[1] << " setor: " << c_t_s[0] << endl;

}

void criaFAT(int pos_inicial, string arquivo){
    //Guarda o nome do arquivo e a posição do seu primeiro setor.
    
    fatl.resize(arquivos+1);
    fatl[arquivos].file_name = arquivo;
    fatl[arquivos].first_sector = pos_inicial;

}
int procuraFAT(){
    //procura o primeiro setor de 0-3000 que não foi utilizado 
    int i = 0, j = 0, k = 0;

    while(fat[i].used == 1){
        
		i++;
    }
    return i;

}

int FAT(){
    //Função pra mostrar a tabela fat
    //Falta fazer
    fstream file;
    int i = 0,j = 0,tamanhoArquivo;
    
    string nome;
    
    cout << "NOME:          TAMANHO EM DISCO         LOCALIZACAO" << endl;
    while(i < arquivos){
		if(fatl[i].file_name.compare("0") != 0){
            nome = fatl[i].file_name;
			file.open(nome.c_str());//abre o arquivo
            file.seekg(0, ios::end);//posiciona no final do arquivo
            tamanhoArquivo = file.tellg();//pega o tamanho em bytes do arquivo
            file.close();

			printf("\n%s%12d Bytes                  ", nome.c_str(), tamanhoArquivo);
			j = fatl[i].first_sector;
            
            
			while(fat[j].eof != 1){
			    printf("%d,", j);
				j = fat[j].next;
			}
            printf("%d", j);
			printf("\n\n");
		}
		i++;
}
    return 0;
}

int escreverArquivo(track_array *cylinder){
    //Função que posiciona os bytes do arquivo txt na estrutura do hd

    fstream file;
    ifstream file2;
    string arquivo;

    cout << "Nome do Arquivo .txt:" << endl;
    cin >> arquivo;

    file.open(arquivo.c_str());//abre o arquivo
    file.seekg(0, ios::end);//posiciona no final do arquivo
    streampos tamanhoArquivo = file.tellg();//pega o tamanho em bytes do arquivo
    file.close();//fecha o arquivo
    
    int pos_inicial = procuraFAT();//posição a partir da qual pode começar a ser gravado o arquivo
    cout << "pos_inicial: " << pos_inicial<<endl;
    cout << "Nome: " << arquivo << " Tamanho: " << tamanhoArquivo << endl;
    int c_t_s[] = {0, 0, 0};//posição do cilindro, trilha e setor
    int clusters = floor(tamanhoArquivo/(512*4))+1;//quantidade de clusters pro arquivo
    //usei floor porque a função ceil não tava funcionando lol(por isso tem o +1)
    cout << "Serão utilizados " << clusters << " para este arquivo." << endl;
    cout << "arquivos: " << arquivos << endl;
    while(pos_inicial%4 == 0 && arquivos > 0){
        fat[pos_inicial].used = 1;
        fat[pos_inicial].next = pos_inicial+1;
        pos_inicial++;
    }
    cout << "pos_inicial2: " << pos_inicial << endl;
    criaFAT(pos_inicial, arquivo);//adiciona o arquivo na tabela FAT
    posicaoHD(pos_inicial, c_t_s);//pega a posição inicial na estrutura do vetor

    int i = 0,j;
    int setor = 0;
    file2.open(arquivo.c_str());

    while(setor < (clusters*4)){
        cout << "cilindro: " << c_t_s[2] << " trilha: " << c_t_s[1] << " setor: " << c_t_s[0] << endl;
        while((i<512)){//escreve cada byte do arquivo na memória
            file2.read(&cylinder[c_t_s[2]].track[c_t_s[1]].sector[c_t_s[0]].bytes_s[i], sizeof(char));
            //cout << cylinder[c_t_s[2]].track[c_t_s[1]].sector[c_t_s[0]].bytes_s[i];
            if(file2.eof() == 1){//verifica se terminou o arquivo
                fat[pos_inicial].eof = 1;
                file2.close();
                fat[pos_inicial].used = 1;
                for(j = 0;j <(pos_inicial%4);j++){
                    fat[pos_inicial+j].used = 1;
                }
                cout << "pos: " << pos_inicial<<endl;
                return 0;
            }
            i++;
        }
        
        fat[pos_inicial].used = 1;
        
        i = 0;
        c_t_s[0]++;
        setor++;
        pos_inicial++;

        //depois de gravar um cluster procura a posição do próximo cluster livre
        if(c_t_s[0]%4 == 0){
            
            int livre = 0;
            while(livre == 0){
                cout << fat[pos_inicial].used << endl;
                cout << "cilindro3: " << c_t_s[2] << " trilha: " << c_t_s[1] << " setor: " << c_t_s[0] << endl;
                if(c_t_s[0] == 60){
                    c_t_s[1]++;
                    c_t_s[0] = 0;
                    
                    if(c_t_s[1] == 5){
                        c_t_s[2]++;
                        c_t_s[1] = 0;
                    
                    }
                    if(c_t_s[2] == 10){
                        cout << "Sem espaço no Disco." << endl;
                        return 0;
                    }
                    while((fat[pos_inicial].used == 1) && (c_t_s[0] != 60)){
                        c_t_s[0] += 4;
                        pos_inicial++;
                    }
                    if(fat[pos_inicial].used == 0){
                        livre = 1;
                    }
                } else {
                    while((fat[pos_inicial].used == 1) && (c_t_s[0] != 60)){
                        c_t_s[0] += 4;
                        pos_inicial++;
                    }
                    if(fat[pos_inicial].used == 0){
                        livre = 1;
                    }
                }
            }
        }
    }
    return 0;
}

int lerArquivo(track_array *cylinder){
    string arquivo;
    string saida = "Saida.txt";
    int i = 0;
    fstream file;
    ofstream file2;
    
    cout << "Nome do Arquivo .txt:" << endl;
    cin >> arquivo;

    //compara a string de entrada com os arquivos da tabela FAT
    while((arquivo.compare(fatl[i].file_name) != 0) && (i <= arquivos)) {
        cout << fatl[i].file_name;
        i++;
    }
    file.open(arquivo.c_str());
    file.seekg(0, ios::end);
    streampos tamanhoArquivo = file.tellg();
    file.close();

    int setor = fatl[i].first_sector;//verifica o primeiro setor do arquivo na tabela FAT
    int c_t_s[] = {0, 0, 0};
    posicaoHD(setor, c_t_s);//procura a posição do setor no hd
    i = 0;

    file2.open(saida.c_str());
    int j = 0;
    while(1){
        while((i<512)){//escreve os bytes da memória pro arquivo de saída
            file2.write(&cylinder[c_t_s[2]].track[c_t_s[1]].sector[c_t_s[0]].bytes_s[i], sizeof(char));
            cout << cylinder[c_t_s[2]].track[c_t_s[1]].sector[c_t_s[0]].bytes_s[i];
            if(tamanhoArquivo == j+2){
                    file2.close();
                    return 0;
                }
            j++;
            i++;
        }
        i = 0;
        c_t_s[0]++;
        if(c_t_s[0] == 60){
            c_t_s[1]++;
            c_t_s[0] = 0;
            if(c_t_s[1] == 5){
                c_t_s[2]++;
                c_t_s[1] = 0;
            }
        }
    }
}

int apagarArquivo(){
    //falta terminar
    string arquivo;
    int pos,j;
    int i = 0;
    
    cout << "Nome do Arquivo .txt:" << endl;
    cin >> arquivo;

    while((fatl[i].file_name != arquivo)){
        if (i < arquivos){
            i++;
        }
        else{
            return 0;
        }
    }
    j = fatl[i].first_sector;
    fatl[i].file_name = "0";
    while(fat[j].eof != 1){
		fat[j].used = 0;
		j = fat[j].next;
    }
    fat[j].used = 0;
    fat[j].eof = 0;

}


int main(){
    int entrada,i;
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
            case 3: i = apagarArquivo();
                    if (i == 0){
                        printf("Arquivo não existe\n");
                    }
                    else{
                        printf("Arquivo excluído com sucesso\n");
                    }
                        
                    break;
            case 4: FAT();
                    break;
            default: return 0;
        }
    }
    delete[] cylinder;
    
    return 0;
}