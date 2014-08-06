/* Gabriel_Siqueira 1210689 3WA */
/* Noemie_Nakamura 1110743 3WA */

#include <stdio.h>
#include <stdlib.h>
#include "gera.h"

#define MAXT 1000  // tamanho máximo da tradução do código SB

void prologo(unsigned char *codigo, int *pos, unsigned int *funcoes, int *numFuncoes);
void call(int numFuncao, char tipoRec, int numRec, char tipoParm, int numParm, unsigned int *funcoes, unsigned char *codigo, int *pos);
void operacao(char tipoRec, int numRec, char tipoOperando1, int numOperando1, char operador, char tipoOperando2, int numOperando2, unsigned char *codigo, int *pos);
void retorno(char tipoRetorno, int retorno, char tipoParm, int numParm, unsigned char *codigo, int *pos);
void finalizacao(unsigned char *codigo, int *pos);
void deslocamentoEbp(unsigned char *codigo, int *pos, char tipo, int numTipo);

static void error (const char *msg, int linha) {
    fprintf(stderr, "ERRO: %s na linha %d\n", msg, linha);
    exit(EXIT_FAILURE);
}

void gera(FILE *f, void **code, funcp *entry){
    int linha = 1, // controle de leitura das linhas
        pos = 0, // controle da posição do vetor com código de entrada
        numFuncoes = 0; //controle da posição do vetor de endereço das funções
    unsigned int funcoes[10]; // *funções é um vetor com os endereços de cada função no código de entrada
    unsigned char *codigo; // *codigo é um vetor com os códigos de máquina
    char c;
    
    // Separar espaço para o código
    codigo = (unsigned char *)malloc(MAXT);
    
    // Enquanto não chegamos ao fim do arquivo, ler cada caracter e interpretar
    while ((c = fgetc(f)) != EOF) {
        switch (c) {
            case 'f': {  /* function */
                char c0;
                if (fscanf(f, "unction%c", &c0) != 1) 
                    error("comando invalido", linha);
                else
                    prologo(codigo, &pos, funcoes, &numFuncoes);
                break;
            }
            case 'e': {  /* end */
                char c0[3];
                if (fscanf(f, "%3c", c0) != 1)
                    error("comando invalido", linha);
                else
                    finalizacao(codigo, &pos);
                break;
            }
            case 'v': case 'p': {  /* atribuicao */
                int i0;
                char v0 = c, c0;
        
                if (fscanf(f, "%d = %c", &i0, &c0) != 2)
                    error("comando invalido", linha);
        
                if (c0 == 'c') { /* call */
                    int n, i1;
                    char v1;
                    if (fscanf(f, "all %d %c%d", &n, &v1, &i1) != 3) 
                        error("comando invalido", linha);
                    else
                        call(n, v0, i0, v1, i1, funcoes, codigo, &pos);
                }
                else { /* operacao aritmetica */
                    int i1, i2;
                    char v2, op;
                    if (fscanf(f, "%d %c %c%d", &i1, &op, &v2, &i2) != 4)
                        error("comando invalido", linha);
                    else
                        operacao(v0, i0, c0, i1, op, v2, i2, codigo, &pos);
                }
                break;
            }
            
            case 'r': {  /* ret */
                int i0, i1;
                char v0, v1;
                if (fscanf(f, "et? %c%d %c%d", &v0, &i0, &v1, &i1) != 4)
                   error("comando invalido", linha);
                else
                    retorno(v0, i0, v1, i1, codigo, &pos); 
                break;
            }
            default: error("comando desconhecido", linha);
        }
        linha++;
        fscanf(f, " ");
        
    } 

    *entry = (funcp) funcoes[numFuncoes-1];
    *code = (void*) codigo;
}

void prologo(unsigned char *codigo, int *pos, unsigned int *funcoes, int *numFuncoes){
    //push  %ebp
    //movl  %esp, %ebp
    //subl   $40, %esp aloca espaço para 10 variaveis locais
    
    funcoes[(*numFuncoes)++] = (unsigned int)&codigo[(*pos)];
    
    codigo[(*pos)++] = 0x55; 
    codigo[(*pos)++] = 0x89;
    codigo[(*pos)++] = 0xe5;
    codigo[(*pos)++] = 0x83;
    codigo[(*pos)++] = 0xec;
    codigo[(*pos)++] = 0x28;

}

void call(int numFuncao, char tipoRec, int numRec, char tipoParm, int numParm, unsigned int *funcoes, unsigned char *codigo, int *pos){
    //push <parametro>
    //call <function>
    //add $4, %esp
    //mov %eax, n(%ebp)
    int *difEndereco; // Arruma a diferença dos endereços para dar call

    if(tipoParm == '$'){
        codigo[(*pos)++] = 0x68; // push const
        *((int *)(&codigo[*pos])) = numParm;
        (*pos) +=4;
    }
    else {
        codigo[(*pos)++] = 0xff; //push v or p
        codigo[(*pos)++] = 0x75;
        deslocamentoEbp(codigo,pos,tipoParm,numParm);
    }
    
    codigo[(*pos)++] = 0xe8; //call
    
    // Ponteiro aponta para onde o código "parou"
    difEndereco = (int*)&codigo[(*pos)];

    (*pos) += 4;
  
    *difEndereco = funcoes[numFuncao] - (unsigned int) &codigo[(*pos)];

    // add 4 to esp
    codigo[(*pos)++] = 0x83;
    codigo[(*pos)++] = 0xc4;
    codigo[(*pos)++] = 0x04;

    
    //movl %eax, V ou P
    codigo[(*pos)++] = 0x89;
    codigo[(*pos)++] = 0x45;
    deslocamentoEbp(codigo,pos,tipoRec,numRec);
    
}

