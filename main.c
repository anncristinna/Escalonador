#include<stdio.h>
#include <stdlib.h>

#define quantum 5

#define DISCO 0
#define FITAMAGNETICA 1
#define IMPRESSORA 2

int tempo_io[] = {5, 10, 15};

typedef struct io{
    int tipo;
    int tempo;
} io;

typedef struct processo{
    int pid;
    
    int tempo_chegada;
    int tempo_cpu;
    
    int tempo_cpu_usado;
    
    io *ios;
    int qtd_io;
    int proximo_io;
    
} processo;

typedef struct no_fila{
    processo *p;
    struct no_fila *proximo;
} no_fila;

typedef struct fila{
    no_fila *inicio;
    no_fila *fim;
} fila;

fila *fila_chegada, *fila_alta_prioridade, *fila_baixa_prioridade, *fila_disco, *fila_magnetica, *fila_impressora;
int numero_de_processos;

// liberando processo
void liberar_processo(processo *p){

    if(p != NULL) {
        free(p -> ios);    
		p -> ios = NULL;
		free(p);
		p = NULL;
	}
}

// imprimindo dados do processo
void imprimir_processo(processo *p){
    if(p == NULL)
        printf("\tProcesso vazio\n\n");
    else {
        printf("Processo com PID %d usou: %d total: %d\n", p->pid, p->tempo_cpu_usado, p->tempo_cpu);    
    }
}


// obtém o próximo inteiro da linha, dividindo no próximo ";"
int proximo_int(char *linha, int *inicio){
    int x = 0;
    int i = *inicio;
	while(linha[i] == ' ' || linha[i] == '\t') {
		i++;
	}
    while(linha[i] != ';'){
        x = 10 * x + linha[i] - '0';
        i++;
    }
    *inicio = i + 1;
    return x;
}

// cria novo processo a partir da linha do arquivo
processo *novo_processo(char *linha, int *tempo){
    printf("novo processo %s", linha);
    int *i = (int *) malloc(sizeof(int));
    *i = 0;
    int pid = proximo_int(linha, i);
    int tempo_chegada = proximo_int(linha, i);
    *tempo = tempo_chegada;
    int tempo_cpu = proximo_int(linha, i);
    int qtd_io = proximo_int(linha, i);
    
    // alocando memória
    processo *p = (processo *) malloc (sizeof (processo));
    if(p == NULL)
        return NULL;
    
    p->pid = pid;
    p->tempo_chegada = tempo_chegada;
    p->tempo_cpu = tempo_cpu;
    
    p->tempo_cpu_usado = 0;
    p->qtd_io = qtd_io;
        
    if (p == NULL) {
        printf("\tProcesso vazio\n\n");
    } else {
        printf("\n--- Processo Detalhes ---\n");
        printf("PID: %d\n", p->pid);
        printf("Tempo de chegada: %d\n", p->tempo_chegada);
        printf("Tempo de CPU total necessario: %d\n", p->tempo_cpu);
        printf("Quantidade de operacoes de I/O: %d\n", p->qtd_io);
    }

    if(qtd_io > 0){
        p->ios = (io *) malloc(qtd_io * sizeof(io));
		if(p -> ios == NULL) return NULL;

//inicializa os ios
        for(int j = 0; j < qtd_io; j++){
            int tipo_io = proximo_int(linha, i);
            int momento_io = proximo_int(linha,i);
            printf("tipo_io %d momento I/O %d\n", tipo_io, momento_io);            
            p->ios[j].tempo = momento_io;
            p->ios[j].tipo = tipo_io;
        }
    }
    else {p->ios = NULL;}

    p->proximo_io = 0;
    
    printf("-------------------------\n\n");
    puts(" ");
    
    return p;
}



// inicializa fila
fila *inicializar_fila(){
    fila *f = (fila *) malloc (sizeof(fila));
	if(f ==NULL) return NULL;

    f->inicio = NULL;
    f->fim = NULL;

    return f;
}

// libera uma fila
void liberar_fila(fila *f){
    if(f != NULL){
        if(f->inicio != NULL){
            free(f->inicio);
            f->inicio = NULL;
        } 
        if(f->fim != NULL){
            free(f->fim);
            f->fim = NULL;
        }
        free(f);
        f = NULL;
    }
}

// insere um novo processo na fila
void inserir(fila *f, processo *p, int tempo){
    if(f -> inicio == NULL){
        f->inicio = (no_fila *) malloc (sizeof(no_fila));
        f->inicio->p = p;
        f->inicio->p->tempo_chegada = tempo;
        f->inicio->proximo = NULL;
        f->fim = f->inicio;   
        //printf("Processo %d inserido no inicio da fila %c\n", p->pid, f->nome_fila); /**/

    } else {
        f->fim->proximo = (no_fila *) malloc (sizeof(no_fila));
        f->fim->proximo->p = p;
        f->fim->proximo->p->tempo_chegada = tempo;
        f->fim->proximo->proximo = NULL;
        f->fim = f->fim->proximo;
        //printf("Processo %d inserido na fila %c\n", p->pid, f->nome_fila); /**/
    }
}

