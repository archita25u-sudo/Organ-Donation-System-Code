/*
 * ORGAN DONATION MANAGEMENT SYSTEM (bare minimum)
 *
 * Hash Table + Linked List -> Donors
 * Queue       -> Standard patients
 * Deque       -> Emergency / Critical patients (critical goes to front)
 * Stack       -> History
 * Binary Tree -> Patient records (sorted by ID)
 * Graph       -> Cities and roads (shortest distance)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 10
#define INF 9999

char *organs[5] = {"Kidney", "Liver", "Heart", "Lung", "Pancreas"};
char *types[4] = {"", "Standard", "Emergency", "Critical"};

/* distance in km between 5 cities (0 = no direct road) */
int road[5][5] = {
    { 0,  4,  0,  8,  0},
    { 4,  0,  3,  0,  0},
    { 0,  3,  0,  2,  7},
    { 8,  0,  2,  0,  5},
    { 0,  0,  7,  5,  0}
};

typedef struct
{
    int id;
    char name[30];
    int organ;      /* 0 to 4 */
    int type;       /* 1 Standard, 2 Emergency, 3 Critical */
    int done;       /* 1 = got an organ */
} Person;

/* one node used for donor list, queue and deque */
typedef struct Node
{
    Person p;
    struct Node *next;
    struct Node *prev;
} Node;

typedef struct
{
    Node *front;
    Node *rear;
} List;

typedef struct Tree
{
    Person p;
    struct Tree *left;
    struct Tree *right;
} Tree;

typedef struct Log
{
    char text[100];
    struct Log *next;
} Log;

Node *table[SIZE];      /* hash table of donors */
List queue[5];          /* standard queue for each organ */
List deque[5];          /* emergency deque for each organ */
Tree *root = NULL;      /* tree of patients */
Log *top = NULL;        /* stack of history */


/* ---------- STACK (history) ---------- */

void push_history(char *msg)
{
    Log *n = malloc(sizeof(Log));
    strcpy(n->text, msg);
    n->next = top;
    top = n;
}

void show_history()
{
    Log *t;
    for (t = top; t != NULL; t = t->next)
        printf("%s\n", t->text);
}


/* ---------- QUEUE / DEQUE ---------- */

/* add at rear (queue: enqueue) */
void insert_rear(List *l, Person p)
{
    Node *n = malloc(sizeof(Node));
    n->p = p;
    n->next = NULL;
    n->prev = l->rear;

    if (l->rear == NULL)
        l->front = n;
    else
        l->rear->next = n;
    l->rear = n;
}

/* add at front (deque only) */
void insert_front(List *l, Person p)
{
    Node *n = malloc(sizeof(Node));
    n->p = p;
    n->prev = NULL;
    n->next = l->front;

    if (l->front == NULL)
        l->rear = n;
    else
        l->front->prev = n;
    l->front = n;
}

/* remove from front (queue: dequeue). Returns 0 if empty */
int remove_front(List *l, Person *p)
{
    Node *n = l->front;
    if (n == NULL)
        return 0;

    *p = n->p;
    l->front = n->next;

    if (l->front == NULL)
        l->rear = NULL;
    else
        l->front->prev = NULL;
    free(n);
    return 1;
}


/* ---------- HASH TABLE + LINKED LIST (donors) ---------- */

Node *find_donor(int id)
{
    Node *t;
    for (t = table[id % SIZE]; t != NULL; t = t->next)
        if (t->p.id == id)
            return t;
    return NULL;
}

void add_donor(Person p)
{
    int i = p.id % SIZE;
    Node *n = malloc(sizeof(Node));
    n->p = p;
    n->next = table[i];     /* add at start of the bucket */
    table[i] = n;
}

void remove_donor(int id)
{
    int i = id % SIZE;
    Node *cur = table[i];
    Node *prev = NULL;

    while (cur->p.id != id)
    {
        prev = cur;
        cur = cur->next;
    }

    if (prev == NULL)
        table[i] = cur->next;
    else
        prev->next = cur->next;
    free(cur);
}

void show_donors()
{
    int i;
    Node *t;
    for (i = 0; i < SIZE; i++)
        for (t = table[i]; t != NULL; t = t->next)
            printf("ID %d, %s, %s\n", t->p.id, t->p.name, organs[t->p.organ]);
}


/* ---------- BINARY TREE (patients) ---------- */

Tree *tree_insert(Tree *t, Person p)
{
    if (t == NULL)
    {
        t = malloc(sizeof(Tree));
        t->p = p;
        t->left = NULL;
        t->right = NULL;
    }
    else if (p.id < t->p.id)
        t->left = tree_insert(t->left, p);
    else
        t->right = tree_insert(t->right, p);
    return t;
}

