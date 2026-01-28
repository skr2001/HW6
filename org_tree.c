#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "org_tree.h"

// Functions
void safe_string_copy(char *dest, const char *src, size_t dest_size);
void parse_and_store(char *dest, const char *line, size_t dest_size);
Node* create_node();
void append_support(Node *hand_node, Node *new_support);
Org build_org_from_clean_file(const char *path);
void print_node(const Node *node);
void print_tree_order(const Org *org);
void free_list(Node *head);
void free_org(Org *org);



/*
 * Manually copies a string from src to dest.
 */
void safe_string_copy(char *dest, const char *src, size_t dest_size) {
    size_t i;
    for (i = 0; i < dest_size - 1 && src[i] != '\0'; i++) {
        // Stop copying if we hit a newline
        if (src[i] == '\r' || src[i] == '\n') {
            break;
        }
        dest[i] = src[i];
    }
    // Always null-terminate
    dest[i] = '\0';
}

/*
 * Helper to extract value from "Label: Value".
 */
void parse_and_store(char *dest, const char *line, size_t dest_size) {
    const char *colon = strchr(line, ':');
    if (colon) {
        const char *val_start = colon + 1;
        // Skip leading space
        if (*val_start == ' ') {
            val_start++;
        }
        safe_string_copy(dest, val_start, dest_size);
    } else {
        // Empty string if parsing fails
        dest[0] = '\0';
    }
}

/*
 * Allocates a node and zeroes out memory.
 */
Node* create_node() {
    Node *new_node = (Node *)malloc(sizeof(Node));
    if (!new_node) {
        printf("Memory allocation failed\n");
        return NULL; 
    }

    // Manually zero out the character arrays
    // This replaces memset(new_node->first, 0, MAX_FIELD);
    int i;
    for (i = 0; i < MAX_FIELD; i++) {
        new_node->first[i] = '\0';
        new_node->second[i] = '\0';
        new_node->fingerprint[i] = '\0';
    }
    
    for (i = 0; i < MAX_POS; i++) {
        new_node->position[i] = '\0';
    }

    new_node->left = NULL;
    new_node->right = NULL;
    new_node->supports_head = NULL;
    new_node->next = NULL;

    return new_node;
}

/*
 * Appends a node to the end of a support linked list.
 */
void append_support(Node *hand_node, Node *new_support) {
    if (!hand_node || !new_support) return;

    if (hand_node->supports_head == NULL) {
        hand_node->supports_head = new_support;
    } else {
        Node *current = hand_node->supports_head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_support;
    }
}

/*
 * Reads the clean file using a fixed 128-byte buffer.
 */
Org build_org_from_clean_file(const char *path) {
    Org org = {NULL, NULL, NULL};
    FILE *fp = fopen(path, "r");

    if (!fp) {
        printf("Error opening file: %s\n", path);
        return org; 
    }

    char line[MAX_FIELD]; // Fixed size 128

    while (fgets(line, sizeof(line), fp)) {
        // Check if line starts with "First Name:"
        if (strstr(line, "First Name:") != line) {
            continue;
        }

        Node *node = create_node();
        if (!node) break;

        //Parse First Name
        parse_and_store(node->first, line, MAX_FIELD);

        //Read & Parse Second Name
        if (fgets(line, sizeof(line), fp)) {
            parse_and_store(node->second, line, MAX_FIELD);
        }

        //Read & Parse Fingerprint
        if (fgets(line, sizeof(line), fp)) {
            parse_and_store(node->fingerprint, line, MAX_FIELD);
        }

        //Read & Parse Position
        if (fgets(line, sizeof(line), fp)) {
            parse_and_store(node->position, line, MAX_POS);
        }

        // Link logic
        if (strcmp(node->position, "Boss") == 0) {
            org.boss = node;
        } 
        else if (strcmp(node->position, "Right Hand") == 0) {
            org.right_hand = node;
            if (org.boss) org.boss->right = node;
        } 
        else if (strcmp(node->position, "Left Hand") == 0) {
            org.left_hand = node;
            if (org.boss) org.boss->left = node;
        } 
        else if (strcmp(node->position, "Support_Right") == 0) {
            if (org.right_hand) {
                append_support(org.right_hand, node);
            } else {
                free(node);
            }
        } 
        else if (strcmp(node->position, "Support_Left") == 0) {
            if (org.left_hand) {
                append_support(org.left_hand, node);
            } else {
                free(node);
            }
        } else {
            free(node); // Unknown position
        }
    }

    fclose(fp);
    return org;
}

/*
 * Prints a single node's data.
 */
void print_node(const Node *node) {
    if (!node) return;
    printf("First Name: %s\n", node->first);
    printf("Second Name: %s\n", node->second);
    printf("Fingerprint: %s\n", node->fingerprint);
    printf("Position: %s\n\n", node->position);
}

/*
 * Prints: Boss -> Left Hand -> Left Supports -> Right Hand -> Right Supports
 */
void print_tree_order(const Org *org) {
    if (!org || !org->boss) return;

    print_node(org->boss);

    if (org->left_hand) {
        print_node(org->left_hand);

        Node *curr = org->left_hand->supports_head;
        while (curr) {
            print_node(curr);
            curr = curr->next;
        }
    }

    if (org->right_hand) {
        print_node(org->right_hand);

        Node *curr = org->right_hand->supports_head;
        while (curr) {
            print_node(curr);
            curr = curr->next;
        }
    }
}

/*
 * Frees a linked list.
 */
void free_list(Node *head) {
    Node *curr = head;
    while (curr) {
        Node *temp = curr;
        curr = curr->next;
        free(temp);
    }
}

/*
 * Frees the entire tree structure.
 */
void free_org(Org *org) {
    if (!org) return;

    if (org->left_hand) {
        free_list(org->left_hand->supports_head);
        free(org->left_hand);                     
    }

    if (org->right_hand) {
        free_list(org->right_hand->supports_head); 
        free(org->right_hand);                     
    }

    if (org->boss) {
        free(org->boss);
    }
    
    org->boss = NULL;
    org->left_hand = NULL;
    org->right_hand = NULL;
}