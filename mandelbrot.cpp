
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <tuple>
#include <time.h>
#include <cmath>
#include <complex>
#include <chrono>
#include <future>

#include "mandelbrot-helpers.hpp"

using namespace std;


/**
 * Compute the Mandelbrot set for each pixel of a given image.
 * Image is the Image data structure for storing RGB image
  * 
 * @param[inout] image
 * @return pixel_count
 * 
*/
int worker(Image &image)
{
    int rows = image.height;
    int cols = image.width;
    
    int pixels_inside=0;
    
    // pixel to be passed to the mandelbrot function
    std::array pixel = {0, 0, 0}; // red, green, blue (each range 0-255)
    complex<double> c;

    for (int row = 0; row < rows; row++) 
    {
        for (int col = 0; col < cols; col++) 
        {
            double dx = ((double)col / cols - 0.75) * 2.0;
            double dy = ((double)row / rows - 0.5) * 2.0;

            c = complex<double>(dx, dy);

            if (mandelbrot_kernel(c, pixel, 0)) // the actual mandelbrot kernel, 0 means black color for mandelbrot pixels
                pixels_inside++; 

            // apply to the image
            image[row][col] = pixel; 
        }
    }

    return pixels_inside;
}

int main(int argc, char **argv)
{
    // arguments 
    int num_threads = 1;
    std::string work_allocation = "static";
    int print_level = 2; // 0 exec time only, 1 exec time and pixel count, 2 exec time, pixel cound and work allocation
    
    // height and width of the output image
    int width = 960, height = 720;

    parse_args(argc, argv, num_threads, work_allocation, height, width, print_level);

    double time;
    int pixels_inside = 0;

    // Generate Mandelbrot set in this image
    Image image(height, width);

    auto t1 = chrono::high_resolution_clock::now();

    pixels_inside = worker(image); 

    auto t2 = chrono::high_resolution_clock::now();

    // save image
    image.save_to_ppm("mandelbrot.ppm");

    if ( print_level >= 2) cout << "Work allocation: " << work_allocation << endl;
    if ( print_level >= 1 ) cout << "Total Mandelbrot pixels: " << pixels_inside << endl;
    cout << chrono::duration<double>(t2 - t1).count() << endl;

    return 0;
}