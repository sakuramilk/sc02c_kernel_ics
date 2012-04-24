/*
 * bmp2splash.cpp
 *
 * Copyright (C) 2012 sakuramilk <c.sakuramilk@gmail.com>
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

//#define _DEBUG

#ifdef _DEBUG
#define PRINT_VAL(val) printf(#val " = %d(0x%08x)\n", (unsigned int)val, (unsigned int)val)
#endif

struct BITMAPFILEHEADER {
  uint16_t bfType;
  uint32_t bfSize;
  uint16_t bfReserved1;
  uint16_t bfReserved2;
  uint32_t bfOffBits;
};

struct BITMAPINFOHEADER {
    uint32_t biSize;
    int32_t  biWidth;
    int32_t  biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t  biXPixPerMeter;
    int32_t  biYPixPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImporant;
};


static inline uint16_t read_byte(uint8_t* &pData) {
    uint16_t value = *pData;
    pData += 1;
    return value;
}

static inline uint16_t read_2byte(uint8_t* &pData) {
    uint16_t value = (*(pData+1) << 8) | *(pData);
    pData += 2;
    return value;
}

static inline uint32_t read_3byte(uint8_t* &pData) {
    uint32_t value = (*(pData+2) << 16) | (*(pData+1) << 8) | *(pData);
    pData += 3;
    return value;
}

static inline uint32_t read_4byte(uint8_t* &pData) {
    uint32_t value = (*(pData+3) << 24) | (*(pData+2) << 16) | (*(pData+1) << 8) | *(pData);
    pData += 4;
    return value;
}

int main(int argc, char** argv)
{
    int ret = 0;
    FILE *fp;
    size_t fileSize = 0;
    fpos_t filepos;
    uint8_t* pFileBuf = NULL;
    uint8_t* pBmpData = NULL;
    struct BITMAPFILEHEADER bmpFileHeader;
    struct BITMAPINFOHEADER bmpInfoHeader;
    uint32_t count = 0;

    if (argc != 2) {
        fprintf(stderr, "Usage: bmp2splash [24bit bmp file]\n");
        ret = -1;
        goto exit;
    }

    if ((fp = fopen(argv[1], "r")) == NULL) {
        fprintf(stderr, "error: can't open bmp file.\n");
        ret = -1;
        goto exit;
    }

    fseek(fp, 0, SEEK_END);
    fgetpos(fp, &filepos);
    fseek(fp, 0, SEEK_SET);
    fileSize = filepos.__pos;

    pFileBuf = (uint8_t*)malloc(fileSize);
    if (!pFileBuf) {
        fprintf(stderr, "error: can't allocation memory.\n");
        ret = -1;
        goto exit;
    }
    pBmpData = pFileBuf;

    fread(pFileBuf, sizeof(uint8_t), fileSize, fp);

    fclose(fp);

    bmpFileHeader.bfType = read_2byte(pBmpData);
    bmpFileHeader.bfSize = read_4byte(pBmpData);
    bmpFileHeader.bfReserved1 = read_2byte(pBmpData);
    bmpFileHeader.bfReserved2 = read_2byte(pBmpData);
    bmpFileHeader.bfOffBits = read_4byte(pBmpData);
#ifdef _DEBUG
    PRINT_VAL(bmpFileHeader.bfType);
    PRINT_VAL(bmpFileHeader.bfSize);
    PRINT_VAL(bmpFileHeader.bfReserved1);
    PRINT_VAL(bmpFileHeader.bfReserved2);
    PRINT_VAL(bmpFileHeader.bfOffBits);
#endif

    bmpInfoHeader.biSize = read_4byte(pBmpData);
    bmpInfoHeader.biWidth = read_4byte(pBmpData);
    bmpInfoHeader.biHeight = read_4byte(pBmpData);
    bmpInfoHeader.biPlanes = read_2byte(pBmpData);
    bmpInfoHeader.biBitCount = read_2byte(pBmpData);
    bmpInfoHeader.biCompression = read_4byte(pBmpData);
    bmpInfoHeader.biSizeImage = read_4byte(pBmpData);
    bmpInfoHeader.biXPixPerMeter = read_4byte(pBmpData);
    bmpInfoHeader.biYPixPerMeter = read_4byte(pBmpData);
    bmpInfoHeader.biClrUsed = read_4byte(pBmpData);
    bmpInfoHeader.biClrImporant = read_4byte(pBmpData);
#ifdef _DEBUG
    PRINT_VAL(bmpInfoHeader.biSize);
    PRINT_VAL(bmpInfoHeader.biWidth);
    PRINT_VAL(bmpInfoHeader.biHeight);
    PRINT_VAL(bmpInfoHeader.biPlanes);
    PRINT_VAL(bmpInfoHeader.biBitCount);
    PRINT_VAL(bmpInfoHeader.biCompression);
    PRINT_VAL(bmpInfoHeader.biSizeImage);
    PRINT_VAL(bmpInfoHeader.biXPixPerMeter);
    PRINT_VAL(bmpInfoHeader.biYPixPerMeter);
    PRINT_VAL(bmpInfoHeader.biClrUsed);
    PRINT_VAL(bmpInfoHeader.biClrImporant);
#endif

    if (bmpInfoHeader.biWidth != 480
    ||  bmpInfoHeader.biHeight != 800
    ||  bmpInfoHeader.biBitCount != 24) {
        fprintf(stderr, "error: bitmap format is not 800x480 24bit.\n");
        ret = -1;
        goto exit;
    }

    uint8_t *pLine;
    printf("const unsigned long LOGO_RGB24[] = {\n");
    for (uint32_t h = 0; h < bmpInfoHeader.biHeight; h++) {
        pLine = pBmpData + (bmpInfoHeader.biHeight - h - 1) * bmpInfoHeader.biWidth * 3;
        for (uint32_t w = 0; w < bmpInfoHeader.biWidth; w++) {
            uint32_t rgba = read_3byte(pLine);
            printf("0x%08x,", rgba);
            if ((++count % 10) == 0) {
                printf("\n");
                count=0;
            }
        }
    }
    printf("};\n");

    fprintf(stderr, "success: splash header output complete.\n");
exit:
    if (pFileBuf) free(pFileBuf);
    return ret;
}






