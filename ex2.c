#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "org_tree.h"

/*
 * Constant: FP_LEN - The length of the fingerprint.
 */
#define FP_LEN 9

// Functions
int check_candidate(Node *node, int *cipher_vals, int mask, int is_xor);
Node* search_org(Org *org, int *cipher_vals, int mask, int is_xor);

/*
 * Verifies if a specific node's fingerprint matches the encrypted data
 * given a specific mask and bitwise operation.
 *
 * node:        The organization node to check.
 * cipher_vals: Array of 9 integer values representing the encrypted bytes.
 * mask:        The 8-bit mask being tested (integer).
 * is_xor:      Flag (1 for XOR operation, 0 for AND operation).
 *
 * Returns: 1 if the fingerprint matches the cipher, 0 otherwise.
 */
int check_candidate(Node *node, int *cipher_vals, int mask, int is_xor) {
    if (!node) return 0;

    // Iterate through all 9 characters of the fingerprint
    for (int i = 0; i < FP_LEN; i++) {
        char plain_char = node->fingerprint[i];
        int computed_val;

        // Apply the candidate mask using the candidate operation
        if (is_xor) {
            computed_val = plain_char ^ mask;
        } else {
            computed_val = plain_char & mask;
        }

        // Compare calculated value against the actual encrypted byte
        if (computed_val != cipher_vals[i]) {
            return 0;
        }
    }
    
    // If all 9 characters matched
    return 1;
}

/*
 * Traverses the entire organization tree to look for a matching fingerprint.
 * The traversal order covers every node: Boss, Left Hand, Right Hand, 
 * and their respective support lists.
 *
 * org:         Pointer to the Organization structure.
 * cipher_vals: Array of encrypted bytes.
 * mask:        The mask to test.
 * is_xor:      Operation flag (1 for XOR, 0 for AND).
 *
 * Returns: Pointer to the matching Node if found, NULL otherwise.
 */
Node* search_org(Org *org, int *cipher_vals, int mask, int is_xor) {
    if (!org) return NULL;

    // Check the Boss (Root)
    if (check_candidate(org->boss, cipher_vals, mask, is_xor)) 
        return org->boss;

    // Check the Left Hand
    if (check_candidate(org->left_hand, cipher_vals, mask, is_xor)) 
        return org->left_hand;

    // Check Left Hand's Support List
    if (org->left_hand) {
        Node *curr = org->left_hand->supports_head;
        while (curr) {
            if (check_candidate(curr, cipher_vals, mask, is_xor)) return curr;
            curr = curr->next;
        }
    }

    // Check the Right Hand
    if (check_candidate(org->right_hand, cipher_vals, mask, is_xor)) 
        return org->right_hand;

    // Check Right Hand's Support List
    if (org->right_hand) {
        Node *curr = org->right_hand->supports_head;
        while (curr) {
            if (check_candidate(curr, cipher_vals, mask, is_xor)) return curr;
            curr = curr->next;
        }
    }

    return NULL; // No match found in the entire organization
}


int main(int argc, char **argv) {
    // Validate command line arguments
    if (argc != 4) {
        printf("Usage: %s <clean_file.txt> <cipher_bits.txt> <mask_start_s>\n", argv[0]);
        return 0;
    }

    char *clean_file_path = argv[1];
    char *cipher_file_path = argv[2];
    int start_mask = atoi(argv[3]);

    // Open the encrypted file
    FILE *cipher_fp = fopen(cipher_file_path, "r");
    if (!cipher_fp) {
        printf("Error opening file: %s\n", cipher_file_path);
        return 0;
    }

    // Parse the 9 lines of binary strings into integers
    int cipher_vals[FP_LEN];
    char line[32];
    int line_count = 0;

    while (line_count < FP_LEN && fgets(line, sizeof(line), cipher_fp)) {
        // Remove trailing newline
        line[strcspn(line, "\r\n")] = 0;
        
        // Convert binary string to integer (base 2)
        cipher_vals[line_count] = (int)strtol(line, NULL, 2);
        line_count++;
    }
    fclose(cipher_fp);

    // Build the organization tree
    Org org = build_org_from_clean_file(clean_file_path);
    
    int found = 0;

    // Try every mask in the range [s, s + 10]
    for (int m = start_mask; m <= start_mask + 10; m++) {
        
        // Test XOR Operation
        Node *match = search_org(&org, cipher_vals, m, 1);
        if (match) {
            printf("Successful Decrypt! The Mask used was mask_%d of type (XOR) and The fingerprint was %s belonging to %s %s\n", 
                   m, match->fingerprint, match->first, match->second);
            found = 1;
            break;
        }

        // Test AND Operation
        match = search_org(&org, cipher_vals, m, 0);
        if (match) {
            printf("Successful Decrypt! The Mask used was mask_%d of type (AND) and The fingerprint was %s belonging to %s %s\n", 
                   m, match->fingerprint, match->first, match->second);
            found = 1;
            break;
        }
    }

    // If loop finishes without setting found flag
    if (!found) {
        printf("Unsuccesful decrypt, Looks like he got away\n");
    }

    // Clean up memory
    free_org(&org);

    return 0;
}