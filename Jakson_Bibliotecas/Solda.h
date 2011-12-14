void ConfigurarRampas();



void SoldaEfetiva();
void IniciarSolda();
float CalcularMediaRms();
void ConfiguraRampasKIR();
void ConfiguraRampasSKT();
void GerenciarSolda();
void CompensarTensao();

#define SIM	0x01
#define NAO	0x00



struct _rampa {
	Uint16 total_semiciclos;
	Uint16 cont_semiciclos;	
	Uint16 passo_skt;
	Uint16 skt;
	Uint16 iRef;  //Referencia: valor da corrente KIR
	Uint16 deltaSkt;
	Uint16 estado; //Ativo ou Inativo
};

//Solda Efetiva, com N impulsos e pausas
struct _padrao {
	Uint32 total_semiciclos;
	Uint32 cont_semiciclos;
	Uint16 total_impulsos;
	Uint16 cont_impulsos;
	Uint16 skt;
	Uint16 modo;
	float iRmsMedia[10];
	float iMediaImpulsos;
	Uint16 estado; //Ativo ou Inativo
};

//Pré e Pós Soldas
struct _prepossolda {
	Uint16 total_semiciclos;
	Uint16 cont_semiciclos;
	Uint16 skt;
	Uint16 iRef;	
	Uint16 modo;
	Uint16 estado; //Ativo ou Inativo
};

// Pausas
struct _pausa {
	Uint16 total_semiciclos;
	Uint16 cont_semiciclos;
};


//Estrutura de um ciclo completo de solda
//com rampas, pausas e impulsos
struct _solda {
	struct _rampa subida;
	struct _rampa descida;
	struct _padrao principal;
	struct _prepossolda preSolda;
	struct _prepossolda posSolda;
	struct _pausa pausa1;
	struct _pausa pausa2;
	struct _pausa pausaPrincipal;
	struct _pausa pausa3;
	struct _pausa manutencao;
	struct _pausa pausa4;
	Uint16 soldar;
	Uint16 tipo;	//1-Individual, 2-Série, 3-Costura, 4-Continua
	Uint16 cont;
	float TensaoRmsMediaRef;
	float TensaoRmsMedia;
};

struct _compensacaoTensao {
	float TensaoRmsMedia;
	float RefTensao;
	float fator;	//fator de compensacao
	float Acumulador;
	Uint16 i;
};

extern struct _solda Solda;


