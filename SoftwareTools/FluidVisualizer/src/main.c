#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>

#define SCREEN_BUFFER_LENGTH (16*16)

// Test accelerometer reading
static float testAccelerometerReading[3] = {0, 0, -9.81};

// 16x16 array of bools indicating which pixels are illuminated
static uint8_t filterBuffer[SCREEN_BUFFER_LENGTH];

// XYZ position of the light directly below the cube
static float lightPosition[3] = {8, 8, -5};

// XYZ positions for each pixel and surface normals for each panel
// The bottom left most corner of the cube is at (0, 0, 0)
// The test panel is the panel on the bottom of the cube
static float surfaceNormals[6][3];
static float pixelPositions[SCREEN_BUFFER_LENGTH][4][1] = {
    {0, 15, 0, 0}, {1, 15, 0, 0}, {2, 15, 0, 0}, {3, 15, 0, 0}, {4, 15, 0, 0}, {5, 15, 0, 0}, {6, 15, 0, 0}, {7, 15, 0, 0}, {8, 15, 0, 0}, {9, 15, 0, 0}, {10, 15, 0, 0}, {11, 15, 0, 0}, {12, 15, 0, 0}, {13, 15, 0, 0}, {14, 15, 0, 0}, {15, 15, 0, 0},
    {0, 14, 0, 0}, {1, 14, 0, 0}, {2, 14, 0, 0}, {3, 14, 0, 0}, {4, 14, 0, 0}, {5, 14, 0, 0}, {6, 14, 0, 0}, {7, 14, 0, 0}, {8, 14, 0, 0}, {9, 14, 0, 0}, {10, 14, 0, 0}, {11, 14, 0, 0}, {12, 14, 0, 0}, {13, 14, 0, 0}, {14, 14, 0, 0}, {15, 14, 0, 0},
    {0, 13, 0, 0}, {1, 13, 0, 0}, {2, 13, 0, 0}, {3, 13, 0, 0}, {4, 13, 0, 0}, {5, 13, 0, 0}, {6, 13, 0, 0}, {7, 13, 0, 0}, {8, 13, 0, 0}, {9, 13, 0, 0}, {10, 13, 0, 0}, {11, 13, 0, 0}, {12, 13, 0, 0}, {13, 13, 0, 0}, {14, 13, 0, 0}, {15, 13, 0, 0},
    {0, 12, 0, 0}, {1, 12, 0, 0}, {2, 12, 0, 0}, {3, 12, 0, 0}, {4, 12, 0, 0}, {5, 12, 0, 0}, {6, 12, 0, 0}, {7, 12, 0, 0}, {8, 12, 0, 0}, {9, 12, 0, 0}, {10, 12, 0, 0}, {11, 12, 0, 0}, {12, 12, 0, 0}, {13, 12, 0, 0}, {14, 12, 0, 0}, {15, 12, 0, 0},
    {0, 11, 0, 0}, {1, 11, 0, 0}, {2, 11, 0, 0}, {3, 11, 0, 0}, {4, 11, 0, 0}, {5, 11, 0, 0}, {6, 11, 0, 0}, {7, 11, 0, 0}, {8, 11, 0, 0}, {9, 11, 0, 0}, {10, 11, 0, 0}, {11, 11, 0, 0}, {12, 11, 0, 0}, {13, 11, 0, 0}, {14, 11, 0, 0}, {15, 11, 0, 0},
    {0, 10, 0, 0}, {1, 10, 0, 0}, {2, 10, 0, 0}, {3, 10, 0, 0}, {4, 10, 0, 0}, {5, 10, 0, 0}, {6, 10, 0, 0}, {7, 10, 0, 0}, {8, 10, 0, 0}, {9, 10, 0, 0}, {10, 10, 0, 0}, {11, 10, 0, 0}, {12, 10, 0, 0}, {13, 10, 0, 0}, {14, 10, 0, 0}, {15, 10, 0, 0},
    {0, 9, 0, 0}, {1, 9, 0, 0}, {2, 9, 0, 0}, {3, 9, 0, 0}, {4, 9, 0, 0}, {5, 9, 0, 0}, {6, 9, 0, 0}, {7, 9, 0, 0}, {8, 9, 0, 0}, {9, 9, 0, 0}, {10, 9, 0, 0}, {11, 9, 0, 0}, {12, 9, 0, 0}, {13, 9, 0, 0}, {14, 9, 0, 0}, {15, 9, 0, 0},
    {0, 8, 0, 0}, {1, 8, 0, 0}, {2, 8, 0, 0}, {3, 8, 0, 0}, {4, 8, 0, 0}, {5, 8, 0, 0}, {6, 8, 0, 0}, {7, 8, 0, 0}, {8, 8, 0, 0}, {9, 8, 0, 0}, {10, 8, 0, 0}, {11, 8, 0, 0}, {12, 8, 0, 0}, {13, 8, 0, 0}, {14, 8, 0, 0}, {15, 8, 0, 0},
    {0, 7, 0, 0}, {1, 7, 0, 0}, {2, 7, 0, 0}, {3, 7, 0, 0}, {4, 7, 0, 0}, {5, 7, 0, 0}, {6, 7, 0, 0}, {7, 7, 0, 0}, {8, 7, 0, 0}, {9, 7, 0, 0}, {10, 7, 0, 0}, {11, 7, 0, 0}, {12, 7, 0, 0}, {13, 7, 0, 0}, {14, 7, 0, 0}, {15, 7, 0, 0},
    {0, 6, 0, 0}, {1, 6, 0, 0}, {2, 6, 0, 0}, {3, 6, 0, 0}, {4, 6, 0, 0}, {5, 6, 0, 0}, {6, 6, 0, 0}, {7, 6, 0, 0}, {8, 6, 0, 0}, {9, 6, 0, 0}, {10, 6, 0, 0}, {11, 6, 0, 0}, {12, 6, 0, 0}, {13, 6, 0, 0}, {14, 6, 0, 0}, {15, 6, 0, 0},
    {0, 5, 0, 0}, {1, 5, 0, 0}, {2, 5, 0, 0}, {3, 5, 0, 0}, {4, 5, 0, 0}, {5, 5, 0, 0}, {6, 5, 0, 0}, {7, 5, 0, 0}, {8, 5, 0, 0}, {9, 5, 0, 0}, {10, 5, 0, 0}, {11, 5, 0, 0}, {12, 5, 0, 0}, {13, 5, 0, 0}, {14, 5, 0, 0}, {15, 5, 0, 0},
    {0, 4, 0, 0}, {1, 4, 0, 0}, {2, 4, 0, 0}, {3, 4, 0, 0}, {4, 4, 0, 0}, {5, 4, 0, 0}, {6, 4, 0, 0}, {7, 4, 0, 0}, {8, 4, 0, 0}, {9, 4, 0, 0}, {10, 4, 0, 0}, {11, 4, 0, 0}, {12, 4, 0, 0}, {13, 4, 0, 0}, {14, 4, 0, 0}, {15, 4, 0, 0},
    {0, 3, 0, 0}, {1, 3, 0, 0}, {2, 3, 0, 0}, {3, 3, 0, 0}, {4, 3, 0, 0}, {5, 3, 0, 0}, {6, 3, 0, 0}, {7, 3, 0, 0}, {8, 3, 0, 0}, {9, 3, 0, 0}, {10, 3, 0, 0}, {11, 3, 0, 0}, {12, 3, 0, 0}, {13, 3, 0, 0}, {14, 3, 0, 0}, {15, 3, 0, 0},
    {0, 2, 0, 0}, {1, 2, 0, 0}, {2, 2, 0, 0}, {3, 2, 0, 0}, {4, 2, 0, 0}, {5, 2, 0, 0}, {6, 2, 0, 0}, {7, 2, 0, 0}, {8, 2, 0, 0}, {9, 2, 0, 0}, {10, 2, 0, 0}, {11, 2, 0, 0}, {12, 2, 0, 0}, {13, 2, 0, 0}, {14, 2, 0, 0}, {15, 2, 0, 0},
    {0, 1, 0, 0}, {1, 1, 0, 0}, {2, 1, 0, 0}, {3, 1, 0, 0}, {4, 1, 0, 0}, {5, 1, 0, 0}, {6, 1, 0, 0}, {7, 1, 0, 0}, {8, 1, 0, 0}, {9, 1, 0, 0}, {10, 1, 0, 0}, {11, 1, 0, 0}, {12, 1, 0, 0}, {13, 1, 0, 0}, {14, 1, 0, 0}, {15, 1, 0, 0},   
    {0, 0, 0, 0}, {1, 0, 0, 0}, {2, 0, 0, 0}, {3, 0, 0, 0}, {4, 0, 0, 0}, {5, 0, 0, 0}, {6, 0, 0, 0}, {7, 0, 0, 0}, {8, 0, 0, 0}, {9, 0, 0, 0}, {10, 0, 0, 0}, {11, 0, 0, 0}, {12, 0, 0, 0}, {13, 0, 0, 0}, {14, 0, 0, 0}, {15, 0, 0, 0},   
}; 

