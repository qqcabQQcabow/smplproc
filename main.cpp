#include "includes.h"
#include "process.h"
#include "setting.h"
#include <TGUI/String.hpp>
#include <cstdlib>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

Settings config;
bool is_dir(fs::file_status s); // если s дикертория, то 1
bool is_pid(fs::path p); // если p pid(число), то 1 
std::string toup(std::string s); // функция перевода в верхний регистр

bool sravnenie(std::string s, std::string s_1); // сравнение двух строк, если s > s_1, то вернет 1

void sort_proc(std::vector<Process> &proc_array, int atr_indx, char c, int last_pin);
std::vector<Process> finder_sort(std::vector<Process> &proc, tgui::String promt_temp, int indx, int last_pin);

std::vector<Process> switch_user_button_sort(std::vector<Process> &proc, tgui::String promt_temp, int last_pin);
void coloring_proc(Process &proc_array_next, tgui::ListView::Ptr &Table, int i, std::vector<std::string> pinned);

int main(void) {
    // получение настроек для вывода 
    config.init();
    
    std::vector<Process> proc_array;
    std::vector<Process> proc_array_next;
    std::vector<Process> proc_array_tick;
    std::vector<std::string> pinned;
    int last_pin = 0;
    fs::path p {"/proc"};
    Process a; // процесс 

std::chrono::seconds oneSecond(1000); // 1 секунда в миллисекуднах

    int jindex = 0;
    for (auto const& dir_entry : std::filesystem::directory_iterator(p)){
        if( is_dir(fs::status(dir_entry.path())) && is_pid(dir_entry.path()) ){
            proc_array.push_back(a);
            proc_array[jindex].makeProc(dir_entry.path(), config.getArrayPrfl(), 0, config.getHeaders());
            proc_array[jindex].initCpuPercent(0, config.getArrayPrfl(), config.getHeaders(), oneSecond);
            jindex++;
        }    
    }
    proc_array_tick = proc_array;
    proc_array_next = proc_array;

    // поиск данных о температуре процессора
    fs::path sys {"/sys/class/hwmon"};
    std::ifstream for_sys;
    int cpu_temp;
    std::string name;
    for (auto const& dir_entry : std::filesystem::directory_iterator(sys)){
        for_sys.open(dir_entry.path() / "name");
        name = "";
        if(for_sys.is_open()){
            std::getline(for_sys, name);
        }
        else{
            std::cout << "ERROR: file " << dir_entry.path() / "name" << " is not exist!!!" << std::endl;
            return 1;
        }
        for_sys.close();
        if(name == "gigabyte_wmi"){
            for_sys.open(dir_entry.path() / "temp3_input");
            name = "";
            if(for_sys.is_open()){
                std::getline(for_sys, name);
            }
            else{
                std::cout << "ERROR: file " << dir_entry.path() / "temp3_input" << " is not exist!!!" << std::endl;
                return 1;
            }
            cpu_temp = std::stoi(name) / 1000;
            for_sys.close();
            sys = dir_entry.path() / "temp3_input";
            break;
        }
        if(name == "coretemp"){
            int num = 0;
            std::string path;
            do{
                num++;
                path = "temp" + std::to_string(num) + "_label"; 
                for_sys.open(dir_entry.path() / path);
                name = "";
                if(for_sys.is_open()){
                    std::getline(for_sys, name);
                }
                else{
                    std::cout << "ERROR: file " << dir_entry.path() / path << " is not exist!!!" << std::endl;
                    return 1;
                }
                for_sys.close();
                if(num == 100) break;
            }while(name != "Package id 0");
            
            if(num == 100){
                cpu_temp = -312;
                break;
            }
            else{
                path = "temp" + std::to_string(num) + "_input";
                for_sys.open(dir_entry.path() / path);
                name = "";
                if(for_sys.is_open()){
                    std::getline(for_sys, name);
                }
                else{
                    std::cout << "ERROR: file " << dir_entry.path() / path << " is not exist!!!" << std::endl;
                    return 1;
                }
                cpu_temp = std::stoi(name) / 1000;
                for_sys.close();
                sys = dir_entry.path() / path;
                break;
            }
            
            
        }
    }
    
    // хранить светлую и теменую тему в config и тянуть её отсюда
    tgui::Theme dark{config.getTheme()};

    // инициалиация окна
    sf::RenderWindow window({800, 600}, "SimpleProcManager", sf::Style::Default);
    tgui::Gui gui;
    window.setFramerateLimit(60);
    window.setVerticalSyncEnabled(false);
    gui.setWindow(window);
    // это тоже хранить в конфиге
    // ???????????????????сделать окно для смены шрифта??????????
    //tgui::Font sfont(config.getFont());
    //gui.setFont(sfont);

    config.initColor();

    // виджет для отображения процессов
    tgui::ListView::Ptr Table = tgui::ListView::create();
    // виджет для строки поиска
    tgui::EditBox::Ptr finder = tgui::EditBox::create();
    // вывод температуры cpu
    tgui::Label::Ptr cpu_t = tgui::Label::create();
    // окно для изменения настроек вывода
    tgui::ChildWindow::Ptr setting_window = tgui::ChildWindow::create();
    // кнопка для вызова окна изменения настроек
    tgui::Button::Ptr setting_button = tgui::Button::create();
    // поле для смены места поиска
    tgui::SpinControl::Ptr finder_val = tgui::SpinControl::create();
    // кнопка для фильтрации процессов по UID
    tgui::Button::Ptr switch_uid_button = tgui::Button::create();

    // параметры строки поиска
    int find_height = 22;
    int find_width = 150;
    finder->setPosition(0,0);
    finder->setSize(find_width, find_height);
    finder->setDefaultText("Enter text:");
    finder->setRenderer(dark.getRenderer("EditBox"));
    finder->setMaximumCharacters(33);

    // параметры выбора места для поиска
    int finder_area_num = -1;
    for(int i = 0; i < config.getCountHead(); i++){
        if(config.getHead(i) == "EXE_NAME"){
            finder_area_num = i+1; 
            break;
        }
    }
    if(finder_area_num < 0) finder_area_num = 1;
    finder_val->setPosition(find_width, 0);
    finder_val->setSize(29, find_height);
    finder_val->setMinimum(1);
    finder_val->setMaximum(config.getCountHead());
    finder_val->setStep(1);
    finder_val->setValue(finder_area_num);
    finder_val->setRenderer(dark.getRenderer("SpinControl"));

    // параметры вывода температуры
    cpu_t->setPosition(find_width + finder_val->getSize().x + 3, 3);
    cpu_t->getRenderer()->setTextColor(sf::Color::Cyan);
    cpu_t->getRenderer()->setTextSize(find_height-7);
    cpu_t->setText("CPU temp is: " + std::to_string(cpu_temp));

    // некоторые параметры ListView
    Table->setRenderer(dark.getRenderer("ListView"));
    Table->setGridLinesWidth(1);
    Table->setTextSize(16);
    Table->setItemHeight(30);
    Table->setHeaderHeight(19);
    Table->getRenderer()->setTextColor(sf::Color(208,208,208));
    Table->setSize(800, 600 - find_height);
    Table->setResizableColumns(true);
    Table->setPosition(0, find_height);
    Table->setVerticalScrollbarValue(Table->getItemCount());
    Table->setShowVerticalGridLines(false);
    Table->setSeparatorWidth(1);

    // параметры кнопки вызова меню настроек 
    setting_button->setText("Setting");
    setting_button->setRenderer(dark.getRenderer("Button"));
    setting_button->setSize(find_width/2, find_height);
    setting_button->setPosition(800 - /*kill_button->getSize().x - */find_width/2, 0);


    switch_uid_button->setRenderer(dark.getRenderer("Button"));
    switch_uid_button->setSize(find_width/2, find_height);
    switch_uid_button->setPosition(800 - setting_button->getSize().x, 0);
    gui.add(switch_uid_button);

    // параметры окна с настройкой
    setting_window->setRenderer(dark.getRenderer("ChildWindow"));
    setting_window->setPosition(300, find_height);
    setting_window->setSize(300, 100);
    setting_window->setTitle("Settings");
    setting_window->setTitleButtons(tgui::ChildWindow::TitleButton::Close);
    setting_window->setResizable(true);
    setting_window->setKeepInParent(true);

    // заполнение заголовков столбцов
    for(int i = 0; i < config.getCountHead(); i++){
        if((config.getHead(i) == "EXE_NAME") || (config.getHead(i) == "UID")) Table->addColumn(config.getHead(i), config.getHeSize(i), tgui::ListView::ColumnAlignment::Left);
        else Table->addColumn(config.getHead(i), config.getHeSize(i), tgui::ListView::ColumnAlignment::Center);
    }
    
    // заполнение списка информацией
    for (int i = 0; i < proc_array.size(); i++) {
        Table->addItem(proc_array[i].getVecToListView());
        Table->setItemIcon(i, config.GoodIcon());
    }
    Table->setVerticalScrollbarValue(0);

    // время обновления информации в секундах
    int delay = DEFAUL_DELAY;

    // лямбда функция для обработки нажатия на заголовок
    int is_sorted = 0;
    unsigned int sort_rule = 0;
    Table->onHeaderClick([&is_sorted, &delay, &Table, &sort_rule](int index){
        sort_rule++;
        sort_rule = sort_rule == 3 ? 0 : sort_rule;
        delay = 0;
        if((is_sorted != index) && (is_sorted > -1)){
            sort_rule = 1;
            Table->setColumnText(is_sorted, config.getHead(is_sorted));
        }
        is_sorted = index;
    });

    // лямбда функция для обработки кнопки смены фильтра по пользователю
    int user = 0;
    std::vector<std::string> user_list;
    sort_proc(proc_array, 0, 'D', last_pin);
    user_list.push_back("all");
    user_list.push_back(proc_array[0].getUID());
    for(int i = 0; i < proc_array.size(); i++){
        if(proc_array[i].getUID() != user_list[user_list.size()-1]){
            user_list.push_back(proc_array[i].getUID());
        }
    }
    switch_uid_button->setText("all");
    switch_uid_button->onPress([&delay, &switch_uid_button, &user, &user_list](const tgui::String &text){
        delay = 0;
        user++;
        if(user >= user_list.size()){
            user = 0;
            switch_uid_button->setText(user_list[user]);
            return;
        }
        switch_uid_button->setText(user_list[user]);
    });
    
    // иерархия функций для выпадающего меню при нажаниии на таблицу
    bool menufl = false;;
    bool enter_flag = false;
    bool full_info_flag = false;
    tgui::Vector2f mose_pos;
    tgui::ListBox::Ptr menu = NULL;
    tgui::EditBox::Ptr sig_enter = NULL;
    tgui::Label::Ptr full_info = NULL;
    // обработка нажания на listwiew
    Table->onRightClick([&pinned, &delay, &last_pin, &full_info, &full_info_flag, &enter_flag, &sig_enter, &Table, &menufl, &mose_pos, &proc_array_next, &proc_array_tick, &gui, &menu, &dark, &finder](int index){
        if((menufl) || (index == -1)){ // если меню уже открыто, то оно удалиться и потом откроется новое на другом месте
            enter_flag = false;
            full_info_flag = false;
            if(sig_enter != NULL){
                gui.remove(sig_enter);  
                sig_enter = NULL;
            } 
            if(full_info != NULL){
                gui.remove(full_info);
                full_info = NULL;
            } 
            if(menu != NULL){
                gui.remove(menu);  
                menu = NULL;
            } 
            if(index == -1){
                Table->setSelectedItem(index);
                return;
            }
        }
        else{
            menufl = true;
        }
        Table->setSelectedItem(index);
        menu = tgui::ListBox::create();
        menu->setRenderer(dark.getRenderer("ListBox"));
        menu->setSize(80, 80);
        menu->setPosition(mose_pos);
        if(menu->getPosition().x + menu->getSize().x > Table->getSize().x){
            menu->setPosition(mose_pos.x - menu->getSize().x, mose_pos.y);
        }
        if(menu->getPosition().y + menu->getSize().y > Table->getSize().y){
            menu->setPosition(menu->getPosition().x, mose_pos.y - menu->getSize().y);
        }
        menu->addItem("Pin");
        for(auto pin : pinned){
            if(proc_array_next[index].getPID() == pin){
               menu->changeItem("Pin", "Unpin");
            }
        }
        menu->addItem("Send Signal");
        menu->addItem("Show more");
        menu->addItem("Kill");
        menu->setFocused(true);
        
        gui.add(menu);
        
        // обработка нажатия на меню
        menu->onMouseRelease([index, &proc_array_tick, &delay, &pinned, &full_info, &full_info_flag, &enter_flag, &sig_enter, &Table, &menufl, &mose_pos, &proc_array_next, &gui, &menu, &dark, &finder](int indx){
            if(indx == KILL-10){
                proc_array_next[index].send_sig(SIGTERM);
                delay = 0;
                if(menu != NULL) menu->removeAllItems();
                menufl = false;
                enter_flag = false;
                full_info_flag = false;
                if(sig_enter != NULL){
                    gui.remove(sig_enter);  
                    sig_enter = NULL;
                } 
                if(full_info != NULL){
                    gui.remove(full_info);
                    full_info = NULL;
                } 
                if(menu != NULL){
                    gui.remove(menu);  
                    menu = NULL;
                } 
            }
            else if(indx == SEND_SIGNAL-10){
                if(enter_flag){
                    enter_flag = false;
                    if(sig_enter != NULL){
                        gui.remove(sig_enter);  
                        sig_enter = NULL;
                    } 
                    return;
                }
                else{
                    enter_flag = true;
                }
                sig_enter = tgui::EditBox::create();
                sig_enter->setRenderer(dark.getRenderer("EditBox"));
                sig_enter->setDefaultText("Enter signal:");
                sig_enter->setPosition(menu->getPosition().x + menu->getSize().x, menu->getItemHeight() + menu->getPosition().y);
                if((sig_enter->getPosition().x + sig_enter->getSize().x) > Table->getSize().x){
                    sig_enter->setPosition(menu->getPosition().x - sig_enter->getSize().x, menu->getPosition().y +  menu->getItemHeight());
                }
                // обработка введённого сигнала(сделать массив в config и cfg с этими данными)
                sig_enter->onReturnKeyPress([&, index](const tgui::String &text){
                    std::string signalName;
                    for(auto s : text) signalName += s;
                    
                    if(config.getIntSig(toup(signalName)) < 0){
                        sig_enter->setText("");
                        sig_enter->setDefaultText("Signal is not eixist!"); // Сигнал не найден
                        enter_flag = false;
                        return;
                    }
                    else{
                        proc_array_next[index].send_sig(config.getIntSig(toup(signalName)));
                    }
                    sig_enter->setText("");
                    sig_enter->setDefaultText("Done!"); // Сигнал не найден
                    
                    if(menu != NULL) menu->removeAllItems();
                    menufl = false;
                    enter_flag = false;
                    full_info_flag = false;
                    if(sig_enter != NULL){
                        gui.remove(sig_enter);  
                        sig_enter = NULL;
                    } 
                    if(full_info != NULL){
                        gui.remove(full_info);
                        full_info = NULL;
                    } 
                    if(menu != NULL){
                        gui.remove(menu);  
                        menu = NULL;
                    } 
                });
                gui.add(sig_enter);
                
            }
            else if(indx == SHOW_INFO-10){
                if(full_info_flag){
                    gui.remove(full_info);
                    full_info_flag = false;
                    return;
                }
                else{
                    full_info_flag = true;
                }
                std::ifstream f_status("/proc/" + proc_array_next[index].getPID() + "/status");
                full_info = tgui::Label::create();
                full_info->setRenderer(dark.getRenderer("Label"));
                full_info->getRenderer()->setBackgroundColor(sf::Color::Black);
                full_info->setScrollbarPolicy(tgui::Scrollbar::Policy::Automatic);
                full_info->getRenderer()->setTextColor(sf::Color::Green);
                std::string temptemp;
                std::string res = "this info dont updating!!!\n";
                if(f_status.is_open()){
                    while(std::getline(f_status, temptemp)){
                        res += temptemp + '\n';
                    }
                }
                full_info->setText(res);
                full_info->setPosition(menu->getPosition().x + menu->getSize().x, menu->getPosition().y + menu->getSize().y - menu->getItemHeight());
                // настройка позиционированния окна
                if((full_info->getPosition().x + 300) > Table->getSize().x){
                    full_info->setPosition(menu->getPosition().x - 300, menu->getPosition().y + menu->getSize().y - menu->getItemHeight());
                }
                if((full_info->getPosition().y + 300) > Table->getSize().y){
                    full_info->setPosition(full_info->getPosition().x, menu->getPosition().y - 300 + menu->getItemHeight());
                }
                full_info->setSize(300, 300);
                f_status.close();
                gui.add(full_info);
            }
            else if(indx == PINS - 10){
                delay = 0;
                if(menu->getItemByIndex(PINS - 10) == "Pin"){
                    pinned.push_back(proc_array_next[index].getPID());
                } 
                else{
                    for(int i = 0; i < pinned.size(); i++){
                        if(pinned[i] == proc_array_next[index].getPID()){
                            pinned.erase(pinned.begin() + i);
                            break;
                        }
                    }
                }   
                if(menu != NULL) menu->removeAllItems();
                menufl = false;
                enter_flag = false;
                full_info_flag = false;
                if(sig_enter != NULL){
                    gui.remove(sig_enter);  
                    sig_enter = NULL;
                } 
                if(full_info != NULL){
                    gui.remove(full_info);
                    full_info = NULL;
                } 
                if(menu != NULL){
                    gui.remove(menu);  
                    menu = NULL;
                } 
            }
        });
    });


    // лямбда функция для обработки строки поиска
    bool finder_flag = false;
    tgui::String finder_text;
    finder->onTextChange([&delay, &finder_text, &finder_flag](const tgui::String& text){
        finder_text = text;
        finder_flag = true;
        if(finder_text == L""){
            finder_flag = false;
        }
        delay = 0;
    });
    // лямбда функция для смены поля поиска
    finder_val->onValueChange([&finder_area_num, &finder_val, &finder, &delay](float value){
        finder_area_num = value;
        finder_val->setValue(finder_area_num);
        if(finder->getText() != ""){
            delay = 0;
        }
    });

    // 2 лямбда функции для окна настроек
    std::vector<std::string> str_from_file;
    bool switcher_window = false;
    setting_window->onClose([&switcher_window]{
        switcher_window = false;
    });
    setting_button->onPress([&dark, &finder_val, &gui, &setting_window, &str_from_file, &is_sorted, &Table, &delay, &switcher_window, &sort_rule](const tgui::String& text){
        if(switcher_window){
            return;
        }
        else{
            switcher_window = true;
        }
        tgui::ScrollablePanel::Ptr but_panel = tgui::ScrollablePanel::create();
        but_panel->setRenderer(dark.getRenderer("ScrollablePanel"));
        tgui::CheckBox::Ptr check_box[ATR_COUNT];
        std::ifstream f_set("cfg.txt");
        str_from_file.clear();
        std::string tmp;
        bool prfl;
        int but_height = 0;
        for(int i = 0; i < ATR_COUNT; i++){
            std::getline(f_set, tmp);
            str_from_file.push_back(tmp);
        }
        f_set.close();
        tmp = "";
        for(int i = 0; i < ATR_COUNT; i++){
            check_box[i] = tgui::CheckBox::create();
            tmp = str_from_file[i];
            std::istringstream iss(tmp);
            iss >> prfl >> tmp;
            check_box[i]->setRenderer(dark.getRenderer("CheckBox"));
            check_box[i]->setText(tmp);
            check_box[i]->setChecked(prfl);
            check_box[i]->setPosition(0, but_height);
            but_height += 20;
            but_panel->add(check_box[i]);
            // сюда заложить ComboBox для выброра шрифта
            
            // лямбда функция для каждой кнопки
            check_box[i]->onChange([i, &finder_val, &str_from_file, &is_sorted, &Table, &delay, &sort_rule](bool checked){
                std::ofstream out("cfg.txt");
                str_from_file[i][0] = (int)checked + 48;
                if(out.is_open()){
                    for(auto i : str_from_file){
                        out << i << std::endl;
                    }
                }
                tgui::String tmp_sort = config.getHead(is_sorted);
                config.destruct();
                config.init();
                std::string t_tmp_sort = "";
                for(auto c : tmp_sort){
                    t_tmp_sort += c;
                }
                bool work_flag = false;
                Table->removeAllColumns();
                finder_val->setMaximum(config.getCountHead());
                // заполнение заголовков столбцов
                for(int i = 0; i < config.getCountHead(); i++){
                    if((config.getHead(i) == "EXE_NAME") || (config.getHead(i) == "UID")) Table->addColumn(config.getHead(i), config.getHeSize(i), tgui::ListView::ColumnAlignment::Left);
                    else Table->addColumn(config.getHead(i), config.getHeSize(i), tgui::ListView::ColumnAlignment::Center);
                }
                for(int i = 0; i < config.getCountHead(); i++){ // смена столбца сортировки(в случаях, когда добавляются или удаляются столбцы перед отсортированным)
                    if(config.getHead(i) == t_tmp_sort){
                        is_sorted = i;
                        work_flag = true;
                        break;
                    }
                }
                if(!work_flag){
                    sort_rule = 0; 
                    is_sorted = config.getCountHead()-1;
                }
                out.close();
                delay = 0;
            });
        }
        setting_window->add(but_panel);
        gui.add(setting_window);
    });

    gui.add(Table);
    gui.add(cpu_t);
    gui.add(finder);
    gui.add(setting_button);
    gui.add(finder_val);

    sf::Clock clock;
    sf::Time elapsedTime;
    int tmp, vert, horiz, selected_item, table_size, ar_size, max_len, iteration;
    while (window.isOpen()) { // пока открыто окно
        sf::Event event;
        while (window.pollEvent(event)) { // если произошло событие
        sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
        mose_pos.x = int(mousePosition.x);
        mose_pos.y = int(mousePosition.y);
            gui.handleEvent(event);
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::Resized) {
                // Обновляем размеры и позицию виджетов при изменении размеров окна
                finder->setPosition(0,0);
                setting_button->setPosition(event.size.width - finder->getSize().x/2, 0);
                switch_uid_button->setPosition(event.size.width - setting_button->getSize().x - finder->getSize().x/2,0);
                
                setting_window->setPosition(event.size.width - find_width/2, find_height);
                setting_window->setSize(100, 300);
                
                Table->setSize(event.size.width, event.size.height - find_height);
                Table->setPosition(0, find_height);
            }
            // сброс строки поиска при нажатии escape
            if((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Escape) && (finder_flag)){
                finder_flag = false;
                finder->setText("");
            }
            // нажатие eneter при выделенном элементе
            if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Enter) && (Table->getSelectedItemIndex() > -1)){
                tmp = Table->getSelectedItemIndex();
                std::cout << "data is: " << proc_array_next[tmp].getStrToPrint() << std::endl;
                std::cout << "cpu  is: " << proc_array_next[tmp].getCpuPercent() << std::endl;
            }
            // условие закрытия выпадающего меню(если совершили прокрутку вне меню, нажали лкм вне меню, нажали на finder, нажали на esc)
            if( (((event.key.code == sf::Keyboard::Escape) && (event.type == sf::Event::KeyPressed)) || (event.type == sf::Event::MouseWheelScrolled) || (finder->isFocused()) || (event.type == sf::Event::MouseButtonPressed && (event.mouseButton.button == sf::Mouse::Left)) ) && (menufl))  {
                tgui::Widget::Ptr widget = gui.getWidgetAtPos(mose_pos, false);
                if( ((widget == menu) || (widget == full_info) || (widget == sig_enter)) && (event.key.code != sf::Keyboard::Escape) ) continue;
                
                Table->setSelectedItem(-1);
                menu->removeAllItems();
                menufl = false;
                enter_flag = false;
                full_info_flag = false;
                if(sig_enter != NULL){
                    gui.remove(sig_enter);  
                    sig_enter = NULL;
                } 
                if(full_info != NULL){
                    gui.remove(full_info);
                    full_info = NULL;
                } 
                if(menu != NULL){
                    gui.remove(menu);  
                    menu = NULL;
                } 
            }
        } 
        elapsedTime = clock.getElapsedTime();
        if (elapsedTime >= sf::seconds(delay)) {
            delay = DEFAUL_DELAY;
            clock.restart();
            proc_array_next.clear();
            // инициализация proc_array данными из proc/pid/stat
            iteration = 0;
            Process temp_t;
            last_pin = 0;
            for (auto const& dir_entry : std::filesystem::directory_iterator(p)){
                if( is_dir(fs::status(dir_entry.path())) && is_pid(dir_entry.path()) ){
                    proc_array_next.push_back(a);
                    // создание процесса
                    proc_array_next[iteration].makeProc(dir_entry.path(), config.getArrayPrfl(), 0, config.getHeaders());

                    // инициализация использования cpu
                    if(iteration < proc_array_tick.size()){
                        proc_array_next[iteration].initCpuPercent(proc_array_tick[iteration].getTotTime(), config.getArrayPrfl(), config.getHeaders(),  std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - proc_array_tick[iteration].getTime()));
                    }
                    else{
                        proc_array_next[iteration].initCpuPercent(0, config.getArrayPrfl(), config.getHeaders(), oneSecond);  
                    }
                    iteration++;
                }    
            }
            
            proc_array_tick = proc_array_next; // хранит копию proc_array_next для последующего вычисления нагрузки cpu
            
            if(!pinned.empty()){ // если есть закреплённые процессы, то из выводит наверх
                for(int i = 0; i < proc_array_next.size(); i++){
                    for(auto pin : pinned){
                        if(proc_array_next[i].getPID() == pin){
                            temp_t = proc_array_next[i];
                            proc_array_next.erase(proc_array_next.begin() + i);
                            proc_array_next.insert(proc_array_next.begin() + last_pin, temp_t);
                            last_pin++;
                        }
                    }
                }
            }
            // обновление температуры процессора
            for_sys.open(sys);
            name = "";
            std::getline(for_sys, name);
            cpu_temp = std::stoi(name) / 1000;
            for_sys.close();
            cpu_t->setText("CPU temp is: " + std::to_string(cpu_temp));
            
            // фильтрация вывода по имени пользователя
            if(user >0){
                proc_array_next = switch_user_button_sort(proc_array_next, user_list[user], last_pin);
            }
            // если что-то ввели в поиск
            if(finder_flag){
                proc_array_next = finder_sort(proc_array_next, finder_text, finder_area_num-1, last_pin);
            }
            
            // сортировака пользовательская(если на заголовок нажали)
            if(config.getCountHead() > 0){
                if(sort_rule == 1){
                    sort_proc(proc_array_next, is_sorted, 'D', last_pin);
                    Table->setColumnText(is_sorted, config.getHead(is_sorted) + ">");
                }
                else if(sort_rule == 2){
                    sort_proc(proc_array_next, is_sorted, 'R', last_pin);
                    Table->setColumnText(is_sorted, config.getHead(is_sorted) + "<");
                }
                else{
                    Table->setColumnText(is_sorted, config.getHead(is_sorted));
                }
            }
            
            // изменение таблицы по некоторым правилам
            vert = Table->getVerticalScrollbarValue();
            horiz = Table->getHorizontalScrollbarValue();
            table_size = Table->getItemCount();
            
            ar_size = proc_array_next.size();
            
            max_len = ar_size > table_size ? ar_size : table_size;
            // если элементов стало меньше
            if(max_len == table_size){
                for(int i = max_len-1; i >= 0; i--){
                    if(i >= ar_size){
                        Table->removeItem(i);
                        continue;
                    }
                    Table->changeItem(i, proc_array_next[i].getVecToListView());
                    coloring_proc(proc_array_next[i], Table, i, pinned);
                }
            }
            // если их стало больше
            else if(max_len == ar_size){
                for(int i = 0; i < max_len; i++){
                    if(i >= table_size){
                        Table->addItem(proc_array_next[i].getVecToListView());
                        coloring_proc(proc_array_next[i], Table, i, pinned);
                        continue;
                    }
                    Table->changeItem(i, proc_array_next[i].getVecToListView()); // смена элемента на новый
                    coloring_proc(proc_array_next[i], Table, i, pinned);
                }
                Table->setHorizontalScrollbarValue(horiz);
                Table->setVerticalScrollbarValue(vert);
            }
            proc_array = proc_array_next; // хранит копию для сравнения при обновлении таблицы
        }
        window.clear();
        gui.draw();
        window.display();
        sf::sleep(sf::milliseconds(10));
    }
    return 0;
}

