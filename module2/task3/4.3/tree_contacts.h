#ifndef TREE_CONTACTS_H
#define TREE_CONTACTS_H_
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define MAX_LENGTH 30 
#define MAX 5
#define CONTACTS_SIZE 160
typedef struct Link
{
    char Social_networks[4][MAX_LENGTH];
    char Profiles_messengers[4][MAX_LENGTH];

}Link;

typedef struct Contacts
{
    char Email[MAX_LENGTH];
    char Number[4][MAX_LENGTH];
}Contacts;

typedef struct Job
{
char Post[MAX_LENGTH];
char Place_work[MAX_LENGTH];
}Job;

typedef struct Person 
{
    int BtreeValue;
    char Name[MAX_LENGTH];
    char LastName[MAX_LENGTH];
    Job Work;
    Contacts ContactInfo;
    Link Profile; 

} Person;

typedef struct btree
{
    Person value;
    struct btree *right , *left;

}btree;

void menu();
Person EnterData();
void Ins_Btree(Person val, btree **q);
btree* balanceTree(btree *root);
void Print_Btree(btree *root, int level);
Person *findByLastName(btree *root, const char *lastName);
int Delete(char key[], btree **node);
void Move(btree *root, int *num);
#endif