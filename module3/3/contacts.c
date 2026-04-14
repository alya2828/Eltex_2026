#include "contacts.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#define FILE_NAME "contacts.dat"

// Открыть файл
int openFile(int flags) {
    int fd = open(FILE_NAME, flags, 0666); // права rw-rw-rw-
    if (fd < 0) {
        perror("Ошибка открытия файла");
    }
    return fd;
}

// Закрыть файл
void closeFile(int fd) {
    if (close(fd) < 0) {
        perror("Ошибка закрытия файла");
    }
}

// Считать контакты из файла
int readContacts(Person contact[], int maxContacts) {
    int fd = openFile(O_RDONLY);
    if (fd < 0) return 0;

    int count = 0;
    while (count < maxContacts && read(fd, &contact[count], sizeof(Person)) == sizeof(Person)) {
        count++;
    }

    closeFile(fd);
    return count;
}

// Записать контакты в файл
void writeContacts(Person contact[], int num) {
    int fd = openFile(O_WRONLY | O_CREAT | O_TRUNC);
    if (fd < 0) return;

    for (int i = 0; i < num; i++) {
        if (write(fd, &contact[i], sizeof(Person)) != sizeof(Person)) {
            perror("Ошибка записи контакта");
        }
    }

    closeFile(fd);
}

void Add(Person contact[], int *num)
{
      if (*num >= CONTACTS_SIZE) {
        printf("Достигнут лимит контактов!\n");
        return;
    }
    int count;
    printf("Введите имя:");
    fgets(contact[*num].Name, MAX_LENGTH, stdin);

    int x = strlen(contact[*num].Name);

    while (x <= 1)
    {
        printf("Введите имя: ");
        fgets(contact[*num].Name, MAX_LENGTH, stdin);
        x = strlen(contact[*num].Name);
    }

    do
    {
        printf("Введите фамилию: ");
        fgets(contact[*num].LastName, MAX_LENGTH, stdin);
        x = strlen(contact[*num].LastName);
    } while (x <= 1);
    printf("Введите должность:");
    fgets(contact[*num].Work.Post, MAX_LENGTH, stdin);
    printf("Введите место работы: ");
    fgets(contact[*num].Work.Place_work, MAX_LENGTH, stdin);

    printf("Введите Email: ");
    fgets(contact[*num].ContactInfo.Email, MAX_LENGTH, stdin);

    for (int i = 0; i < MAX; i++)
    {
        strcpy(contact[*num].ContactInfo.Number[i], "");

        strcpy(contact[*num].Profile.Social_networks[i],"");

        strcpy(contact[*num].Profile.Profiles_messengers[i],"");
    }

    count = 0;
    char buf[MAX_LENGTH];
    while (count < MAX)
    {
        printf("Введите номер телефона %d ", count + 1);
        fgets(buf, MAX_LENGTH, stdin);
        if (strlen(buf) < 2){
            break;        
        }
        if (strlen(buf) < 10){
            printf("Неправильно записан номер\n");
            continue;        
        }
        strcpy(contact[*num].ContactInfo.Number[count], buf);

        count++;
    }

    count = 0;
    while (count < MAX)
    {
        printf("Введите ссылки на профиль %d ", count + 1);

        fgets(buf, MAX_LENGTH, stdin);
        if (strlen(buf) < 2)
            break;
        strcpy(contact[*num].Profile.Social_networks[count], buf);
        count++;
    }

    count = 0;
    while (count < MAX)
    {
        printf("Введите профили на мессенджеры %d ", count + 1);
        fgets(buf, MAX_LENGTH, stdin);
        if (strlen(buf) < 2)
            break;
        strcpy(contact[*num].Profile.Profiles_messengers[count], buf);

        count++;
    }

    (*num)++;
}

void Delete(Person contact[], int *num)
{
        if (*num == 0) {
        printf("Список контактов пуст!\n");
        return;
    }
    printf("Укажите какой контакт удалить:");
    int x;
    scanf("%d", &x);
    getchar();

    // Перемещаем все контакты после удаленного на одну позицию вперед
    for (int i = x - 1; i < *num - 1; i++)
    {
        contact[i] = contact[i + 1];
    }

    (*num)--;

    printf("Контакт успешно удален\n");
}

void Print(Person contact[], int *num)
{

    printf("Список контактов:\n");
    for (int i = 0; i < *num; i++)
    {
        printf("Контакт %d\n\n", i + 1);
        printf("Имя: %s\n", contact[i].Name);
        printf("Фамилия: %s\n", contact[i].LastName);
        printf("Должность: %s\n", contact[i].Work.Post);
        printf("Место работы: %s\n", contact[i].Work.Place_work);
        printf("Email: %s\n", contact[i].ContactInfo.Email);

        for (int j = 0; j < MAX; j++)
        {
            if (strlen(contact[i].ContactInfo.Number[j]) < 2)
                break;
            printf("Номер телефона: %s\n", contact[i].ContactInfo.Number[j]);
        }
        
        for (int j = 0; j < MAX; j++)
        {
            if (strlen(contact[i].Profile.Social_networks[j]) < 2)
                break;
            printf("Ссылки на страницы: %s\n", contact[i].Profile.Social_networks[j]);
            
        }
        
        for (int j = 0; j < MAX; j++)
        {
            if (strlen(contact[i].Profile.Profiles_messengers[j]) < 2)
                break;
            printf("Профили в мессенджерах: %s\n", contact[i].Profile.Profiles_messengers[j]);
        }
    }
}

void Move(Person contact[], int *num)
{
    int x;
    int p;
    int index;

    printf("Введите контакт, который необходимо изменить:");
    scanf("%d", &x);
    getchar();
    if (x < 1 || x > *num)
    {
        printf("Упс, такого контакта нет! Попробуй еще раз n");
        scanf("%d", &x); 
        getchar();
    }
    printf("Введите, что необхдимо изменить\n 1 - Имя\n 2 - Фамилия\n");
    scanf("%d", &p);
    getchar();

    index = x - 1;

    if (p == 1)
    {
        printf("Введите имя: ");
        fgets(contact[index].Name, sizeof(contact[index].Name), stdin);
        printf("Имя измененно!");
    }

    if (p == 2)
    {
        printf("Введите фамилию: ");
        scanf("%s", contact[index].LastName);
        getchar();
        printf("Фамилия успешно изменина!");
    }
}