bool is_dir(fs::file_status s){
    if(s.type() == fs::file_type::directory) return true;
    return false;
}

bool is_pid(fs::path p){
    std::string tmp;
    tmp = p;
    int i = tmp.length()-1;
    while(tmp[i] != '/'){
        if((tmp[i] < '0') || (tmp[i] > '9')){
            return false;
        }
        i--;
    }
    return true;
}

std::string toup(std::string s){ // функция перевода в верхний регистр
	std::string temp = "";
	for(int i = 0; i < s.length(); i++){
		if( ( (int)s[i] >= 97 ) && ( (int)s[i] <= 122) ) temp +=s[i] - 32; // английские буквы ascii
		else temp += s[i]; // если буква уже большая
	}
	return temp;
}

bool sravnenie(std::string s, std::string s_1){ // сравнение двух строк, если s > s_1, то вернет 1
    std::string upperS = toup(s); // Преобразование первой строки в верхний регистр с учетом русского алфавита
    std::string upperS_1 = toup(s_1); // Преобразование второй строки в верхний регистр с учетом русского алфавита
   
    int min = s.length() < s_1.length() ? s.length() : s_1.length(); // Находим минимальную длину строк
    
    for (int i = 0; i < min; i++) {
        if (upperS[i] > upperS_1[i]) {
            return true; // Если код символа первой строки больше, возвращаем 1
        } else if (upperS[i] < upperS_1[i]) {
            return false; // Если код символа второй строки больше, возвращаем 0
        }
    }
    return s.length() > s_1.length() ? true : false; // Если первая строка длиннее, возвращаем 1, иначе 0
  
}

