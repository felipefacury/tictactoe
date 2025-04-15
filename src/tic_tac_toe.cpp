#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <array>

// Classe TicTacToe
class TicTacToe {
private:
    std::array<std::array<char, 3>, 3> board; // Tabuleiro do jogo
    std::mutex board_mutex; // Mutex para controle de acesso ao tabuleiro
    std::condition_variable turn_cv; // Variável de condição para alternância de turnos
    char current_player; // Jogador atual ('X' ou 'O')
    bool game_over; // Estado do jogo
    char winner; // Vencedor do jogo

public:
    TicTacToe() {
        // Inicializar o tabuleiro e as variáveis do jogo
        for (auto& row : board) {
            row.fill(' ');
        }
        this->game_over = false;
        this->current_player = 'X'; // Jogador X começa
        this->winner = ' '; // Nenhum vencedor inicialmente
    }

    void display_board() {
        // Exibir o tabuleiro no console
        system("clear");

        for (const auto& row : board) {
            int i = 0;
            for (const auto& cell : row) {
                std::cout << cell;
                if (i < 2) {
                    std::cout << " | ";
                }
                i++;
            }
            std::cout << std::endl;
            if(&row != &board.back()) {
                std::cout << "---------" << std::endl;
            }
        }
        std::cout << '\n';
        std::cout << "Current player: " << this->current_player << std::endl; 
    }

    bool make_move(char player, int row, int col) {
        // Implementar a lógica para realizar uma jogada no tabuleiro
        // Utilizar mutex para controle de acesso
        // Utilizar variável de condição para alternância de turnos
        if (game_over) {
            std::cerr << "O jogo já terminou!" << std::endl;
            return false; // Jogo já terminado
        }

        std::unique_lock<std::mutex> lock(board_mutex);
        while (player != this->current_player){
            turn_cv.wait(lock);
        }
        
        if (row < 0 || row >= 3 || col < 0 || col >= 3 || board[row][col] != ' ') {
            std::cerr << "Jogada inválida!" << std::endl;
            return false;
        }
        board[row][col] = player;
        display_board();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        this->current_player = (this->current_player == 'X') ? 'O' : 'X';
        this->turn_cv.notify_all();
        
        return true;
    }

    bool check_win(char player) {
        // Verificar se o jogador atual venceu o jogo
        // Check row
        for (int i = 0; i < 3; ++i) {
            if (board[i][0] == player && board[i][1] == player && board[i][2] == player) {
                this->game_over = true;
                this->winner = player;
                return true;
            }
        }
        // Check column
        for (int i = 0; i < 3; ++i) {
            if (board[0][i] == player && board[1][i] == player && board[2][i] == player) {
                this->game_over = true;
                this->winner = player;
                return true;
            }
        }
        // Check diagonal
        if (board[0][0] == player && board[1][1] == player && board[2][2] == player) {
            this->game_over = true;
            this->winner = player;
            return true;
        }
        if (board[0][2] == player && board[1][1] == player && board[2][0] == player) {
            this->game_over = true;
            this->winner = player;
            return true;
        }
        return false;
    }

    bool check_draw() {
        // Verificar se houve um empate
        for(auto& row : board) {
            for(auto& cell : row) {
                if(cell == ' ')
                    return false;

            }
        }
        this->game_over = true;
        this->winner = 'D';
        return !check_win('X') && !check_win('O');
    }

    bool is_game_over() {
        // Retornar se o jogo terminou
        return this->game_over;
    }

    char get_winner() {
        // Retornar o vencedor do jogo ('X', 'O', ou 'D' para empate)
        if (!this->game_over)
            return ' ';
        return this->winner;
    }
};

// Classe Player
class Player {
private:
    TicTacToe& game; // Referência para o jogo
    char symbol; // Símbolo do jogador ('X' ou 'O')
    std::string strategy; // Estratégia do jogador

public:
    Player(TicTacToe& g, char s, std::string strat) 
        : game(g), symbol(s), strategy(strat) {}

    void play() {
        // Executar jogadas de acordo com a estratégia escolhida
        if (strategy == "sequential") {
            play_sequential();
        } else if (strategy == "random") {
            play_random();
        } else {
            std::cerr << "Estratégia inválida!" << std::endl;
            return;
        }
    }

private:
    void play_sequential() {
        // Implementar a estratégia sequencial de jogadas
        int break_count = 0;
        while (!game.is_game_over()) {

            for (int i = 0; i < 3 && !game.is_game_over(); ++i) {

                for (int j = 0; j < 3 && !game.is_game_over(); ++j) {

                    if (game.make_move(symbol, i, j)) {
                        if (game.check_win(symbol)) {
                            break;
                            break_count++;
                        } else if (game.check_draw()) {
                            break;
                            break_count++;
                        }
                    }
                }
                if (break_count > 0)
                    break;
            }
        }
    }

    void play_random() {
        // Implementar a estratégia aleatória de jogadas
        while (!this->game.is_game_over()) {
            int row = rand() % 3;
            int col = rand() % 3;

            if (game.make_move(symbol, row, col)) {
                game.check_win(symbol);
                game.check_draw();
            }
        }
    }
};

// Função principal
int main() {
    // Inicializar o jogo e os jogadores
    TicTacToe game;
    Player p1(game, 'X', "sequential");
    Player p2(game, 'O', "random");

    // Criar as threads para os jogadores
    std::thread t1(&Player::play, &p1);
    std::thread t2(&Player::play, &p2);

    // Aguardar o término das threads
    t1.join();
    t2.join();

    // Exibir o resultado final do jogo
    char winner = game.get_winner();
    if (winner == 'X') {
        std::cout << "\nJogador X venceu!" << std::endl;
    } else if (winner == 'O') {
        std::cout << "\nJogador O venceu!" << std::endl;
    } else if (winner == 'D') {
        std::cout << "\nEmpate!" << std::endl;
    }
    
    return 0;
}
