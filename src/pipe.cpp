#include <iostream>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include "libppm.h"
#include <cstdint>
#include <chrono>

using namespace std;

// S1: Smoothening step
struct image_t* S1_smoothen(struct image_t *input_image) {
    struct image_t* smoo = new struct image_t;
    smoo->height = input_image->height;
    smoo->width = input_image->width;
    smoo->image_pixels = new uint8_t**[smoo->height];

    for (int i = 0; i < smoo->height; i++) {
        smoo->image_pixels[i] = new uint8_t*[smoo->width];
        for (int j = 0; j < smoo->width; j++) {
            smoo->image_pixels[i][j] = new uint8_t[3];
        }
    }

    for (int i = 0; i < input_image->height; i++) {
        for (int j = 0; j < input_image->width; j++) {
            for (int k = 0; k < 3; k++) {
                if (i - 1 < 0 || j - 1 < 0 || i + 1 >= input_image->height || j + 1 >= input_image->width)
                    continue;
                uint8_t val = 0;
                val += input_image->image_pixels[i][j][k];
                val += input_image->image_pixels[i - 1][j][k];
                val += input_image->image_pixels[i - 1][j - 1][k];
                val += input_image->image_pixels[i + 1][j][k];
                val += input_image->image_pixels[i + 1][j - 1][k];
                val += input_image->image_pixels[i][j - 1][k];
                val += input_image->image_pixels[i][j + 1][k];
                val += input_image->image_pixels[i + 1][j + 1][k];
                val += input_image->image_pixels[i - 1][j + 1][k];
                val /= 9;

                smoo->image_pixels[i][j][k] = val;
            }
        }
    }

    return smoo;
}

// S2: Finding details step
struct image_t* S2_find_details(struct image_t *input_image, struct image_t *smoothened_image) {
    for (int i = 0; i < input_image->height; i++) {
        for (int j = 0; j < input_image->width; j++) {
            for (int k = 0; k < 3; k++) {
                if (input_image->image_pixels[i][j][k] - smoothened_image->image_pixels[i][j][k] >= 0)
                    input_image->image_pixels[i][j][k] -= smoothened_image->image_pixels[i][j][k];
            }
        }
    }
    return input_image;
}

// S3: Sharpening step
struct image_t* S3_sharpen(struct image_t *input_image, struct image_t *details_image) {
    for (int i = 0; i < input_image->height; i++) {
        for (int j = 0; j < input_image->width; j++) {
            for (int k = 0; k < 3; k++) {
                if (details_image->image_pixels[i][j][k] + input_image->image_pixels[i][j][k] <= 255)
                    details_image->image_pixels[i][j][k] += input_image->image_pixels[i][j][k];
            }
        }
    }
    return details_image;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        cout << "usage: ./a.out input_file output_file\n";
        exit(0);
    }

    struct image_t *input_image = read_ppm_file(argv[1]);

    // Create pipes for communication
    int pipe_s1_s2[2], pipe_s2_s3[2];
    pipe(pipe_s1_s2);
    pipe(pipe_s2_s3);

    pid_t pid1 = fork();
    if (pid1 == 0) {
        // Process 1 - Apply S1 smoothening
        close(pipe_s1_s2[0]);  // Close unused read end
        struct image_t *smoothened_image = S1_smoothen(input_image);

        // Write smoothened image to pipe
        write(pipe_s1_s2[1], smoothened_image->image_pixels, input_image->height * input_image->width * 3 * sizeof(uint8_t));
        close(pipe_s1_s2[1]);  // Close write end
        exit(0);
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        // Process 2 - Apply S2 details finding
        close(pipe_s1_s2[1]);  // Close unused write end
        close(pipe_s2_s3[0]);  // Close unused read end

        struct image_t *smoothened_image = new image_t;
        smoothened_image->height = input_image->height;
        smoothened_image->width = input_image->width;
        smoothened_image->image_pixels = new uint8_t**[input_image->height];
        for (int i = 0; i < input_image->height; i++) {
            smoothened_image->image_pixels[i] = new uint8_t*[input_image->width];
            for (int j = 0; j < input_image->width; j++) {
                smoothened_image->image_pixels[i][j] = new uint8_t[3];
            }
        }

        // Read smoothened image from pipe
        read(pipe_s1_s2[0], smoothened_image->image_pixels, input_image->height * input_image->width * 3 * sizeof(uint8_t));
        close(pipe_s1_s2[0]);  // Close read end

        struct image_t *details_image = S2_find_details(input_image, smoothened_image);

        // Write details image to pipe
        write(pipe_s2_s3[1], details_image->image_pixels, input_image->height * input_image->width * 3 * sizeof(uint8_t));
        close(pipe_s2_s3[1]);  // Close write end
        exit(0);
    }

    pid_t pid3 = fork();
    if (pid3 == 0) {
        // Process 3 - Apply S3 sharpening and write to file
        close(pipe_s2_s3[1]);  // Close unused write end

        struct image_t *details_image = new image_t;
        details_image->height = input_image->height;
        details_image->width = input_image->width;
        details_image->image_pixels = new uint8_t**[input_image->height];
        for (int i = 0; i < input_image->height; i++) {
            details_image->image_pixels[i] = new uint8_t*[input_image->width];
            for (int j = 0; j < input_image->width; j++) {
                details_image->image_pixels[i][j] = new uint8_t[3];
            }
        }

        // Read details image from pipe
        read(pipe_s2_s3[0], details_image->image_pixels, input_image->height * input_image->width * 3 * sizeof(uint8_t));
        close(pipe_s2_s3[0]);  // Close read end

        struct image_t *sharpened_image = S3_sharpen(input_image, details_image);
        
        // Write final image to file
        write_ppm_file(argv[2], sharpened_image);
        exit(0);
    }

    // Wait for all child processes to complete
    waitpid(pid1, nullptr, 0);
    waitpid(pid2, nullptr, 0);
    waitpid(pid3, nullptr, 0);

    return 0;
}
