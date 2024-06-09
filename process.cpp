#include "includes.h"
#include "process.h"

std::string Process::getATR(int indx){
    if(indx >= proc_atributes.size()){
        return "!!!";
    }
    return Process::proc_atributes[indx];
}
void Process::initMem(bool* fl, std::string path, std::vector<std::string> headers){
    std::ifstream f_mem(path + "/statm");
    long int size, resident, shared;
    double res;

    if(f_mem.is_open()){
        f_mem >> size >> resident >> shared;
        f_mem.close();
    }
    else{
        std::cout << "ERROR: file: " << path + "/statm" << " is not exist!!!" << std::endl; 
        std::exit(1);
    }

    res = double(resident) * sysconf(_SC_PAGESIZE);
    res = res / 1024 / 1024;

    std::string temp = std::to_string(res);
    temp.erase(temp.end()-4, temp.end());

    for(int i = 0; i < headers.size(); i++){
        if(headers[i] == "MEM"){
            Process::proc_atributes.insert(Process::proc_atributes.begin() + i, temp + "Mb");
            break;
        }
    }

}

std::chrono::time_point<std::chrono::high_resolution_clock> Process::getTime(){
    return Process::prev_time;    
}

void Process::initCpuPercent(double prev, bool* fl, std::vector<std::string> headers, std::chrono::duration<long long, std::milli> diff){
    double total_tact = 0;
    for(int i = 0; i < Process::data_for_cpu.size()-1; i++){ // сумма utime stime cstime cutime
        total_tact += std::stoi(Process::data_for_cpu[i]);
    }
    Process::cpu_percent = (abs(prev - total_tact ) / (double(sysconf(_SC_CLK_TCK))*diff.count()/1000) ) * 100; // расчет использования cpu
    Process::total_time = total_tact;
    std::string temp = "";
    if(prev == 0){ // только что появился процесс
        Process::cpu_percent = 0;
    }

    if(fl[CPU]){
        for(int i = 0; i < 4; i++){
            temp += std::to_string(Process::cpu_percent)[i]; 
        }
        for(int i = 0; i < headers.size(); i++){
            if(headers[i] == "CPU"){
                Process::proc_atributes.insert(Process::proc_atributes.begin() + i, temp+"%");
                break;
            }
        }
    }
}
double Process::getTotTime(){
    return Process::total_time;
}
bool Process::makeProc(std::string path, bool* fl, double prev, std::vector<std::string> headers){
    std::ifstream fl_stat;
    std::string str;
    fl_stat.open(path + "/loginuid");
    if(fl_stat.is_open()){
        std::getline(fl_stat, str);
        fl_stat.close();
    }
    else{
        std::cout << "ERROR: file: " << path + "/loginuid" << " is not exist!!!" << std::endl;
        std::exit(1);
    }
    if(str.length() > 6) str = "0";
    unsigned int uid_num = std::stoi(str);
    struct passwd *pw = getpwuid(uid_num);
    if(fl[0]) Process::proc_atributes.push_back(pw->pw_name);
    Process::Uid = pw->pw_name;
    str = "";
    fl_stat.open(path + "/stat");
    if(fl_stat.is_open()){
        std::getline(fl_stat, str);
        fl_stat.close();
    }
    else{
        std::cout << "ERROR: file: " << path + "/stat" << " is not exist!!!" << std::endl; 
        std::exit(1);
    }
    int i = 0;
    int indx = 0;
    int cont_count = 0;
    std::string temp="";
    while(i < str.length()){
        if(str[i] != ' '){
            if(str[i] == '(')  {
                i++;
                while(str[i] != ')'){
                    temp += str[i];
                    i++;
                }
                i++;
                continue;
            }
            temp += str[i];
            i++;
        }
        else{
            // исключение атрибутов, не входящих в stat из чтения /proc/pid/stat: их вывод учитывается при их инициализации
            if(indx == UID) indx++; 
            if(indx == MEM) indx++;
            if(indx == CPU) indx++;
            
            if((indx > 36) && (cont_count < 3)){ // пропуск 3 пунктов, которые не валидны
                cont_count++;
                temp = "";
                i++;
                continue;
            }
            if(indx == PID){ // сохранение pid в отдельное место
                Process::Pid = temp;
            }
            
            if(fl[indx]) Process::proc_atributes.push_back(temp);
            // лучше делать if(fl[CPU]) и поместить это условие перед вызовом initCPU, но тогда при выборе влажка cpu в настройках 1 секунду будет 100 нагрузка(что не верно)
            switch (indx) { // данные для расчёта cpu 
                case UTIME:
                    Process::data_for_cpu.push_back(temp);
                    break;
                case STIME:
                    Process::data_for_cpu.push_back(temp);
                    break;
                case CUTIME:
                    Process::data_for_cpu.push_back(temp);
                    break;
                case CSTIME:
                    Process::data_for_cpu.push_back(temp);
                    break;
                case STARTTIME:
                    Process::data_for_cpu.push_back(temp);
                    break;
            }
            temp = "";
            i++;
            indx++;
        }
    }
    if(fl[indx]) Process::proc_atributes.push_back(temp);
    if(fl[MEM])Process::initMem(fl, path, headers);
    //Process::initCpuPercent(prev, fl, headers);
    double total_tact = 0;
    fl_stat.close();
    Process::prev_time = std::chrono::high_resolution_clock::now();
    return 1;
}
std::string Process::getStrToPrint(){
    std::string to_print = "";
    for(int i = 0; i < proc_atributes.size(); i++){
        to_print += Process::proc_atributes[i] + "\t";
    }
    return to_print;
}
std::vector<tgui::String> Process::getVecToListView(){
    std::vector<tgui::String> res;
    for(int i = 0; i < proc_atributes.size(); i++){
        res.push_back(Process::proc_atributes[i]);
    }
    return res;
}
bool Process::Kill_proc(){
    pid_t must_dead = std::stoi(Process::Pid);
    return kill(must_dead, SIGTERM);
}

bool Process::send_sig(int sig){
    pid_t must_sg = std::stoi(Process::Pid);
    return kill(must_sg, sig);
}

double Process::getCpuPercent(){
    return Process::cpu_percent;
}

std::string Process::getUID(){
    return Process::Uid;
}
std::string Process::getPID(){
    return Process::Pid;
}
