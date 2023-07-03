#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

#define MAX_COMMANDS 5
#define MAX_ARGS 10
#define MAX_INPUT_LENGTH 100

pid_t foreground_process = 0;
int foreground_pid = 0; // Variável para armazenar o PID do processo de foreground em execução

// Função de tratamento de sinal para os sinais SIGINT, SIGQUIT e SIGTSTP
void signal_handler(int signum)
{
    if (foreground_pid != 0)
    {
        // Se houver um processo de foreground em execução, permita que ele receba o sinal e execute o tratamento default
        signal(signum, SIG_DFL);
        kill(foreground_pid, signum);
    }
    else
    {
        // Se não houver processo de foreground em execução, imprima uma mensagem de aviso
        printf("Não adianta me enviar o sinal por Ctrl-... . Estou vacinado!!\n");
    }
}

void ExecutaComando(char **args, int foreground)
{
    pid_t pid = fork();

    if (pid < 0)
    {
        perror("Erro ao criar processo");
        exit(1);
    }
    else if (pid == 0)
    {
        // Processo filho
        execvp(args[0], args);
        perror("Erro ao executar o comando");
        exit(1);
    }
    else
    {
        // Processo pai
        if (foreground)
        {
            foreground_process = pid;
            waitpid(pid, NULL, 0);
            foreground_process = 0;
        }
        else
        {
            // Não espera pelo processo filho se for executado em segundo plano
            printf("Processo iniciado em segundo plano com PID %d\n", pid);
        }
    }
}
void handle_sigusr1(int signum)
{
    if (foreground_process > 0)
    {
        kill(foreground_process, SIGUSR1);
    }

    exit(0);
}

void TrataEntrada(char *input)
{
    char *comandos[MAX_COMMANDS];
    int num_comandos = 0;
    char *token;

    // Divide a linha de comando em comandos separados
    token = strtok(input, "<3");
    while (token != NULL && num_comandos < MAX_COMMANDS)
    {
        comandos[num_comandos++] = token;
        token = strtok(NULL, "<3");
    }

    // Configura o tratamento do sinal SIGUSR1
    signal(SIGUSR1, handle_sigusr1);
    setsid();
    // Executa cada comando em um processo separado
    for (int i = 0; i < num_comandos; i++)
    {
        char *args[MAX_ARGS];
        int num_args = 0;
        int foreground = 0;

        // Verifica se o comando deve ser executado em foreground
        if (i == num_comandos - 1 && strchr(comandos[i], '%') != NULL)
        {
            foreground = 1;
            comandos[i][strcspn(comandos[i], "%")] = '\0';
        }

        // Divide o comando em argumentos separados
        token = strtok(comandos[i], " \t\n");
        while (token != NULL && num_args < MAX_ARGS)
        {
            args[num_args++] = token;
            token = strtok(NULL, " \t\n");
        }

        // Adiciona NULL ao final do vetor de argumentos
        args[num_args] = NULL;

        // Executa o comando
        ExecutaComando(args, foreground);
    }
}

int main()
{
    // Configurar os manipuladores de sinal para SIGINT, SIGQUIT e SIGTSTP
    signal(SIGINT, signal_handler);
    signal(SIGQUIT, signal_handler);
    signal(SIGTSTP, signal_handler);

    char entrada[MAX_INPUT_LENGTH];
    while (1)
    {
        printf("acsh> ");
        fflush(stdout);

        // Lê a linha de comando do usuário
        if (fgets(entrada, sizeof(entrada), stdin) == NULL)
        {
            break;
        }

        // Remove o caractere de nova linha
        entrada[strcspn(entrada, "\n")] = '\0';

        // Verifica se a operação interna "exit" foi digitada
        if (strcmp(entrada, "exit") == 0)
        {
            // Finaliza todos os processos de background na mesma sessão
            kill(0, SIGUSR1);
            break;
        }

        // Verifica se a operação interna "cd" foi digitada
        if (strncmp(entrada, "cd ", 3) == 0)
        {
            // Extrai o diretório do comando
            char *directory = entrada + 3;

            // Remove espaços em branco adicionais no início e no final do diretório
            while (*directory == ' ')
            {
                directory++;
            }
            int len = strlen(directory);
            while (len > 0 && directory[len - 1] == ' ')
            {
                directory[--len] = '\0';
            }

            // Muda para o diretório especificado
            if (chdir(directory) != 0)
            {
                perror("Erro ao mudar de diretório");
            }

            continue;
        }


        // Processa a linha de comando
        TrataEntrada(entrada);
    }

    return 0;
}