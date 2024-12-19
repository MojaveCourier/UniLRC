#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>

struct ParitySlice
{
    int offset;
    int size;
    char *slice_ptr;

    ParitySlice() : offset(0), size(0), slice_ptr(nullptr) {}
    // the slice_ptr must be allocated on heap, otherwise will cause failure free
    explicit ParitySlice(int offset, int size, char *data)
        : offset(offset), size(size), slice_ptr(data) {}

    ~ParitySlice()
    {
        delete[] slice_ptr;
    }

    // 禁止复制构造函数和赋值运算符
    ParitySlice(const ParitySlice &) = delete;
    ParitySlice &operator=(const ParitySlice &) = delete;

    // 允许移动构造函数和赋值运算符
    ParitySlice(ParitySlice &&other) noexcept
        : offset(other.offset), size(other.size), slice_ptr(other.slice_ptr)
    {
        other.slice_ptr = nullptr;
    }

    ParitySlice &operator=(ParitySlice &&other) noexcept
    {
        if (this != &other)
        {
            delete[] slice_ptr;
            offset = other.offset;
            size = other.size;
            slice_ptr = other.slice_ptr;
            other.slice_ptr = nullptr;
        }
        return *this;
    }
};

struct Person
{
    char *name;
    int age;
    double height;

    Person() : name(nullptr), age(0), height(0.0) {}
    explicit Person(const std::string &name, int age, double height)
        : age(age), height(height)
    {
        this->name = new char[name.size() + 1];
        std::strcpy(this->name, name.c_str());
    }

    ~Person()
    {
        delete[] name;
    }

    // 禁止复制构造函数和赋值运算符，以防止内存泄漏
    Person(const Person &) = delete;
    Person &operator=(const Person &) = delete;

    // 允许移动构造函数和赋值运算符
    Person(Person &&other) noexcept
        : name(other.name), age(other.age), height(other.height)
    {
        other.name = nullptr;
    }

    Person &operator=(Person &&other) noexcept
    {
        if (this != &other)
        {
            delete[] name;
            name = other.name;
            age = other.age;
            height = other.height;
            other.name = nullptr;
        }
        return *this;
    }
};

void serialize(const std::string &filename, const ParitySlice &slice)
{
    std::ofstream outFile(filename, std::ios::out | std::ios::binary | std::ios::app);
    if (outFile.is_open())
    {
        // Serialize a single struct
        outFile.write(reinterpret_cast<const char *>(&slice.offset), sizeof(slice.offset));
        outFile.write(reinterpret_cast<const char *>(&slice.size), sizeof(slice.size));
        outFile.write(slice.slice_ptr, slice.size);
        outFile.flush();
        outFile.close();
    }
    else
    {
        std::cerr << "Unable to open file for writing." << std::endl;
    }
}

// void serialize(const std::string &filename, const Person &person)
// {
//     std::ofstream outFile(filename, std::ios::out | std::ios::binary | std::ios::app);
//     if (outFile.is_open())
//     {
//         // Serialize a single struct
//         outFile.write(reinterpret_cast<const char *>(&person.age), sizeof(person.age));
//         outFile.write(reinterpret_cast<const char *>(&person.height), sizeof(person.height));

//         // Write string length and content
//         size_t nameLength = std::strlen(person.name);
//         outFile.write(reinterpret_cast<const char *>(&nameLength), sizeof(nameLength));
//         outFile.write(person.name, nameLength);

//         outFile.close();
//     }
//     else
//     {
//         std::cerr << "Unable to open file for writing." << std::endl;
//     }
// }

std::vector<ParitySlice> deserialize(const std::string &filename)
{
    std::vector<ParitySlice> slices;
    std::ifstream inFile(filename, std::ios::in | std::ios::binary);
    if (inFile.is_open())
    {
        // Read until end of file
        while (inFile.peek() != EOF)
        {
            ParitySlice slice;

            // Read basic data types
            inFile.read(reinterpret_cast<char *>(&slice.offset), sizeof(slice.offset));
            inFile.read(reinterpret_cast<char *>(&slice.size), sizeof(slice.size));

            // for output, append a \0 at the end
            // slice.slice_ptr = new char[slice.size + 1];
            // inFile.read(slice.slice_ptr, slice.size);
            // slice.slice_ptr[slice.size] = '\0';

            // for no output
            slice.slice_ptr = new char[slice.size];
            inFile.read(slice.slice_ptr, slice.size);

            slices.push_back(std::move(slice));
        }
        inFile.close();
    }
    else
    {
        std::cerr << "Unable to open file for reading." << std::endl;
    }
    return slices;
}

// std::vector<Person> deserialize(const std::string &filename)
// {
//     std::vector<Person> people;
//     std::ifstream inFile(filename, std::ios::in | std::ios::binary);
//     if (inFile.is_open())
//     {
//         // Read until end of file
//         while (inFile.peek() != EOF)
//         {
//             Person person;

//             // Read basic data types
//             inFile.read(reinterpret_cast<char *>(&person.age), sizeof(person.age));
//             inFile.read(reinterpret_cast<char *>(&person.height), sizeof(person.height));

