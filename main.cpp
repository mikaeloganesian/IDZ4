#include <iostream>
#include <pthread.h>
#include <cstdlib>
#include <ctime>
#include <unistd.h>

// Структура для передачи данных между потоками
struct ThreadData {
    int item_value; // Рыночная стоимость добычи
    bool item_ready; // Флаг, указывающий, что предмет готов
    pthread_mutex_t mutex; // Мьютекс для синхронизации
    pthread_cond_t cond_ivanov; // Условные переменные для синхронизации
    pthread_cond_t cond_petrov;
    pthread_cond_t cond_necheporuk;
};

// Функция для генерации случайного числа в заданном диапазоне
int random_in_range(int min, int max) {
    return min + rand() % (max - min + 1);
}

// Функция для прапорщика Иванова
void* ivanov(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    std::cout << "Иванов начал работу." << std::endl;

    for (int i = 0; i < 5; ++i) { // Моделируем вынос 5 партий имущества
        // Случайное время выполнения задачи
        int time = random_in_range(1, 3);
        sleep(time);

        // Захватываем мьютекс и обновляем данные
        pthread_mutex_lock(&data->mutex);
        data->item_value = random_in_range(100, 500); // Случайная стоимость предмета
        data->item_ready = true;
        std::cout << "Иванов вынес партию имущества стоимостью " << data->item_value << " руб." << std::endl;

        // Сигнализируем Петрову, что предмет готов
        pthread_cond_signal(&data->cond_petrov);
        pthread_mutex_unlock(&data->mutex);
    }

    std::cout << "Иванов закончил работу." << std::endl;
    pthread_exit(NULL);
}

// Функция для прапорщика Петрова
void* petrov(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    std::cout << "Петров начал работу." << std::endl;

    for (int i = 0; i < 5; ++i) { // Моделируем погрузку 5 партий имущества
        // Захватываем мьютекс и ждем, пока Иванов вынесет предмет
        pthread_mutex_lock(&data->mutex);
        while (!data->item_ready) {
            pthread_cond_wait(&data->cond_petrov, &data->mutex);
        }

        // Случайное время выполнения задачи
        int time = random_in_range(1, 3);
        sleep(time);

        std::cout << "Петров погрузил партию имущества стоимостью " << data->item_value << " руб." << std::endl;
        data->item_ready = false;

        // Сигнализируем Нечепоруку, что предмет погружен
        pthread_cond_signal(&data->cond_necheporuk);
        pthread_mutex_unlock(&data->mutex);
    }

    std::cout << "Петров закончил работу." << std::endl;
    pthread_exit(NULL);
}

// Функция для прапорщика Нечепорука
void* necheporuk(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    std::cout << "Нечепорук начал работу." << std::endl;

    int total_value = 0;

    for (int i = 0; i < 5; ++i) { // Моделируем подсчет стоимости 5 партий имущества
        // Захватываем мьютекс и ждем, пока Петров погрузит предмет
        pthread_mutex_lock(&data->mutex);
        while (data->item_ready) {
            pthread_cond_wait(&data->cond_necheporuk, &data->mutex);
        }

        // Случайное время выполнения задачи
        int time = random_in_range(1, 3);
        sleep(time);

        total_value += data->item_value;
        std::cout << "Нечепорук подсчитал, что общая стоимость добычи: " << total_value << " руб." << std::endl;

        pthread_mutex_unlock(&data->mutex);
    }

    std::cout << "Нечепорук закончил работу." << std::endl;
    pthread_exit(NULL);
}

int main() {
    srand(time(NULL)); // Инициализация генератора случайных чисел

    // Инициализация структуры данных
    ThreadData data;
    data.item_ready = false;
    pthread_mutex_init(&data.mutex, NULL);
    pthread_cond_init(&data.cond_ivanov, NULL);
    pthread_cond_init(&data.cond_petrov, NULL);
    pthread_cond_init(&data.cond_necheporuk, NULL);

    // Создание потоков
    pthread_t thread_ivanov, thread_petrov, thread_necheporuk;
    pthread_create(&thread_ivanov, NULL, ivanov, (void*)&data);
    pthread_create(&thread_petrov, NULL, petrov, (void*)&data);
    pthread_create(&thread_necheporuk, NULL, necheporuk, (void*)&data);

    // Ожидание завершения потоков
    pthread_join(thread_ivanov, NULL);
    pthread_join(thread_petrov, NULL);
    pthread_join(thread_necheporuk, NULL);

    // Уничтожение мьютексов и условных переменных
    pthread_mutex_destroy(&data.mutex);
    pthread_cond_destroy(&data.cond_ivanov);
    pthread_cond_destroy(&data.cond_petrov);
    pthread_cond_destroy(&data.cond_necheporuk);

    std::cout << "Программа завершена." << std::endl;
    return 0;
}