// Python helper will interpret any non zero value as white and zero as black
void writeFilterBufferToFile(void)
{
    FILE* file = fopen("./image.txt", "w");
    for (int i = 0; i < SCREEN_BUFFER_LENGTH - 1; i++)
    {
        fprintf(file, "%x,", filterBuffer[i]);
    }
    fprintf(file, "%x", filterBuffer[SCREEN_BUFFER_LENGTH - 1]);
    fclose(file);
}

float accelerometerToPitch(float* accelerometerData)
{
    // Assume g = 9.81 for now
    return asin(accelerometerData[0] / 9.81);
}

float accelerometerToYaw(float* accelerometerData)
{
    return atan(accelerometerData[1] / accelerometerData[2]);
}

static inline void Matrix4x4MultiplyBy4x1(float src1[4][4], float src2[4][1], float dest[4][1])
{
    dest[0][0] = src1[0][0] * src2[0][0] + src1[0][1] * src2[1][0] + src1[0][2] * src2[2][0] + src1[0][3] * src2[3][0]; 
    dest[1][0] = src1[1][0] * src2[0][0] + src1[1][1] * src2[1][0] + src1[1][2] * src2[2][0] + src1[1][3] * src2[3][0]; 
    dest[2][0] = src1[2][0] * src2[0][0] + src1[2][1] * src2[1][0] + src1[2][2] * src2[2][0] + src1[2][3] * src2[3][0]; 
    dest[3][0] = src1[3][0] * src2[0][0] + src1[3][1] * src2[1][0] + src1[3][2] * src2[2][0] + src1[3][3] * src2[3][0]; 
};

