#include <iostream>
#include <SFML/Graphics.hpp>
#include <fstream>
#include <sstream>
#include <cmath>

using namespace std;

/* установка и считывание битов */
void setBit(size_t x, size_t y, unsigned char bit);
unsigned char getBit(size_t x, size_t y);
/* возврат координат в рамки */
void modCoordinates();
/* вставка и считывание данных */
void insertData(string data);
string readData();

// метки начала и конца строки
const unsigned char markStart = 0x55;
const unsigned char markEnd = 0xAA;

// изображение
sf::Image image;

// размеры изображения
size_t width;
size_t height;

// координаты
size_t currentX, currentY;

// компонента следующего бита
char color; // 0 - red  1 - green 2 - blue

int main()
{
    setlocale(0, "ru");

    string path;
    // путь к изображению
    cout << "Введите путь к изображению: ";
    cin >> path;
    // path = "C:\\Users\\HyperPaint\\Desktop\\steganography\\x64\\Debug\\image.png";
    // открытие картинки
    // если не открылась
    if (!image.loadFromFile(path))
    {
        cout << "Не удалось открыть файл" << endl;
        system("pause");
        return 0;
    }
    // запись размеров картинки
    width = image.getSize().x;
    height = image.getSize().y;
    // прочитать данные
    cout << "Изображение содержит данные:" << endl << readData() << endl;
    // путь к файлу с текстом
    cout << "Введите путь текстового файла: ";
    cin >> path;
    // открытие файла
    fstream file;
    file.open(path);
    // если не открылся
    if (!file.is_open())
    {
        file.close();
        cout << "Не удалось открыть файл" << endl;
        system("pause");
        return 0;
    }
    // получение текста из файла
    stringstream fileToString;
    // устанавливаю указатель на потоковый буфер
    fileToString << file.rdbuf();
    // считываю файл как одну строку
    string message = fileToString.str();
    file.close();
    // записать данные
    insertData(message);
    cout << "В изображение записано:" << endl << message << endl;

    system("pause");
    return 0;
}

// записать бит
void setBit(size_t x, size_t y, unsigned char bit)
{
    sf::Color pixel = image.getPixel(x, y);
    switch (color)
    {
    case 0:
        if (bit)
        {
            // установить крайний бит в 1
            pixel.r |= 0x01;
        }
        else
        {
            // установить крайний бит в 0
            pixel.r &= 0xfe;
        }
        break;

    case 1:
        if (bit)
        {
            // установить крайний бит в 1
            pixel.g |= 0x01;
        }
        else
        {
            // установить крайний бит в 0
            pixel.g &= 0xfe;
        }
        break;

    case 2:
        if (bit)
        {
            // установить крайний бит в 1
            pixel.b |= 0x01;
        }
        else
        {
            // установить крайний бит в 0
            pixel.b &= 0xfe;
        }
        break;

    default:
        throw;
    }

    image.setPixel(x, y, pixel);
    color++;
    if (color == 3)
    {
        color = 0;
        currentX++;
        modCoordinates();
    }
}

// получить бит
unsigned char getBit(size_t x, size_t y)
{
    sf::Color pixel = image.getPixel(x, y);
    unsigned char bit;
    switch (color)
    {
    case 0:
        bit = pixel.r % 2;
        break;

    case 1:
        bit = pixel.g % 2;
        break;

    case 2:
        bit = pixel.b % 2;

        break;

    default:
        throw;
    }

    color++;
    if (color == 3)
    {
        color = 0;
        currentX++;
        modCoordinates();
    }
    return bit;
}

void modCoordinates()
{
    // проверка координат
    if (currentX == width)
    {
        currentX = 0;
        currentY++;
        if (currentY == height)
        {
            throw; // координата перешла границу
        }
    }
}

// записать в изображение
void insertData(string message)
{
    // добавляю метки
    message = (char)markStart + message + (char)markEnd;
    
    // создаю массив
    int messageLength = message.length();
    int bitsLength = messageLength * 8;
    unsigned char* bits = new unsigned char[bitsLength];
    // перевод строки в массив битов
    for (int message_i = 0; message_i < messageLength; message_i++)
    {
        for (int bit_i = 0; bit_i < 8; bit_i++)
        {
            // вытаскиваю биты
            if (( (unsigned char)message[message_i] & (unsigned char)(1 << (7 - bit_i)) ) > 0)
            {
                bits[message_i * 8 + bit_i] = 1;
            }
            else
            {
                bits[message_i * 8 + bit_i] = 0;
            }
        }
    }
    
    currentX = 0, currentY = 0; // координаты
    color = 0;
    size_t writed = 0; // количество записанных бит
    // запись в изображение
    while (writed < bitsLength)
    {
        // установка цвета
        setBit(currentX, currentY, bits[writed]);
        writed++;
    }

    // сохранение изображения
    image.saveToFile("marked.png");
}

// прочитать изображение
string readData()
{
    string result = ""; // текст
    unsigned char symbol = 0; // текущая буква
    sf::Color pixel; // текущий пиксель
    currentX = 0, currentY = 0; // координаты
    color = 0;

    // считывание метки
    symbol |= getBit(currentX, currentY) << 7;
    symbol |= getBit(currentX, currentY) << 6;
    symbol |= getBit(currentX, currentY) << 5;
    symbol |= getBit(currentX, currentY) << 4;
    symbol |= getBit(currentX, currentY) << 3;
    symbol |= getBit(currentX, currentY) << 2;
    symbol |= getBit(currentX, currentY) << 1;
    symbol |= getBit(currentX, currentY);

    // проверка метки
    if (symbol != markStart)
    {
        result += "(нет меток)";
        return result;
    }
    
    // прочитать изображение
    do
    {
        // зануление байта
        symbol = 0;

        // считывание байта
        for (int bit_i = 0; bit_i < 8; bit_i++)
        {
            symbol |= getBit(currentX, currentY) << (7 - bit_i);
        }

        // сохранение байта
        result += (char)symbol;
    // пока не будет считан байт с особенной маркой
    } while (symbol != markEnd);
    result[result.length() - 1] = '\0';
    return result;
}