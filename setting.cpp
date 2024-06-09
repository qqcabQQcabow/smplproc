#include "includes.h"
#include <string>
#include "setting.h"

void Settings::init(){
    std::ifstream f;
    f.open("visual.txt");
    if(!f.is_open()){
        std::cout << "ERROR: file: \"visual.txt\" is not exist" << std::endl;
        std::exit(1);
    }
    f >> Settings::font_name >> Settings::theme;
    f.close();
    f.open("cfg.txt");
    if(!f.is_open()){
        std::cout << "ERROR: \"cfg.txt\" is not exist!!!" << std::endl;
        std::exit(1);
    }
    std::string tmp;
    int indx = 0;
    int h_size;
    bool is_string;
    while(indx < ATR_COUNT){
        tmp = "";
        std::getline(f, tmp);
        std::istringstream iss(tmp);
        iss >> Settings::printflag[indx] >> tmp >> h_size >> is_string; 
        if(printflag[indx]){
            Settings::count_head++;
            Settings::headers.push_back(tmp);
            Settings::headers_size.push_back(h_size);
            Settings::is_string.push_back(is_string);
        }
        indx++;
    }
    f.close();
    Settings::signals["SIGHUP"] = SIGHUP;
    Settings::signals["SIGINT"] = SIGINT;
    Settings::signals["SIGQUIT"] = SIGQUIT;
    Settings::signals["SIGILL"] = SIGILL;
    Settings::signals["SIGTRAP"] = SIGTRAP;
    Settings::signals["SIGABRT"] = SIGABRT;
    Settings::signals["SIGBUS"] = SIGBUS;
    Settings::signals["SIGFPE"] = SIGFPE;
    Settings::signals["SIGKILL"] = SIGKILL;
    Settings::signals["SIGUSR1"] = SIGUSR1;
    Settings::signals["SIGSEGV"] = SIGSEGV;
    Settings::signals["SIGUSR2"] = SIGUSR2;
    Settings::signals["SIGPIPE"] = SIGPIPE;
    Settings::signals["SIGALRM"] = SIGALRM;
    Settings::signals["SIGTERM"] = SIGTERM;
    Settings::signals["SIGSTKFLT"] = SIGSTKFLT;
    Settings::signals["SIGCHLD"] = SIGCHLD;
    Settings::signals["SIGCONT"] = SIGCONT;
    Settings::signals["SIGSTOP"] = SIGSTOP;
    Settings::signals["SIGTSTP"] = SIGTSTP;
    Settings::signals["SIGTTIN"] = SIGTTIN;
    Settings::signals["SIGTTOU"] = SIGTTOU;
    Settings::signals["SIGURG"] =  SIGURG;
    Settings::signals["SIGXCPU"] = SIGXCPU;
    Settings::signals["SIGXFSZ"] = SIGXFSZ;
    Settings::signals["SIGVTALRM"] = SIGVTALRM;
    Settings::signals["SIGPROF"] = SIGPROF;
    Settings::signals["SIGWINCH"] = SIGWINCH;
    Settings::signals["SIGIO"] = SIGIO;
    Settings::signals["SIGPWR"] = SIGPWR;
    Settings::signals["SIGSYS"] = SIGSYS;
}
bool* Settings::getArrayPrfl(){
    return Settings::printflag;
}
std::string Settings::getTheme(){
    return Settings::theme;
}
std::string Settings::getFont(){
    return Settings::font_name;
}
bool Settings::getIsStr(int indx){
    return Settings::is_string[indx];
}
std::string Settings::getHead(int indx){
    return Settings::headers[indx];
}
int Settings::getCountHead(){
    return Settings::count_head;
}
int Settings::getHeSize(int indx){
    return Settings::headers_size[indx];
}
void Settings::destruct(){
    Settings::headers.clear();
    Settings::headers_size.clear();
    Settings::is_string.clear();
    Settings::count_head = 0;
}
std::vector<std::string> Settings::getHeaders(){
    return Settings::headers;
}
tgui::Texture Settings::GoodIcon(){
    return Settings::good_icon;
}
tgui::Texture Settings::MedIcon(){
    return Settings::med_icon;
}
tgui::Texture Settings::HightIcon(){
    return Settings::hight_icon;
}
void Settings::initColor(){
    Settings::good_icon = tgui::Texture("green.png", tgui::UIntRect(0,0,3,14), tgui::UIntRect(0,0,0,0), true);
    Settings::med_icon = tgui::Texture("orange.png", tgui::UIntRect(0,0,3,14), tgui::UIntRect(0,0,0,0), true);
    Settings::hight_icon = tgui::Texture("red.png", tgui::UIntRect(0,0,3,14), tgui::UIntRect(0,0,0,0), true);
    
}

int Settings::getIntSig(std::string s){
    if(Settings::signals.count(s) > 0){
        return signals[s];
    }
    else{
        return -1;
    }
}
