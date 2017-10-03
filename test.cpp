#include <stdio.h>
#include "ws2812-rpi.h"
#include "bmp_read.h"

int main(int argc, char **argv){

    if(argc <= 2) {
        printf("Usage: %s [your picture].bmp [brightness in 100%]\n", argv[0]);
        printf("Example: %s test.bmp 0.8\n", argv[0]);
        return 0;
    }
    
    // keep a variable of brightness
    float brightness = 1.0;
    if(argc == 3) {
        brightness = atof(argv[2]);
    }

    char* fileName = argv[1];

    //bitmap_image image(argv[1]);
    bmp_reader bmp(fileName);
    const unsigned int height = bmp.m_fileInfoHeader.biHeight;
    const unsigned int width  = bmp.m_fileInfoHeader.biWidth;
    
    printf("Width: %ld, Height: %ld\n", width, height);
    
    
    NeoPixel* n = new NeoPixel(144);
    n->setBrightness(brightness);
    //n->effectsDemo();
    static int step = 0;
    for(int i = 0; i < width; ++i) {
        for(int j = 0; j < 144; ++j) {
            //printf("(r, g, b) => (%ld, %ld, %ld)\n", bmp.m_rgb[j][i].r, bmp.m_rgb[j][i].g, bmp.m_rgb[j][i].b);
            n->setPixelColor(j, bmp.m_rgb[j][i].r, bmp.m_rgb[j][i].g, bmp.m_rgb[j][i].b);
        }
        usleep(1000);
        n->show();
    }

    n->clear();
    n->show();
    delete n;

    return 0;
}