/* Gabriel da Silva Siqueira - 1210689 */
#include <stdio.h>
#include "bigint.h"

void big_copy(BigInt res, BigInt a){
	int i;
  	if(res == a) return; 
  	for(i=0;i<NUM_BYTES;i++) // Copia cada byte (unsigned char) de um BigInt para outro
  		res[i]=a[i];
}

void compl2(BigInt res,BigInt a){
	int i;
	BigInt temp;
	for(i=0;i<NUM_BYTES;i++){ //Byte a byte inverte o valor
		res[i]=~a[i]; 
	}
	big_val(temp,1); // Soma um, obtendo o complemento a 2
	big_sum(res,res,temp);	
}


void big_uval (BigInt res, unsigned int uval){
	int i;
	for (i= 0;i<4;i++){
		res[i] = uval>>(8*i) &0xFF; // Copia o equivalente a 1 byte do unsigned int para uma posição do vetor de unsigned char
	}
	for(i=4;i<NUM_BYTES;i++) // Preenche o restante com zeros
		res[i]=0;
}

void big_val (BigInt res, int val){
	int i;
	if (val>=0) // Se for um valor positivo, chama a função uval
		big_uval(res,val);
	else{
		for (i= 0;i<4;i++){
		res[i] = val>>(8*i) &0xFF; // Copia o equivalente a 1 byte do inteiro
		}
		for(i=4;i<NUM_BYTES;i++){ // Preenche o resto com 1 
		res[i]=0xFF;
		}
	}
}

void big_sum (BigInt res, BigInt a, BigInt b){
	int carry=0,j;
	unsigned short soma;
	for (j=0;j<NUM_BYTES;j++){
		soma=a[j]+b[j]+carry; // Soma byte a byte  + o overflow
		carry=soma>>8; // Carry = o byte que "sobra"/mais a esquerda da soma
		res[j]=soma&0xFF; // Pega apenas a parte da soma que "cabe" em 1 byte
	}
}	

void big_sub (BigInt res, BigInt a, BigInt b){
	BigInt minusB; 
	compl2(minusB,b); // Diferença = soma com o negativo
	big_sum(res,a,minusB); // o uso da temporária minusB se dá para não alterar o valor de nenhum dos parâmetros
}

int big_cmp(BigInt a, BigInt b){
	if((a[NUM_BYTES-1]>>7) > (b[NUM_BYTES-1]>>7)) // um negativo e um positivo = resposta trivial
		return -1;
	else if ((a[15]>>7) < (b[15]>>7))
		return 1;
	else 
		return big_ucmp(a,b); // dois negativos ou dois positivos, chama ucmp
} 

int big_ucmp(BigInt a, BigInt b){
	int i;
	for (i = (NUM_BYTES-1); i >=0; i--) // compara bit a bit até achar um maior que o outro
	{
		if(a[i]>b[i])
			return 1;
		else if(a[i]<b[i])
			return -1;
	}
	return 0;
}

void big_shl (BigInt res, BigInt a, int n){
	int i,p,q,r=0;
	unsigned char carry=0;
	BigInt aux;			//uso do auxilar caso queira-se atribuir o shift à propria variavel parâmetro ex: a=a>>n |  big_shl(a,a,n)
	big_uval(aux, 0); 	// Zera a variável auxiliar
	p=n%8;
	q=n/8;
	for(i=0; i<NUM_BYTES && r<(NUM_BYTES-1); i++) // Ao mesmo tempo shifta um número inteiro n/8 de bytes e um número n%8 de bits 
	{
		r=q+i;
		aux[r]=(a[i]<<p)|carry;
		carry=a[i]>>(8-p); // pega os bits do carry shiftando para "lado oposto" 
	}
	big_copy(res,aux);
}

void big_shr (BigInt res, BigInt a, int n){		// mesmo princípio da função anterior, só que pro outro lado
	int i,p,q,r=0;
	unsigned char carry=0;
	BigInt aux;
	big_uval(aux, 0);
	p=n%8;
	q=n/8;
	for(i=(NUM_BYTES-1); i>=q && r>=0; i--)
	{
		r=i-q;
		aux[r]=(a[i]>>p)|carry;
		carry=a[i]<<(8-p);
	}
	big_copy(res,aux);
}


void big_mul(BigInt res, BigInt a, BigInt b){
	big_umul(res,a,b);
}	

void big_umul (BigInt res, BigInt a, BigInt b) {
	int i,j,aux,carry;
	BigInt temp, part_result;
	big_val(temp, 0);				// uso do temporário para que se possa fazer a=a*b e semelhantes
	for(i=0; i<NUM_BYTES; i++) {
		big_val(part_result,0);		// variavel para armazenar os resultados parciais
		carry=0;
		for(j=i; j<NUM_BYTES; j++) { 
			aux=((a[i]*b[j-i])+carry);  // multiplica o i do a com todos do b guarda o resultado em aux
			part_result[j] =aux&0xFF; //  resultado parcial recebe apenas a parte que "cabe" em 1 byte
			carry=aux>>8;                // carry pega o overflow
		}
		big_sum(temp,temp,part_result);      // soma o resultado parcial da multiplicacao de a[i] por b com a os resultados anteriores, a cada iteração
	}
	big_copy(res,temp);
}

// Pode ser feito altravés de um algoritmo de shift + soma que parece ser mais eficiente, mas não cheguei a implementar


// experimental
void big_pow(BigInt res, BigInt a, BigInt b){
	BigInt i,One;
	big_val(One,1);
	big_copy(res,a);
	for (big_val(i,0);big_cmp(i,b) != 0;big_sum(i,i,One)){
		big_umul(res, res, a);
	}
}