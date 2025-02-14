#include <iostream>
#include <opencv2/opencv.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

using namespace cv;
using namespace std;


void drawText(Mat& img, const string& text, const string& fontPath, int fontSize, Point position, Scalar color) {
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        cerr << "Could not initialize FreeType library" << endl;
        return;
    }

    FT_Face face;
    if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
        cerr << "Failed to load font: " << fontPath << endl;
        FT_Done_FreeType(ft);
        return;
    }

    FT_Set_Pixel_Sizes(face, 0, fontSize);

    int x = position.x;
    int baselineY = position.y;

    for (char c : text) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            cerr << "Failed to load character: " << c << endl;
            continue;
        }

        FT_GlyphSlot g = face->glyph;

        // Handle spaces correctly
        if (c == ' ') {
            x += g->advance.x >> 6;
            continue;
        }

        if (g->bitmap.buffer == nullptr || g->bitmap.rows == 0 || g->bitmap.width == 0) {
            cerr << "Skipping empty glyph for character: " << c << endl;
            x += g->advance.x >> 6;
            continue;
        }

        Mat glyphBitmap(g->bitmap.rows, g->bitmap.width, CV_8UC1, g->bitmap.buffer);
        Mat glyphMask;
        cvtColor(glyphBitmap, glyphMask, COLOR_GRAY2BGR);  // Convert grayscale to BGR

        // Position the glyph correctly
        int x_offset = x + g->bitmap_left;
        int y_offset = baselineY - g->bitmap_top;

        if (x_offset < 0 || y_offset < 0 || x_offset + glyphBitmap.cols > img.cols || y_offset + glyphBitmap.rows > img.rows) {
            cerr << "Skipping character out of bounds: " << c << endl;
            x += g->advance.x >> 6;
            continue;
        }

        Mat imgROI = img(Rect(x_offset, y_offset, glyphBitmap.cols, glyphBitmap.rows));

        // Apply text color and blend glyph onto image
        for (int row = 0; row < glyphBitmap.rows; row++) {
            for (int col = 0; col < glyphBitmap.cols; col++) {
                uchar glyphValue = glyphBitmap.at<uchar>(row, col);
                if (glyphValue > 0) {
                    imgROI.at<Vec3b>(row, col) = Vec3b(
                        (color[0] * glyphValue) / 255,
                        (color[1] * glyphValue) / 255,
                        (color[2] * glyphValue) / 255
                    );
                }
            }
        }

        // Move cursor to the next character position
        x += g->advance.x >> 6;
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}


int main() {
    Mat img(600, 600, CV_8UC3, Scalar(255, 255, 255));
    string fontPath = "roboto/static/Roboto-Medium.ttf";
    string imgNamePath = "output.png";

    drawText(img, "Hello, World!", fontPath, 32, Point(50, 100), Scalar(0, 0, 0));
    imwrite(imgNamePath, img);
    cout << "Image saved as " << imgNamePath << endl;

    return 0;
}

