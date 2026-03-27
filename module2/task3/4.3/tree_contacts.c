#include "tree_contacts.h"

Person EnterData()
{

    Person person;
    printf("Введите имя:");
    fgets(person.Name, MAX_LENGTH, stdin);
    while (strlen(person.Name) <= 1)
    {
        printf("Введите имя:");
        fgets(person.Name, MAX_LENGTH, stdin);
    }

    printf("Введите фамилию: ");
    fgets(person.LastName, MAX_LENGTH, stdin);
    while (strlen(person.LastName) <= 1)
    {
        printf("Введите фамилию: ");
        fgets(person.LastName, MAX_LENGTH, stdin);
    }

    printf("Введите должность:");
    fgets(person.Work.Post, MAX_LENGTH, stdin);
    printf("Введите место работы: ");
    fgets(person.Work.Place_work, MAX_LENGTH, stdin);

    printf("Введите Email: ");
    fgets(person.ContactInfo.Email, MAX_LENGTH, stdin);

    int count = 0;
    char buf[MAX_LENGTH];
    while (count < MAX)
    {
        printf("Введите номер телефона %d ", count + 1);
        getchar();
        fgets(buf, MAX_LENGTH, stdin);

        if (strlen(buf) < 2)
        {
            buf[0] = '\0';
            strcpy(person.ContactInfo.Number[count], buf);
            break;
        }

        if (strlen(buf) < 10)
        {
            printf("Неправильно записан номер\n");
            buf[0] = '\0';

            continue;
        }
        strcpy(person.ContactInfo.Number[count], buf);

        count++;
    }

    count = 0;
    while (count < MAX)
    {
        printf("Введите ссылки на профиль %d ", count + 1);

        fgets(buf, MAX_LENGTH, stdin);
        if (strlen(buf) < 2)
        {
            buf[0] = '\0';
            strcpy(person.Profile.Social_networks[count], buf);
            break;
        }
        strcpy(person.Profile.Social_networks[count], buf);
        count++;
    }

    count = 0;
    while (count < MAX)
    {
        printf("Введите профили на мессенджеры %d ", count + 1);
        fgets(buf, MAX_LENGTH, stdin);
        if (strlen(buf) < 2)
        {
            buf[0] = '\0';

            strcpy(person.Profile.Profiles_messengers[count], buf);
            break;
        }
        strcpy(person.Profile.Profiles_messengers[count], buf);

        count++;
    }
    person.BtreeValue = strlen(person.LastName);
    return person;
}

void Ins_Btree(Person val, btree **q)
{

    if (*q == NULL)
    {
        *q = malloc(sizeof(btree));
        (*q)->left = (*q)->right = NULL;
        (*q)->value = val;
        return;
    }
    if ((*q)->value.BtreeValue > val.BtreeValue)
        Ins_Btree(val, &(*q)->left);
    else
        Ins_Btree(val, &(*q)->right);
}

// Считаем количество узлов
int countNodes(btree *root)
{
    if (!root)
        return 0;
    return 1 + countNodes(root->left) + countNodes(root->right);
}

// Заполняем массив узлов через inorder обход
void storeInorder(btree *root, Person arr[], int *index)
{
    if (!root)
        return;
    storeInorder(root->left, arr, index);
    arr[*index] = root->value;
    (*index)++;
    storeInorder(root->right, arr, index);
}

// Строим сбалансированное дерево из массива
btree *buildBalanced(Person arr[], int start, int end)
{
    if (start > end)
        return NULL;

    int mid = (start + end) / 2;
    btree *node = malloc(sizeof(btree));
    node->value = arr[mid];
    node->left = buildBalanced(arr, start, mid - 1);
    node->right = buildBalanced(arr, mid + 1, end);
    return node;
}

// Периодическая балансировка дерева
btree *balanceTree(btree *root)
{
    int n = countNodes(root);//сколько узлов в дереве
    if (n == 0)
        return NULL;

    Person *arr = malloc(n * sizeof(Person));
    int index = 0;
    storeInorder(root, arr, &index);//переносим дерево в массив,-лево-корень-право

    btree *newRoot = buildBalanced(arr, 0, n - 1);//середина дерева
    free(arr);
    return newRoot;
}

// Проверка
void inorderPrint(btree *root)//сначало лево-корень-право
{
    if (!root)
        return;
    inorderPrint(root->left);
    printf("%d ", root->value.BtreeValue);
    inorderPrint(root->right);
}

