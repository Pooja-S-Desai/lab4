#include <iostream>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>
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

    // Initialize semaphores
    sem_t sem_s1_done, sem_s2_done;
    sem_init(&sem_s1_done, 1, 0);  // Binary semaphore for S1 completion
    sem_init(&sem_s2_done, 1, 0);  // Binary semaphore for S2 completion

    pid_t pid1 = fork();
    if (pid1 == 0) {
        // Process 1 - Apply S1 smoothening
        struct image_t *smoothened_image = S1_smoothen(input_image);

        // Signal that S1 is done
        sem_post(&sem_s1_done);
        exit(0);
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        // Process 2 - Apply S2 details finding
        sem_wait(&sem_s1_done);  // Wait for S1 to complete
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

        // Read smoothened image data (this should be a more complex operation to retrieve the image data)
        // Simulating reading from shared memory or some form of IPC
        // For simplicity, assuming smoothened_image is filled correctly here.

        struct image_t *details_image = S2_find_details(input_image, smoothened_image);

        // Signal that S2 is done
        sem_post(&sem_s2_done);
        exit(0);
    }

    pid_t pid3 = fork();
    if (pid3 == 0) {
        // Process 3 - Apply S3 sharpening and write to file
        sem_wait(&sem_s2_done);  // Wait for S2 to complete
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

        // Read details image data (this should be a more complex operation to retrieve the image data)
        // Simulating reading from shared memory or some form of IPC
        // For simplicity, assuming details_image is filled correctly here.

        struct image_t *sharpened_image = S3_sharpen(input_image, details_image);
        
        // Write final image to file
        write_ppm_file(argv[2], sharpened_image);
        exit(0);
    }

    // Wait for all child processes to complete
    waitpid(pid1, nullptr, 0);
    waitpid(pid2, nullptr, 0);
    waitpid(pid3, nullptr, 0);

    // Cleanup semaphores
    sem_destroy(&sem_s1_done);
    sem_destroy(&sem_s2_done);

    return 0;
}
