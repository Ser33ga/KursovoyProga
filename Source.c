#include <stdio.h>
#include <locale.h>
#include <math.h>

#define POINTS_COUNT 5
#define K_PARAM 3
#define L_LIMIT 2.0f

typedef struct {
    int time;
    int type;
    int count;
} Incident;

int main(void) {
    setlocale(LC_ALL, "Russian");

    float web[POINTS_COUNT] = { 45.5f, 46.0f, 46.5f, 47.0f, 52.9f };
    float dbs[POINTS_COUNT] = { 55.6f, 53.6f, 57.0f, 55.0f, 53.0f };
    float iop[POINTS_COUNT] = { 33.9f, 33.1f, 32.4f, 31.6f, 30.9f };

    float q_web[POINTS_COUNT] = { 0.0f };
    float q_dbs[POINTS_COUNT] = { 0.0f };
    float q_iop[POINTS_COUNT] = { 0.0f };

    int state_web[POINTS_COUNT] = { 0 };
    int state_dbs[POINTS_COUNT] = { 0 };
    int state_iop[POINTS_COUNT] = { 0 };

    Incident incidents[POINTS_COUNT];
    int incident_count = 0;

    int i;

    printf("Контроль облачной инфраструктуры (Вариант 3, КТ1)\n\n");

    printf("Исходные данные телеметрии:\n");
    printf("Отсчет\tWEB\tDBS\tIOP\n");
    for (i = 0; i < POINTS_COUNT; i++) {
        printf("%d\t%.1f\t%.1f\t%.1f\n", i + 1, web[i], dbs[i], iop[i]);
    }
    printf("\n");

    for (i = K_PARAM; i < POINTS_COUNT; i++) {
        q_web[i] = 100.0f * (web[i] - web[i - K_PARAM]) / ((fabsf(web[i]) + fabsf(web[i - K_PARAM])) / 2.0f);
        q_dbs[i] = 100.0f * (dbs[i] - dbs[i - K_PARAM]) / ((fabsf(dbs[i]) + fabsf(dbs[i - K_PARAM])) / 2.0f);
        q_iop[i] = 100.0f * (iop[i] - iop[i - K_PARAM]) / ((fabsf(iop[i]) + fabsf(iop[i - K_PARAM])) / 2.0f);
    }

    printf("Симметрированное относительное изменение Q (%%) при k = %d:\n", K_PARAM);
    printf("Отсчет\tQ(WEB)\t\tQ(DBS)\t\tQ(IOP)\n");
    for (i = K_PARAM; i < POINTS_COUNT; i++) {
        printf("%d\t%+7.2f%%\t%+7.2f%%\t%+7.2f%%\n", i + 1, q_web[i], q_dbs[i], q_iop[i]);
    }
    printf("\n");

    for (i = 0; i < POINTS_COUNT; i++) {
        int prob_count = 0;

        if (i >= K_PARAM) {
            if (q_web[i] < -L_LIMIT || q_web[i] > L_LIMIT) {
                state_web[i] = 1;
            }
            if (q_dbs[i] < -L_LIMIT || q_dbs[i] > L_LIMIT) {
                state_dbs[i] = 1;
            }
            if (q_iop[i] < -L_LIMIT || q_iop[i] > L_LIMIT) {
                state_iop[i] = 1;
            }
        }

        prob_count = state_web[i] + state_dbs[i] + state_iop[i];

        if (state_web[i] && (state_dbs[i] || state_iop[i])) {
            incidents[incident_count].time = i + 1;
            incidents[incident_count].type = 1;
            incidents[incident_count].count = prob_count;
            incident_count++;
        }
    }

    printf("Состояние компонентов (порог L = %.1f%%):\n", L_LIMIT);
    printf("Отсчет\tWEB\t\tDBS\t\tIOP\n");
    for (i = 0; i < POINTS_COUNT; i++) {
        printf("%d\t%s\t%s\t%s\n",
            i + 1,
            state_web[i] ? "ТРЕВОГА" : "НОРМА  ",
            state_dbs[i] ? "ТРЕВОГА" : "НОРМА  ",
            state_iop[i] ? "ТРЕВОГА" : "НОРМА  ");
    }
    printf("\n");

    printf("Журнал инцидентов (WEB в тревоге + сбой других узлов):\n");
    if (incident_count == 0) {
        printf("Инцидентов не обнаружено.\n");
    } else {
        for (i = 0; i < incident_count; i++) {
            printf("Отсчет %d: зафиксирован инцидент (тип %d, проблемных узлов: %d)\n",
                incidents[i].time, incidents[i].type, incidents[i].count);
        }
    }

    return 0;
}