void sort_proc(std::vector<Process> &proc_array, int atr_indx, char c, int last_pin){
    Process tmp;
    for(int i = last_pin; i < proc_array.size(); i++){
        for(int j = i+1; j < proc_array.size(); j++){
            if(c == 'R'){
                if(config.getIsStr(atr_indx)){
                    if(!sravnenie(proc_array[i].getATR(atr_indx), proc_array[j].getATR(atr_indx))){
                        tmp = proc_array[i];
                        proc_array[i] = proc_array[j];
                        proc_array[j] = tmp;
                    }
                }
                else{
                    if(std::stoll(proc_array[i].getATR(atr_indx)) < std::stoll(proc_array[j].getATR(atr_indx))){
                        tmp = proc_array[i];
                        proc_array[i] = proc_array[j];
                        proc_array[j] = tmp;
                    }
                }
                continue;
            }
            if(config.getIsStr(atr_indx)){
                if(sravnenie(proc_array[i].getATR(atr_indx), proc_array[j].getATR(atr_indx))){
                    tmp = proc_array[i];
                    proc_array[i] = proc_array[j];
                    proc_array[j] = tmp;
                }
            }
            else{
                if(std::stoll(proc_array[i].getATR(atr_indx)) > std::stoll(proc_array[j].getATR(atr_indx))){
                    tmp = proc_array[i];
                    proc_array[i] = proc_array[j];
                    proc_array[j] = tmp;
                }
            }
        } 
    }
}

