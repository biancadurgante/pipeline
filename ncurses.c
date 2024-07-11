#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

typedef enum { tipo_R = 0, tipo_I = 1, tipo_J = 2 } Tipo_inst;

typedef struct {
    Tipo_inst tipo_inst;
    char inst_char[18];
    int opcode;
    int rs;
    int rt;
    int rd;
    int funct;
    int imm;
    int addr;
} mem;

typedef struct No {
    int reg_backup[8];
    int pc_backup;
    int dados_backup[256];
    int clock;
    int indice;
    struct No *prox;
    struct No *ant;
} No;

typedef struct _backup {
    No *primeiro;
    No *ultimo;
    int tamanho;
} Backup;

int binario_para_decimal_c2(const char *binario, int bits);
void preencher_memoria_inst(FILE *file_mem, mem *memoria, int linhas_mem);
char *busca(mem *memoria, int pc);
int binario_para_decimal(const char *binario);
int ula(mem *memoria, int *registradores, int pc, int *flag);
void overflow(mem memoria, int *registradores, int *flag);
void decodificacao(mem *memoria_inst, char *instrucao_buscada, int pc);
int controle(mem *instrucoes, int linhas_mem, int *registradores, int *pc, int *flag);
int acessa_memoria(mem *instrucoes, int pc, int escrita, int *registradores, int *dados, int *ALUOutToReg, int ALUOut);
void imprimeregistradores(int *registradores);
void imprimeMemoriaDados(int *dados);
void imprime_decodificacao(mem *memoria_inst, int i);
void salva_estado_memoria(int *dados);
void salva_asm(mem *memoria_inst, int j);
void imprimeInstrucoes(mem *memoria_inst, int linhas_mem);
void imprime_asm(mem *memoria_inst, int j);
void decimalParaBinario(int valor, mem *memoria, int indice);
No *criaNo(int *registradores, int pc, mem *memoria, int clock, int indice);
void insereNo(Backup *backup, No *no);
Backup *criaBackup();
void restauraUltimoBackup(Backup *backup, int *registradores, int *pc, mem *memoria, int *clock, int *indice);
void carrega_memoria_dados(int *dados);
void finaliza_instrucao(int *registradores, mem *instrucoes, int ALUOutToReg, int MEM, int indice, int *pc);
void reiniciar_simulador(int *registradores, mem *memoria_inst, int *dados, int *pc, int *clock, int *pipelineE1, int *pipelineE2, int *pipelineE3, int *pipelineE4, int *pipelineE5, int *linhas_mem);

