#include "includes.h"
#include <TGUI/Texture.hpp>

class Settings{
private:
    bool printflag[ATR_COUNT];
    std::vector<bool> is_string;
    std::vector<int> headers_size;
    int count_head = 0;
    std::vector<std::string> headers;
    std::string font_name;
    std::string theme;
    tgui::Texture good_icon;
    tgui::Texture med_icon;
    tgui::Texture hight_icon;
    std::map<std::string, int> signals;
public:
    void init();

    std::vector<int> getSeq();
    bool* getArrayPrfl();
    bool getIsStr(int indx);
    std::string getHead(int indx);
    int getCountHead();
    int getHeSize(int indx);
    void destruct();
    std::vector<std::string> getHeaders();
    tgui::Texture GoodIcon();
    tgui::Texture MedIcon();
    tgui::Texture HightIcon();
    void initColor();
    std::string getTheme();
    std::string getFont();    
    int getIntSig(std::string s);
};