std::vector<Process> finder_sort(std::vector<Process> &proc, tgui::String promt_temp, int indx, int last_pin){
    std::vector<Process> with_finder;
    std::string promt;
    int pint = 0, sdvg = 1, j = 0;
    for(auto i : promt_temp){
        promt += i;
    }
    bool append_fl;
    int min_len;
    for(int i = 0; i < proc.size(); i++){
        pint = 0; j = 0; sdvg = 0;
        append_fl = true;
        if(i >= last_pin){
            if(proc[i].getATR(indx).length() < promt.length()){ // если промт длинее 
                continue;
            }
            // проверка на вхождение s1  в  s2 (сделать выбор между ними??)
            while(pint < promt.length()){
                if((sdvg + promt.length()) > proc[i].getATR(indx).length()){ // если сдвиг стал больше длины s1 и цикл ещё не завершился
                    append_fl = false;
                    break;
                }
                if(proc[i].getATR(indx)[j] != promt[pint]){
                    sdvg++;
                    j = sdvg;
                    pint = 0;
                    continue;
                }
                pint++;
                j++;
            }
            // просто сравнение двух строк
            //min_len = proc[i].getATR(indx).length() < promt.length() ? proc[i].getATR(indx).length() : promt.length();
            //for(int j = 0; j < min_len; j++){
            //    if(proc[i].getATR(indx).length() < promt.length()){
            //        append_fl = false; 
            //        break;
            //    }
            //    if(std::toupper(proc[i].getATR(indx)[j]) != std::toupper(promt[j])){
            //        append_fl = false;
            //        break;
            //    }
            //}
        }
        if(append_fl){
            with_finder.push_back(proc[i]);
        }
    }
    return with_finder;
}

