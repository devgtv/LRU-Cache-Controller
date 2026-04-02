#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

// Estrutura do nó da lista duplamente encadeada
typedef struct Node {
    int key;
    int value;
    struct Node* prev;
    struct Node* next;
} Node;

// Estrutura do cache LRU
typedef struct LRUCache {
    int capacity;
    int size;
    Node* head;
    Node* tail;
} LRUCache;

// Cores
#define COLOR_HIT 1
#define COLOR_MISS 2
#define COLOR_INSERT 3
#define COLOR_REMOVE 4
#define COLOR_BORDER 5
#define COLOR_TITLE 6
#define COLOR_MENU 7

// Dimensões da tela
int max_y, max_x;

// Criar um novo nó
Node* createNode(int key, int value) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->key = key;
    newNode->value = value;
    newNode->prev = NULL;
    newNode->next = NULL;
    return newNode;
}

// Inicializar o cache LRU
LRUCache* createCache(int capacity) {
    LRUCache* cache = (LRUCache*)malloc(sizeof(LRUCache));
    cache->capacity = capacity;
    cache->size = 0;
    cache->head = NULL;
    cache->tail = NULL;
    return cache;
}

// Mover um nó para o início da lista
void moveToHead(LRUCache* cache, Node* node) {
    if (node == cache->head) return;
    
    if (node->prev) node->prev->next = node->next;
    if (node->next) node->next->prev = node->prev;
    if (node == cache->tail) cache->tail = node->prev;
    
    node->next = cache->head;
    node->prev = NULL;
    if (cache->head) cache->head->prev = node;
    cache->head = node;
}

// Remover o último nó (LRU)
void removeTail(LRUCache* cache) {
    if (cache->tail == NULL) return;
    
    Node* temp = cache->tail;
    cache->tail = cache->tail->prev;
    if (cache->tail) cache->tail->next = NULL;
    else cache->head = NULL;
    
    free(temp);
    cache->size--;
}

// Buscar um valor no cache
int get(LRUCache* cache, int key) {
    Node* current = cache->head;
    
    while (current != NULL) {
        if (current->key == key) {
            moveToHead(cache, current);
            return current->value;
        }
        current = current->next;
    }
    
    return -1;
}

// Inserir ou atualizar um valor no cache
int put(LRUCache* cache, int key, int value) {
    // Verificar se a chave já existe
    Node* current = cache->head;
    while (current != NULL) {
        if (current->key == key) {
            current->value = value;
            moveToHead(cache, current);
            return 1; // Atualizado
        }
        current = current->next;
    }
    
    // Criar novo nó
    Node* newNode = createNode(key, value);
    
    // Se o cache estiver cheio, remover o LRU
    if (cache->size >= cache->capacity) {
        removeTail(cache);
    }
    
    // Inserir no início
    newNode->next = cache->head;
    if (cache->head) cache->head->prev = newNode;
    cache->head = newNode;
    
    if (cache->size == 0) cache->tail = newNode;
    
    cache->size++;
    return 0; // Inserido
}

// Liberar memória do cache
void freeCache(LRUCache* cache) {
    Node* current = cache->head;
    while (current != NULL) {
        Node* temp = current;
        current = current->next;
        free(temp);
    }
    free(cache);
}

// Desenhar borda de uma janela
void drawBorder(int y, int x, int height, int width, const char* title) {
    attron(COLOR_PAIR(COLOR_BORDER));
    
    // Linha superior
    mvaddch(y, x, ACS_ULCORNER);
    for (int i = 1; i < width - 1; i++) mvaddch(y, x + i, ACS_HLINE);
    mvaddch(y, x + width - 1, ACS_URCORNER);
    
    // Linhas laterais
    for (int i = 1; i < height - 1; i++) {
        mvaddch(y + i, x, ACS_VLINE);
        mvaddch(y + i, x + width - 1, ACS_VLINE);
    }
    
    // Linha inferior
    mvaddch(y + height - 1, x, ACS_LLCORNER);
    for (int i = 1; i < width - 1; i++) mvaddch(y + height - 1, x + i, ACS_HLINE);
    mvaddch(y + height - 1, x + width - 1, ACS_LRCORNER);
    
    attroff(COLOR_PAIR(COLOR_BORDER));
    
    // Título
    if (title) {
        attron(COLOR_PAIR(COLOR_TITLE) | A_BOLD);
        mvprintw(y, x + 2, " %s ", title);
        attroff(COLOR_PAIR(COLOR_TITLE) | A_BOLD);
    }
}

