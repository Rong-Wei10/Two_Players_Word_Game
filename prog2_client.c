#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <fcntl.h>


// Function to split the word with space
char* split_string(char* str) {
    size_t length = strlen(str);
    char* result = malloc(2 * length + 1);
    // Iterate over each character in the input string
    for (size_t i = 0; i < length; i++) {
        // Copy the current character to the new string
        result[2 * i] = str[i];
        // Insert a space after each letter, except for the last character
        if (i < length - 1) {
            result[2 * i + 1] = ' ';
        }
    }
    result[2 * length] = '\0';
    return result;
}

void play_game(int server_socket) {
    uint8_t initialPlayer = 0;
    read(server_socket, &initialPlayer, sizeof(uint8_t));
    if(initialPlayer == 1){
        printf("You are Player 1... the game will begin when Player 2 joins...\n");
    }
    // Read player information from the server
    char player_num;
    uint8_t board_size;
    uint8_t seconds_per_turn;
    read(server_socket, &player_num, sizeof(char));
    read(server_socket, &board_size, sizeof(uint8_t));
    read(server_socket, &seconds_per_turn, sizeof(uint8_t));

    if(player_num == '2'){
        printf("You are Player 2...\n");
    }
    printf("Board size: %d\n", board_size);
    printf("Seconds per turn: %d\n", seconds_per_turn);

    // // Game variables
    uint8_t round_number = 0;
    uint8_t player_score = 0;
    uint8_t opponent_score = 0;
    uint8_t playerResult = 0;
    uint8_t turn = 1;
    char turn_indicator = ' ';

    while(1){
        // Read information for start of each round
        read(server_socket, &round_number, sizeof(uint8_t));
        read(server_socket, &player_score, sizeof(uint8_t));
        read(server_socket, &opponent_score, sizeof(uint8_t));

        printf("\nRound %d...\n", round_number);
        printf("Score is %d-%d\n", player_score, opponent_score);

        // Read board from server and printout
        char board[board_size + 1];
        read(server_socket, board, board_size + 1);
        char* splitOutput = split_string(board);
        printf("Board: %s\n", splitOutput);
        free(splitOutput);

        turn = 1;
        while(turn == 1){
            // Read indicator to determine you are active player or inactive
            read(server_socket, &turn_indicator, sizeof(char));
            // Active player
            if(turn_indicator == 'Y'){
                printf("Your turn, enter a word: ");

                fflush(stdout);

                // Set stdin to non-blocking mode
                int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
                if (flags == -1) {
                    perror("Failed to get stdin flags");
                    return;
                }
                if (fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK) == -1) {
                    perror("Failed to set stdin to non-blocking mode");
                    return;
                }

                struct timeval timeout;
                timeout.tv_sec = seconds_per_turn;
                timeout.tv_usec = 0;

                fd_set fds;
                FD_ZERO(&fds);
                FD_SET(STDIN_FILENO, &fds);

                int result = select(STDIN_FILENO + 1, &fds, NULL, NULL, &timeout);

                char word[256];
                fgets(word, sizeof(word), stdin);
                word[strcspn(word, "\n")] = '\0'; 

                // Send the word to the server
                uint8_t word_length = strlen(word);
                write(server_socket, &word_length, sizeof(uint8_t));
                write(server_socket, word, word_length);
                read(server_socket, &turn, sizeof(uint8_t));
                if(turn == 1 && result != 0){
                    printf("Valid word!\n");
                }else{
                    printf("Invalid word!\n");
                }   
            }else{ //Inactive Player
                printf("Please wait for opponent to enter word...\n");
                read(server_socket, &turn, sizeof(uint8_t));
                if(turn == 1){
                    uint8_t word_length;
                    read(server_socket, &word_length, sizeof(uint8_t));
                    char word[word_length + 1];
                    read(server_socket, word, word_length + 1);
                    word[word_length] = '\0';
                    printf("Opponent entered \"%s\"\n", word);
                }else{
                    printf("Opponent lost the round!\n");
                }
            }
        }
        read(server_socket, &player_score, sizeof(uint8_t));
        read(server_socket, &opponent_score, sizeof(uint8_t));
        if (player_score >= 3 || opponent_score >= 3) {
            break;
        }
    }
    if(player_score >= 3){
        printf("You won!\n");
    }else{
        printf("You lost!\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_address> <port>\n", argv[0]);
        exit(1);
    }

    char *server_address = argv[1];
    int port = atoi(argv[2]);

    // Create a socket for the client
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Failed to create socket");
        exit(1);
    }

    // Connect to the server
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, server_address, &server_addr.sin_addr) <= 0) {
        perror("Invalid server address");
        exit(1);
    }
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Failed to connect to the server");
        exit(1);
    }

    // Call the function to handle the game logic for the client
    play_game(client_socket);

    close(client_socket);
    return 0;
}