std::vector<Process> switch_user_button_sort(std::vector<Process> &proc, tgui::String promt_temp, int last_pin){
    std::vector<Process> with_finder;
    std::string promt;
    for(auto i : promt_temp){
        promt += i;
    }
    bool append_fl;
    int min_len;
    for(int i = 0; i < proc.size(); i++){
        append_fl = true;
        if(i >= last_pin){
            min_len = proc[i].getUID().length() < promt.length() ? proc[i].getUID().length() : promt.length();
            for(int j = 0; j < min_len; j++){
                if(proc[i].getUID().length() < promt.length()){
                    append_fl = false; 
                    break;
                }
                if(std::toupper(proc[i].getUID()[j]) != std::toupper(promt[j])){
                    append_fl = false;
                    break;
                }
            }
        }
        if(append_fl){
            with_finder.push_back(proc[i]);
        }
    }
    return with_finder;
}

void coloring_proc(Process &proc_array_next, tgui::ListView::Ptr &Table, int i, std::vector<std::string> pinned){
    for(auto pin : pinned){
        if(proc_array_next.getPID() == pin){
            tgui::Texture pin_icon(Table->getItemIcon(i).getId(), tgui::UIntRect{0,0,4,4}, tgui::UIntRect{0,0,0,0}, true);
            Table->setItemIcon(i, pin_icon);
            return;
        }
    }
    if(proc_array_next.getCpuPercent() < 35){
        Table->setItemIcon(i, config.GoodIcon());    
    }
    else if(proc_array_next.getCpuPercent() < 70){
        Table->setItemIcon(i, config.MedIcon());
    }
    else{
        Table->setItemIcon(i, config.HightIcon());
    }
}
