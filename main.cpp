#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <ctime>
#include <string>
#include <cstdlib>
#include <cstring>

#ifdef _WIN32
    #include <windows.h>
    #define IS_WINDOWS true
    #define getpid _getpid
#else
    #include <unistd.h>
    #define IS_WINDOWS false
#endif

std::ofstream log_file;
std::mutex log_mutex;

std::atomic<int> counter(0);
std::atomic<bool> is_copy_running(false);
std::atomic<bool> is_running(true);


std::string get_current_time_string() {
    auto now = std::chrono::system_clock::now();
    auto time_point = std::chrono::system_clock::to_time_t(now);
    std::tm time_tm = *std::localtime(&time_point);
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &time_tm);
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    sprintf(buf + strlen(buf), ":%03d", static_cast<int>(milliseconds.count()));
    return std::string(buf);
}


void log_message(const std::string& message) {
    std::lock_guard<std::mutex> lock(log_mutex);
    log_file << get_current_time_string() << " " << message << std::endl;
}


void timer_increment() {
    while (is_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        counter.fetch_add(1, std::memory_order_relaxed);
    }
}


void start_child_processes() {
    if (is_copy_running.exchange(true)) {
        log_message("Copy already running, skipping new process.");
        return;
    }


    std::thread([]() {
        int pid = getpid();
        log_message("Child process 1 started: " + std::to_string(pid));
        counter.fetch_add(10, std::memory_order_relaxed);
        log_message("Child process 1 finished: " + std::to_string(pid));
        is_copy_running = false;
    }).detach();

 
    std::thread([]() {
        int pid = getpid();
        log_message("Child process 2 started: " + std::to_string(pid));
        counter.fetch_add(counter.load(std::memory_order_relaxed) * 2 - counter.load(), std::memory_order_relaxed);
        std::this_thread::sleep_for(std::chrono::seconds(2));
        counter.fetch_add(counter.load(std::memory_order_relaxed) / 2 - counter.load(), std::memory_order_relaxed);
        log_message("Child process 2 finished: " + std::to_string(pid));
        is_copy_running = false;
    }).detach();
}


void log_status() {
    while (is_running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        int pid = getpid();
        log_message("Main process ID: " + std::to_string(pid) + " Counter: " + std::to_string(counter.load()));
    }
}


void handle_user_input() {
    while (is_running) {
        std::string input;
        std::cout << "Enter a new value for the counter or 'exit' to quit: ";
        std::getline(std::cin, input);

        if (input == "exit") {
            is_running = false;
        } else {
            try {
                int new_value = std::stoi(input);
                counter.store(new_value, std::memory_order_relaxed);
            } catch (std::exception& e) {
                std::cout << "Invalid input. Please enter an integer." << std::endl;
            }
        }
    }
}

int main(int argc, char* argv[]) {
    log_file.open("program_log.txt", std::ios::out | std::ios::app);
    if (!log_file.is_open()) {
        std::cerr << "Failed to open log file." << std::endl;
        return 1;
    }

    int pid = getpid();
    log_message("Program started with PID: " + std::to_string(pid));


    std::thread timer_thread(timer_increment);
    std::thread log_thread(log_status);
    std::thread copy_thread([&]() {
        while (is_running) {
            std::this_thread::sleep_for(std::chrono::seconds(3));
            start_child_processes();
        }
    });
    std::thread user_input_thread(handle_user_input);


    timer_thread.join();
    log_thread.join();
    copy_thread.join();
    user_input_thread.join();

    log_file.close();
    return 0;
}