// retorna o processo no início da fila ou NULL se a fila estiver vazia
processo *inicio(fila *f){
    if(f -> inicio == NULL)
        return NULL;
    return f->inicio->p;
}

// remove o elemento do início da fila, se não estiver vazia
void remover(fila *f){
    if(f -> inicio == NULL){
        return;
    }
    // apenas um elemento
    if(f->inicio == f->fim){
        free(f->inicio);        
        f->inicio = f->fim = NULL;
        
    }
    else{
        no_fila *novo_inicio = f->inicio->proximo;
        free(f->inicio);
        f->inicio = novo_inicio;
    }
}

ssize_t getline(char **lineptr, size_t *n, FILE *stream) {
    size_t size = 0;
    int c;
    if (!lineptr || !n || !stream) return -1;

    if (*lineptr == NULL) {
        *n = 128;  // Tamanho inicial do buffer
        *lineptr = (char *)malloc(*n);
        if (*lineptr == NULL) return -1;
    }

    char *buffer = *lineptr;
    size_t len = 0;

    while ((c = fgetc(stream)) != EOF) {
        if (len + 1 >= *n) {
            *n *= 2;
            *lineptr = realloc(buffer, *n);
            if (*lineptr == NULL) return -1;
            buffer = *lineptr;
        }
        buffer[len++] = c;

        if (c == '\n') break;
    }

    if (len == 0 && c == EOF) return -1;  // Fim do arquivo

    buffer[len] = '\0';  // Adiciona o terminador de string
    return len;
}

// lê processos do arquivo e inicializa filas
void inicializar(char *nome_arquivo){

    printf("nome do arquivo %s\n\n", nome_arquivo);
    fila_chegada = inicializar_fila(); 
         
    numero_de_processos = 0; 

    // lê do arquivo   
    FILE * arquivo_ptr;
    char * linha = NULL;
    size_t len = 0;
    ssize_t read;
    arquivo_ptr = fopen(nome_arquivo, "r");
    if (arquivo_ptr == NULL){
        printf("Nao leu nada\n");
        exit(0);
    }

    while ((read = getline(&linha, &len, arquivo_ptr)) != -1) {
        int *tempo = (int *) malloc (sizeof(tempo));
        processo *proximo_processo = novo_processo(linha, tempo);
        inserir(fila_chegada, proximo_processo, *tempo);
        numero_de_processos++;
    }

    fclose(arquivo_ptr);
    if (linha != NULL){
        free(linha);
    }
    
    fila_alta_prioridade = inicializar_fila(); 
    fila_baixa_prioridade = inicializar_fila();
    fila_disco = inicializar_fila();
    fila_magnetica = inicializar_fila();
    fila_impressora = inicializar_fila();
}

int minimo(int x, int y){
    if(x < y) return x;
    return y;
}
int maximo(int x, int y){
    if(x > y) return x;
    return y;
}

int processos_finalizados = 0;
int tempo_relogio = 0;
int tempo_disco = 0;
int tempo_impressora = 0;
int tempo_magnetica = 0;

// testa se o processo voltou das I/O
void retorno_de_ios(){
    while(!(fila_disco -> inicio == NULL)){
        tempo_disco = maximo(tempo_disco, inicio(fila_disco)->tempo_chegada);
        if(tempo_disco + tempo_io[DISCO] <= tempo_relogio){
            printf("\nProcesso %d usou DISCO do tempo %d ao tempo %d\n", inicio(fila_disco)->pid, tempo_disco, tempo_disco + tempo_io[DISCO]);
            tempo_disco += tempo_io[DISCO];
            inserir(fila_baixa_prioridade, inicio(fila_disco), tempo_disco);    
            remover(fila_disco);        
        } else {
            break;
        }            
    }
    
    while(!(fila_magnetica -> inicio == NULL)){
        tempo_magnetica = maximo(tempo_magnetica, inicio(fila_magnetica)->tempo_chegada);
        if(tempo_magnetica + tempo_io[FITAMAGNETICA] <= tempo_relogio){
            printf("\nProcesso %d usou FITA MAGNETICA do tempo %d ao tempo %d\n", inicio(fila_magnetica)->pid, tempo_magnetica, tempo_magnetica + tempo_io[FITAMAGNETICA]);
            tempo_magnetica += tempo_io[FITAMAGNETICA];
            inserir(fila_alta_prioridade, inicio(fila_magnetica), tempo_magnetica);    
            remover(fila_magnetica);        
        } else {
            break;
        }            
    }
    
    while(!(fila_impressora -> inicio == NULL)){
        tempo_impressora = maximo(tempo_impressora, inicio(fila_impressora)->tempo_chegada);
        if(tempo_impressora + tempo_io[IMPRESSORA] <= tempo_relogio){
            printf("\nProcesso %d usou IMPRESSORA do tempo %d ao tempo %d\n", inicio(fila_impressora)->pid, tempo_impressora, tempo_impressora + tempo_io[IMPRESSORA]);
            imprimir_processo(inicio(fila_impressora));
            tempo_impressora += tempo_io[IMPRESSORA];
            inserir(fila_alta_prioridade, inicio(fila_impressora), tempo_impressora);    
            remover(fila_impressora);
                
        } else {
            break;
        }            
    }
}

