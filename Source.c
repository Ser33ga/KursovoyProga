#define _CRT_SECURE_NO_WARNINGS
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

float calc_q(float x_curr, float x_prev);
int check_state(float q, float limit);
int check_incident(int state_web, int state_dbs, int state_iop);

int main(void) {
    float web, dbs, iop;
    float web1 = 0.0f, web2 = 0.0f, web3 = 0.0f;
    float dbs1 = 0.0f, dbs2 = 0.0f, dbs3 = 0.0f;
    float iop1 = 0.0f, iop2 = 0.0f, iop3 = 0.0f;
    float q_web, q_dbs, q_iop;
    int state_web, state_dbs, state_iop;
    int prob_count;
    int is_incident;
    int step = 0;
    Incident inc;

    setlocale(LC_ALL, "Russian");

    printf("Контроль облачной инфраструктуры (Вариант 3, КТ1)\n\n");

    while (step < POINTS_COUNT) {
        printf("Отсчет %d - введите WEB DBS IOP: ", step + 1);

        if (scanf("%f", &web) != 1 || web == -99999.0f) {
            break;
        }
        if (scanf("%f", &dbs) != 1 || dbs == -99999.0f) {
            break;
        }
        if (scanf("%f", &iop) != 1 || iop == -99999.0f) {
            break;
        }

        step++;

        if (step == 1) {
            web1 = web;
            dbs1 = dbs;
            iop1 = iop;
            printf("Отсчет %d: накопление окна (1/%d). НОРМА. Инцидентов нет.\n\n", step, K_PARAM);
        } else if (step == 2) {
            web2 = web;
            dbs2 = dbs;
            iop2 = iop;
            printf("Отсчет %d: накопление окна (2/%d). НОРМА. Инцидентов нет.\n\n", step, K_PARAM);
        } else if (step == 3) {
            web3 = web;
            dbs3 = dbs;
            iop3 = iop;
            printf("Отсчет %d: окно заполнено (3/%d). НОРМА. Инцидентов нет.\n\n", step, K_PARAM);
        } else {
            q_web = calc_q(web, web1);
            q_dbs = calc_q(dbs, dbs1);
            q_iop = calc_q(iop, iop1);

            state_web = check_state(q_web, L_LIMIT);
            state_dbs = check_state(q_dbs, L_LIMIT);
            state_iop = check_state(q_iop, L_LIMIT);

            is_incident = check_incident(state_web, state_dbs, state_iop);
            prob_count = state_web + state_dbs + state_iop;

            printf("Отсчет %d:\n", step);
            printf("  WEB: %.1f, Q = %+6.2f%% (%s)\n", web, q_web, state_web ? "ТРЕВОГА" : "НОРМА");
            printf("  DBS: %.1f, Q = %+6.2f%% (%s)\n", dbs, q_dbs, state_dbs ? "ТРЕВОГА" : "НОРМА");
            printf("  IOP: %.1f, Q = %+6.2f%% (%s)\n", iop, q_iop, state_iop ? "ТРЕВОГА" : "НОРМА");

            if (is_incident) {
                inc.time = step;
                inc.type = 1;
                inc.count = prob_count;
                printf("  Инцидент зафиксирован (тип %d, проблемных узлов: %d)\n\n", inc.type, inc.count);
            } else {
                printf("  Инцидентов нет.\n\n");
            }

            web1 = web2;
            web2 = web3;
            web3 = web;

            dbs1 = dbs2;
            dbs2 = dbs3;
            dbs3 = dbs;

            iop1 = iop2;
            iop2 = iop3;
            iop3 = iop;
        }
    }

    return 0;
}

float calc_q(float x_curr, float x_prev) {
    float denom = (fabsf(x_curr) + fabsf(x_prev)) / 2.0f;
    if (denom < 1e-6f) {
        return 0.0f;
    }
    return 100.0f * (x_curr - x_prev) / denom;
}

int check_state(float q, float limit) {
    if (q < -limit || q > limit) {
        return 1;
    }
    return 0;
}

int check_incident(int state_web, int state_dbs, int state_iop) {
    if (state_web == 1 && (state_dbs == 1 || state_iop == 1)) {
        return 1;
    }
    return 0;
}
