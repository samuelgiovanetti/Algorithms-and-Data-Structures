#include "nonna.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <random>

using namespace std;

static int C, G, M;
static int dimensione_popolazione=0; // inizializzo una popolazione a 0 poi verrà aggiornata in base a input
static int dimensione_individuo=0;
static int cicli_pmv=5000; //nel trovare il vicino migliore operiamo cicli_pmv combinazioni di geni

int best_score;


// Inizializzazione della matrice di DP
void initializeDP(vector<vector<int>>& dp, vector<vector<int>>& max_nodo_destra, vector<int>& posizione, const vector<vector<int>>& lista, const vector<int>& genoma, int C, int M) {
    int limite = 0;
    dp = vector<vector<int>>(C+1, vector<int>(G+1, 0));

    for (int i = 0; i <= C; i++) {
        for (int j = 0; j <= lista[i].size(); j++) {
            if (i == 0 || j == 0)
                dp[i][j] = 0;
            else if (lista[i-1][j-1] <= j)
                dp[i][j] = max(lista[i-1][] + dp[i-1][j-[i-1]],  dp[i-1][j]);
            else
                dp[i][j] = dp[i-1][j];
        }
    }
}

// Calcolo del fitness per una permutazione
int fitness(vector<vector<int>>& dp, vector<vector<int>>& max_nodo_destra, vector<int>& posizione, const vector<int>& genoma, int C) {
    return dp[C][posizione[genoma[C-1]]];
}

// Aggiornamento del fitness per un'inversione
int updateFitness(vector<vector<int>>& dp, vector<vector<int>>& max_nodo_destra, vector<int>& posizione, vector<int>& genoma, int pos_i, int pos_j, const vector<vector<int>>& lista, int C) {
    vector<vector<int>> max_nodo_destra_locale = max_nodo_destra;
    int pos = posizione[pos_i];
    int intersezioni = max_nodo_destra_locale[pos][1];
    pos++;
    int limite = pos;

    for (int i = pos_i; i < C; i++) {
        for (int j = 0; j < lista[genoma[i]].size(); j++) {
            max_nodo_destra_locale[pos][0] = lista[genoma[i]][j];
            
            for(int k = 0; k < limite; k++) {
                if (lista[genoma[i]][j] < max_nodo_destra_locale[k][0]) {
                    intersezioni += 1;
                }
            }
            max_nodo_destra_locale[pos][1] = intersezioni;
            pos++;
        }
        limite = pos;
    }
    
    if(intersezioni < fitness(dp, max_nodo_destra, posizione, genoma, C)) {
        max_nodo_destra = max_nodo_destra_locale;
    }

    return intersezioni;
}


int fitness(vector<vector<int>> lista, vector<int> genoma, vector<vector<int>> &max_nodo_destra, vector<int> &posizione) {//ritorna il numero di intereszione di una data permutazione
    //per contare le intersezioni mi creo una lista max_nodo_destra(M) in cui ci metto i "gomitoli" raggiunti dai "centrini", 
    //se un altro centrino poi raggiunge un gomitolo nella lista con valore inferiore a se stesso, incrememntiamo le intersezioni
    int intersezioni=0, limite=0; 
    //limite indica il valore massimo sul quale itero la lista max_nodo_destra(M); 
    // mi garantisce che l'incremento non viene fatto anche sui centrini raggiunti dallo stesso gomitolo (visto che è impossibile)
    //vector<int> max_nodo_destra(M);

    //max_nodo_destra.clear();
    max_nodo_destra=vector<vector<int>>(M+cicli_pmv + 1, vector<int>(2, 0));
    //posizione.clear();
    posizione=vector<int>(C+cicli_pmv+1,0);

    int pos=1;

    for (int i = 0; i < C; i++) {
        for (int j = 0; j < lista[genoma[i]].size(); j++) {
            max_nodo_destra[pos][0]=lista[genoma[i]][j];
            
            for(int k=0; k<limite;k++){
                if (lista[genoma[i]][j] < max_nodo_destra[k][0]) {
                    intersezioni += 1;
                }   
            }
            max_nodo_destra[pos][1]=intersezioni;
            posizione[genoma[i]+1]++;
            pos++;
        } 
        posizione[genoma[i]+1]=posizione[genoma[i]+1]+lista[genoma[i]].size(); 
        limite=pos;
    }
  
    for (int i=1; i<G+1; i++) {
        posizione[i]=posizione[i]+posizione[i-1]; 
    }

    return intersezioni;
}

