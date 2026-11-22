#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <locale.h>
#include <math.h>

#define MAX_POINTS 100
#define TEST_POINTS 5
#define DEFAULT_K 3
#define DEFAULT_L 2.0f
#define EPS 1e-6f
#define PERCENT_SCALE 100.0f
#define HALF_DIVISOR 2.0f
#define INCIDENT_TYPE_COMBO 1
#define INPUT_TRIPLE 3

#define STATUS_OK 0
#define STATUS_NO_HISTORY 1
#define STATUS_DIV_ZERO 2
#define STATUS_INVALID 3

#define STATE_NORMAL 0
#define STATE_ALARM 1
#define STATE_ERROR -1

#define MENU_LOAD_TEST 1
#define MENU_MANUAL_INPUT 2
#define MENU_SETTINGS 3
#define MENU_ANALYZE 4
#define MENU_INCIDENTS 5
#define MENU_EXIT 0
#define INVALID_CHOICE -1

typedef struct {
    int time;
    int type;
    int count;
} Incident;

int validate_pair(float x_curr, float x_prev);
float calc_q(float x_curr, float x_prev);
int check_state(float q, float limit, int calc_status);
int check_incident(int state_web, int state_dbs, int state_iop);
int analyze_component(float x[], float q[], int status[], int state[], int n, int k, float limit);
int collect_incidents(int state_web[], int state_dbs[], int state_iop[], int n, Incident incidents[]);
int load_test_data(float web[], float dbs[], float iop[]);
int input_series(float web[], float dbs[], float iop[], int max_points);
void print_telemetry(float web[], float dbs[], float iop[], int n);
void print_component_report(const char name[], float x[], float q[], int status[], int state[], int n);
void print_incidents(Incident incidents[], int inc_count);

int main(void) {
    float web[MAX_POINTS], dbs[MAX_POINTS], iop[MAX_POINTS];
    float q_web[MAX_POINTS], q_dbs[MAX_POINTS], q_iop[MAX_POINTS];
    int st_web[MAX_POINTS], st_dbs[MAX_POINTS], st_iop[MAX_POINTS];
    int state_web[MAX_POINTS], state_dbs[MAX_POINTS], state_iop[MAX_POINTS];
    Incident incidents[MAX_POINTS];
    int n, k, has_data, is_calculated, inc_count, err_count;
    int choice, new_n, new_k, ch;
    float limit, new_l;

    setlocale(LC_CTYPE, "RUS");

    n = 0;
    k = DEFAULT_K;
    limit = DEFAULT_L;
    has_data = 0;
    is_calculated = 0;
    inc_count = 0;
    err_count = 0;

    printf("Программа контроля облачной инфраструктуры (Вариант 3, КТ2).\n");
    printf("Выполняет расчет показателя Q для WEB, DBS, IOP и выявление инцидентов.\n\n");

    do {
        printf("Текущий режим: отсчетов n = %d, шаг k = %d, порог L = %.2f%%\n", n, k, limit);
        printf("Меню режима работы:\n");
        printf("1 - Загрузить тестовый набор варианта 3 (5 отсчетов)\n");
        printf("2 - Ввести временной ряд вручную\n");
        printf("3 - Настроить параметры анализа (шаг k и порог L)\n");
        printf("4 - Выполнить расчет и вывести результаты анализа\n");
        printf("5 - Показать журнал инцидентов\n");
        printf("0 - Выход\n");
        printf("Ваш выбор: ");

        if (scanf("%d", &choice) != 1) {
            while ((ch = getchar()) != '\n' && ch != EOF);
            choice = INVALID_CHOICE;
        }

        switch (choice) {
        case MENU_LOAD_TEST:
            n = load_test_data(web, dbs, iop);
            has_data = 1;
            is_calculated = 0;
            printf("\nЗагружен тестовый набор варианта 3 (%d отсчетов).\n", n);
            print_telemetry(web, dbs, iop, n);
            break;

        case MENU_MANUAL_INPUT:
            new_n = input_series(web, dbs, iop, MAX_POINTS);
            if (new_n > 0) {
                n = new_n;
                has_data = 1;
                is_calculated = 0;
                printf("\nВведено отсчетов: %d.\n", n);
                print_telemetry(web, dbs, iop, n);
            }
            break;

        case MENU_SETTINGS:
            printf("\nТекущие параметры: шаг k = %d, порог L = %.2f%%\n", k, limit);
            printf("Введите новый шаг сравнения k (от 1 до %d): ", MAX_POINTS - 1);
            if (scanf("%d", &new_k) != 1 || new_k < 1 || new_k >= MAX_POINTS) {
                while ((ch = getchar()) != '\n' && ch != EOF);
                printf("Ошибка: шаг k должен быть целым числом от 1 до %d.\n\n", MAX_POINTS - 1);
                break;
            }
            printf("Введите новый порог L (%%, больше 0): ");
            if (scanf("%f", &new_l) != 1 || !isfinite(new_l) || new_l <= 0.0f) {
                while ((ch = getchar()) != '\n' && ch != EOF);
                printf("Ошибка: порог L должен быть положительным числом.\n\n");
                break;
            }
            k = new_k;
            limit = new_l;
            is_calculated = 0;
            printf("Параметры обновлены: k = %d, L = %.2f%%\n", k, limit);
            if (has_data && k >= n) {
                printf("Предупреждение: при k >= n (%d >= %d) для всех отсчетов не хватит предыстории.\n", k, n);
            }
            printf("\n");
            break;

        case MENU_ANALYZE:
            if (!has_data) {
                printf("\nДанные телеметрии не загружены. Выберите пункт 1 или 2.\n\n");
                break;
            }
            err_count = 0;
            err_count += analyze_component(web, q_web, st_web, state_web, n, k, limit);
            err_count += analyze_component(dbs, q_dbs, st_dbs, state_dbs, n, k, limit);
            err_count += analyze_component(iop, q_iop, st_iop, state_iop, n, k, limit);
            inc_count = collect_incidents(state_web, state_dbs, state_iop, n, incidents);
            is_calculated = 1;

            printf("\nРезультаты анализа (отсчетов: %d, шаг k = %d, порог L = %.2f%%):\n", n, k, limit);
            print_component_report("WEB", web, q_web, st_web, state_web, n);
            print_component_report("DBS", dbs, q_dbs, st_dbs, state_dbs, n);
            print_component_report("IOP", iop, q_iop, st_iop, state_iop, n);
            printf("\nОшибок вычисления формулы: %d\n", err_count);
            print_incidents(incidents, inc_count);
            printf("\n");
            break;

        case MENU_INCIDENTS:
            if (!has_data) {
                printf("\nДанные телеметрии не загружены. Выберите пункт 1 или 2.\n\n");
                break;
            }
            if (!is_calculated) {
                printf("\nСначала выполните расчет анализа (пункт 4).\n\n");
                break;
            }
            printf("\n");
            print_incidents(incidents, inc_count);
            printf("\n");
            break;

        case MENU_EXIT:
            printf("Завершение работы программы.\n");
            break;

        default:
            printf("\nНеверный пункт меню. Повторите выбор.\n\n");
            break;
        }
    } while (choice != MENU_EXIT);

    return 0;
}

