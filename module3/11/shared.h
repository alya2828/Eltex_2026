#ifndef SHARED_H
#define SHARED_H

#define MAX_NUMS 100

typedef struct {
    int numbers[MAX_NUMS];
    int count;

    int min;
    int max;

    int ready;// флан данные готовы записал значение
    int done;// флаг обработано, ребенок вычислил мин и макс

    int stop;
} SharedData;

#endif