int pmv_fitness(vector<vector<int>> lista, vector<int> &genoma, int pos_i, int pos_j, vector<vector<int>> &max_nodo_destra, vector<int> &posizione) {
    vector<vector<int>> max_nodo_destra_locale=max_nodo_destra;

    int pos=posizione[pos_i];
    int intersezioni=max_nodo_destra_locale[pos][1];
    pos++;
    int limite=pos;

    for (int i = pos_i; i < C; i++) {
        for (int j = 0; j < lista[genoma[i]].size(); j++) {
            max_nodo_destra_locale[pos][0]=lista[genoma[i]][j];
            
            for(int k=0; k<limite;k++){
                if (lista[genoma[i]][j] < max_nodo_destra_locale[k][0]) {
                    intersezioni += 1;
                }   
            }
            max_nodo_destra_locale[pos][1]=intersezioni;
            pos++;
        }
        limite=pos;
    }
    
    if(intersezioni<best_score){
        max_nodo_destra=max_nodo_destra_locale;
    }

    

    return intersezioni;
}

void grandi_miglior_vicino(vector<vector<int>> lista, ostream& out, vector<vector<int>>& dp, vector<vector<int>>& max_nodo_destra, vector<int>& posizione, vector<int>& migliore_individuo) {
    // Invece di iterare su tutti gli individui, procediamo scambiando casualmente i geni dello stesso individuo
    for (int k = 0; k < cicli_pmv; k++) {
        vector<int> nuovo_individuo = migliore_individuo; // Crea elemento scambiando posizioni casuali
        // Supponiamo che 't' sia un contatore che diminuisce con il tempo
        int contatore = dimensione_individuo;

        // Ora, quando generi 'i' e 'j', usa 't' invece di 'dimensione_individuo'
        int i = contatore - 1 - rand() % contatore;
        int j = (i + contatore - 1 - rand() % (contatore - 1)) % contatore;
        contatore--;

        swap(nuovo_individuo[i], nuovo_individuo[j]);

        int nuovo_individuo_score = updateFitness(dp, max_nodo_destra, posizione, nuovo_individuo, min(i, j), max(i, j), lista, dimensione_individuo);

        if (nuovo_individuo_score < best_score) { // Se becco un nuovo miglior individuo lo rimpiazzo
            migliore_individuo = nuovo_individuo;
            best_score = nuovo_individuo_score;

            cout << best_score << endl;
            out << best_score << endl;

            for (int i = 0; i < dimensione_individuo; i++) {
                cout << migliore_individuo[i] << " ";
                out << migliore_individuo[i] << " ";
            }
            cout << endl << "***" << endl;
            out << endl << "***" << endl;
        }
    }
}

int main() {
    ifstream in("input/input.txt");

    if (in.fail()) {
        cout << "Errore aprendo il file input.txt" << endl;
        exit(1);
    }
    in >> C >> G >> M;

    dimensione_individuo = C; // Un individuo è una permutazione di gomitoli

    vector<vector<int>> lista(M);

    for (int i = 0; i < M; i++) {
        int u, v;
        in >> u >> v;
        lista[u].push_back(v);
    }

    vector<int> individuo(dimensione_individuo);
    
    for (int i = 0; i < dimensione_individuo; i++) {
        individuo[i] = i;
    }

    vector<vector<int>> dp;
    vector<vector<int>> max_nodo_destra;
    vector<int> posizione; 
    
    initializeDP(dp, max_nodo_destra, posizione, lista, individuo, C, M);
    best_score = fitness(dp, max_nodo_destra, posizione, individuo, C);
    vector<int> migliore_individuo = individuo;

    ofstream out("output.txt");

    if (out.fail()) {
        cout << "Errore aprendo il file output.txt" << endl;
        exit(1);
    }

    cout << best_score << endl;
    out << best_score << endl;

    for (int i = 0; i < dimensione_individuo; i++) {
        cout << migliore_individuo[i] << " ";
        out << migliore_individuo[i] << " ";
    }
    cout << endl << "***" << endl;
    out << endl << "***" << endl;

    grandi_miglior_vicino(lista, out, dp, max_nodo_destra, posizione, migliore_individuo);

    cout << best_score << endl;
    out << best_score << endl;

    for (int i = 0; i < dimensione_individuo; i++) {
        cout << migliore_individuo[i] << " ";
        out << migliore_individuo[i] << " ";
    }
    cout << endl << "***" << endl;
    out << endl << "***" << endl;

    out.close();
    in.close();

    return 0;
}
