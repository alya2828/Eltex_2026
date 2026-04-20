#ifndef SHARED_H
#define SHARED_H

#define MAX_NUMS 100

typedef struct {
    int numbers[MAX_NUMS];
    int count;

    int min;
    int max;

    int ready;// флан данные готовы
    int done;// флаг обработано

    int stop;
} SharedData;

#endif