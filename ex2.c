#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "org_tree.h"

#define FP_LEN 9


static void print_success(int mask, char *op, char* fingerprint, char* First_Name, char* Second_Name)
{
    printf("Successful Decrypt! The Mask used was mask_%d of type (%s) and The fingerprint was %.*s belonging to %s %s\n",
                       mask, op, FP_LEN, fingerprint, First_Name, Second_Name);
}

static void print_unsuccess()
{
    printf("Unsuccesful decrypt, Looks like he got away\n");
}

int check_candidate(Node *node, int *cipher_vals, int mask, int is_xor) {
    if (!node) return 0;

    for (int i = 0; i < FP_LEN; i++) {
        char plain_char = node->fingerprint[i];
        int computed_val;

        if (is_xor) {
            computed_val = plain_char ^ mask;
        } else {
            computed_val = plain_char & mask;
        }

        if (computed_val != cipher_vals[i]) {
            return 0;
        }
    }
    return 1;
}

Node* search_org(Org *org, int *cipher_vals, int mask, int is_xor) {
    if (!org) return NULL;

    if (check_candidate(org->boss, cipher_vals, mask, is_xor)) 
        return org->boss;

    if (check_candidate(org->left_hand, cipher_vals, mask, is_xor)) 
        return org->left_hand;

    if (org->left_hand) {
        Node *curr = org->left_hand->supports_head;
        while (curr) {
            if (check_candidate(curr, cipher_vals, mask, is_xor)) return curr;
            curr = curr->next;
        }
    }

    if (check_candidate(org->right_hand, cipher_vals, mask, is_xor)) 
        return org->right_hand;

    if (org->right_hand) {
        Node *curr = org->right_hand->supports_head;
        while (curr) {
            if (check_candidate(curr, cipher_vals, mask, is_xor)) return curr;
            curr = curr->next;
        }
    }

    return NULL;
}

int main(int argc, char **argv) {
    if (argc != 4) {
        printf("Usage: %s <clean_file.txt> <cipher_bits.txt> <mask_start_s>\n", argv[0]);
        return 0;
    }

    char *clean_file_path = argv[1];
    char *cipher_file_path = argv[2];
    int start_mask = atoi(argv[3]);

    FILE *cipher_fp = fopen(cipher_file_path, "r");
    if (!cipher_fp) {
        printf("Error opening file: %s\n", cipher_file_path);
        return 0;
    }

    int cipher_vals[FP_LEN];
    char line[32];
    int line_count = 0;

    while (line_count < FP_LEN && fgets(line, sizeof(line), cipher_fp)) {
        line[strcspn(line, "\r\n")] = 0;
        cipher_vals[line_count] = (int)strtol(line, NULL, 2);
        line_count++;
    }
    fclose(cipher_fp);

    Org org = build_org_from_clean_file(clean_file_path);
    
    int found = 0;

    for (int m = start_mask; m <= start_mask + 10; m++) {
        Node *match = search_org(&org, cipher_vals, m, 1);
        if (match) {
            print_success(m, "XOR", match->fingerprint, match->first, match->second);
            found = 1;
            break;
        }

        match = search_org(&org, cipher_vals, m, 0);
        if (match) {
            print_success(m, "AND", match->fingerprint, match->first, match->second);
            found = 1;
            break;
        }
    }

    if (!found) {
        print_unsuccess();
    }

    free_org(&org);

    return 0;
}