int main() {
    int registradores[8] = {0};
    mem memoria_inst[256];
    int dados[256] = {0};
    FILE *file_mem;
    int linhas_mem = 0;
    char barraN;
    int menu;
    int pc = 0; // Inicializa pc com zero
    int flag = 0;
    char instrucao_buscada[18];
    int ALUOut = 0;
    int clock = 0; // Inicia em 0 e sofre o primeiro incremento quando iniciamos o simulador
    int n_inst = 0;
    int MEM = 0;
    int ALUOutToReg;
    int pipelineE1 = 0, pipelineE2 = 0, pipelineE3 = 0, pipelineE4 = 0, pipelineE5 = 0; // A ideia é que cada clock tenha a sua própria variável para guardar o pc (índice)

    initscr(); // Inicia a biblioteca ncurses
    cbreak(); // Desabilita buffer de linha
    noecho(); // Não mostra os caracteres digitados
    keypad(stdscr, TRUE); // Habilita o uso de teclas de função

    // Define cores para a interface
    start_color();
    init_pair(1, COLOR_CYAN, COLOR_BLACK); // Para o menu
    init_pair(2, COLOR_YELLOW, COLOR_BLACK); // Para os cantos inferiores

    // Loop principal do programa
    do {
        clear();
        int yMax, xMax;
        getmaxyx(stdscr, yMax, xMax);
        int boxHeight = 2;
        // Desenha a box no centro da tela
        int boxWidth = 2;
        int boxStartY = (yMax - boxHeight) / 5;
        int boxStartX = (xMax - boxWidth) / 5;
        box(stdscr, 0, 0);
        // Imprime o menu fixo no topo
        attron(COLOR_PAIR(2));
        mvprintw(1, (xMax - 72) / 2, "================================================================================");
        mvprintw(2, (xMax - 72) / 2, "                      ＭＩＮＩ ＭＩＰＳ ＰＩＰＥＬＩＮＥ              ");
        mvprintw(3, (xMax - 72) / 2, "|||||                                                                      |||||");
        mvprintw(4, (xMax - 72) / 2, "|||||           1 - I N I C I A R    O    S I M U L A D O R                |||||");
        mvprintw(5, (xMax - 72) / 2, "|||||                        2 - C L O C K                                 |||||");
        mvprintw(6, (xMax - 72) / 2, "||||| 3 - I M P R I M I R    A    M E M O R I A   D E   I N S T R U C O E S|||||");
        mvprintw(7, (xMax - 72) / 2, "|||||                    4 - R E I N I C I A R                             |||||");
        mvprintw(8, (xMax - 72) / 2, "|||||                         0 - S A I R                                  |||||");
        mvprintw(9, (xMax - 72) / 2, "|||||                                                                      |||||");        
        mvprintw(10, (xMax - 72) / 2, "================================================================================");
        attroff(COLOR_PAIR(2));
// Calcula as coordenadas para o centro da tela
int yCenter = yMax / 2;
int xCenter = xMax / 2;

// Define as dimensões do retângulo
boxWidth = 20;
boxHeight = 10;
 boxStartY = yCenter - boxHeight / 2;
 boxStartX = xCenter - boxWidth / 2;

// Desenha o retângulo
attron(COLOR_PAIR(2));  // Use a cor definida para os cantos inferiores
for (int i = 0; i < 3; i++) {
    mvprintw(boxStartY + i, boxStartX - 20, "==================================================================");
}
for (int i = 0; i < 3; i++) {
    mvprintw(boxStartY + i+8, boxStartX - 20, "==================================================================");
}
attroff(COLOR_PAIR(2));

	attron(COLOR_PAIR(1));
	for(int x = 2; x<24; x++){
		mvprintw(x,1, "#######################################");
	}
	for(int y = 49; y<51; y++){
		mvprintw(y,1, "#######################################");
	}
	attron(COLOR_PAIR(1));
	for(int x = 2; x<24; x++){
		mvprintw(x,170, "#######################################");
	}
		for(int y = 49; y<51; y++){
		mvprintw(y,170, "#######################################");
	}
		mvprintw(48,81, "PROJETO INTEGRADOR II - PROJETO 3 MINI MIPS PIPELINE");
		mvprintw(49,97, "COM INTERFACE GRAFICA");

 	attroff(COLOR_PAIR(1));
        // Imprime os registradores no canto inferior esquerdo
        attron(COLOR_PAIR(2));
        mvprintw(yMax - 17, 1, "===== ＲＥＧＩＳＴＲＡＤＯＲＥＳ ==");
        for (int i = 0; i < 8; i++) {
            mvprintw(yMax - 16 + i, 11, "registrador [%d]: %d", i, registradores[i]);
        }
        mvprintw(yMax - 8, 1, "===================================");
        attroff(COLOR_PAIR(2));

        // Imprime a memória de dados no canto inferior direito
        attron(COLOR_PAIR(2));
        mvprintw(yMax - 21, xMax - 48, "======== M E M O R I A  D E  D A D O S ========");
        int linha, coluna;
        for (linha = 0; linha < 16; linha++) {
            for (coluna = 0; coluna < 16; coluna++) {
                int indice = linha * 16 + coluna;
                mvprintw(yMax - 20 + linha, xMax - 48 + coluna * 3, " %-6i", dados[indice]);
            }
        }
        mvprintw(yMax - 5, xMax - 48, "===============================================");
        attroff(COLOR_PAIR(2));

        // Atualiza a tela
        refresh();

        // Lê a entrada do usuário
        menu = getch() - '0';

        switch (menu) {
            case 1:
                if ((file_mem = fopen("multiplicacao.mem", "r")) == NULL) {
                    mvprintw(LINES / 2, (COLS - 32) / 2, "Erro: memoria nao foi carregada.\n");
                    getch();
                    endwin();
                    return 1;
                }
                mvprintw(LINES / 2, (COLS - 32) / 2, "ＳＩＭＵＬＡＤＯＲ ＩＮＩＣＩＡＤＯ\n");
                while ((barraN = fgetc(file_mem)) != EOF) {
                    if (barraN == '\n')
                        linhas_mem++;
                }

                rewind(file_mem);
                preencher_memoria_inst(file_mem, memoria_inst, linhas_mem);
                fclose(file_mem);
                carrega_memoria_dados(dados);
                clock++;
                break;

            case 2:
                if (pipelineE5 < linhas_mem) {
                    if (pc >= 4) {
                        mvprintw((LINES / 2) - 2, ((COLS - 20) /  2) - 4, "ESTAGIO 5 -> [PC = %i] ", pipelineE5);
                        imprime_decodificacao(memoria_inst, pipelineE5);
                        finaliza_instrucao(registradores, memoria_inst, ALUOutToReg, MEM, pipelineE5, &pc);
                        pipelineE5++;
                    }
                    if (pc >= 3 && pipelineE4 < linhas_mem) {
                        mvprintw((LINES / 2) - 1, ((COLS - 20) /  2) - 4, "ESTAGIO 4 -> [PC = %i] ", pipelineE4);
                        imprime_decodificacao(memoria_inst, pipelineE4);
                        MEM = acessa_memoria(memoria_inst, pipelineE4, ALUOut, registradores, dados, &ALUOutToReg, ALUOut);
                        pipelineE4++;
                    }
                    if (pc >= 2 && pipelineE3 < linhas_mem) {
                        mvprintw(LINES / 2, ((COLS - 20) /  2) - 4, "ESTAGIO 3 -> [PC = %i] ", pipelineE3);
                        imprime_decodificacao(memoria_inst, pipelineE3);
                        ALUOut = ula(memoria_inst, registradores, pipelineE3, &flag);
                        pipelineE3++;
                    }
                    if (pc >= 1 && pipelineE2 < linhas_mem) {
                        mvprintw((LINES / 2) + 1, ((COLS - 20) /  2) - 4, "ESTAGIO 2 -> [PC = %i] ", pipelineE2);
                        decodificacao(memoria_inst, instrucao_buscada, pipelineE2);
                        imprime_decodificacao(memoria_inst, pipelineE2);
                        pipelineE2++;
                    }

                    if (pipelineE1 < linhas_mem) {
                        strcpy(instrucao_buscada, busca(memoria_inst, pipelineE1));
                        mvprintw((LINES / 2) + 2, ((COLS - 20) /  2) - 4, "ESTAGIO 1 -> [PC = %i]\n", pipelineE1);
                        pc++;
                        pipelineE1 = pc;
                    }
                } else {
                    mvprintw(LINES / 2, ((COLS - 20) /  2) - 4, "Nao ha mais instrucoes a serem buscadas!\n");
                }
                break;

            case 3:
            imprimeInstrucoes(memoria_inst, linhas_mem);
                //imprimeMemoriaDados(dados);
                break;
            case 4:
                // Reiniciar o simulador
                reiniciar_simulador(registradores, memoria_inst, dados, &pc, &clock, &pipelineE1, &pipelineE2, &pipelineE3, &pipelineE4, &pipelineE5, &linhas_mem);
                mvprintw(yMax / 2, ((xMax - 32) / 2)+9, "Simulador reiniciado.");
                getch(); // Aguarda o usuário pressionar uma tecla
                break;

            case 0:
                mvprintw(LINES / 2, ((COLS - 32) / 2)+13, "fechando...\n");
                break;

            default:
                mvprintw(LINES / 2, ((COLS - 32) / 2) + 10, "Opcao invalida!\n");
        }

        refresh();

        // Aguarda pressionar qualquer tecla para continuar
        getch();

    } while (menu != 0);

    endwin(); // Finaliza a biblioteca ncurses

    return 0;
}

