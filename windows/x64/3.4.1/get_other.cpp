#include "main.h"

/***************************************************************************************************/
/* возвращает запрашивает номер кабинета и название здания                                         */
/***************************************************************************************************/


bool TInventory::get_other()
{

std::vector<std::string> menu_kab = {
        "Старшая сестра",
        "Сестра хозяйка",
        "Заведующая",
        "Ординаторская",
        "Остальное..."
    };
    std::cout << "Введите номер кабинета ... ";
    int choice = show_menu_v(menu_kab, 27, 5);

    switch (choice) {
        case 0: { 
	    id_office = "sts";
	    clearlines_down(1);
            clearLines_up( 4 );
            std::cout << "Введите номер кабинета ... " << id_office << std::endl;
            break;
        }

        case 1: { 
	    id_office = "sth"; 
	    id_office = "ordin"; 
	    clearlines_down(1);
            clearLines_up( 4 );
            std::cout << "Введите номер кабинета ... " << id_office << std::endl;
            break;
        }

        case 2: { 
            id_office = "zaved"; 
	    clearlines_down(1);
            clearLines_up( 4 );
            std::cout << "Введите номер кабинета ... " << id_office << std::endl;
            break;
        }

        case 3: { 
	    id_office = "ordin"; 
	    clearlines_down(1);
            clearLines_up( 4 );
            std::cout << "Введите номер кабинета ... " << id_office << std::endl;
            break;
        }

        case 4: {
	    clearlines_down(1);
            clearLines_up( 5 );
            std::cout << "\nВведите номер кабинета вручную: ";
            std::getline(std::cin, id_office);
            break;
        }
    }

    std::string full_name;


std::vector<std::string> menu_address = {
        "Поликлиника №1",
        "Хирургия",
        "Приемный покой",
        "Скорая помощь",
        "Детство поликлиника",
        "Детство стационар",
        "Туберкулезный диспансер",
        "Псих диспансер",
        "Остальное..."
    };

    int count = static_cast<int>(menu_address.size()) - 25;
    std::cout << "Введите адрес: ... ";
    choice = show_menu_v(menu_address, 19, 6);

    switch (choice) {
        case 0: { 
	    id_structure = "pol1";
	    clearlines_down( 7 );
            clearLines_up( count );
            std::cout << "Введите адрес: " << id_structure << std::endl;
            break;
        }

        case 1: { 
	    id_structure = "hyr"; 
	    clearlines_down(7);
            clearLines_up( count );
            std::cout << "Введите адрес: " << id_structure << std::endl;
            break;
        }

        case 2: { 
            id_structure = "ppokoy"; 
	    clearlines_down(6);
            clearLines_up( count );
            std::cout << "Введите адрес: " << id_structure << std::endl;
            break;
        }

        case 3: { 
	    id_structure = "ordin"; 
	    clearlines_down(6);
            clearLines_up( 6 );
            std::cout << "Введите адрес: " << id_structure << std::endl;
            break;
        }

        case 4: {
	    clearlines_down(6);
            clearLines_up( 6 );
            std::cout << "\nВведите адрес вручную: ";
            std::getline(std::cin, id_structure );
            break;
        }
    }

    return true;
}