// adiciona novo processo de chegada
void chegada(){
    while(!(fila_chegada -> inicio == NULL) && inicio(fila_chegada)->tempo_chegada <= tempo_relogio){
        printf("--------------------------------------------\n");
        printf("Chegou: ");
        processo *processo_chegada = inicio(fila_chegada);
        imprimir_processo(processo_chegada);
        printf("--------------------------------------------\n\n");
        remover(fila_chegada); 
        inserir(fila_alta_prioridade, processo_chegada, processo_chegada->tempo_chegada);
    }
}

// executa o próximo processo
void executar_proximo_processo(int fila_atual){
    processo *proximo_processo;
    if(fila_atual == 0){ // baixa prioridade
        proximo_processo = inicio(fila_baixa_prioridade);
        remover(fila_baixa_prioridade);    
        printf("Executando (baixa prioridade):\n");
    } else { // alta prioridade
        proximo_processo = inicio(fila_alta_prioridade);
        remover(fila_alta_prioridade);    
        printf("Executando (alta prioridade):\n");
    }

    //delta_tempo representa quanto tempo o processo pode executar nesta rodada.
    int delta_tempo = minimo(quantum, proximo_processo->tempo_cpu - proximo_processo->tempo_cpu_usado);
    
    //Verificar se o processo tem I/O pendente
    int tempo_ate_proximo_io = -1;
    
    if(proximo_processo->proximo_io < proximo_processo->qtd_io) // ainda tem I/O             
        tempo_ate_proximo_io = proximo_processo->ios[proximo_processo->proximo_io].tempo - proximo_processo->tempo_cpu_usado;
    
    if(tempo_ate_proximo_io != -1 && delta_tempo >= tempo_ate_proximo_io){
        
        delta_tempo = tempo_ate_proximo_io;
        proximo_processo->proximo_io++;
        proximo_processo->tempo_cpu_usado += delta_tempo;
        imprimir_processo(proximo_processo);
        // ainda precisa de tempo para terminar        
        int proximo_tempo_relogio = tempo_relogio + delta_tempo;
        while(tempo_relogio < proximo_tempo_relogio){                
            tempo_relogio++;            
            if(tempo_relogio < proximo_tempo_relogio)
                printf("TEMPO: %d\n\n", tempo_relogio);
            chegada(); 
            retorno_de_ios();
        }
        if(proximo_processo->ios[proximo_processo->proximo_io - 1].tipo == DISCO){
            inserir(fila_disco, proximo_processo, tempo_relogio);
        } else if (proximo_processo->ios[proximo_processo->proximo_io - 1].tipo == FITAMAGNETICA){
            inserir(fila_magnetica, proximo_processo, tempo_relogio);                
        } else { // impressora
            inserir(fila_impressora, proximo_processo, tempo_relogio);            
        }
                    
    } else {
        proximo_processo->tempo_cpu_usado += delta_tempo;
        imprimir_processo(proximo_processo);
        // ainda precisa de tempo para terminar        
        int proximo_tempo_relogio = tempo_relogio + delta_tempo;
        while(tempo_relogio < proximo_tempo_relogio){                
            tempo_relogio++;
            if(tempo_relogio < proximo_tempo_relogio)
                printf("TEMPO: %d\n\n", tempo_relogio);
            chegada();
            retorno_de_ios();    
        }
        if(proximo_processo->tempo_cpu_usado < proximo_processo->tempo_cpu){
            inserir(fila_baixa_prioridade, proximo_processo, tempo_relogio);                
        } else {
            printf("\n------ Processo Finalizado ------\n");
            printf("PID: %d\n", proximo_processo->pid);
            printf("Tempo Total de CPU Usado: %d\n", proximo_processo->tempo_cpu_usado);
            printf("---------------------------------\n\n");
            liberar_processo(proximo_processo);

            processos_finalizados++;
        }
    }
}

// Simulação de Round Robin
void round_robin(){
    while(processos_finalizados < numero_de_processos){
        printf("TEMPO: %d\n\n", tempo_relogio);
        
        chegada(); 
        retorno_de_ios();    
        if(!(fila_alta_prioridade -> inicio == NULL)){ 
            executar_proximo_processo(1);
        } else if(!(fila_baixa_prioridade -> inicio == NULL)){ 
            executar_proximo_processo(0);
        } else {
            // todos os processos estão em I/O ou não chegaram
            tempo_relogio++;        
        }
    }
    
    printf("Finalizada a simulacao no tempo %d\n\n", tempo_relogio);
    
    liberar_fila(fila_chegada);
    liberar_fila(fila_alta_prioridade);
    liberar_fila(fila_baixa_prioridade);
    liberar_fila(fila_disco);
    liberar_fila(fila_magnetica);
    liberar_fila(fila_impressora);    
}

int main(int argc, char *argv[]){    
    inicializar(argv[1]);
    round_robin();
}
