#ifndef TRIE_H
#define TRIE_H
#include <stdbool.h>
#include <stdlib.h>

#define ALPHABET_SIZE 26

// Trie node structure
typedef struct TrieNode {
    struct TrieNode* children[ALPHABET_SIZE];
    bool isEndOfWord;
} TrieNode;

TrieNode* createNode();
void insertWord(TrieNode* root, const char* word);
void trie_new(TrieNode *root, char *board, uint8_t board_size);
bool trie_search(TrieNode* root, const char* word);
void freeNode(TrieNode* node);
void clearTrie(TrieNode* root);
void freeAll(TrieNode* root);
void printTrieNode(TrieNode* node);

#endif