// Desenhar o cache visualmente
void drawCache(LRUCache* cache, int y, int x) {
    int boxWidth = 12;
    int boxHeight = 5;
    int spacing = 2;
    
    attron(COLOR_PAIR(COLOR_TITLE) | A_BOLD);
    mvprintw(y - 2, x, "Estado do Cache (MRU -> LRU)");
    attroff(COLOR_PAIR(COLOR_TITLE) | A_BOLD);
    
    mvprintw(y - 1, x, "Tamanho: %d/%d", cache->size, cache->capacity);
    
    Node* current = cache->head;
    int pos = 0;
    
    while (current != NULL) {
        int bx = x + pos * (boxWidth + spacing);
        
        // Caixa do elemento
        attron(COLOR_PAIR(COLOR_BORDER));
        for (int i = 0; i < boxHeight; i++) {
            for (int j = 0; j < boxWidth; j++) {
                if (i == 0 || i == boxHeight - 1 || j == 0 || j == boxWidth - 1) {
                    if ((i == 0 && j == 0)) mvaddch(y + i, bx + j, ACS_ULCORNER);
                    else if ((i == 0 && j == boxWidth - 1)) mvaddch(y + i, bx + j, ACS_URCORNER);
                    else if ((i == boxHeight - 1 && j == 0)) mvaddch(y + i, bx + j, ACS_LLCORNER);
                    else if ((i == boxHeight - 1 && j == boxWidth - 1)) mvaddch(y + i, bx + j, ACS_LRCORNER);
                    else if (i == 0 || i == boxHeight - 1) mvaddch(y + i, bx + j, ACS_HLINE);
                    else mvaddch(y + i, bx + j, ACS_VLINE);
                }
            }
        }
        attroff(COLOR_PAIR(COLOR_BORDER));
        
        // Conteúdo
        attron(A_BOLD);
        mvprintw(y + 1, bx + 2, "Key: %d", current->key);
        mvprintw(y + 2, bx + 2, "Val: %d", current->value);
        attroff(A_BOLD);
        
        // Indicador MRU/LRU
        if (pos == 0) {
            attron(COLOR_PAIR(COLOR_HIT));
            mvprintw(y + 3, bx + 2, "  MRU  ");
            attroff(COLOR_PAIR(COLOR_HIT));
        } else if (current->next == NULL) {
            attron(COLOR_PAIR(COLOR_REMOVE));
            mvprintw(y + 3, bx + 2, "  LRU  ");
            attroff(COLOR_PAIR(COLOR_REMOVE));
        }
        
        // Seta para o próximo
        if (current->next != NULL) {
            attron(COLOR_PAIR(COLOR_MENU));
            mvprintw(y + 2, bx + boxWidth, "->");
            attroff(COLOR_PAIR(COLOR_MENU));
        }
        
        current = current->next;
        pos++;
    }
    
    if (cache->head == NULL) {
        mvprintw(y + 2, x, "(Cache vazio)");
    }
}

// Desenhar menu
void drawMenu(int y, int x) {
    drawBorder(y, x, 8, 30, "MENU");
    
    attron(COLOR_PAIR(COLOR_MENU));
    mvprintw(y + 2, x + 2, "[1] Inserir elemento");
    mvprintw(y + 3, x + 2, "[2] Buscar elemento");
    mvprintw(y + 4, x + 2, "[3] Mostrar estado");
    mvprintw(y + 5, x + 2, "[4] Limpar cache");
    mvprintw(y + 6, x + 2, "[Q] Sair");
    attroff(COLOR_PAIR(COLOR_MENU));
}

// Desenhar área de log
void drawLogArea(int y, int x, int height, int width) {
    drawBorder(y, x, height, width, "LOG DE OPERACOES");
}

