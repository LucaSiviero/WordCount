####
Studente: Luca Siviero 05225012012
Docente: Carmine Spagnuolo
####

Il progetto wordcount richiede di implementare una soluzione parallela per il calcolo delle occorrenze delle singole parole all'interno di una directory di file.

La soluzione parallela che è stata implementata si basa sull’idea che i file vanno letti in maniera partizionata da parte di ogni processore, che si occupano di contare le occorrenze di tutte le parole all’interno della propria partizione. Quando ogni processore termina il conteggio delle occorrenze, esso invia al processo master il numero totale delle occorrenze per la propria partizione. Quindi, il master procede a un conteggio finale che rappresenta il numero di occorrenze per ogni singola word nell’intera directory di file.

I passi del progetto si dividono in tre parti fondamentali:
1.	Dividere le porzioni di testo su cui i processi dovranno contare le occorrenze.
2.	Eseguire il calcolo delle occorrenze sui singoli processi worker.
3.	Restituire tutto al master.

La struttura "Dataset" contiene un intero che rappresenta la grandezza del dataset, un array di interi che rappresenta il conteggio delle occorrenze per ogni singola parola e un array di stringhe.
Le stringhe non sono un tipo definito in C. Tuttavia, dover comunicare array di array di caratteri era scomodo, quindi si è proceduto alla definizione di una nuova struttura: String.

La parte iniziale non è stata parallelizzata. Questo comporta un limite importante quando si analizzano i tempi di esecuzione.
Infatti, la prima parte viene eseguita solo dal processore master, che si occupa di leggere l'intera directory di file, di salvare tutti i dati nel proprio Dataset e di inviare ad ogni singolo processore il numero di word specificato nella dimensione della partizione.
Consideriamo il resto della divisione tra il numero di word totali nella directory di file e il numero di processori senza il master.
Ogni processore è ordinato secondo il rank di MPI_COMM_WORLD e riceve un numero di word equo, ma non identico.
I processori con rank <= del resto della divisione ricevono una word in più, mentre quelli con rank > del resto ricevono un numero di word pari al quoziente della divisione tra numero di word e numero di processori.

Nel file viene usata la libreria countLibrary.c che contiene la definizione delle strutture e la definizione di due funzioni fondamentali.
La prima funzione checkPunctuations(char[] string) prende in input un array di caratteri e trasforma i caratteri a lowercase. Poi, controlla la punteggiatura per assicurarsi che le intere parole vengano considerate senza i caratteri di punteggiatura.
Invece, la funzione checkExistance(Dataset *dt) prende in input un puntatore a una struttura Dataset e usa due indici, i e j = i+1 con cui scorre l’intero array di String per controllare che le occorrenze si ripetano sull’indice i. Nel caso in cui questo dovesse accadere, il contatore delle occorrenze in posizione i viene aggiornato con una somma tra il suo valore attuale e il valore del contatore delle occorrenze in posizione j. Il contatore delle occorrenze in posizione j viene portato a 0, senza essere cancellato.
Una cancellazione della word e del suo contatore delle occorrenze comportorebbe la necessità di riposizionare gli indici nel dataset.

A questo punto, ogni processore ha contato le sue occorrenze e non deve fare altro che inviarle al processore master, che si occuperà di organizzarle in un unico dataset facendo un conteggio finale con una nuova chiamata a checkExistance().
Alcune stampe di valori catturati con MPI_Wtime() permettono di vedere quali sono i tempi di esecuzione per il programma.