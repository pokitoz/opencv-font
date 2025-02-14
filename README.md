sudo apt-get install libopencv-dev

g++ font.cpp -o text_render `pkg-config --cflags --libs opencv4 freetype2`