int validate_pair(float x_curr, float x_prev) {
    float denom;

    if (!isfinite(x_curr) || !isfinite(x_prev)) {
        return STATUS_INVALID;
    }

    denom = (fabsf(x_curr) + fabsf(x_prev)) / HALF_DIVISOR;
    if (denom < EPS) {
        return STATUS_DIV_ZERO;
    }

    return STATUS_OK;
}

float calc_q(float x_curr, float x_prev) {
    float denom;

    if (validate_pair(x_curr, x_prev) != STATUS_OK) {
        return 0.0f;
    }

    denom = (fabsf(x_curr) + fabsf(x_prev)) / HALF_DIVISOR;
    return PERCENT_SCALE * (x_curr - x_prev) / denom;
}

int check_state(float q, float limit, int calc_status) {
    if (calc_status == STATUS_NO_HISTORY) {
        return STATE_NORMAL;
    }
    if (calc_status != STATUS_OK) {
        return STATE_ERROR;
    }
    if (q < -limit || q > limit) {
        return STATE_ALARM;
    }
    return STATE_NORMAL;
}

int check_incident(int state_web, int state_dbs, int state_iop) {
    if (state_web == STATE_ALARM && (state_dbs == STATE_ALARM || state_iop == STATE_ALARM)) {
        return 1;
    }
    return 0;
}

int analyze_component(float x[], float q[], int status[], int state[], int n, int k, float limit) {
    int i, err_count;

    err_count = 0;

    for (i = 0; i < n; i++) {
        if (i < k) {
            status[i] = STATUS_NO_HISTORY;
            q[i] = 0.0f;
            state[i] = STATE_NORMAL;
        } else {
            status[i] = validate_pair(x[i], x[i - k]);
            if (status[i] == STATUS_OK) {
                q[i] = calc_q(x[i], x[i - k]);
                if (!isfinite(q[i])) {
                    status[i] = STATUS_INVALID;
                    q[i] = 0.0f;
                }
            } else {
                q[i] = 0.0f;
            }

            state[i] = check_state(q[i], limit, status[i]);
            if (status[i] == STATUS_DIV_ZERO || status[i] == STATUS_INVALID) {
                err_count++;
            }
        }
    }

    return err_count;
}