void reiniciar_simulador(int *registradores, mem *memoria_inst, int *dados, int *pc, int *clock, int *pipelineE1, int *pipelineE2, int *pipelineE3, int *pipelineE4, int *pipelineE5, int *linhas_mem) {
    // Zerar registradores
    memset(registradores, 0, sizeof(int) * 8);

    // Zerar memória de instruções (opcionalmente, você pode recarregar as instruções se estiverem em um arquivo)
    memset(memoria_inst, 0, sizeof(mem) * 256);

    // Limpar memória de dados
    memset(dados, 0, sizeof(int) * 256);

    // Resetar contadores e variáveis de estado
    *pc = 0;
    *clock = 0;
    *pipelineE1 = 0;
    *pipelineE2 = 0;
    *pipelineE3 = 0;
    *pipelineE4 = 0;
    *pipelineE5 = 0;
    *linhas_mem = 0;

    // Outras inicializações necessárias
}

void imprimeregistradores(int *registradores) {
    int yMax, xMax;
    getmaxyx(stdscr, yMax, xMax);

    // Define a posição para imprimir no canto inferior esquerdo
    int startY = yMax - 11; // Início da impressão, ajustado para caber 8 registradores + cabeçalho e rodapé
    int startX = 1; // Começa da borda esquerda da tela

    // Imprime o cabeçalho
    mvprintw(startY, startX, "========== REGISTRADORES ==========\n");

    // Imprime cada registrador
    for (int i = 0; i < 8; i++) {
        mvprintw(startY + i + 1, startX + 10, "registradore[%d]: %d\n", i, registradores[i]);
    }

    // Rodapé
    mvprintw(startY + 9, startX, "===================================\n");
}

