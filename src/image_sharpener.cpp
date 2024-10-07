#include <iostream>
#include "libppm.h"
#include <cstdint>
#include <chrono>

using namespace std;

struct image_t* S1_smoothen(struct image_t *input_image)
{

struct image_t* smoo = new struct image_t;
smoo->height = input_image->height;
smoo->width = input_image->width;
smoo->image_pixels = new uint8_t**[smoo->height];
for(int i = 0; i < smoo->height; i++)
{
smoo->image_pixels[i] = new uint8_t*[smoo->width];
for(int j = 0; j < smoo->width; j++)
{
smoo->image_pixels[i][j] = new uint8_t[3];
}

}

for(int i = 0; i < input_image->height; i++)
{
for(int j = 0; j < input_image->width; j++)
{
for(int k = 0; k < 3; k++)
{
if(i-1<0 || j-1<0 || i+1>=input_image->height || j+1>=input_image->width)
continue;
uint8_t val =0;
val += input_image->image_pixels[i][j][k];

val += input_image->image_pixels[i-1][j][k];
val += input_image->image_pixels[i-1][j-1][k];
val += input_image->image_pixels[i+1][j][k];
val += input_image->image_pixels[i+1][j-1][k];
val += input_image->image_pixels[i][j-1][k];
val += input_image->image_pixels[i][j+1][k];
val += input_image->image_pixels[i+1][j+1][k];
val += input_image->image_pixels[i-1][j+1][k];
val/=9;

smoo->image_pixels[i][j][k] = val;
// cout << smoo->image_pixels[i][j][k] << endl;
}
}
}

//cout << "hi" << endl;

return smoo;
}

struct image_t* S2_find_details(struct image_t *input_image, struct image_t *smoothened_image)
{
// TODO
for(int i = 0; i < input_image->height; i++)
{
for(int j = 0; j < input_image->width; j++)
{
for(int k = 0; k < 3; k++)
{
if(input_image->image_pixels[i][j][k] - smoothened_image->image_pixels[i][j][k]>=0)
input_image->image_pixels[i][j][k]-= smoothened_image->image_pixels[i][j][k];
}
}
}
return input_image;
}

struct image_t* S3_sharpen(struct image_t *input_image, struct image_t *details_image)
{
// TODO
for(int i = 0; i < input_image->height; i++)
{
for(int j = 0; j < input_image->width; j++)
{
for(int k = 0; k < 3; k++)
{
if(details_image->image_pixels[i][j][k]+input_image->image_pixels[i][j][k]<=255)
details_image->image_pixels[i][j][k]+= input_image->image_pixels[i][j][k];
}
}
}
return details_image;
}

int main(int argc, char **argv)
{
if(argc != 3)
{
cout << "usage: ./a.out \n\n";
exit(0);
}
struct image_t *input_image = read_ppm_file(argv[1]);
struct image_t *smoothened_image = S1_smoothen(input_image);
struct image_t *details_image = S2_find_details(input_image, smoothened_image);
struct image_t *sharpened_image = S3_sharpen(input_image, details_image);
write_ppm_file(argv[2], sharpened_image);
// auto start = chrono::steady_clock::now();
//     auto end = chrono::steady_clock::now();
//     chrono::duration<double> sumOne = chrono::duration<double>::zero();
//     chrono::duration<double> sumTwo = chrono::duration<double>::zero();
//     chrono::duration<double> sumThree = chrono::duration<double>::zero();
//     chrono::duration<double> sumFour = chrono::duration<double>::zero();
//     chrono::duration<double> sumFive = chrono::duration<double>::zero();

//     for (int i = 0; i < 7; i++)
//     {
//         start = chrono::steady_clock::now();
//         struct image_t *input_image = read_ppm_file(argv[1]);
//         end = chrono::steady_clock::now();
//         chrono::duration<double> elapsed_seconds = end - start;
//         sumOne += elapsed_seconds;
// //        cout << elapsed_seconds.count() << endl;

//         start = chrono::steady_clock::now();
//         struct image_t *smoothened_image = S1_smoothen(input_image);
//         end = chrono::steady_clock::now();
//         elapsed_seconds = end - start;
//         sumTwo += elapsed_seconds;
// //        cout << elapsed_seconds.count() << endl;

//         start = chrono::steady_clock::now();
//         struct image_t *details_image = S2_find_details(input_image, smoothened_image);
//         end = chrono::steady_clock::now();
//         elapsed_seconds = end - start;
//         sumThree += elapsed_seconds;
// //        cout << elapsed_seconds.count() << endl;

//         start = chrono::steady_clock::now();
//         struct image_t *sharpened_image = S3_sharpen(input_image, details_image);
//         end = chrono::steady_clock::now();
//         elapsed_seconds = end - start;
//         sumFour += elapsed_seconds;
// //        cout << elapsed_seconds.count() << endl;

//         start = chrono::steady_clock::now();
//         write_ppm_file(argv[2], sharpened_image);
//         end = chrono::steady_clock::now();
//         elapsed_seconds = end - start;
//         sumFive += elapsed_seconds;
// //        cout << elapsed_seconds.count() << endl;
//     }

//     cout << sumOne.count() / 7 << endl;
//     cout << sumTwo.count() / 7 << endl;
//     cout << sumThree.count() / 7 << endl;
//     cout << sumFour.count() / 7 << endl;
//     cout << sumFive.count() / 7 << endl;

    return 0;
}
