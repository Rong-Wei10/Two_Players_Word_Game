#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include "trie.h"

TrieNode *root; // Global variable for the root of the trie
#define MAX_WORDS 100

// Function to validate a word using the trie
bool validate_word(const char *word, const char *board, TrieNode *root) {
    int board_letter_count[26] = {0};
    // Count the occurrences of each letter on the board
    for (int i = 0; i < strlen(board); i++) {
        board_letter_count[board[i] - 'a']++;
    }
    // Check if the word is formed using only letters from the board
    for (int i = 0; i < strlen(word); i++) {
        int index = word[i] - 'a';
        // If the letter doesn't exist on the board or it appears more times in the word than it appears on the board, return false
        if (board_letter_count[index] == 0 || board_letter_count[index] < 0) {
            return false;
        }
        board_letter_count[index]--;
    }
    // Check if the word exists in the trie
    return trie_search(root, word);
}


void handle_client(int client_socket, int player_count, const int* player_sockets, uint8_t board_size, uint8_t seconds_per_turn, TrieNode *root) {

    // Initialize variables
    char player_num;
    char player1 = '1';
    char player2 = '2';
    int client_socket1 = player_sockets[0];
    int client_socket2 = player_sockets[1];

    write(client_socket1, &player1, sizeof(char));
    printf("Player %c connected\n", player1);
    write(client_socket2, &player2, sizeof(char));
    printf("Player %c connected\n", player2);

    // Send board size and seconds per turn to both players
    for(int i = 0; i < 2; i++){
        int current = player_sockets[i];
        write(current, &board_size, sizeof(uint8_t));
        write(current, &seconds_per_turn, sizeof(uint8_t));
    }

    // Initialize variables for Round and Turn
    uint8_t player1_score = 0;
    uint8_t player2_score = 0;
    uint8_t round_number = 1;
    uint8_t playerResult = 0;
    uint8_t turn = 1;
    char turn_indicator = ' ';

    while(1){
        // Send round number and scores to both players
        for(int i = 0; i < 2; i++){
            int current = player_sockets[i];
            write(current, &round_number, sizeof(uint8_t));
            if (current == client_socket1) {
                write(current, &player1_score, sizeof(uint8_t));
                write(current, &player2_score, sizeof(uint8_t));
            } else {
                write(current, &player2_score, sizeof(uint8_t));
                write(current, &player1_score, sizeof(uint8_t));
            }
        }

        // Define active and inactive player
        int active_player = (round_number % 2 == 1) ? client_socket1 : client_socket2;
        int inactive_player = (active_player == client_socket1) ? client_socket2 : client_socket1;

        // Create board by boardsize and send to both player
        char board[board_size + 1];
        trie_new(root, board, board_size);
        board[board_size] = '\0';
        write(active_player, board, board_size + 1);
        write(inactive_player, board, board_size + 1);

        // Array to store the valid words entered by players
        char valid_words[MAX_WORDS][256];
        int num_valid_words = 0;

        turn = 1;
        while (turn == 1) {
            // Send indicator to distinguish active/inactive player
            turn_indicator = 'Y';
            write(active_player, &turn_indicator, sizeof(char));
            turn_indicator = 'N';
            write(inactive_player, &turn_indicator, sizeof(char));

            // Read word that enter from active player
            uint8_t word_length;
            read(active_player, &word_length, sizeof(uint8_t));
            char word[word_length + 1];
            read(active_player, word, word_length + 1);
            word[word_length] = '\0';

            // Check if the word is already in the valid words array
            bool is_word_used = false;
            for (int i = 0; i < num_valid_words; i++) {
                if (strcmp(word, valid_words[i]) == 0) {
                    is_word_used = true;
                    break;
                }
            }

            // Check word is valid
            bool is_word_valid = validate_word(word, board, root);
            if(is_word_valid && !is_word_used){
                turn = 1;
                size_t destination_size = sizeof(valid_words[num_valid_words]);
                strncpy(valid_words[num_valid_words], word, destination_size - 1);
                valid_words[num_valid_words][destination_size - 1] = '\0';
                num_valid_words++;
                write(active_player, &turn, sizeof(uint8_t));
                write(inactive_player, &turn, sizeof(uint8_t));
                write(inactive_player, &word_length, sizeof(uint8_t));
                write(inactive_player, word, word_length + 1);
            }else{
                turn = 0;
                write(active_player, &turn, sizeof(uint8_t));
                write(inactive_player, &turn, sizeof(uint8_t));
                if(client_socket1 == inactive_player){
                    player1_score = player1_score + 1;
                }else{
                    player2_score = player2_score + 1;
                }
                round_number = round_number + 1;
            }
            int temp = active_player;
            active_player = inactive_player;
            inactive_player = temp;
        }
        for(int i = 0; i < 2; i++){
            int current = player_sockets[i];
            if (current == client_socket1) {
                write(current, &player1_score, sizeof(uint8_t));
                write(current, &player2_score, sizeof(uint8_t));
            } else {
                write(current, &player2_score, sizeof(uint8_t));
                write(current, &player1_score, sizeof(uint8_t));
            }
        }
        
        if(player1_score >= 3 || player2_score >= 3){
            printf("Game Over\n");
            break;
        }
    }
    close(client_socket1);
    close(client_socket2);
}


int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <port> <board_size> <seconds_per_round> <dictionary_path>\n", argv[0]);
        exit(1);
    }

    // Conver Arguments
    int port = atoi(argv[1]);
    uint8_t board_size = atoi(argv[2]);
    uint8_t seconds_per_round = atoi(argv[3]);
    char *dictionary_path = argv[4];

    // Load valid dictionary words into the trie
    root = createNode(); // Initialize the root of the trie
    FILE *file = fopen(dictionary_path, "r");
    if (file == NULL) {
        perror("Failed to open dictionary file");
        exit(1);
    }
    char word[50];
    while (fgets(word, sizeof(word), file) != NULL) {
        // Remove newline character at the end
        word[strcspn(word, "\n")] = '\0';
        insertWord(root, word); // Insert the word into the trie
    }
    fclose(file);

    // Create a TCP socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Failed to create socket");
        exit(1);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Failed to bind socket");
        exit(1);
    }

    if (listen(server_socket, 5) == -1) {
        perror("Failed to listen for connections");
        exit(1);
    }

    int player_count = 0;
    int player_sockets[2] = { -1, -1 };

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        // Create and accept client sockets for both player
        for (int i = 0; i < 2; i++) {
            int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
            if (client_socket == -1) {
                perror("Failed to accept client connection");
                continue;
            }
            player_sockets[i] = client_socket;
            uint8_t initialPlayer = 1;
            // Distinguish Player1 and Player2
            if(i == 0){
                write(client_socket, &initialPlayer, sizeof(uint8_t));
            }else{
                initialPlayer = 2;
                write(client_socket, &initialPlayer, sizeof(uint8_t));
            }
            player_count++;
        }

        if (player_count > 1){
            if (fork() == 0) { // Fork a child process to handle the clients
                handle_client(player_sockets[0], player_count, player_sockets, board_size, seconds_per_round, root);
                exit(0);
            } else {
                //Re-initialize the player struct
                player_count = 0;
                player_sockets[0] = -1;
                player_sockets[1] = -1;
            }    
        }

    }

    close(server_socket);
    return 0;
}