void imprimeMemoriaDados(int *dados) {
    clear();
    int yMax, xMax;
    getmaxyx(stdscr, yMax, xMax);

    // Define a posição para imprimir no canto inferior direito
    int startY = yMax - 18; // Início da impressão, ajustado para caber 16 dados + cabeçalho e rodapé
    int startX = xMax - 48; // Começa da borda direita da tela, ajustado para 48 caracteres de largura

    // Imprime o cabeçalho
    mvprintw(startY, startX, "============== MEMORIA DE DADOS ===============\n");

    // Imprime os dados
    int linha, coluna;
    for (linha = 0; linha < 16; linha++) {
        for (coluna = 0; coluna < 16; coluna++) {
            int indice = linha * 16 + coluna;
            mvprintw(startY + linha + 1, startX + coluna * 3, " %-6i", dados[indice]);
        }
        printw("\n");
    }

    // Rodapé
    mvprintw(startY + 17, startX, "===============================================\n");

    refresh(); // Atualiza a tela para mostrar o texto impresso
}

void carrega_memoria_dados(int *dados)
{
FILE *arquivo;
    int i;
    char aux[16];
    if ((arquivo = fopen("memoriadados.data", "r")) == NULL) {
        //printf("\nErro na abertura do arquivo da memoria de dados");
        return;
    }
    for (i = 0; i < 256; i++) {
        fscanf(arquivo, "%s", aux);
        dados[i]=atoi(aux);
    }
    fclose(arquivo);
}

void finaliza_instrucao(int *registradores, mem *instrucoes, int ALUOutToReg, int MEM, int indice, int *pc)
{
	switch (instrucoes[indice].opcode) 
		{
            case 0://Instrução aritmética -> tipo_R
                registradores[instrucoes[indice].rd] = ALUOutToReg;
                break;
            case 4: //Addi -> tipo_I
                registradores[instrucoes[indice].rt] = ALUOutToReg;
                break;
            case 15://sw -> tipo_I
                break;
            case 11://lw -> tipo_I
                registradores[instrucoes[indice].rt] = MEM;
                break;
            case 8://beq -> tipo_I
                *pc = ALUOutToReg;
                break;
            case 2://j -> tipo_J
		*pc = ALUOutToReg;
                break;
        }
}


void preencher_memoria_inst(FILE *file_mem, mem *memoria, int linhas_mem) {
    for (int i = 0; i < linhas_mem; i++)
        {
            fgets(memoria[i].inst_char, sizeof(memoria[i].inst_char), file_mem);
        }
}        

int binario_para_decimal_c2(const char *binario, int bits) {
    int decimal = 0;
    int sinal = 0;


    if (binario[0] == '1') {
        sinal = 1;
    }

    for (int i = 0; i < bits; i++) {
        decimal = decimal * 2 + (binario[i] - '0');
    }


    if (sinal) {
        decimal -= (1 << bits);
    }

    return decimal;
}


char *busca(mem *memoria, int pc)
{
    return memoria[pc].inst_char;
}

