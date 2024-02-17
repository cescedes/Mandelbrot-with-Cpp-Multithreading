
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <tuple>
#include <time.h>
#include <cmath>
#include <complex>
#include <chrono>
#include <thread>
#include <atomic>
#include "mandelbrot-helpers.hpp"

using namespace std;
std::atomic<int> pixels_inside_atomic{0};

/**
 * Static work allocation
 * 
 * @param[inout] image
 * @param[in] thread_id
 * @param[in] num_threads
 * @param[inout] pixels_inside
 * 
*/
void worker_static(Image &image, int thread_id, int num_threads, std::atomic<int> &pixels_inside) {
    int rows = image.height;
    int cols = image.width;

    complex<double> c;
    int color = (255 * thread_id) / num_threads;

    for (int row = thread_id; row < rows; row += num_threads) {
        array<int, 3> pixel = {0, 0, 0};

        for (int col = 0; col < cols; col++) {
            double dx = ((double)col / cols - 0.75) * 2;
            double dy = ((double)row / rows - 0.5) * 2;

            c = complex<double>(dx, dy);

            if (mandelbrot_kernel(c, pixel, color)) {
                pixels_inside_atomic++;
            }

            image[row][col] = pixel;
        }
    }
}


/**
 * Dynamic work allocation
 * 
 * @param[inout] image
 * @param[in] thread_id
 * @param[in] num_threads
 * @param[inout] idx
 * @param[inout] pixel_inside
 * 
 * @return (pixels_inside could also be used as a return value)
*/
void worker_dynamic(Image &image, int thread_id, int num_threads, std::atomic<int> &pixels_inside) {
    int rows = image.height;
    int cols = image.width;

    complex<double> c;
    int color = (255 * thread_id) / num_threads;

    for (int row = 0; row < rows; ++row) {
        if ((row % num_threads) == thread_id) {
            for (int col = 0; col < cols; col++) {
                double dx = ((double)col / cols - 0.75) * 2;
                double dy = ((double)row / rows - 0.5) * 2;

                c = complex<double>(dx, dy);
                
                array<int, 3> pixel = {0, 0, 0};

                if (mandelbrot_kernel(c, pixel, color)) {
                    pixels_inside++;
                }

                image[row][col] = pixel;
            }
        }
    }
}


int main(int argc, char **argv)
{
    // arguments 
    int num_threads = 1;
    string work_allocation = "static";
    int print_level = 2; // 0 exec time only, 1 exec time and pixel count, 2 exec time, pixel cound and work allocation
    
    // height and width of the output image
    int width = 960, height = 720;

    parse_args(argc, argv, num_threads, work_allocation, height, width, print_level);

    double time;
    int pixels_inside = 0;

    // Generate Mandelbrot set in this image
    Image image(height, width);

    auto t1 = chrono::high_resolution_clock::now();
    vector<thread> threads;

    for (int tid = 0; tid < num_threads; ++tid) {
    if (work_allocation == "static") {
        threads.emplace_back(worker_static, std::ref(image), tid, num_threads, std::ref(pixels_inside_atomic));} 
    else if (work_allocation == "dynamic") {
        threads.emplace_back(worker_dynamic, std::ref(image), tid, num_threads, std::ref(pixels_inside_atomic));}
    }

    for (int tid=0; tid < num_threads; ++tid){
        threads[tid].join();
    }
    
    pixels_inside = pixels_inside_atomic.load();

    auto t2 = chrono::high_resolution_clock::now();

    // save image
    image.save_to_ppm("mandelbrot.ppm");

    if ( print_level >= 2) cout << "Work allocation: " << work_allocation << endl;
    if ( print_level >= 1 ) cout << "Total Mandelbrot pixels: " << pixels_inside << endl;
    cout << chrono::duration<double>(t2 - t1).count() << endl;

    return 0;
}