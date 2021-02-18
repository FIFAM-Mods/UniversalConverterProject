#include "PortraitDialog.h"
#include "libpng/png.h"

using namespace plugin;

bool gPortraitImagePNGResult = false;
wchar_t const *gPortraitImageFilename = nullptr;
int *gPortraitImageOutWidth = nullptr;
int *gPortraitImageOutHeight = nullptr;
int *gPortraitImageOutDepth = nullptr;

bool OnGetPortraitImageInfo(wchar_t const *filename, int *outWidth, int *outHeight, int *outDepth) {
    gPortraitImagePNGResult = false;
    gPortraitImageFilename = filename;
    gPortraitImageOutWidth = outWidth;
    gPortraitImageOutHeight = outHeight;
    gPortraitImageOutDepth = outDepth;
    return CallAndReturn<bool, 0x9A9570>(filename, outWidth, outHeight, outDepth) || gPortraitImagePNGResult;
}

inline static struct IO {
    unsigned char *data;
    unsigned int size;
    unsigned int numReadBytes;

    void Init(unsigned char *Data, unsigned int Size) {
        data = Data;
        size = Size;
        numReadBytes = 0;
    }
} gPngIO;

static void PngReadFunction(png_structp png_ptr, png_bytep data, png_uint_32 length) {
    IO *myIo = (IO *)png_get_io_ptr(png_ptr);
    if (myIo) {
        if (length <= myIo->size - myIo->numReadBytes) {
            memcpy(data, &myIo->data[myIo->numReadBytes], length);
            myIo->numReadBytes += length;
        }
    }
}

int OnPortraitTpiCheck(wchar_t const *str, wchar_t const *substr) {
    if (!CallAndReturn<int, 0x1493FCB>(str, L"png")) {
        void *sys = *raw_ptr<void *>((void *)0x30ABBC0);
        unsigned char *pngData = CallVirtualMethodAndReturn<unsigned char *, 1>(sys, gPortraitImageFilename, 0);
        if (pngData) {
            unsigned int dataSize = CallVirtualMethodAndReturn<unsigned int, 16>(sys, pngData);
            if (dataSize > 0) {
                png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
                if (png_ptr) {
                    png_infop info_ptr = png_create_info_struct(png_ptr);
                    if (info_ptr) {
                        gPngIO.Init(pngData, dataSize);
                        png_set_read_fn(png_ptr, &gPngIO, PngReadFunction);
                        png_read_info(png_ptr, info_ptr);
                        unsigned int width = 0;
                        unsigned int height = 0;
                        int depth = 0;
                        int color_type = 0;
                        png_uint_32 result = png_get_IHDR(png_ptr, info_ptr, &width, &height, &depth, &color_type, 0, 0, 0);
                        if (result == 1 && depth == 8 && (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_RGB_ALPHA)) {
                            gPortraitImagePNGResult = true;
                            *gPortraitImageOutWidth = width;
                            *gPortraitImageOutHeight = height;
                            *gPortraitImageOutDepth = depth;
                        }
                        png_destroy_read_struct(&png_ptr, &info_ptr, 0);
                    }
                    else
                        png_destroy_read_struct(&png_ptr, 0, 0);
                }
            }
            CallVirtualMethod<15>(sys, pngData);
        }
        return 1;
    }
    else
        return CallAndReturn<int, 0x1493FCB>(str, substr);
}

BOOL PortraitDimensionsCheck(int width, int height) {
    return width > 0 && height > 0 && width <= 4096 && height <= 4096;
}

void METHOD OnShowPortraitFileDialogForMyCareer(void *t, DUMMY_ARG, int a2, int a3, int a4) {
    *raw_ptr<int>(t, 0x5940) = 5; // images limit
    CallMethod<0x9ACDF0>(t, a2, a3, a4);
}

bool BadgeDimensionsCheck(void *dynShape, wchar_t const *filename) {
    int imageDesc[5];
    CallVirtualMethod<5>(dynShape, imageDesc, filename, 0x400000, 0, 0);
    return imageDesc[0] > 0 && imageDesc[1] > 0 && imageDesc[0] <= 4096 && imageDesc[1] <= 4096;
}

void PatchPortraitDialog(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        patch::RedirectCall(0x9A995A, OnGetPortraitImageInfo);
        patch::RedirectCall(0x9A9AB0, OnGetPortraitImageInfo);
        patch::RedirectCall(0x9ABED8, OnGetPortraitImageInfo);
        patch::RedirectCall(0x9AC4E3, OnGetPortraitImageInfo);
        patch::RedirectCall(0x9AC9B2, OnGetPortraitImageInfo);
        patch::RedirectCall(0x9A9749, OnPortraitTpiCheck);
        patch::RedirectJump(0x9A7820, PortraitDimensionsCheck);
        patch::SetPointer(0x9A80BD + 1, L"*.tga;*.bmp;*.png");
        // fix: set 5 images limit for MyCareer manager photos selection
        patch::RedirectCall(0x7CF5FB, OnShowPortraitFileDialogForMyCareer);

        // club badges
        //patch::SetPointer(0x9A0AAE + 1, L"*.tga;*.bmp;*.png");
        //patch::RedirectCall(0x9A0E2E, BadgeDimensionsCheck);
    }
}
