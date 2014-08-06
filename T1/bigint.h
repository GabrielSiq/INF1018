#define NUM_BYTES 16	
typedef unsigned char BigInt[NUM_BYTES];

/* Atribuicao */

/* res = val (extensao com sinal) */
void big_val (BigInt res, int val);

/* res = uval (extensao sem sinal) */
void big_uval (BigInt res, unsigned int uval);

/* Operacoes aritmeticas */

/* res = a + b */
void big_sum (BigInt res, BigInt a, BigInt b);

/* res = a - b */
void big_sub (BigInt res, BigInt a, BigInt b);

/* res = a * b (com sinal) */
void big_mul (BigInt res, BigInt a, BigInt b);

/* res = a * b (sem sinal) */
void big_umul (BigInt res, BigInt a, BigInt b);

/* Operacoes de deslocamento */

/* res = a << n */
void big_shl (BigInt res, BigInt a, int n);

/* res = a >> n (logico) */
void big_shr (BigInt res, BigInt a, int n);

/* Comparacao: retorna -1 (a < b), 0 (a == b), 1 (a > b) */

/* comparacao com sinal */
int big_cmp(BigInt a, BigInt b);

/* comparacao sem sinal */
int big_ucmp(BigInt a, BigInt b);

//experimental

void big_pow(BigInt res, BigInt a, BigInt b);