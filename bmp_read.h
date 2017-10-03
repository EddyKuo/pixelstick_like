#ifndef __BMP_REDER_H__
#define __BMP_REDER_H__
#include <iostream>
#include <fstream>
#include <string>

using namespace std;
// CIEXYZTRIPLE stuff

typedef int _FXPT2DOT30;

typedef struct {
    _FXPT2DOT30 ciexyzX;
    _FXPT2DOT30 ciexyzY;
    _FXPT2DOT30 ciexyzZ;
} _CIEXYZ;

typedef struct {
    _CIEXYZ  ciexyzRed;
    _CIEXYZ  ciexyzGreen;
    _CIEXYZ  ciexyzBlue;
} _CIEXYZTRIPLE;

// bitmap file header
typedef struct {
    unsigned short bfType;
    unsigned int   bfSize;
    unsigned short bfReserved1;
    unsigned short bfReserved2;
    unsigned int   bfOffBits;
} _BITMAPFILEHEADER;

// bitmap info header
typedef struct {
    unsigned int   biSize;
    unsigned int   biWidth;
    unsigned int   biHeight;
    unsigned short biPlanes;
    unsigned short biBitCount;
    unsigned int   biCompression;
    unsigned int   biSizeImage;
    unsigned int   biXPelsPerMeter;
    unsigned int   biYPelsPerMeter;
    unsigned int   biClrUsed;
    unsigned int   biClrImportant;
    unsigned int   biRedMask;
    unsigned int   biGreenMask;
    unsigned int   biBlueMask;
    unsigned int   biAlphaMask;
    unsigned int   biCSType;
    _CIEXYZTRIPLE   biEndpoints;
    unsigned int   biGammaRed;
    unsigned int   biGammaGreen;
    unsigned int   biGammaBlue;
    unsigned int   biIntent;
    unsigned int   biProfileData;
    unsigned int   biProfileSize;
    unsigned int   biReserved;
} _BITMAPINFOHEADER;

// rgb quad
typedef struct {
    unsigned char  b;
    unsigned char  g;
    unsigned char  r;
    unsigned char  rgbReserved;
} _RGBQUAD;


struct bmp_reader {
    // read bytes

    bmp_reader(char* fileName) {
        m_fileName = fileName;
        parse_file();
    }

    ~bmp_reader() {
        for (unsigned int i = 0; i < m_fileInfoHeader.biHeight; i++) {
            delete m_rgb[i];
        }

        delete m_rgb;
    }

    template <typename Type>
    void read(std::ifstream &fp, Type &result, std::size_t size) {
        fp.read(reinterpret_cast<char*>(&result), size);
    }

    // bit extract
    unsigned char bitextract(const unsigned int byte, const unsigned int mask) {
        if (mask == 0) {
            return 0;
        }

        int
            maskBufer = mask,
            maskPadding = 0;

        while (!(maskBufer & 1)) {
            maskBufer >>= 1;
            maskPadding++;
        }

        return (byte & mask) >> maskPadding;
    }