int ula(mem *memoria, int *registradores, int pc, int *flag){
	
		switch(memoria[pc].opcode)
		{
			case 0:		
				switch(memoria[pc].funct)
				{
					case 0: // add
						overflow(memoria[pc], registradores, flag);
						return registradores[memoria[pc].rs] + registradores[memoria[pc].rt];
						break;
					case 2: // sub
						overflow(memoria[pc], registradores, flag);
						return registradores[memoria[pc].rs] - registradores[memoria[pc].rt];
						break;
					case 4: // and
						overflow(memoria[pc], registradores, flag);
						return registradores[memoria[pc].rs] & registradores[memoria[pc].rt];
						break;
					case 5: // or
						overflow(memoria[pc], registradores, flag);
						return registradores[memoria[pc].rs] + registradores[memoria[pc].rt];
						break;
				}
				break;
			case 4://addi
				overflow(memoria[pc], registradores, flag);
				return registradores[memoria[pc].rs] + memoria[pc].imm;
				break;
			case 8://beq
			    printf("\nWARNING! Neste simulador, instruções de desvio podem não fornecer o resultado esperado.\n\n");  //HEDUARDO, se der coloca algo chamativo em vermelho com o ncurses :D
				if (registradores[memoria[pc].rs] == registradores[memoria[pc].rt]){
                    	return (pc + 1) + memoria[pc].imm;    			
				break;
			case 2://jump
			      printf("\nWARNING! Neste simulador, instruções de desvio podem não fornecer o resultado esperado.\n\n"); //HEDUARDO, se der coloca algo chamativo em vermelho com o ncurses :D
				  return memoria[pc].addr;
				break;
			case 11:
				return registradores[memoria[pc].rs] + memoria[pc].imm;
				break;
			case 15:
				return registradores[memoria[pc].rs] + memoria[pc].imm;
				break;
		}
	}

}

void overflow(mem memoria, int *registradores, int *flag){
 
    if (memoria.imm < -32 || memoria.imm > 31 || registradores[memoria.rt] < -128 || registradores[memoria.rt] > 127){
        printf("\nImpossível realizar operação! Overflow!");
        registradores[memoria.rt]=0;
        *flag=1;
    }
    else{
         if (registradores[memoria.rd] < -128 || registradores[memoria.rd] > 127){
        registradores[memoria.rd]=0;
        printf("\nImpossível realizar operação! Overflow!");
        *flag=1;
        }

    }
}

int binario_para_decimal(const char *binario) {
    int decimal = 0;
    while (*binario != '\0') {
        decimal = decimal * 2 + (*binario - '0');
        binario++;
    }
    return decimal;
}


void decodificacao(mem *memoria_inst, char *instrucao_buscada, int pc) {
    char opcode_temp[6], rs_temp[4], rt_temp[4], rd_temp[4], funct_temp[4], imm_temp[7], addr_temp[8];
   
    strncpy(opcode_temp, instrucao_buscada, 4);
    opcode_temp[4] = '\0';
    memoria_inst[pc].opcode = binario_para_decimal(opcode_temp);

    if (memoria_inst[pc].opcode == 0)
        memoria_inst[pc].tipo_inst = tipo_R;
    else if (memoria_inst[pc].opcode == 4 || memoria_inst[pc].opcode == 11 || memoria_inst[pc].opcode == 15 ||
             memoria_inst[pc].opcode == 8)
        memoria_inst[pc].tipo_inst = tipo_I;
    else if (memoria_inst[pc].opcode == 2)
        memoria_inst[pc].tipo_inst = tipo_J;

    switch (memoria_inst[pc].tipo_inst) {
        case tipo_R:
            strncpy(rs_temp, instrucao_buscada + 4, 3);
            rs_temp[3] = '\0';
            strncpy(rt_temp, instrucao_buscada + 7, 3);
            rt_temp[3] = '\0';
            strncpy(rd_temp, instrucao_buscada + 10, 3);
            rd_temp[3] = '\0';
            strncpy(funct_temp, instrucao_buscada + 13, 3);
            funct_temp[3] = '\0';
            memoria_inst[pc].rs = binario_para_decimal(rs_temp);
            memoria_inst[pc].rt = binario_para_decimal(rt_temp);
            memoria_inst[pc].rd = binario_para_decimal(rd_temp);
            memoria_inst[pc].funct = binario_para_decimal(funct_temp);
            break;

        case tipo_I:
            strncpy(rs_temp, instrucao_buscada + 4, 3);
            rs_temp[3] = '\0';
            strncpy(rt_temp, instrucao_buscada + 7, 3);
            rt_temp[3] = '\0';
            strncpy(imm_temp, instrucao_buscada + 10, 6);
            imm_temp[6] = '\0';
            memoria_inst[pc].rs = binario_para_decimal(rs_temp);
            memoria_inst[pc].rt = binario_para_decimal(rt_temp);
            memoria_inst[pc].imm = binario_para_decimal_c2(imm_temp, 6);
            break;

        case tipo_J:
            strncpy(addr_temp, instrucao_buscada + 9, 7);
            addr_temp[7] = '\0';
            memoria_inst[pc].addr = binario_para_decimal_c2(addr_temp, 7);
            break;
    }
    
}


