#ifndef COMANDOS_H_
#define COMANDOS_H_


// Constantes utilizados no protocolo de comunicaçao
#define CMD_SOLDAR    0x01
#define CMD_CICLOS    0x02
#define CMD_SKT       0x03
#define CMD_KIR       0x04
#define CMD_TESTE	  0x41
#define CMD_ERRO	  0xFF


#define	TIPO_SOLDA			0x01

#define PRESOLDA_CICLOS		0x02
#define PRESOLDA_SKT		0x03
#define PRESOLDA_KIR		0x04

#define TEMPO_FRIO1_CICLOS	0x05

#define SUBIDA_CICLOS		0x06

#define PRINCIPAL_CICLOS	0x07
#define PRINCIPAL_SKT		0x08
#define PRINCIPAL_KIR		0x09
#define PRINCIPAL_IMPULSOS	0x0A
#define PRINCIPAL_PAUSA		0x0B

#define DESCIDA_CICLOS		0x0C

#define TEMPO_FRIO2_CICLOS  0x0D

#define POSSOLDA_CICLOS		0x0E
#define POSSOLDA_SKT		0x0F
#define POSSOLDA_KIR		0x10

#define TEMPO_FRIO3_CICLOS	0x11

#define MANUTENCAO_CICLOS	0x12

#define PAUSA4_CICLOS		0x13

#define SOLDAR				0x50

#define REPORTAR_ERROS		0x60

#define SELECAO_MUX         0x80




#define SINC		  0x1B
// fim constantes comunicaçao


void comandos_enviar_corrente(float corrente);
void Comandos(char funcao, unsigned long int param);

#endif /*COMANDOS_H_*/
