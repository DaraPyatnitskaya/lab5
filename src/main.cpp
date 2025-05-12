#include "image.hpp"
#include <iostream>

int main() {
    const size_t width = 1000;
    const size_t height = 1000;

    std::cout << "Generating random image (" << width << "x" << height << ")...\n";
    Image image = RandomImage(width, height);

    std::cout << "Original image:\n";
    printColoredImage(image);

    // Последовательное размытие
    Image seq_blurred;
    measureTime("Sequential blur", [&]() {
        seq_blurred = sequentialBlur(image);
    });
    printColoredImage(seq_blurred);

    // Параллельное размытие (4 потока)
    Image par_blurred;
    measureTime("Parallel blur (4 threads)", [&]() {
        par_blurred = parallel(image, 4);
    });

    // Пример с атомарными операциями
    std::cout << "\nAtomic operations example:\n";
    atomicExample();

    return 0;
}