//             // Read string length and content
//             size_t nameLength;
//             inFile.read(reinterpret_cast<char *>(&nameLength), sizeof(nameLength));

//             person.name = new char[nameLength + 1];
//             inFile.read(person.name, nameLength);
//             person.name[nameLength] = '\0';

//             people.push_back(std::move(person));
//         }
//         inFile.close();
//     }
//     else
//     {
//         std::cerr << "Unable to open file for reading." << std::endl;
//     }
//     return people;
// }

int main()
{
    char *data1 = new char[5];
    for (int i = 0; i < 5; i++)
    {
        data1[i] = 'a';
    }
    char *data2 = new char[3];
    for (int i = 0; i < 3; i++)
    {
        data2[i] = 'b';
    }
    char *data3 = new char[7];
    for (int i = 0; i < 7; i++)
    {
        data3[i] = 'c';
    }

    ParitySlice slice1(1, 5, data1);
    ParitySlice slice2(2, 3, data2);
    ParitySlice slice3(3, 7, data3);

    serialize("slices.bin", slice1);
    serialize("slices.bin", slice2);
    serialize("slices.bin", slice3);

    std::vector<ParitySlice> slices = deserialize("slices.bin");
    for (const auto &slice : slices)
    {
        std::cout << "Offset: " << slice.offset << ", Size: " << slice.size << ", Data: " << slice.slice_ptr << std::endl;
    }

    // // Create test data
    // Person p1("Alice", 25, 175.5);
    // Person p2("Bob", 30, 180.0);
    // Person p3("Charlie", 35, 168.5);

    // // Test serializing multiple structs to the same file
    // serialize("persons.bin", p1);
    // serialize("persons.bin", p2);
    // serialize("persons.bin", p3);

    // // Test deserialization
    // std::vector<Person> deserializedPeople = deserialize("persons.bin");

    // // Print results
    // for (const auto &person : deserializedPeople)
    // {
    //     std::cout << "Name: " << person.name << std::endl;
    //     std::cout << "Age: " << person.age << std::endl;
    //     std::cout << "Height: " << person.height << std::endl;
    //     std::cout << "------------------------" << std::endl;
    // }

    return 0;
}

// void serialize(const std::string &filename, const std::vector<Person> &people)
// {
//     std::ofstream outFile(filename, std::ios::out | std::ios::binary);
//     if (outFile.is_open())
//     {
//         // 写入结构体数量
//         size_t size = people.size();
//         outFile.write(reinterpret_cast<const char *>(&size), sizeof(size));

//         // 序列化每个结构体
//         for (const auto &person : people)
//         {
//             // 写入基本数据类型
//             outFile.write(reinterpret_cast<const char *>(&person.age), sizeof(person.age));
//             outFile.write(reinterpret_cast<const char *>(&person.height), sizeof(person.height));

//             // 写入字符串长度和内容
//             size_t nameLength = std::strlen(person.name);
//             outFile.write(reinterpret_cast<const char *>(&nameLength), sizeof(nameLength));
//             outFile.write(person.name, nameLength);
//         }
//         outFile.close();
//     }
//     else
//     {
//         std::cerr << "Unable to open file for writing." << std::endl;
//     }
// }

// std::vector<Person> deserialize(const std::string &filename)
// {
//     std::vector<Person> people;
//     std::ifstream inFile(filename, std::ios::in | std::ios::binary);
//     if (inFile.is_open())
//     {
//         // 读取结构体数量
//         size_t size;
//         inFile.read(reinterpret_cast<char *>(&size), sizeof(size));

//         // 反序列化每个结构体
//         for (size_t i = 0; i < size; ++i)
//         {
//             Person person;
//             // 读取基本数据类型
//             inFile.read(reinterpret_cast<char *>(&person.age), sizeof(person.age));
//             inFile.read(reinterpret_cast<char *>(&person.height), sizeof(person.height));

//             // 读取字符串长度和内容
//             size_t nameLength;
//             inFile.read(reinterpret_cast<char *>(&nameLength), sizeof(nameLength));
//             person.name = new char[nameLength + 1];
//             inFile.read(person.name, nameLength);
//             person.name[nameLength] = '\0'; // 确保字符串以null字符结尾
//             people.push_back(std::move(person));
//         }
//         inFile.close();
//     }
//     else
//     {
//         std::cerr << "Unable to open file for reading." << std::endl;
//     }
//     return people;
// }

// int main()
// {
//     // 创建一些Person对象
//     std::vector<Person> people;
//     people.emplace_back("John Doe", 30, 1.75);
//     people.emplace_back("Jane Smith", 25, 1.65);
//     people.emplace_back("LQL", 28, 2.65);
//     people.emplace_back("XLL", 38, 2.25);

//     // 序列化并保存到文件
//     serialize("people.dat", people);

//     // 从文件反序列化并读取
//     std::vector<Person> loadedPeople = deserialize("people.dat");

//     // 打印加载的数据以验证
//     for (const auto &person : loadedPeople)
//     {
//         std::cout << "Name: " << person.name << ", Age: " << person.age << ", Height: " << person.height << std::endl;
//     }

//     return 0;
// }