int acessa_memoria(mem *instrucoes, int pc, int escrita, int *registradores, int *dados, int *ALUOutToReg, int ALUOut)
{
    switch(instrucoes[pc].opcode)
        {
            case 0://Instrução aritmética -> tipo_R
                *ALUOutToReg = ALUOut;
                break;
            case 4: //Addi -> tipo_I
                *ALUOutToReg = ALUOut;
                break;
            case 8://beq -> tipo_I
                *ALUOutToReg = ALUOut;
                break;
            case 2://j -> tipo_J
				*ALUOutToReg = ALUOut;
                break;
            case 15: // se for sw transfere pra respectiva posicao da memória;
                registradores[escrita] = dados[instrucoes[pc].rt];
                break;
            case 11: // se for lw transfere pro rt
                return dados[escrita];
                break;
        }
        
}







void imprime_decodificacao(mem *memoria_inst, int i) {
    switch (memoria_inst[i].tipo_inst) {
        case tipo_R:
            switch (memoria_inst[i].funct) {
                case 0:
                    printw("add $%i, $%i, $%i\n", memoria_inst[i].rd, memoria_inst[i].rs, memoria_inst[i].rt);
                    break;
                case 2:
                    printw("sub $%i, $%i, $%i\n", memoria_inst[i].rd, memoria_inst[i].rs, memoria_inst[i].rt);
                    break;
                case 4:
                    printw("and $%i, $%i, $%i\n", memoria_inst[i].rd, memoria_inst[i].rs, memoria_inst[i].rt);
                    break;
                case 5:
                    printw("or $%i, $%i, $%i\n", memoria_inst[i].rd, memoria_inst[i].rs, memoria_inst[i].rt);
                    break;
            }
            break;
        case tipo_I:
            switch (memoria_inst[i].opcode) {
                case 4:
                    printw("addi $%i, $%i, %i\n", memoria_inst[i].rs, memoria_inst[i].rt, memoria_inst[i].imm);
                    break;
                case 11:
                    printw("lw $%i, %i($%i)\n", memoria_inst[i].rt, memoria_inst[i].imm, memoria_inst[i].rs);
                    break;
                case 15:
                    printw("sw $%i, %i($%i)\n", memoria_inst[i].rt, memoria_inst[i].imm, memoria_inst[i].rs);
                    break;
                case 8:
                    printw("beq $%i, $%i, %i\n", memoria_inst[i].rs, memoria_inst[i].rt, memoria_inst[i].imm);
                    break;
            }
            break;
        case tipo_J:
            printw("J %i\n", memoria_inst[i].addr);
            break;
    }
    refresh(); // Atualiza a tela para mostrar o texto impresso
}

void imprimeInstrucoes(mem *memoria_inst, int linhas_mem) {
    clear();
    int yMax, xMax;
    getmaxyx(stdscr, yMax, xMax);

    // Calcula a posição para centralizar a impressão das instruções
    int startY = (yMax - linhas_mem - 2) / 2; // Centraliza verticalmente, considerando linhas_mem + 2 linhas de cabeçalho e rodapé
    int startX = (xMax - 20) / 2; // Centraliza horizontalmente, considerando 20 caracteres de largura total

    mvprintw(startY, startX, "==== ＩＮＳＴＲＵＣＯＥＳ ====\n");

    int linha;
    for (linha = 0; linha < linhas_mem; linha++) {
        if (strlen(memoria_inst[linha].inst_char) > 10) {
            mvprintw(startY + linha + 1, startX + 7, "%s\n", memoria_inst[linha].inst_char);
        }
    }

    mvprintw(startY + linhas_mem + 1, startX, "===============================\n");
    refresh(); // Atualiza a tela para mostrar o texto impresso
}

void salva_estado_memoria(int *dados) {
    FILE *arquivo;
    int i;
    if ((arquivo = fopen("memoriadados.data", "w")) == NULL) {
        //printf("Erro na abertura do arquivo");
        return;
    }
    for (i = 0; i < 256; i++) {
		fprintf(arquivo,"%i\n", dados[i]);                
    }
    fclose(arquivo);
}

