#include <iostream>
#include <vector>
#include <algorithm>
#include <thread>
#include <future>
#include <execution>
#include <iterator>
#include <cmath>

// A helper for small data sizes - insertion sort
template <typename T>
void insertion_sort(std::vector<T>& arr, int start, int end) {
    for (int i = start + 1; i <= end; ++i) {
        T key = std::move(arr[i]);
        int j = i - 1;
        while (j >= start && arr[j] > key) {
            arr[j + 1] = std::move(arr[j]);
            --j;
        }
        arr[j + 1] = std::move(key);
    }
}

// Merge two sorted halves into one
template <typename T>
void merge(std::vector<T>& arr, int left, int mid, int right) {
    int n = right - left + 1;

    thread_local std::vector<T> buffer;
    if ((int)buffer.size() < n) buffer.resize(n);

    auto left_begin = std::make_move_iterator(arr.begin() + left);
    auto left_end = std::make_move_iterator(arr.begin() + mid + 1);
    auto right_begin = std::make_move_iterator(arr.begin() + mid + 1);
    auto right_end = std::make_move_iterator(arr.begin() + right + 1);

    auto buf_end = std::merge(left_begin, left_end, right_begin, right_end, buffer.begin());
    std::move(buffer.begin(), buf_end, arr.begin() + left);
}

// Parallel merge sort with adaptive behavior
template <typename T>
void oblivion_sort_recursive(std::vector<T>& arr, int left, int right, int depth = 0) {
    const int INSERTION_THRESHOLD = 24; // tuned down to reduce overhead

    if (left >= right) return;

    if (right - left + 1 <= INSERTION_THRESHOLD) {
        insertion_sort(arr, left, right);
        return;
    }

    int mid = left + (right - left) / 2;

    if (depth > 0) { // Use parallelism at higher tree depths
        auto left_future = std::async(std::launch::async, oblivion_sort_recursive<T>, std::ref(arr), left, mid, depth - 1);
        oblivion_sort_recursive(arr, mid + 1, right, depth - 1);
        left_future.wait();
    } else {
        oblivion_sort_recursive(arr, left, mid, depth - 1);
        oblivion_sort_recursive(arr, mid + 1, right, depth - 1);
    }

    // Skip merge when already in order
    if (arr[mid] <= arr[mid + 1]) return;

    merge(arr, left, mid, right);
}

template <typename T>
void oblivion_sort(std::vector<T>& arr) {
    if (arr.size() < 2) return;

    unsigned num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) num_threads = 1;

    int max_depth = static_cast<int>(std::log2(num_threads)) + 2;
    if (max_depth < 0) max_depth = 0;

    oblivion_sort_recursive(arr, 0, static_cast<int>(arr.size()) - 1, max_depth);
}

int main() {
    std::vector<int> data = {42, 3, 24, -5, 17, 13, 8, 100, 1, -20};

    std::cout << "Before: ";
    for (const auto& num : data) std::cout << num << " ";
    std::cout << std::endl;

    oblivion_sort(data);

    std::cout << "After: ";
    for (const auto& num : data) std::cout << num << " ";
    std::cout << std::endl;

    return 0;
}
