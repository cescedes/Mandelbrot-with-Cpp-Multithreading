/**
 *   This file contains a set of helper function. 
 *   Comment: You probably do need to change any of these.
*/
#include <iostream>
#include <vector>
#include <cmath>
#include <complex>
#include <fstream>
#include <array>

/**
 * Image data structure
 * 2D => (y-dim, x-dim) + 3 integer values representin RGB channels
 * Init:
 *   Image image(800, 600);

 * Access first color (red) of a pixel at position (34, 53):
 *  image[34][53][0]
*/

struct Image {

    std::vector<std::array<int, 3>> data;

    int height;
    int width;

    Image() {}
    
    Image(int height, int width, const std::array<int, 3>& val = {0,0,0}) 
        : height(height), width(width), data(height*width, val )
    { }

    ~Image() {
        clear();
        height = 0;
        width = 0;
    }

    void clear() {
        data.clear();
    }

    std::array<int, 3>* operator[](unsigned int row) {
        return &data[row*width];
    }

    void save_to_ppm(std::string filename) {
        std::ofstream ofs(filename, std::ofstream::out);
        ofs << "P3" << std::endl;
        ofs << width << " " << height << std::endl;
        ofs << 255 << std::endl;

        for (int row = 0; row < height; row++)
        {
            for (int col = 0; col < width; col++)
            {
                ofs << " " << (unsigned int)this->operator[](row)[col][0] << " " << (unsigned int)this->operator[](row)[col][1] << " " << (unsigned int)this->operator[](row)[col][2] << std::endl;
            }
        }
        ofs.close();
    }

    std::string to_string() {
        std::stringstream ss;
        for (int row = 0; row < height; row++) {
            for (int col = 0; col < width; col++) {
                for (int channel = 0; channel < 3; channel++)
                    ss << this->operator[](row)[col][channel] << " ";
            }
            ss << std::endl;
        }
        return ss.str();
    }
};

// Test if point c belongs to the Mandelbrot set
bool mandelbrot_kernel(std::complex<double> c, std::array<int, 3>& pixel, int color=0)
{
    int max_iterations = 2048, iteration = 0;
    std::complex<double> z(0, 0);

    while (abs(z) <= 4 && (iteration < max_iterations))
    {
        z = z * z + c;
        iteration++;
    }
    
    if (iteration != max_iterations)
        pixel = {255, 255, 255};
    else
        pixel = { color, color, color }; // black rgb(0,0,0)

    return (iteration < max_iterations);
};


/**
 * Compute the Mandelbrot set for each pixel of a given image.
 * Image is the Image data structure for storing RGB image
  * 
 * @param[inout] image
 * @return pixel_count
 * 
*/
int worker_sequential(Image &image)
{
    int rows = image.height;
    int cols = image.width;
    
    int pixels_inside=0;

    // pixel to be passed to the mandelbrot function
    std::array<int,3> pixel = {0, 0, 0}; // red, green, blue (each range 0-255)
    std::complex<double> c;

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            double dx = ((double)col / cols - 0.7) * 2;
            double dy = ((double)row / rows - 0.5) * 2;

            c = std::complex<double>(dx, dy);

            if (mandelbrot_kernel(c, pixel, 0)) // the actual mandelbrot kernel, 0 means black color for mandelbrot pixels
                pixels_inside++;

            // apply to the image
            image[row][col] = pixel;
        }
    }
    
    return pixels_inside;
};

/**
 * Parsing arguments
 * 
*/
void parse_args(int argc, char **argv, int& num_threads, std::string& work_allocation, int& height, int& width, int& print_level) {
    std::string usage("Usage: --num-threads <integer> --work-allocation <static | dynamic> --height <integer> --width <integer> --print-level <integer> ");

    for (int i = 1; i < argc; ++i) {
        if ( std::string(argv[i]).compare("--num-threads") == 0 ) {
            num_threads=std::stoi(argv[++i]);
        } else if ( std::string(argv[i]).compare("--work-allocation") == 0 ) {
            work_allocation=argv[++i];            
        } else if ( std::string(argv[i]).compare("--height") == 0 ) {
            height=std::stoi(argv[++i]);
        } else if ( std::string(argv[i]).compare("--width") == 0 ) {
            width=std::stoi(argv[++i]);
        } else if ( std::string(argv[i]).compare("--print-level") == 0 ) {
            print_level=std::stoi(argv[++i]);            
        } else if (  std::string(argv[i]).compare("--help") == 0 ) {
            std::cout << usage << std::endl;
            exit(-1);
        }
    }
};