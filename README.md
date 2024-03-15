# Spell Checker

## Overview

This spell checker is a command-line tool designed to check the spelling of words in text files against a provided dictionary. It handles various cases, such as trailing punctuation, hyphenated words, and capitalization variations, ensuring comprehensive spell checking across documents.

## Installation

### Prerequisites

- A C compiler (GCC recommended)
- Make (optional for building the program using the Makefile)

## Compiling the program
gcc -o spchk spchk.c
make 

## Usage
./spchk path/to/dictionary.txt path/to/textfile.txt
for directroies:
./spchk path/to/dictionary.txt path/to/directory

## Features
-  Check spelling of words in text files against a provided dictionary
-  Handle trailing punctuation
-  Handle hyphenated words
-  Handle capitalization variations
-  Comprehensive spell checking across documents

## Data Structure
Use of a hash table for efficiently storing and searching words from the dictionary
As ususal hash tabe it maps keys to values, allowing for fast data retrieval based on the key. The keys are the words from the dictionary, and the values are simply the presence of these words in the hash table.

- The hash function converts the key (word) into an integer, which determines the index in the hash table array where the word should be stored.

- Iterates over each character of the input word, using its ASCII value does calculations that results in large range of indices. The result is then modulated by the table size to ensure it fits within the bounds of the hash table array.

In the code collisions are handled using chaining, where each index in the hash table points to a linked list  of nodes. Each node stores a word and a pointer to the next node in the list.

When a collision happens, the new word is added to the beginning of the list at that index, ensuring that all words hashing to the same index are stored together.

## Insertion
-To insert a word into the hash table, the insertWord function first calculates the word's hash index. It then creates a new node for the word and inserts this node at the beginning of the linked list at that hash index

## Searching
The findWord function searches for a word in the hash table. It calculates the hash index for the word and then iterates through the linked list at that index, comparing each stored word with the target word

## Helper functions
- stripPunctuation
This function takes a word as input and returns a new string with leading and trailing punctuation removed. This is crucial for accurately comparing words from the text files to the dictionary, as punctuation is not part of the word

- toAllCaps and toInitialCap
These functions convert a word to all uppercase or capitalize only the first letter, respectively. They're used to handle case variations when checking spelling

- isHyphenated ,checkHyphenatedWord
isHyphenated checks if a word contains a hyphen, which suggests it may be a compound word. 
checkHyphenatedWord then verifies whether each component of a hyphenated word is in the dictionary, handling compound words correctly.

- isTextFile
This function checks if a file name ends with .txt, indicating that it is a text file and should be processed by the spell checker

- loadDictionary 
Function loads words from the dictionary file into the hash table. It reads the file in chunks, processing each word and its variations (original, initial capitalization, all uppercase) by inserting them into the hash table

- checkSpelling 
Reads a file, tokenizes it into words (considering hyphenation and punctuation), and checks each word against the dictionary loaded into the hash table. It reports any words not found in the dictionary, potentially indicating a spelling mistake

## Tests:
mix.txt:
This text file includes various test cases, such as:
    - Words with leading and trailing punctuation.
    - Hyphenated words and their recognition based on the dictionary.
    - Words with apostrophes to ensure they're correctly identified as part of the word.
    - Capitalization variations and their correct identification based on the dictionary.
    - Edge cases with adjacent punctuation, special characters, and unusual capitalizations.
cap.txt
    - This file tests the spell checker's handling of words with different capitalizations, including words that should be recognized in their regular, initial capital, and all capitals forms, as well as a specific case that must match the dictionary exactly (MacDonald).
hyphen.txt
    -This file contains hyphenated words to test if the spell checker correctly identifies them as spelled correctly when all component words are in the dictionary.
punct.txt
    -This file tests the ability of the checker to ignore common sentence punctuation, quotation marks at the end and start of a word, and brackets.

