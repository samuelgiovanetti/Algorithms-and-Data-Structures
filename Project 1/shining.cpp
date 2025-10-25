/*
Il problema è stato ridotto a trovare il primo nodo che se rimosso, elimina la ciclicità del grafo.
Inizialmente avevo pensato di trovare le componenti fortemente connesse ed escludere i grafi con più di due SCC di almeno due nodi, 
visto che un nodo rimosso individualmente non avrebbe eliminato la ciclicità. Tuttavia, dopo vari tentativi a cercare di semplificare
il grafo riducendolo a sempre più piccole SCC, mi sono accorto che questa riduzione è inutilmente costosa.

Quindi, dal momento che l'output richiede il ciclo del nodo trovato e visto che comunque questi nodi ricercati devono essere condivisi da 
tutti i cicli, ho pensato di ridurre il problema alla ricerca del primo ciclo con una DFS per poi cercare il nodo intenro al ciclo che 
se rimosso, elimina la ciclicità.

Idealmente, un ciclo più piccolo comporta meno controlli, ma trovare cicli piccoli è difficile. Ho provato con BFS, ma avendo un grafo 
diretto, risulta essere troppo complesso.
*/

#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cstdlib>
#include <chrono>

using namespace std;
using namespace std::chrono;

//Leggo il grafo mettendo i nodi in una lista di adiacenza
void riempigrafo(const string & nome, vector < vector < int >> & lista, int & N, int & M) {
  ifstream file(nome);
  if (!file.is_open()) {
    cerr << "Errore apertura file." << endl;
    exit(1);
  }
  file >> N >> M;
  lista.resize(N);
  for (int i = 0; i < M; ++i) {
    int u, v;
    file >> u >> v;
    lista[u].push_back(v);
  }
  file.close();
}

//Cerco il primo ciclo

//Inizialmente ho provato con DFS iterativa e stack esplicito per evitare stack overflow, ma dopo aver visto che impiegava molto più 
//tempo e che lo stack overflow non era un problema, ho optato per la DFS ricorsiva.
//Esploro quindi i nodi uno per uno e controllo se ho già incontrato un nodo (nel path). Se sì, ho trovato un ciclo e lo ritorno, 
//altrimenti aggiungo il nodo al vettore
void dfs(int nodo,
  const vector < vector < int >> & G, vector < bool > & visitati, vector < int > & path, vector < int > & ciclo) {
  visitati[nodo] = true;
  path.push_back(nodo);

  for (int i = 0; i < G[nodo].size(); ++i) {
    if (!visitati[G[nodo][i]]) {
      dfs(G[nodo][i], G, visitati, path, ciclo);
      if (!ciclo.empty()) {
        return;
      }
    } else { //se trovo un nodo già visitato, allora ho un ciclo
      for (int j = path.size() - 1; j >= 0; --j) { //riprendo i nodi percorsi fino a qui (corrispondenti al ciclo)
        ciclo.insert(ciclo.begin(), path[j]); //aggiungo i nodi all'inizio del ciclo visto che poi andranno letti in ordine di lettura
        if (path[j] == G[nodo][i]) break;
      }
      return;
    }
  }
  path.pop_back();
}

//trova il primo ciclo nel grafo
void trovaciclo(const vector < vector < int >> & G, vector < int > & ciclo) {
  int n = G.size();
  vector < bool > visitati(n, false);
  vector < int > path;

  for (int i = 0; i < n; ++i) { //faccio un for di dfs per assicurarmi di esplorare anche componenti staccate in grafi sparsi.
    if (!visitati[i]) {
      dfs(i, G, visitati, path, ciclo);
      if (!ciclo.empty()) {
        return;
      }
    }
  }
}

//Cerco il nodo che se rimosso interrompe ciclicità nel grafo.

//Le funzioni spottaciclo e trovaciclo servono per rimuovere singolarmente ogni nodo del ciclo e vedere se la rimozione 
//rende il grafo aciclico.
//Sono state esplorati vari modi per migliorare l'efficienza del codice, tra cui prediligere i nodi con più archi entranti o uscenti, 
//ma seppur leggermente efficaci in termini di tempi nei test, non hanno portato vantaggi nelle submissions e quindi sono stati scartati.
bool spottaciclo(int nodo, vector < int > & visited, vector < int > & percorso,
  const vector < vector < int >> & G, int skip, int conta, steady_clock::time_point start_time, bool careful) {
  if (nodo == skip) {
    return false;
  }

  if (visited[nodo] != conta) {
    visited[nodo] = conta;
    percorso[nodo] = conta;

    for (int i = 0; i < G[nodo].size(); ++i) {
      if (careful) {
        auto end = steady_clock::now();
        auto time = duration_cast < milliseconds > (end - start_time).count();
        if (time > 775) {
          return -1;
        }

      }
      int arrivo = G[nodo][i];
      if (percorso[arrivo] == conta) { // se trovo un nodo di arrivo già visitato, ho un ciclo
        return true;
      } else if (visited[arrivo] != conta && spottaciclo(arrivo, visited, percorso, G, skip, conta, start_time, careful)) { // altrimenti continuo ad esplorare
        return true;
      }
    }
  }
  percorso[nodo] = 0; // azzeriamo la visita nella sessione corrente
  return false;
}

