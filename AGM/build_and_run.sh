g++ -std=c++17 -Wall -o image_segmenter main.cpp Disjoint.cpp Segmenter.cpp GaussianBlur.cpp -I. -lpng -lm 
./image_segmenter 