void imprime_asm(mem *memoria_inst, int j) {
    printf("INSTRUCOES .ASM:\n");
	for (int i = 0; i < j; i++) {
	    if(strlen(memoria_inst[i].inst_char) > 10)
	    {
            switch (memoria_inst[i].tipo_inst) {
                case tipo_R:
                    switch (memoria_inst[i].funct) {
                        case 0:
                            printf("add $%i, $%i, $%i\n", memoria_inst[i].rd, memoria_inst[i].rs, memoria_inst[i].rt);
                            break;
                        case 2:
                            printf("sub $%i, $%i, $%i\n", memoria_inst[i].rd, memoria_inst[i].rs, memoria_inst[i].rt);
                            break;
                        case 4:
                            printf("and $%i, $%i, $%i\n", memoria_inst[i].rd, memoria_inst[i].rs, memoria_inst[i].rt);
                            break;
                        case 5:
                            printf("or $%i, $%i, $%i\n", memoria_inst[i].rd, memoria_inst[i].rs, memoria_inst[i].rt);
                            break;
                    }
                    break;
                case tipo_I:
                    switch (memoria_inst[i].opcode) {
                        case 4:
                            printf("addi $%i, $%i, %i\n", memoria_inst[i].rs, memoria_inst[i].rt, memoria_inst[i].imm);
                            break;
                        case 11:
                            printf("lw $%i, %i($%i)\n", memoria_inst[i].rt, memoria_inst[i].imm, memoria_inst[i].rs);
                            break;
                        case 15:
                            printf("sw $%i, %i($%i)\n", memoria_inst[i].rt, memoria_inst[i].imm, memoria_inst[i].rs);
                            break;
                        case 8:
                            printf("beq $%i, $%i, %i\n", memoria_inst[i].rs, memoria_inst[i].rt, memoria_inst[i].imm);
                            break;
                    }
                    break;
                case tipo_J:
                    printf("J %i\n", memoria_inst[i].addr);
                    break;
            }
	    }
    }
}


void salva_asm(mem *memoria_inst, int j) {
    FILE *arquivo;
    if ((arquivo = fopen("ProgramaAssembly.asm", "w")) == NULL) {
        printf("Erro na abertura do arquivo");
        return;
    }
    for (int i = 0; i < j; i++) {
        if(strlen(memoria_inst[i].inst_char) > 10)
	    {
            switch (memoria_inst[i].tipo_inst) {
                case tipo_R:
                    switch (memoria_inst[i].funct) {
                        case 0:
                            fprintf(arquivo, "add $%i, $%i, $%i\n", memoria_inst[i].rd, memoria_inst[i].rs, memoria_inst[i].rt);
                            break;
                        case 2:
                            fprintf(arquivo, "sub $%i, $%i, $%i\n", memoria_inst[i].rd, memoria_inst[i].rs, memoria_inst[i].rt);
                            break;
                        case 4:
                            fprintf(arquivo, "and $%i, $%i, $%i\n", memoria_inst[i].rd, memoria_inst[i].rs, memoria_inst[i].rt);
                            break;
                        case 5:
                            fprintf(arquivo, "or $%i, $%i, $%i\n", memoria_inst[i].rd, memoria_inst[i].rs, memoria_inst[i].rt);
                            break;
                    }
                    break;
                case tipo_I:
                    switch (memoria_inst[i].opcode) {
                        case 4:
                            fprintf(arquivo, "addi $%i, $%i, %i\n", memoria_inst[i].rs, memoria_inst[i].rt, memoria_inst[i].imm);
                            break;
                        case 11:
                            fprintf(arquivo, "lw $%i, %i($%i)\n", memoria_inst[i].rt, memoria_inst[i].imm, memoria_inst[i].rs);
                            break;
                        case 15:
                            fprintf(arquivo, "sw $%i, %i($%i)\n", memoria_inst[i].rt, memoria_inst[i].imm, memoria_inst[i].rs);
                            break;
                        case 8:
                            fprintf(arquivo, "beq $%i, $%i, %i\n", memoria_inst[i].rs, memoria_inst[i].rt, memoria_inst[i].imm);
                            break;
                    }
                    break;
                case tipo_J:
                    fprintf(arquivo, "J %i\n", memoria_inst[i].addr);
                    break;
            
            }
        }
    }
    fclose(arquivo);
}



