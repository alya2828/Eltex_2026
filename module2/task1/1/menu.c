#include "contacts.h"

int main()
{
    menu();

    return 0;
}

void menu()
{
    Person contact[CONTACTS_SIZE];
    int num = 0;
    int x;

    while (1)
    {
        printf("\n Меню: \n");
        printf("1 - Добавить контакт\n");
        printf("2 - Вывести контакты\n");
        printf("3 - Удалить контакт\n");
        printf("4 - Изменить контакт\n");
        printf("5 - Выход\n");
        printf("Выберите действие: ");
        scanf("%d", &x);
        getchar();

        switch (x)
        {
        case 1:
            Add(contact, &num);
            break;
        case 2:
            Print(contact, &num);
            break;
        case 3:
            Delete(contact, &num);
            break;
        case 4:
            Move(contact, &num);
            break;
        case 5:
            return;
        default:
            printf("Некорректный выбор\n");
            break;
        }
    }
    return;
}
