#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <stdint.h>
#include "trie.h"

// Function to create a new trie node
TrieNode* createNode() {
    TrieNode* newNode = (TrieNode*)malloc(sizeof(TrieNode));
    if (newNode) {
        newNode->isEndOfWord = false;
        for (int i = 0; i < ALPHABET_SIZE; i++) {
            newNode->children[i] = NULL;
        }
    }
    return newNode;
}


// Helper function to get a random character from a given string
char getRandomChar(const char *str) {
    int len = strlen(str);
    int randomIndex = rand() % len;
    return str[randomIndex];
}

// Function to generate a board with random letters that leads to at least one valid word
void trie_new(TrieNode *root, char *board, uint8_t board_size) {
    const char *letters = "abcdefghijklmnopqrstuvwxyz";
    srand(time(NULL)); // Seed the random number generator

    // Initialize the board with random letters
    uint8_t vowels_count = 0;
    for (uint8_t i = 0; i < board_size - 1; i++) {
        board[i] = getRandomChar(letters);
        if (strchr("aeiou", board[i]) != NULL) {
            vowels_count++;
        }
    }
    // Randomly choose the last character as a vowel if no vowels in the first (K - 1) characters
    if (vowels_count == 0) {
        board[board_size - 1] = getRandomChar("aeiou");
    } else {
        board[board_size - 1] = getRandomChar(letters);
    }
    board[board_size] = '\0';
}


// Function to insert a word into the trie
void insertWord(TrieNode* root, const char* word) {
    TrieNode* currentNode = root;
    for (int i = 0; word[i] != '\0'; i++) {
        int index = word[i] - 'a';
        if (!currentNode->children[index]) {
            currentNode->children[index] = createNode();
        }
        currentNode = currentNode->children[index];
    }
    currentNode->isEndOfWord = true;
}

// Function to search for a word in the trie
bool trie_search(TrieNode* root, const char* word) {
    TrieNode* currentNode = root;
    for (int i = 0; word[i] != '\0'; i++) {
        int index = word[i] - 'a';
        if (!currentNode || !currentNode->children[index]) {
            return false;  // Word is not present in the trie
        }
        currentNode = currentNode->children[index];
    }
    return currentNode && currentNode->isEndOfWord;
}


// Function to delete a trie node
void freeNode(TrieNode* node) {
    if (node) {
        for (int i = 0; i < ALPHABET_SIZE; i++) {
            freeNode(node->children[i]);
        }
        free(node);
    }
}

// Function to clear the entire trie
void clearTrie(TrieNode* root) {
    if (root) {
        for (int i = 0; i < ALPHABET_SIZE; i++) {
            clearTrie(root->children[i]);
            root->children[i] = NULL;
        }
        root->isEndOfWord = false;
    }
}

// Function to delete the entire trie
void freeAll(TrieNode* root) {
    clearTrie(root);
    freeNode(root);
}

// Function to print the trie
void printTrieNode(TrieNode* node) {
    if (node == NULL) {
        return;
    }
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (node->children[i] != NULL) {
            printf("Child %c exists\n", 'a' + i);
            printTrieNode(node->children[i]);
        }
    }
    if (node->isEndOfWord) {
        printf("End of Word\n");
    }
}

