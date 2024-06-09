#include "includes.h"

class Process{
private:
    std::vector<std::string> proc_atributes; // атрибуты процесса, которые отмечены в bool* printfl
    std::vector<std::string> data_for_cpu; // данные 5 атрибутов для расчета cpu_percent
    double cpu_percent; // использование процессора в процентах
    double total_time = 0; // общее колличество тиков процесса
    std::string Pid;
    std::string Uid;
    std::chrono::time_point<std::chrono::high_resolution_clock> prev_time;
public:
    double getTotTime(); // получить total_time

    void initUID(std::ifstream &f); // инициализация uid
    
    std::string getUID(); // получить uid

    void initMem(bool* fl, std::string path, std::vector<std::string> headers); // инициализация использования памяти

    bool makeProc(std::string path, bool* fl, double prev, std::vector<std::string> headers); // инициализовать процесс из файла /proc/PID/stat/

    double getCpuPercent(); // получить cpu_percent

    bool Kill_proc(); // убить процесс

    std::string getStrToPrint(); // получить строку из proc_atributes
    
    std::vector<tgui::String> getVecToListView(); // получить вектор для ListWiew с атрибутами процесса
    
    void initCpuPercent(double prev, bool* fl, std::vector<std::string> headers, std::chrono::duration<long long, std::milli> diff); // инициализация cpu_percent

    std::string getPID(); // получить pid

    std::string getATR(int indx); // получить атрибут из proc_atributes по индексу

    bool send_sig(int sig); // отправить сигнал

    std::chrono::time_point<std::chrono::high_resolution_clock> getTime(); // получить время инициализации процесса
};
