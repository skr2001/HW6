#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    char *firstName;
    char *secondName;
    char *fingerprint;
    char *position;
} Entry;

typedef struct Node {
    Entry data;
    struct Node *next;
} Node;

void trim(char *str) {
    if (!str) return;

    int len = strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1])) {
        str[len - 1] = '\0';
        len--;
    }

    int start = 0;
    while (str[start] && isspace((unsigned char)str[start])) {
        start++;
    }

    if (start > 0) {
        memmove(str, str + start, len - start + 1);
    }
}

int is_bad_char(char c) {
    return (c == '#' || c == '?' || c == '!' || c == '&' || c == '$' || c == '\n' || c == '\r'|| c == '@');
}

void free_list(Node *head) {
    while (head) {
        Node *temp = head;
        head = head->next;
        free(temp->data.firstName);
        free(temp->data.secondName);
        free(temp->data.fingerprint);
        free(temp->data.position);
        free(temp);
    }
}

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s <input_corrupted.txt> <output_clean.txt>\n", argv[0]);
        return 0;
    }

    FILE *in = fopen(argv[1], "r");
    if (!in) {
        printf("Error opening file: %s\n", argv[1]);
        return 0;
    }

    fseek(in, 0, SEEK_END);
    long fsize = ftell(in);
    fseek(in, 0, SEEK_SET);

    char *clean_data = malloc(fsize + 1);
    if (!clean_data) { fclose(in); return 0; }

    int j = 0;
    int c;
    while ((c = fgetc(in)) != EOF) {
        if (!is_bad_char((char)c)) {
            clean_data[j++] = (char)c;
        }
    }
    clean_data[j] = '\0';
    fclose(in);

    Node *head = NULL;
    Node *tail = NULL;
    char *labels[] = {"First Name:", "Second Name:", "Fingerprint:", "Position:"};
    char *cursor = clean_data;

    while ((cursor = strstr(cursor, labels[0])) != NULL) {
        char *p2 = strstr(cursor, labels[1]);
        if (!p2) break;

        char *p3 = strstr(p2, labels[2]);
        if (!p3) break;

        char *p4 = strstr(p3, labels[3]);
        if (!p4) break;

        Entry e;
        
        char *val1 = cursor + strlen(labels[0]);
        e.firstName = strndup(val1, p2 - val1);

        char *val2 = p2 + strlen(labels[1]);
        e.secondName = strndup(val2, p3 - val2);

        char *val3 = p3 + strlen(labels[2]);
        e.fingerprint = strndup(val3, p4 - val3);

        char *val4 = p4 + strlen(labels[3]);
        char *next_entry = strstr(val4, labels[0]);
        if (next_entry) {
            e.position = strndup(val4, next_entry - val4);
        } else {
            e.position = strdup(val4);
        }

        if (e.firstName && e.secondName && e.fingerprint && e.position) {
            trim(e.firstName); 
            trim(e.secondName); 
            trim(e.fingerprint); 
            trim(e.position);

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
                Node *newNode = malloc(sizeof(Node));
                if (newNode) {
                    newNode->data = e;
                    newNode->next = NULL;
                    if (!head) head = newNode;
                    else tail->next = newNode;
                    tail = newNode;
                }
            } else {
                free(e.firstName); free(e.secondName); 
                free(e.fingerprint); free(e.position);
            }
        }
        
        cursor = p4 + strlen(labels[3]);
    }

    FILE *out = fopen(argv[2], "w");
    if (!out) { printf("Error opening file: %s\n", argv[2]); free(clean_data); free_list(head); return 0; }

    char *rank[] = {"Boss", "Right Hand", "Left Hand", "Support_Right", "Support_Left"};
    for (int r = 0; r < 5; r++) {
        Node *curr = head;
        while (curr!=NULL) {
            printf("%s!\n",curr->data.position);
            if (strcmp(curr->data.position, rank[r])==0) {
                
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