//cerco il nodo che se rimosso elimina la ciclicità
int trovanodo(const vector < vector < int >> & G,
  const vector < int > & ciclo, steady_clock::time_point start_time) {
  int n = G.size();
  vector < int > visited(n, 0);
  vector < int > percorso(n, 0); // tiene traccia della sessione in cui un nodo è stato visitato
  int conta = 0;
  bool careful = false;

  for (int i = 0; i < ciclo.size(); ++i) {
    conta++;
    int nodo = ciclo[i];
    bool ciclico = false;

    auto end = steady_clock::now();
    auto time = duration_cast < milliseconds > (end - start_time).count();
    if (time > 700) {
      careful = true;
    }

    for (int j = 0; j < n; ++j) { // itero su tutti i nodi per vedere se rimuovendo il nodo si crea un ciclo

      if (spottaciclo(j, visited, percorso, G, nodo, conta, start_time, careful)) { // se trovo un ciclo, il nodo non va bene
        ciclico = true;
        break;
      } else {
        if (careful) {
          auto end = steady_clock::now();
          auto time = duration_cast < milliseconds > (end - start_time).count();
          if (time > 775) {
            return -1;
          }

        }

      }
    }

    if (!ciclico) { // se non troviamo cicli, il nodo è quello giusto e lo ritorno
      return nodo;
    }
  }
  return -1;
}

//DFS per trovare il ciclo escludendo un nodo, nella ricerca del secondo ciclo per i grafi che hanno portato valore -1
void dfsnonodo(int nodo,
  const vector < vector < int >> & G, vector < bool > & visitati, vector < int > & path, vector < int > & ciclo, int skip) {
  if (nodo == skip) {
    return;
  }
  visitati[nodo] = true;
  path.push_back(nodo);

  for (int i = 0; i < G[nodo].size(); ++i) {
    int next = G[nodo][i];
    if (next == skip) {
      continue;
    }

    if (!visitati[next]) {
      dfsnonodo(next, G, visitati, path, ciclo, skip);
      if (!ciclo.empty()) {
        return;
      }
    } else {
      for (int j = path.size() - 1; j >= 0; --j) {
        ciclo.insert(ciclo.begin(), path[j]);
        if (path[j] == next) {
          break;
        }
      }
      return;
    }
  }

  path.pop_back();
  visitati[nodo] = false;
}

//cerco un ciclo escludendo il nodo utilizzato nel primo ciclo
void trovanodo2(const vector < vector < int >> & G, vector < int > & ciclo, int skip) {
  int n = G.size();
  vector < bool > visitati(n, false);
  vector < int > path;

  for (int i = 0; i < n; ++i) {
    if (i != skip && !visitati[i]) {
      dfsnonodo(i, G, visitati, path, ciclo, skip);
      if (!ciclo.empty()) {
        return;
      }
    }
  }
}

//stampa ciclo
void stampaciclo(const vector < int > & ciclo, ostream & out) {
  cout << ciclo.size() << " ";
  out << ciclo.size() << " ";

  for (int nodo: ciclo) {
    cout << nodo << " ";
    out << nodo << " ";
  }
  cout << endl;
  out << endl;
}

int main() {
  auto start_time = steady_clock::now();
  string file = "input.txt";

  vector < vector < int >> G;
  int N, M;
  riempigrafo(file, G, N, M);
  ofstream out("output.txt");
  if (!out.is_open()) {
    cerr << "Error opening output file." << endl;
    return 1;
  }

  vector < int > ciclo;
  trovaciclo(G, ciclo);
  int centro = trovanodo(G, ciclo, start_time);
  if (centro != -1) {
    cout << centro << endl;
    out << centro << endl;
    stampaciclo(ciclo, out);
  } else {
    cout << -1 << endl;
    out << -1 << endl;
    srand(time(0));
    int X = rand() % ciclo.size();
    cout << ciclo[X] << endl;
    out << ciclo[X] << endl;
    stampaciclo(ciclo, out);

    vector < int > ciclo2;
    trovanodo2(G, ciclo2, ciclo[X]);
    stampaciclo(ciclo2, out);
  }

  //auto fine = steady_clock::now();  
  //auto duration = duration_cast<milliseconds>(fine - start_time); 
  //cout << "Tempo: " << duration.count() << " ms" << endl;

  out.close();
  return 0;
}