#pragma once
#include <qpixmap.h>
#include <vector>
#include <string>

//typedef  struct
//{
//    std::string path;
//    double ratio;
//} Img;
//
//typedef struct
//{
//    std::string headline;
//    std::vector<std::string> addition;
//} Title;

//typedef struct
//{
//    std::string sub_title;
//    std::vector<std::string> body_text;
//} Text;

typedef struct
{
    std::vector<std::string> word_list;
} Text;


class Article
{
public:
	Article(){};
    std::string title_;
    std::vector<Text> text_list_;
    std::vector<QPixmap> img_list_;
    std::vector<std::pair<std::vector<int>, int>> ref_list_;
    std::vector<std::pair<int, double>> ref_ratio_list_;
};