Tree *tree_search(Tree *t, int id)
{
    if (t == NULL || t->p.id == id)
        return t;
    if (id < t->p.id)
        return tree_search(t->left, id);
    return tree_search(t->right, id);
}

void tree_inorder(Tree *t)
{
    if (t == NULL)
        return;
    tree_inorder(t->left);
    printf("ID %d, %s, needs %s, %s, %s\n", t->p.id, t->p.name, organs[t->p.organ],
           types[t->p.type], t->p.done ? "received organ" : "waiting");
    tree_inorder(t->right);
}


/* ---------- GRAPH (shortest distance) ---------- */

int shortest(int from, int to)
{
    int dist[5], visited[5];
    int i, j, u;

    for (i = 0; i < 5; i++)
    {
        dist[i] = INF;
        visited[i] = 0;
    }
    dist[from] = 0;

    for (i = 0; i < 5; i++)
    {
        /* pick the closest city not visited yet */
        u = -1;
        for (j = 0; j < 5; j++)
            if (visited[j] == 0 && (u == -1 || dist[j] < dist[u]))
                u = j;
        visited[u] = 1;

        /* update its neighbours */
        for (j = 0; j < 5; j++)
            if (road[u][j] > 0 && dist[u] + road[u][j] < dist[j])
                dist[j] = dist[u] + road[u][j];
    }
    return dist[to];
}


/* ---------- ALLOCATE ORGAN ---------- */

void allocate(int id)
{
    Node *d = find_donor(id);
    Person patient;
    char msg[100];

    if (d == NULL)
    {
        printf("Donor not found.\n");
        return;
    }

    /* emergency deque first, then standard queue */
    if (remove_front(&deque[d->p.organ], &patient) == 0 &&
        remove_front(&queue[d->p.organ], &patient) == 0)
    {
        printf("Nobody is waiting for %s.\n", organs[d->p.organ]);
        return;
    }

    printf("%s's %s goes to %s.\n", d->p.name, organs[d->p.organ], patient.name);
    sprintf(msg, "%s from donor %d given to %s", organs[d->p.organ], id, patient.name);
    push_history(msg);

    tree_search(root, patient.id)->p.done = 1;
    remove_donor(id);
}


/* ---------- MAIN ---------- */

int main()
{
    int choice, id, from, to;
    Person p;
    Node *d;
    Tree *t;
    char msg[100];

    do
    {
        printf("\n1. Register donor\n2. Add patient\n3. Search donor\n");
        printf("4. Search patient\n5. Allocate organ\n6. History\n7. Show all\n8. Shortest distance between cities\n0. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        if (choice == 1 || choice == 2)
        {
            printf("Enter ID: ");
            scanf("%d", &p.id);
            printf("Enter name (one word): ");
            scanf("%s", p.name);
            printf("Organ (1 Kidney, 2 Liver, 3 Heart, 4 Lung, 5 Pancreas): ");
            scanf("%d", &p.organ);
            p.organ--;
            p.type = 0;
            p.done = 0;

            if (choice == 1)
            {
                add_donor(p);
                sprintf(msg, "Donor %d (%s) registered", p.id, organs[p.organ]);
                push_history(msg);
            }
            else
            {
                printf("Type (1 Standard, 2 Emergency, 3 Critical): ");
                scanf("%d", &p.type);

                if (p.type == 1)
                    insert_rear(&queue[p.organ], p);
                else if (p.type == 2)
                    insert_rear(&deque[p.organ], p);
                else
                    insert_front(&deque[p.organ], p);

                root = tree_insert(root, p);
                sprintf(msg, "%s patient %d added", types[p.type], p.id);
                push_history(msg);
            }
        }
        else if (choice == 3)
        {
            printf("Enter donor ID: ");
            scanf("%d", &id);
            d = find_donor(id);
            if (d == NULL)
                printf("Donor not found.\n");
            else
                printf("Found: %s, %s\n", d->p.name, organs[d->p.organ]);
        }
        else if (choice == 4)
        {
            printf("Enter patient ID: ");
            scanf("%d", &id);
            t = tree_search(root, id);
            if (t == NULL)
                printf("Patient not found.\n");
            else
                printf("Found: %s, needs %s, %s\n", t->p.name, organs[t->p.organ],
                       t->p.done ? "received organ" : "waiting");
        }
        else if (choice == 5)
        {
            printf("Enter donor ID: ");
            scanf("%d", &id);
            allocate(id);
        }
        else if (choice == 6)
            show_history();
        else if (choice == 7)
        {
            show_donors();
            tree_inorder(root);
        }
        else if (choice == 8)
        {
            printf("From city (1 to 5): ");
            scanf("%d", &from);
            printf("To city (1 to 5): ");
            scanf("%d", &to);
            printf("Shortest distance: %d km\n", shortest(from - 1, to - 1));
        }
    } while (choice != 0);

    return 0;
}