// Adicionar mensagem ao log
void addLog(const char* msg, int logY, int logX, int logWidth) {
    static int logLine = 0;
    
    // Limpar área se necessário
    if (logLine >= 8) {
        logLine = 0;
        for (int i = 0; i < 8; i++) {
            move(logY + 1 + i, logX + 1);
            for (int j = 0; j < logWidth - 2; j++) addch(' ');
        }
    }
    
    mvprintw(logY + 1 + logLine, logX + 2, "%-*s", logWidth - 4, msg);
    logLine++;
}

// Tela de configuração inicial
int setupScreen() {
    int capacity = 0;
    
    clear();
    drawBorder(3, 10, 10, 60, "CONFIGURACAO DO CACHE LRU");
    
    attron(COLOR_PAIR(COLOR_TITLE) | A_BOLD);
    mvprintw(6, 20, "SIMULADOR DE CACHE LRU");
    attroff(COLOR_PAIR(COLOR_TITLE) | A_BOLD);
    
    mvprintw(8, 15, "Digite a capacidade do cache: ");
    
    echo();
    scanw("%d", &capacity);
    noecho();
    
    if (capacity <= 0 || capacity > 20) {
        attron(COLOR_PAIR(COLOR_MISS));
        mvprintw(10, 15, "Capacidade deve ser entre 1 e 20!");
        attroff(COLOR_PAIR(COLOR_MISS));
        refresh();
        getch();
        return setupScreen();
    }
    
    return capacity;
}

