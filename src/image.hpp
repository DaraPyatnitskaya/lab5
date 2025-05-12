#pragma once
#include <vector>
#include <cstdint>
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <numeric>
#include <thread>
#include <mutex>
#include <chrono>
#include <atomic>

// Структура для представления цвета пикселя (RGB)
struct Color {
    uint8_t r; // Красный (0–255)
    uint8_t g; // Зелёный (0–255)
    uint8_t b; // Синий (0–255)

    Color() : r(0), g(0), b(0) {}
    Color(uint8_t red, uint8_t green, uint8_t blue) : r(red), g(green), b(blue) {}
};

using Image = std::vector<std::vector<Color>>;

// Генерация случайного изображения заданного размера
Image RandomImage(size_t width, size_t height) {
    Image image(height, std::vector<Color>(width));
    std::srand(std::time(0));
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            image[y][x] = Color(std::rand() % 256, std::rand() % 256, std::rand() % 256);
        }
    }
    return image;
}

// Функция для получения среднего цвета в окрестности 3×3
Color averageColor(const Image& image, int x, int y) {
    int sum_r = 0, sum_g = 0, sum_b = 0;
    int count = 0;

    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            int nx = x + dx;
            int ny = y + dy;
            if (nx >= 0 && ny >= 0 && nx < image[0].size() && ny < image.size()) {
                sum_r += image[ny][nx].r;
                sum_g += image[ny][nx].g;
                sum_b += image[ny][nx].b;
                count++;
            }
        }
    }

    return Color(static_cast<uint8_t>(sum_r / count),
                 static_cast<uint8_t>(sum_g / count),
                 static_cast<uint8_t>(sum_b / count));
}

// Последовательное размытие изображения
Image sequentialBlur(const Image& input) {
    Image output = input;
    for (size_t y = 0; y < input.size(); ++y) {
        for (size_t x = 0; x < input[y].size(); ++x) {
            output[y][x] = averageColor(input, x, y);
        }
    }
    return output;
}

// Параллельное размытие с использованием std::thread
Image parallel(const Image& input, int num_threads = 4) {
    Image output = input;
    std::vector<std::thread> threads;
    int height = input.size();
    int strip_height = height / num_threads;

    auto blur_task = [&](int start_y, int end_y) {
        for (int y = start_y; y < end_y; ++y) {
            for (size_t x = 0; x < input[y].size(); ++x) {
                output[y][x] = averageColor(input, x, y);
            }
        }
    };

    for (int i = 0; i < num_threads; ++i) {
        int start_y = i * strip_height;
        int end_y = (i == num_threads - 1) ? height : (i + 1) * strip_height;
        threads.emplace_back(blur_task, start_y, end_y);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    return output;
}

// Функция для измерения времени выполнения
template<typename Func>
void measureTime(const std::string& name, Func func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << name << " took " << duration.count() << " ms\n";
}

// Пример с атомарными операциями
void atomicExample() {
    const int num_iterations = 1'000'000;
    const int num_threads = 4;

    // Счётчик с мьютексом
    {
        int counter = 0;
        std::mutex counter_mutex;
        std::vector<std::thread> threads;

        auto task = [&]() {
            for (int i = 0; i < num_iterations; ++i) {
                std::lock_guard<std::mutex> lock(counter_mutex);
                counter++;
            }
        };

        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back(task);
        }
        for (auto& thread : threads) {
            thread.join();
        }
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Mutex counter: " << counter << ", time: " << duration.count() << " ms\n";
    }

    // Счётчик с std::atomic
    {
        std::atomic<int> counter(0);
        std::vector<std::thread> threads;

        auto task = [&]() {
            for (int i = 0; i < num_iterations; ++i) {
                counter++;
            }
        };

        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back(task);
        }
        for (auto& thread : threads) {
            thread.join();
        }
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Atomic counter: " << counter << ", time: " << duration.count() << " ms\n";
    }
}

// Вывод изображения в терминале с использованием ANSI-цветов
void printColoredImage(const Image& image, int max_size = 10) {
    int height = std::min<int>(image.size(), max_size);
    int width = std::min<int>(image[0].size(), max_size);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const Color& c = image[y][x];
            std::printf("\033[38;2;%d;%d;%dm@", c.r, c.g, c.b);
        }
        std::printf("\033[0m\n");
    }
}
