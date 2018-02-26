#include <iostream>
#include "ConverterApp.h"

int main(int argc, char* argv[])
{
    try
    {
        ConverterApp app;

        app.init(argc, argv);
        return app.run();
    }
    catch(std::exception& ex)
    {
        // Тут мы не можем писать ошибку в базу данных,
        // т.к. не подключены к ней.
        // Так что выдаем ошибку в консоль.
        std::cerr << "error: " << ex.what() << std::endl;
        return 1;
    }
}