int collect_incidents(int state_web[], int state_dbs[], int state_iop[], int n, Incident incidents[]) {
    int i, inc_count;

    inc_count = 0;

    for (i = 0; i < n; i++) {
        if (check_incident(state_web[i], state_dbs[i], state_iop[i])) {
            incidents[inc_count].time = i + 1;
            incidents[inc_count].type = INCIDENT_TYPE_COMBO;
            incidents[inc_count].count =
                (state_web[i] == STATE_ALARM) +
                (state_dbs[i] == STATE_ALARM) +
                (state_iop[i] == STATE_ALARM);
            inc_count++;
        }
    }

    return inc_count;
}

int load_test_data(float web[], float dbs[], float iop[]) {
    float test_web[TEST_POINTS] = { 45.5f, 46.0f, 46.5f, 47.0f, 52.9f };
    float test_dbs[TEST_POINTS] = { 55.6f, 53.6f, 57.0f, 55.0f, 53.0f };
    float test_iop[TEST_POINTS] = { 33.9f, 33.1f, 32.4f, 31.6f, 30.9f };
    int i;

    for (i = 0; i < TEST_POINTS; i++) {
        web[i] = test_web[i];
        dbs[i] = test_dbs[i];
        iop[i] = test_iop[i];
    }

    return TEST_POINTS;
}

int input_series(float web[], float dbs[], float iop[], int max_points) {
    int count, i, ch;
    float w, d, p;

    printf("\nВведите количество отсчетов (от 1 до %d): ", max_points);
    if (scanf("%d", &count) != 1 || count < 1 || count > max_points) {
        while ((ch = getchar()) != '\n' && ch != EOF);
        printf("Ошибка: количество отсчетов должно быть от 1 до %d.\n\n", max_points);
        return 0;
    }

    i = 0;
    while (i < count) {
        printf("Отсчет %d (WEB DBS IOP): ", i + 1);
        if (scanf("%f %f %f", &w, &d, &p) != INPUT_TRIPLE) {
            while ((ch = getchar()) != '\n' && ch != EOF);
            printf("Ошибка ввода. Введите три числа для отсчета %d.\n", i + 1);
        } else {
            web[i] = w;
            dbs[i] = d;
            iop[i] = p;
            i++;
        }
    }

    return count;
}

void print_telemetry(float web[], float dbs[], float iop[], int n) {
    int i;

    printf("\nИсходные данные телеметрии:\n");
    printf("%-7s %-10s %-10s %-10s\n", "Отсчет", "WEB", "DBS", "IOP");
    for (i = 0; i < n; i++) {
        printf("%-7d %-10.2f %-10.2f %-10.2f\n", i + 1, web[i], dbs[i], iop[i]);
    }
    printf("\n");
}

void print_component_report(const char name[], float x[], float q[], int status[], int state[], int n) {
    int i;

    printf("\nКомпонент %s:\n", name);
    printf("%-7s %-10s %-14s %-12s\n", "Отсчет", "Значение", "Q (%)", "Состояние");
    for (i = 0; i < n; i++) {
        printf("%-7d %-10.2f ", i + 1, x[i]);
        if (status[i] == STATUS_NO_HISTORY) {
            printf("%-14s ", "--- (i < k)");
        } else if (status[i] == STATUS_DIV_ZERO) {
            printf("%-14s ", "ОШИБКА (/0)");
        } else if (status[i] == STATUS_INVALID) {
            printf("%-14s ", "ОШИБКА (число)");
        } else {
            printf("%-+14.2f ", q[i]);
        }

        if (state[i] == STATE_ALARM) {
            printf("ТРЕВОГА\n");
        } else if (state[i] == STATE_ERROR) {
            printf("ОШИБКА\n");
        } else {
            printf("НОРМА\n");
        }
    }
}

void print_incidents(Incident incidents[], int inc_count) {
    int i;

    printf("Журнал инцидентов (правило: WEB в ТРЕВОГА и хотя бы один другой в ТРЕВОГА):\n");
    if (inc_count == 0) {
        printf("Инцидентов не обнаружено.\n");
    } else {
        for (i = 0; i < inc_count; i++) {
            printf("  Отсчет %d: инцидент (тип %d, проблемных узлов: %d)\n",
                incidents[i].time, incidents[i].type, incidents[i].count);
        }
    }
}