    void parse_file() {
        std::ifstream fileStream(m_fileName.c_str(), std::ifstream::binary);
        if (!fileStream) {
            std::cout << "Error opening file '" << m_fileName << "'." << std::endl;
        }

        // read file header
        m_fileHeader;
        read(fileStream, m_fileHeader.bfType, sizeof(m_fileHeader.bfType));
        read(fileStream, m_fileHeader.bfSize, sizeof(m_fileHeader.bfSize));
        read(fileStream, m_fileHeader.bfReserved1, sizeof(m_fileHeader.bfReserved1));
        read(fileStream, m_fileHeader.bfReserved2, sizeof(m_fileHeader.bfReserved2));
        read(fileStream, m_fileHeader.bfOffBits, sizeof(m_fileHeader.bfOffBits));

        if (m_fileHeader.bfType != 0x4D42) {
            std::cout << "Error: '" << m_fileName << "' is not BMP file." << std::endl;
        }

        // read file info header
        read(fileStream, m_fileInfoHeader.biSize, sizeof(m_fileInfoHeader.biSize));

        // bmp core
        if (m_fileInfoHeader.biSize >= 12) {
            read(fileStream, m_fileInfoHeader.biWidth, sizeof(m_fileInfoHeader.biWidth));
            read(fileStream, m_fileInfoHeader.biHeight, sizeof(m_fileInfoHeader.biHeight));
            read(fileStream, m_fileInfoHeader.biPlanes, sizeof(m_fileInfoHeader.biPlanes));
            read(fileStream, m_fileInfoHeader.biBitCount, sizeof(m_fileInfoHeader.biBitCount));
        }

        // color count
        int colorsCount = m_fileInfoHeader.biBitCount >> 3;
        if (colorsCount < 3) {
            colorsCount = 3;
        }

        int bitsOnColor = m_fileInfoHeader.biBitCount / colorsCount;
        int maskValue = (1 << bitsOnColor) - 1;

        // bmp v1
        if (m_fileInfoHeader.biSize >= 40) {
            read(fileStream, m_fileInfoHeader.biCompression, sizeof(m_fileInfoHeader.biCompression));
            read(fileStream, m_fileInfoHeader.biSizeImage, sizeof(m_fileInfoHeader.biSizeImage));
            read(fileStream, m_fileInfoHeader.biXPelsPerMeter, sizeof(m_fileInfoHeader.biXPelsPerMeter));
            read(fileStream, m_fileInfoHeader.biYPelsPerMeter, sizeof(m_fileInfoHeader.biYPelsPerMeter));
            read(fileStream, m_fileInfoHeader.biClrUsed, sizeof(m_fileInfoHeader.biClrUsed));
            read(fileStream, m_fileInfoHeader.biClrImportant, sizeof(m_fileInfoHeader.biClrImportant));
        }

        // bmp v2
        m_fileInfoHeader.biRedMask = 0;
        m_fileInfoHeader.biGreenMask = 0;
        m_fileInfoHeader.biBlueMask = 0;

        if (m_fileInfoHeader.biSize >= 52) {
            read(fileStream, m_fileInfoHeader.biRedMask, sizeof(m_fileInfoHeader.biRedMask));
            read(fileStream, m_fileInfoHeader.biGreenMask, sizeof(m_fileInfoHeader.biGreenMask));
            read(fileStream, m_fileInfoHeader.biBlueMask, sizeof(m_fileInfoHeader.biBlueMask));
        }

        // color mask
        if (m_fileInfoHeader.biRedMask == 0 || m_fileInfoHeader.biGreenMask == 0 || m_fileInfoHeader.biBlueMask == 0) {
            m_fileInfoHeader.biRedMask = maskValue << (bitsOnColor * 2);
            m_fileInfoHeader.biGreenMask = maskValue << bitsOnColor;
            m_fileInfoHeader.biBlueMask = maskValue;
        }

        // bmp v3
        if (m_fileInfoHeader.biSize >= 56) {
            read(fileStream, m_fileInfoHeader.biAlphaMask, sizeof(m_fileInfoHeader.biAlphaMask));
        }
        else {
            m_fileInfoHeader.biAlphaMask = maskValue << (bitsOnColor * 3);
        }

        // bmp v4
        if (m_fileInfoHeader.biSize >= 108) {
            read(fileStream, m_fileInfoHeader.biCSType, sizeof(m_fileInfoHeader.biCSType));
            read(fileStream, m_fileInfoHeader.biEndpoints, sizeof(m_fileInfoHeader.biEndpoints));
            read(fileStream, m_fileInfoHeader.biGammaRed, sizeof(m_fileInfoHeader.biGammaRed));
            read(fileStream, m_fileInfoHeader.biGammaGreen, sizeof(m_fileInfoHeader.biGammaGreen));
            read(fileStream, m_fileInfoHeader.biGammaBlue, sizeof(m_fileInfoHeader.biGammaBlue));
        }

        // bmp v5
        if (m_fileInfoHeader.biSize >= 124) {
            read(fileStream, m_fileInfoHeader.biIntent, sizeof(m_fileInfoHeader.biIntent));
            read(fileStream, m_fileInfoHeader.biProfileData, sizeof(m_fileInfoHeader.biProfileData));
            read(fileStream, m_fileInfoHeader.biProfileSize, sizeof(m_fileInfoHeader.biProfileSize));
            read(fileStream, m_fileInfoHeader.biReserved, sizeof(m_fileInfoHeader.biReserved));
        }

        if (m_fileInfoHeader.biSize != 12 && m_fileInfoHeader.biSize != 40 && m_fileInfoHeader.biSize != 52 &&
            m_fileInfoHeader.biSize != 56 && m_fileInfoHeader.biSize != 108 && m_fileInfoHeader.biSize != 124) {
            std::cout << "Error: Unsupported BMP format." << std::endl;
        }

        if (m_fileInfoHeader.biBitCount != 16 && m_fileInfoHeader.biBitCount != 24 && m_fileInfoHeader.biBitCount != 32) {
            std::cout << "Error: Unsupported BMP bit count." << std::endl;
        }

        if (m_fileInfoHeader.biCompression != 0 && m_fileInfoHeader.biCompression != 3) {
            std::cout << "Error: Unsupported BMP compression." << std::endl;
        }

        // rgb info
        m_rgb = new _RGBQUAD*[m_fileInfoHeader.biHeight];

        for (unsigned int i = 0; i < m_fileInfoHeader.biHeight; i++) {
            m_rgb[i] = new _RGBQUAD[m_fileInfoHeader.biWidth];
        }

        int linePadding = (4 - ((3 * m_fileInfoHeader.biWidth) % 4)) % 4;
        //int linePadding = ((m_fileInfoHeader.biWidth * (m_fileInfoHeader.biBitCount / 8)) % 4) & 3;

        unsigned int bufer;

        for (unsigned int i = 0; i < m_fileInfoHeader.biHeight; i++) {
            for (unsigned int j = 0; j < m_fileInfoHeader.biWidth; j++) {
                read(fileStream, bufer, m_fileInfoHeader.biBitCount / 8);

                m_rgb[i][j].r = bitextract(bufer, m_fileInfoHeader.biRedMask);
                m_rgb[i][j].g = bitextract(bufer, m_fileInfoHeader.biGreenMask);
                m_rgb[i][j].b = bitextract(bufer, m_fileInfoHeader.biBlueMask);
                m_rgb[i][j].rgbReserved = bitextract(bufer, m_fileInfoHeader.biAlphaMask);
            }
            // skip padding
            fileStream.seekg(linePadding, std::ios_base::cur);
        }
    } // end of parse file

    //////////////////////////////////////////////////////////////////////////

    _RGBQUAD **m_rgb;
    _BITMAPFILEHEADER m_fileHeader;
    _BITMAPINFOHEADER m_fileInfoHeader;
    string m_fileName;

};




#endif // MAIN_H_INCLUDED