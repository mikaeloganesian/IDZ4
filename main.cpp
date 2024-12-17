/*
Оганесян Микаэл, БПИ-234. Вариант - 30
    Условие варианта:
         Темной-темной ночью прапорщики Иванов, Петров и Нечепорук
         занимаются хищением военного имущества со склада родной
         военной части. Будучи умными людьми и отличниками боевой
         и строевой подготовки, прапорщики ввели разделение труда.
         Иванов выносит имущество со склада и передает его в руки
         Петрову, который грузит его в грузовик. Нечепорук стоит
         на шухере и заодно подсчитывает рыночную стоимость добычи
         поле погрузки в грузовик очередной партии похищенного.
         Требуется составить многопоточное приложение, моделирующее
         деятельность прапорщиков–потоков. Необходимо учесть случайное
         время выполнения каждым прапорщиком своей боевой задачи
         и организовать в программе корректную их синхронизацию.
*/

// A file with the appropriate name is saved in the cmake-build-debug/ folder
// When entering the file name, enter with ".txt"
#include <iostream>
#include <pthread.h>
#include <unistd.h> // For sleep
#include <cstdlib> // For rand() and srand()
#include <ctime>   // For time()
#include <fstream> // For file handling

using namespace std;

// Global variables and synchronization
pthread_mutex_t mtx;          // Mutex for synchronizing access to shared data
pthread_cond_t item_ready;    // Condition variable for thread communication
bool is_item_ready = false;   // Flag indicating that an item is ready for transfer

int total_value = 0;          // Total value of loaded items
int item_counter = 0;         // Counter for loaded items
int TOTAL_ITEMS;              // Number of items to process

// Delay time ranges
int ivanov_min_time, ivanov_max_time;
int petrov_min_time, petrov_max_time;
int necheporuk_min_time, necheporuk_max_time;

// Item value range
int value_min, value_max;

// File stream for saving program results
ofstream result_file;

// Generate random delay time within a given range
int get_random_time(int min_time, int max_time) {
    return rand() % (max_time - min_time + 1) + min_time;
}

// Function for Ivanov's thread
void* Ivanov(void* arg) {
    for (int i = 0; i < TOTAL_ITEMS; i++) {
        sleep(get_random_time(ivanov_min_time, ivanov_max_time)); // Simulate item removal time
        pthread_mutex_lock(&mtx);   // Lock the mutex
        cout << "Ivanov: Removed item #" << i + 1 << " from the warehouse." << endl;
        result_file << "Ivanov: Removed item #" << i + 1 << " from the warehouse." << endl;

        is_item_ready = true;         // Set the item as ready for transfer
        pthread_cond_signal(&item_ready); // Signal Petrov that the item is ready

        pthread_mutex_unlock(&mtx); // Unlock the mutex
    }
    pthread_exit(nullptr);
}

// Function for Petrov's thread
void* Petrov(void* arg) {
    for (int i = 0; i < TOTAL_ITEMS; i++) {
        pthread_mutex_lock(&mtx);
        while (!is_item_ready) {  // Wait for Ivanov's signal
            pthread_cond_wait(&item_ready, &mtx);
        }

        sleep(get_random_time(petrov_min_time, petrov_max_time)); // Simulate item loading time
        cout << "Petrov: Loaded item #" << i + 1 << " into the truck." << endl;
        result_file << "Petrov: Loaded item #" << i + 1 << " into the truck." << endl;

        is_item_ready = false; // Reset the item ready flag
        item_counter++;        // Increment the loaded item counter
        pthread_mutex_unlock(&mtx);
    }
    pthread_exit(nullptr);
}

// Function for Necheporuk's thread
void* Necheporuk(void* arg) {
    for (int i = 0; i < TOTAL_ITEMS; i++) {
        pthread_mutex_lock(&mtx);

        // Calculate the market value of the loaded item
        int item_value = get_random_time(value_min, value_max);
        total_value += item_value;

        cout << "Necheporuk: Calculated the value of item #" << i + 1
             << ": " << item_value << " rubles." << endl;
        result_file << "Necheporuk: Calculated the value of item #" << i + 1
                    << ": " << item_value << " rubles." << endl;

        pthread_mutex_unlock(&mtx);
        sleep(get_random_time(necheporuk_min_time, necheporuk_max_time)); // Simulate time on lookout
    }

    pthread_exit(nullptr);
}

int main() {
    srand(time(0)); // Initialize random number generator
    string filename;
    cout << "Enter the filename to save the program's operation history: ";
    cin >> filename;
    if (filename.empty()) {
        filename = "results.txt";
    }

    // Open the file for writing results
    result_file.open(filename);
    if (!result_file) {
        cerr << "Error: Unable to open the file for writing results." << endl;
        return 1;
    }

    // User input for program configuration
    cout << "Enter the number of items: ";
    cin >> TOTAL_ITEMS;

    cout << "Enter Ivanov's work time range (minimum and maximum in seconds): ";
    cin >> ivanov_min_time >> ivanov_max_time;

    cout << "Enter Petrov's work time range (minimum and maximum in seconds): ";
    cin >> petrov_min_time >> petrov_max_time;

    cout << "Enter Necheporuk's work time range (minimum and maximum in seconds): ";
    cin >> necheporuk_min_time >> necheporuk_max_time;

    cout << "Enter the item value range (minimum and maximum in rubles): ";
    cin >> value_min >> value_max;

    // Initialize mutex and condition variable
    pthread_mutex_init(&mtx, nullptr);
    pthread_cond_init(&item_ready, nullptr);

    pthread_t ivanov_thread, petrov_thread, necheporuk_thread;

    cout << "\nThe operation of the smart warrant officers has started.\n" << endl;
    result_file << "The operation of the warrant officers started at night.\n" << endl;

    // Create threads
    pthread_create(&ivanov_thread, nullptr, Ivanov, nullptr);
    pthread_create(&petrov_thread, nullptr, Petrov, nullptr);
    pthread_create(&necheporuk_thread, nullptr, Necheporuk, nullptr);

    // Wait for threads to finish
    pthread_join(ivanov_thread, nullptr);
    pthread_join(petrov_thread, nullptr);
    pthread_join(necheporuk_thread, nullptr);

    // Program conclusion
    cout << "\nAll items have been successfully loaded." << endl;
    cout << "Total value of the loot: " << total_value << " rubles." << endl;

    result_file << "\nAll items have been successfully loaded." << endl;
    result_file << "Total value of the loot: " << total_value << " rubles." << endl;

    // Close the file
    result_file.close();

    // Destroy the mutex and condition variable
    pthread_mutex_destroy(&mtx);
    pthread_cond_destroy(&item_ready);

    cout << "End of program execution." << endl;
    return 0;
}