int Delete(char key[], btree **node)
{
    btree *t, *up;
    if (*node == NULL)
        return 0; // нет такого значения в дереве

    if (strcmp((*node)->value.LastName, key) == 0)
    {
        // если значение находится в листе, то удаляем лист
        if (((*node)->left == NULL) && ((*node)->right == NULL))
        {
            free(*node);
            *node = NULL;
            printf("Delete List\n");
            return 1;
        }
        // если у вершины только правый потомок, то перебрасываем
        // связь на вершину ниже удаляемой в правом поддереве
        if ((*node)->left == NULL)
        {
            t = *node;
            *node = (*node)->right;
            free(t);
            printf("Delete Left = 0\n");
            return 1;
        }
        // если у вершины только левый потомок, то перебрасываем
        // связь на вершину ниже удаляемой в левом поддереве
        if ((*node)->right == NULL)
        {
            t = *node;
            *node = (*node)->left;
            free(t);
            printf("Delete Right = 0\n");
            return 1;
        }
        // если у вершины два потомка, то заменяем удаляемое значение
        // на значение самого правого элемента в левом поддереве
        up = *node;
        t = (*node)->left; // идем в левое поддерево
        // спускаемся до крайнего правого узла
        while (t->right != NULL)
        {
            up = t;
            t = t->right;
        }
        // копируем значение из правого узла вместо удаляемого значения
        (*node)->value = t->value;
        // если ниже удаляемого больше, чем одна вершина
        if (up != (*node))
        {
            // если крайний правый не лист, то «отбрасываем» его из дерева
            if (t->left != NULL)
                up->right = t->left;
            else
                up->right = NULL; // удаляем лист
        }
        // если ниже удаляемого одна вершина, удаляем лист
        else
            (*node)->left = t->left;
        // освобождаем память – удаляем крайнюю
        // правую вершину
        free(t);
        printf("Delete Two\n");
        return 1;
    }
    // поиск ключа в левом или правом поддереве
    if ((*node)->value.BtreeValue < strlen(key))
        return Delete(key, &(*node)->right);

    return Delete(key, &(*node)->left);
}

void Move(btree *root, int *num)
{
    int x;
    int p;
    int index;
    char lastName[MAX_LENGTH];
    char buffer[MAX_LENGTH];

    Person *contact = NULL;
    // введите фамилию контакта который нужно изменить
    do
    {
        printf("Введите фамилию контакта, который необходимо изменить: ");
        fgets(lastName, MAX_LENGTH, stdin);

        size_t len = strlen(lastName);
        if (len > 0 && lastName[len - 1] == '\n')
            lastName[len - 1] = '\0';
        contact = findByLastName(root, lastName);
        if (contact != NULL)
        {
            break;
        }
        printf("Упс, такого контакта нет! Попробуй еще раз n");
        scanf("%d", &x);
        getchar();
    } while (contact == NULL);

    printf("Введите, что необхдимо изменить\n 1 - Имя\n 2 - Фамилия\n3-номер");
    scanf("%d", &p);
    getchar();

    index = x - 1;

    if (p == 1)
    {
        printf("Введите имя: ");
        fgets(contact->Name, sizeof(contact->Name), stdin);
        size_t len = strlen(contact->Name);
        if (len > 0 && contact->Name[len - 1] == '\n')
            contact->Name[len - 1] = '\0';
        printf("Имя измененно!");
    }
    if (p == 2) // фамилия
    {
        printf("Введите фамилию: ");
        fgets(contact->LastName, sizeof(contact->LastName), stdin);

        size_t len = strlen(contact->LastName);
        if (len > 0 && contact->LastName[len - 1] == '\n')
            contact->LastName[len - 1] = '\0';

        if (strlen(contact->LastName) != contact->BtreeValue)
        {
            contact->BtreeValue = strlen(contact->LastName);
            root = balanceTree(root);
        }

        printf("Фамилия успешно изменена!\n");
    }

    if (p == 3)
    {
        int numIndex;

        // спрашиваем у пользователя, какой номер изменить
        do
        {
            printf("Какой номер хотите изменить? (1-4): ");
            scanf("%d", &numIndex);
            getchar(); // очищаем буфер после scanf
            if (numIndex >= 1 && numIndex <= 4)
                break;
            printf("Ошибка! Введите число от 1 до 4.\n");
        } while (1);

        printf("Введите новый номер: ");
        fgets(contact->ContactInfo.Number[numIndex - 1], MAX_LENGTH, stdin);

        // убираем символ новой строки
        size_t len = strlen(contact->ContactInfo.Number[numIndex - 1]);
        if (len > 0 && contact->ContactInfo.Number[numIndex - 1][len - 1] == '\n')
            contact->ContactInfo.Number[numIndex - 1][len - 1] = '\0';

        printf("Номер успешно изменён!\n");
    }
}

Person *findByLastName(btree *root, const char *lastName)
{
    if (!root)
        return NULL;

    // Сначала проверяем текущий узел
    if (strcmp(root->value.LastName, lastName) == 0)
        return &root->value;

    // Проверяем левое поддерево
    Person *found = findByLastName(root->left, lastName);
    if (found)
        return found;

    // Проверяем правое поддерево
    return findByLastName(root->right, lastName);
}

/*void Print_Btree(btree *root, int space)
{
    if (root == NULL)
        return;
    space += 5;
    Print_Btree(root->right, space);
    // печатаем текущий узел
    printf("\n");
    for (int i = 5; i < space; i++)
        printf(" ");

    printf("%s (%d)\n", root->value.LastName, root->value.BtreeValue);
    Print_Btree(root->left, space);
}*/
void Print_Btree(btree *root, int level)
{
    if (root == NULL) return;

    Print_Btree(root->right, level + 1);

    for (int i = 0; i < level; i++)
        printf("    ");

    printf("-> %s (%d)\n",
           root->value.LastName,
           root->value.BtreeValue);

    Print_Btree(root->left, level + 1);
}