int main() {
    // Inicializar ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    
    // Inicializar cores
    if (has_colors()) {
        start_color();
        init_pair(COLOR_HIT, COLOR_GREEN, COLOR_BLACK);
        init_pair(COLOR_MISS, COLOR_RED, COLOR_BLACK);
        init_pair(COLOR_INSERT, COLOR_CYAN, COLOR_BLACK);
        init_pair(COLOR_REMOVE, COLOR_YELLOW, COLOR_BLACK);
        init_pair(COLOR_BORDER, COLOR_WHITE, COLOR_BLACK);
        init_pair(COLOR_TITLE, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(COLOR_MENU, COLOR_WHITE, COLOR_BLACK);
    }
    
    // Obter dimensões
    getmaxyx(stdscr, max_y, max_x);
    
    // Tela de setup
    int capacity = setupScreen();
    
    LRUCache* cache = createCache(capacity);
    
    int running = 1;
    
    while (running) {
        clear();
        
        // Título
        attron(COLOR_PAIR(COLOR_TITLE) | A_BOLD);
        mvprintw(1, (max_x - 30) / 2, "SIMULADOR DE CACHE LRU");
        attroff(COLOR_PAIR(COLOR_TITLE) | A_BOLD);
        
        // Informações
        mvprintw(2, 2, "Capacidade: %d | Tamanho atual: %d", capacity, cache->size);
        
        // Desenhar cache (posição central)
        drawCache(cache, 5, 5);
        
        // Desenhar menu (direita)
        drawMenu(5, max_x - 35);
        
        // Desenhar área de log (inferior)
        drawLogArea(15, 5, 10, max_x - 10);
        
        // Instrução
        attron(COLOR_PAIR(COLOR_MENU));
        mvprintw(max_y - 2, 5, "Selecione uma opcao (1-4 ou Q): ");
        attroff(COLOR_PAIR(COLOR_MENU));
        
        refresh();
        
        int ch = getch();
        
        switch(ch) {
            case '1':
                // Inserir
                {
                    int key, value;
                    curs_set(1);
                    echo();
                    mvprintw(max_y - 2, 40, "Chave: ");
                    scanw("%d", &key);
                    mvprintw(max_y - 2, 55, "Valor: ");
                    scanw("%d", &value);
                    noecho();
                    curs_set(0);
                    
                    int result = put(cache, key, value);
                    if (result == 0) {
                        attron(COLOR_PAIR(COLOR_INSERT));
                        addLog("INSERT: Adicionado elemento", 15, 5, max_x - 10);
                        attroff(COLOR_PAIR(COLOR_INSERT));
                    } else {
                        attron(COLOR_PAIR(COLOR_HIT));
                        addLog("UPDATE: Atualizado elemento", 15, 5, max_x - 10);
                        attroff(COLOR_PAIR(COLOR_HIT));
                    }
                }
                break;
                
            case '2':
                // Buscar
                {
                    int key;
                    curs_set(1);
                    echo();
                    mvprintw(max_y - 2, 40, "Chave para buscar: ");
                    scanw("%d", &key);
                    noecho();
                    curs_set(0);
                    
                    int value = get(cache, key);
                    
                    // Redesenhar tela para mostrar resultado
                    clear();
                    attron(COLOR_PAIR(COLOR_TITLE) | A_BOLD);
                    mvprintw(1, (max_x - 30) / 2, "SIMULADOR DE CACHE LRU");
                    attroff(COLOR_PAIR(COLOR_TITLE) | A_BOLD);
                    mvprintw(2, 2, "Capacidade: %d | Tamanho atual: %d", capacity, cache->size);
                    drawCache(cache, 5, 5);
                    drawMenu(5, max_x - 35);
                    drawLogArea(15, 5, 10, max_x - 10);
                    
                    if (value != -1) {
                        char msg[100];
                        snprintf(msg, sizeof(msg), "HIT: Chave %d = %d", key, value);
                        attron(COLOR_PAIR(COLOR_HIT));
                        addLog(msg, 15, 5, max_x - 10);
                        attroff(COLOR_PAIR(COLOR_HIT));
                        
                        // Destaque visual no elemento encontrado
                        attron(COLOR_PAIR(COLOR_HIT) | A_BOLD);
                        mvprintw(13, 5, ">>> CACHE HIT: Elemento %d encontrado! <<<", key);
                        attroff(COLOR_PAIR(COLOR_HIT) | A_BOLD);
                    } else {
                        char msg[100];
                        snprintf(msg, sizeof(msg), "MISS: Chave %d nao encontrada", key);
                        attron(COLOR_PAIR(COLOR_MISS));
                        addLog(msg, 15, 5, max_x - 10);
                        attroff(COLOR_PAIR(COLOR_MISS));
                        
                        attron(COLOR_PAIR(COLOR_MISS) | A_BOLD);
                        mvprintw(13, 5, ">>> CACHE MISS: Elemento %d nao existe! <<<", key);
                        attroff(COLOR_PAIR(COLOR_MISS) | A_BOLD);
                    }
                    
                    attron(COLOR_PAIR(COLOR_MENU));
                    mvprintw(max_y - 2, 5, "Pressione qualquer tecla para continuar...");
                    attroff(COLOR_PAIR(COLOR_MENU));
                    refresh();
                    getch();
                }
                break;
                
            case '3':
                // Mostrar estado (forçar atualização e pausa)
                {
                    clear();
                    attron(COLOR_PAIR(COLOR_TITLE) | A_BOLD);
                    mvprintw(1, (max_x - 30) / 2, "SIMULADOR DE CACHE LRU");
                    attroff(COLOR_PAIR(COLOR_TITLE) | A_BOLD);
                    mvprintw(2, 2, "Capacidade: %d | Tamanho atual: %d", capacity, cache->size);
                    drawCache(cache, 5, 5);
                    drawMenu(5, max_x - 35);
                    drawLogArea(15, 5, 10, max_x - 10);
                    
                    attron(COLOR_PAIR(COLOR_INSERT));
                    addLog("Estado atualizado na tela", 15, 5, max_x - 10);
                    attroff(COLOR_PAIR(COLOR_INSERT));
                    
                    attron(COLOR_PAIR(COLOR_MENU));
                    mvprintw(max_y - 2, 5, "Pressione qualquer tecla para continuar...");
                    attroff(COLOR_PAIR(COLOR_MENU));
                    refresh();
                    getch();
                }
                break;
                
            case '4':
                // Limpar cache
                freeCache(cache);
                cache = createCache(capacity);
                addLog("Cache limpo!", 15, 5, max_x - 10);
                break;
                
            case 'q':
            case 'Q':
                running = 0;
                break;
        }
    }
    
    // Limpar
    freeCache(cache);
    endwin();
    
    printf("Simulador encerrado.\n");
    
    return 0;
}