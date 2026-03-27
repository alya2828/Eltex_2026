#include "tree_contacts.h"

int main()
{
    menu();

    return 0;
}

void menu()
{
    btree *root = NULL;
    int x;
    static int insert_count = 0;

    while (1)
    {
        printf("\n Меню: \n");
        printf("1 - Добавить контакт\n");
        printf("2 - Показать дерево\n");
        printf("3 - Удалить контакт\n");
        printf("4 - Изменить контакт\n");
        printf("5 - Сбалансировать дерево\n");
        printf("6 - Выход\n");
        printf("Выберите действие: ");

        scanf("%d", &x);
        while(getchar() != '\n'); // очистка буфера

        switch (x)
        {
        case 1:
        {
            Person p = EnterData();
            Ins_Btree(p, &root); // добавление контакта 
            insert_count++;

            // периодическая балансировка (например каждые 5 вставок)
            if (insert_count % 2 == 0)
            {
                root = balanceTree(root);
                printf("Дерево автоматически сбалансировано!\n");
            }
            break;
        }

        case 2:
            printf("\nСтруктура дерева:\n");
            Print_Btree(root, 0);
            break;

        case 3:
        {
            char lastName[MAX_LENGTH];
            printf("Введите фамилию для удаления: ");
            fgets(lastName, MAX_LENGTH, stdin);

            size_t len = strlen(lastName);
            if (len > 0 && lastName[len - 1] == '\n')
                lastName[len - 1] = '\0';

            Delete(lastName, &root);
            break;
        }

        case 4:
            Move(root, NULL);
            break;

        case 5:
            root = balanceTree(root);
            printf("Дерево сбалансировано вручную!\n");
            break;

        case 6:
            return;

        default:
            printf("Некорректный выбор\n");
        }
    }
}