static inline void Matrix4x4MultiplyBy4x4(float src1[4][4], float src2[4][4], float dest[4][4])
{
    dest[0][0] = src1[0][0] * src2[0][0] + src1[0][1] * src2[1][0] + src1[0][2] * src2[2][0] + src1[0][3] * src2[3][0]; 
    dest[0][1] = src1[0][0] * src2[0][1] + src1[0][1] * src2[1][1] + src1[0][2] * src2[2][1] + src1[0][3] * src2[3][1]; 
    dest[0][2] = src1[0][0] * src2[0][2] + src1[0][1] * src2[1][2] + src1[0][2] * src2[2][2] + src1[0][3] * src2[3][2]; 
    dest[0][3] = src1[0][0] * src2[0][3] + src1[0][1] * src2[1][3] + src1[0][2] * src2[2][3] + src1[0][3] * src2[3][3]; 
    dest[1][0] = src1[1][0] * src2[0][0] + src1[1][1] * src2[1][0] + src1[1][2] * src2[2][0] + src1[1][3] * src2[3][0]; 
    dest[1][1] = src1[1][0] * src2[0][1] + src1[1][1] * src2[1][1] + src1[1][2] * src2[2][1] + src1[1][3] * src2[3][1]; 
    dest[1][2] = src1[1][0] * src2[0][2] + src1[1][1] * src2[1][2] + src1[1][2] * src2[2][2] + src1[1][3] * src2[3][2]; 
    dest[1][3] = src1[1][0] * src2[0][3] + src1[1][1] * src2[1][3] + src1[1][2] * src2[2][3] + src1[1][3] * src2[3][3]; 
    dest[2][0] = src1[2][0] * src2[0][0] + src1[2][1] * src2[1][0] + src1[2][2] * src2[2][0] + src1[2][3] * src2[3][0]; 
    dest[2][1] = src1[2][0] * src2[0][1] + src1[2][1] * src2[1][1] + src1[2][2] * src2[2][1] + src1[2][3] * src2[3][1]; 
    dest[2][2] = src1[2][0] * src2[0][2] + src1[2][1] * src2[1][2] + src1[2][2] * src2[2][2] + src1[2][3] * src2[3][2]; 
    dest[2][3] = src1[2][0] * src2[0][3] + src1[2][1] * src2[1][3] + src1[2][2] * src2[2][3] + src1[2][3] * src2[3][3]; 
    dest[3][0] = src1[3][0] * src2[0][0] + src1[3][1] * src2[1][0] + src1[3][2] * src2[2][0] + src1[3][3] * src2[3][0]; 
    dest[3][1] = src1[3][0] * src2[0][1] + src1[3][1] * src2[1][1] + src1[3][2] * src2[2][1] + src1[3][3] * src2[3][1]; 
    dest[3][2] = src1[3][0] * src2[0][2] + src1[3][1] * src2[1][2] + src1[3][2] * src2[2][2] + src1[3][3] * src2[3][2]; 
    dest[3][3] = src1[3][0] * src2[0][3] + src1[3][1] * src2[1][3] + src1[3][2] * src2[2][3] + src1[3][3] * src2[3][3]; 
};

// Need to have center of cube at the origin but for ease hard coded values put the bottom left at origin, need to move
void moveToOrigin(float* positions)
{


}

void applyPitchRollRotation(float* positions, float pitch, float roll)
{
    // Move all the positions so that the cube is centered at the origin
    float initialTranslationMatrix[4][4] = {
        {1, 0, 0, -8},
        {0, 1, 0, -8},
        {0, 0, 1, -8},
        {0, 0, 0,  1}
    };

    // Pitch is rotation around y axis
    // Roll is rotation around x axis
    // Create one single combined rotation matrix
    float rotationMatrix[4][4] = {
        {}
    };

    float endingTranslationMatrix[4][4] = {
        {1, 0, 0, 8},
        {0, 1, 0, 8},
        {0, 0, 1, 8},
        {0, 0, 0, 1}
    };


    // Revert the initial position change
}

int main()
{
    // Get the current pitch and yaw
    float pitch = accelerometerToPitch(testAccelerometerReading);
    float yaw = accelerometerToYaw(testAccelerometerReading);

    // Rotate the pixel positions based on the pitch and the yaw
    pitch is y 
    roll is x


    writeFilterBufferToFile();
    return 0;
}







