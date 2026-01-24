#include <iostream>
#include <vector>
#include <algorithm>
#include <thread>
#include <future>
#include <execution>
#include <iterator>
#include <cstdlib>

// A helper for small data sizes - insertion sort
template <typename T>
void insertion_sort(std::vector<T>& arr, int start, int end) {
    for (int i = start + 1; i <= end; ++i) {
        T key = arr[i];
        int j = i - 1;
        while (j >= start && arr[j] > key) {
            arr[j + 1] = arr[j];
            --j;
        }
        arr[j + 1] = key;
    }
}

// Merge two sorted halves into one
template <typename T>
void merge(std::vector<T>& arr, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    std::vector<T> leftArr(arr.begin() + left, arr.begin() + mid + 1);
    std::vector<T> rightArr(arr.begin() + mid + 1, arr.begin() + right + 1);

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (leftArr[i] <= rightArr[j]) {
            arr[k++] = leftArr[i++];
        } else {
            arr[k++] = rightArr[j++];
        }
    }

    while (i < n1) {
        arr[k++] = leftArr[i++];
    }

    while (j < n2) {
        arr[k++] = rightArr[j++];
    }
}

// Parallel merge sort with adaptive behavior
template <typename T>
void oblivion_sort_recursive(std::vector<T>& arr, int left, int right, int depth = 0) {
    const int INSERTION_THRESHOLD = 64; // Use insertion sort for small chunks

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

    merge(arr, left, mid, right);
}

template <typename T>
void oblivion_sort(std::vector<T>& arr) {
    int num_threads = std::thread::hardware_concurrency();
    int max_depth = (int)std::log2(num_threads > 0 ? num_threads : 1) + 2; // Max depth based on concurrency

    oblivion_sort_recursive(arr, 0, arr.size() - 1, max_depth);
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
