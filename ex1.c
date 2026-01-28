#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Struct: Entry
 * Represents a single person's record in the organization.
 * Holds dynamically allocated strings for their details.
 */
typedef struct {
    char *firstName;
    char *secondName;
    char *fingerprint;
    char *position;
} Entry;

/*
 * Struct: Node
 * A node for the singly linked list. 
 * Contains an Entry structure and a pointer to the next node.
 */
typedef struct Node {
    Entry data;
    struct Node *next;
} Node;

// Functions declarartions.
char *duplicate_string_segment(const char *src, size_t n);
void trim(char *str);
int is_corruption_char(char c);
void free_list(Node *head);

/*
 * Allocates memory for 'n' characters + 1 for the null terminator,
 * copies the segment from the source, and returns the new string.
 *
 * src: The source string to copy from.
 * n:   The number of characters to copy.
 */
char *duplicate_string_segment(const char *src, size_t n) {
    char *dest = malloc(n + 1);
    if (dest) {
        strncpy(dest, src, n);
        dest[n] = '\0';
    }
    return dest;
}

/*
 * Modifies a string in-place to remove leading and trailing whitespace.
 *
 * str: The string to be trimmed.
 */
void trim(char *str) {
    if (!str) return;

    // Find the end of the string and remove trailing spaces
    int len = strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1])) {
        str[len - 1] = '\0';
        len--;
    }

    // Find the start of the valid content (skip leading spaces)
    int start = 0;
    while (str[start] && isspace((unsigned char)str[start])) {
        start++;
    }

    // Shift the content to the beginning of the buffer manually
    if (start > 0) {
        int i;
        for (i = 0; i < len - start; i++) {
            str[i] = str[start + i];
        }
        str[i] = '\0';
    }
}

/*
 * Checks if a character belongs to the set of corruption characters 
 *
 * c: The character to check.
 * returns: 1 if it is a bad character, 0 otherwise.
 */
int is_corruption_char(char c) {
    // PDF defines corruption set: # ? ! & $
    // We also strip newlines to fix fragmented words (e.g., "Fir\nst Name")
    return (c == '#' || c == '?' || c == '!' || c == '&' || c == '$' || c == '@'|| c == '\n' || c == '\r');
}

/*
 * Iterates through the linked list and frees all allocated memory,
 * including the strings inside each Entry and the Nodes themselves.
 *
 * head: The start of the linked list.
 */
void free_list(Node *head) {
    while (head) {
        Node *temp = head;
        head = head->next;
        // Free the strings inside the entry
        free(temp->data.firstName);
        free(temp->data.secondName);
        free(temp->data.fingerprint);
        free(temp->data.position);
        // Free the node itself
        free(temp);
    }
}


int main(int argc, char **argv) {
    // Validate command line arguments
    if (argc != 3) {
        printf("Usage: %s <input_corrupted.txt> <output_clean.txt>\n", argv[0]);
        return 0;
    }

    // Open input file
    FILE *in = fopen(argv[1], "r");
    if (!in) {
        printf("Error opening file: %s\n", argv[1]);
        return 0;
    }

    // Determine file size to allocate buffer
    fseek(in, 0, SEEK_END);
    long size = ftell(in);
    fseek(in, 0, SEEK_SET);

    char *clean_data = malloc(size + 1);
    if (!clean_data) { fclose(in); return 0; }

    // Read file char by char, skipping corruption to reconstruct the stream
    int j = 0;
    int c;
    while ((c = fgetc(in)) != EOF) {
        if (!is_corruption_char((char)c)) {
            clean_data[j++] = (char)c;
        }
    }
    clean_data[j] = '\0';
    fclose(in);

    Node *head = NULL;
    Node *tail = NULL;
    char *labels[] = {"First Name:", "Second Name:", "Fingerprint:", "Position:"};
    char *cursor = clean_data;

    // Loop through the clean string finding patterns of labels
    while ((cursor = strstr(cursor, labels[0])) != NULL) {
        char *p2 = strstr(cursor, labels[1]);
        if (!p2) break;

        char *p3 = strstr(p2, labels[2]);
        if (!p3) break;

        char *p4 = strstr(p3, labels[3]);
        if (!p4) break;

        Entry e;
        
        // Extract First Name
        char *val1 = cursor + strlen(labels[0]);
        e.firstName = duplicate_string_segment(val1, p2 - val1);

        // Extract Second Name
        char *val2 = p2 + strlen(labels[1]);
        e.secondName = duplicate_string_segment(val2, p3 - val2);

        // Extract Fingerprint
        char *val3 = p3 + strlen(labels[2]);
        e.fingerprint = duplicate_string_segment(val3, p4 - val3);

        // Extract Position
        // Position ends at the start of the next "First Name" OR end of file
        char *val4 = p4 + strlen(labels[3]);
        char *next_entry = strstr(val4, labels[0]);
        if (next_entry) {
            e.position = duplicate_string_segment(val4, next_entry - val4);
        } else {
            // Last entry in the file
            e.position = duplicate_string_segment(val4, strlen(val4));
        }

        if (e.firstName && e.secondName && e.fingerprint && e.position) {
            // Clean up the extracted
            trim(e.firstName); 
            trim(e.secondName); 
            trim(e.fingerprint); 
            trim(e.position);

            // Check if this fingerprint already exists in our list
            int is_dup = 0;
            Node *temp = head;
            while (temp) {
                if (strcmp(temp->data.fingerprint, e.fingerprint) == 0) {
                    is_dup = 1;
                    break;
                }
                temp = temp->next;
            }

            if (!is_dup) {
                // Add to linked list
                Node *newNode = malloc(sizeof(Node));
                if (newNode) {
                    newNode->data = e;
                    newNode->next = NULL;
                    if (!head) head = newNode;
                    else tail->next = newNode;
                    tail = newNode;
                }
            } else {
                // Duplicate found: discard this entry and free temp memory
                free(e.firstName); free(e.secondName); 
                free(e.fingerprint); free(e.position);
            }
        }
        
        // Advance cursor to process next entry
        cursor = p4 + strlen(labels[3]);
    }

    FILE *out = fopen(argv[2], "w");
    if (!out) { 
        printf("Error opening file: %s\n", argv[2]); 
        free(clean_data); 
        free_list(head); 
        return 0; 
    }

    // Define order of output
    char *rank[] = {"Boss", "Right Hand", "Left Hand", "Support_Right", "Support_Left"};
    
    for (int r = 0; r < 5; r++) {
        Node *curr = head;
        while (curr != NULL) {
            if (strcmp(curr->data.position, rank[r]) == 0) {
                fprintf(out, "First Name: %s\nSecond Name: %s\nFingerprint: %s\nPosition: %s\n\n",
                        curr->data.firstName, curr->data.secondName, curr->data.fingerprint, curr->data.position);
            }
            curr = curr->next;
        }
    }

    fclose(out);
    free(clean_data);
    free_list(head);
    
    return 0;
}