void operacao(char tipoRec, int numRec, 
              char tipoOperando1, int numOperando1, 
              char operador, 
              char tipoOperando2, int numOperando2, 
              unsigned char *codigo, int *pos){
    /*---------------------------------------------
    Por exemplo:
    Se v0 = v1 + p1, 
        addl v1 com p1 ---> p1 = p1 + v1 ou
        addl p1 com v1 ---> v1 = v1 + p1

    PASSOS:
    Mover <operando1> para um registrador
    Somar o <operando2> ao registrador
    Depois fazer movl do registrador para v0.
    ---------------------------------------------*/
    
    // movl <operando1>, %eax
    if(tipoOperando1 == '$') {
        codigo[(*pos)++] = 0xb8;
        *((int *)(&codigo[*pos])) = numOperando1;
        (*pos) += 4; 
    }
    else {
        codigo[(*pos)++] = 0x8b;
        codigo[(*pos)++] = 0x45;
        
       deslocamentoEbp(codigo,pos,tipoOperando1,numOperando1);
    }
    
    // <operacao> <operando2>, %eax
    if(operador == '+') {
        if(tipoOperando2 == '$'){
            codigo[(*pos)++] = 0x05;
            *((int *)(&codigo[*pos])) = numOperando2;
            (*pos) += 4;
        }
        else{
            codigo[(*pos)++] =  0x03;
            codigo[(*pos)++] =  0x45;
            
        deslocamentoEbp(codigo,pos,tipoOperando2,numOperando2);
        }
    }
    
    // Se operador '-', subl
    else if(operador == '-') {
        if(tipoOperando2 == '$'){
            codigo[(*pos)++] = 0x2d;
            *((int *)(&codigo[*pos])) = numOperando2;
            (*pos) += 4;
        }
        else{
            codigo[(*pos)++] =  0x2b;
            codigo[(*pos)++] =  0x45;
            
        deslocamentoEbp(codigo,pos,tipoOperando2,numOperando2);
        }
    }
    
    // Se operador '*', imul
    else {
        if(tipoOperando2 == '$'){
            codigo[(*pos)++] = 0x69;
            codigo[(*pos)++] = 0xc0;
            *((int *)(&codigo[*pos])) = numOperando2;
            (*pos) += 4;
        }
        else{
            codigo[(*pos)++] =  0x0f;
            codigo[(*pos)++] =  0xaf;
            codigo[(*pos)++] =  0x45;
            
        deslocamentoEbp(codigo,pos,tipoOperando2,numOperando2);
        }
    }
    
    // movl %eax, <receptor>
    codigo[(*pos)++] = 0x89;
    codigo[(*pos)++] = 0x45;
    
    deslocamentoEbp(codigo, pos, tipoRec, numRec);
}

void retorno(char tipoRetorno, int retorno, char tipoParm, int numParm, unsigned char *codigo, int *pos){
    // Se ele não ler $0, a função não retorna
    if(tipoRetorno == '$' && retorno != 0)
        return;

    // Se ret? é seguido de v dígito ou p dígito
    else if(tipoRetorno == 'v' || tipoRetorno == 'p'){
        // cmpl $0, [-]4n(%ebp)
        codigo[(*pos)++] = 0x83;
        codigo[(*pos)++] = 0x7d;
        
        deslocamentoEbp(codigo,pos,tipoRetorno,retorno);

        codigo[(*pos)++] = 0x00;
    }
    
    // Se estivermos retornando um número
    if(tipoParm == '$') {
        if(tipoRetorno != '$' ) { //insere o JNE, exceto no caso de ret? $0 X
        // pula 5 bytes + finalizacao (9)
         codigo[(*pos)++] = 0x75;
         codigo[(*pos)++] = 0x09;
        }
        //movl <número>, %eax
        codigo[(*pos)++] = 0xb8; 
        *((int *)(&codigo[*pos])) = numParm;
        
        (*pos) += 4; 
        finalizacao(codigo, pos);
    }
    else { 
        if(tipoRetorno != '$'){ //insere o JNE, exceto no caso de ret? $0 X
        // pula 3 bytes + finalizacao (7)
         codigo[(*pos)++] = 0x75;
         codigo[(*pos)++] = 0x07;
        }
        //movl <var local = -4n(%ebp)>, %eax
        codigo[(*pos)++] = 0x8b;
        codigo[(*pos)++] = 0x45;
        
        deslocamentoEbp(codigo,pos,tipoParm,numParm);
            
        finalizacao(codigo, pos);
    }
}

void finalizacao(unsigned char *codigo, int *pos){
    //movl  %ebp, %esp
    //pop   %ebp
    //ret
    
    codigo[(*pos)++] = 0x89; 
    codigo[(*pos)++] = 0xec;
    codigo[(*pos)++] = 0x5d;
    codigo[(*pos)++] = 0xc3;
}

void deslocamentoEbp(unsigned char *codigo, int *pos, char tipo, int numTipo){
    if(tipo == 'v')
        codigo[(*pos)++] = -4*(numTipo + 1);  
    else
        codigo[(*pos)++] = 4*(numTipo + 2);
}

void libera(void *code){
    free(code);
}