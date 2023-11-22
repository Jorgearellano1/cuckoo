#include <iostream>
#include <string>
#include <climits>
#include "httplib.h"


using namespace std;
using namespace httplib;

class CuckooHashing
{
private:
    static const int ver = 2;
    int **hashtable;
    int *pos;
    int size;
    int total_attempts;

public:
    CuckooHashing(int initial_size = 16)
    {
        size = initial_size;
        hashtable = new int *[ver];
        pos = new int[ver];
        for (int i = 0; i < ver; ++i)
            hashtable[i] = new int[size];
        initTable();
    }

    ~CuckooHashing()
    {
        for (int i = 0; i < ver; ++i)
            delete[] hashtable[i];
        delete[] hashtable;
        delete[] pos;
    }

    void initTable()
    {
        for (int j = 0; j < size; j++)
            for (int i = 0; i < ver; i++)
                hashtable[i][j] = INT_MIN;
    }

    int hash(int function, int key)
    {
        switch (function)
        {
            case 1:
                return key % size;
            case 2:
                return (key / size) % size;
        }
        return 0;
    }

    void place(int key, int tableID, int cnt, int n, int &total_attempts)
    {

        if (cnt == n)
        {
            cout << key << " sin posicion" << endl;
            cout << "REHASH" << endl;
            rehash();
            return;
        }

        for (int i = 0; i < ver; i++)
        {
            pos[i] = hash(i + 1, key);
            if (hashtable[i][pos[i]] == key)
                return;
        }

        if (hashtable[tableID][pos[tableID]] != INT_MIN)
        {
            int displaced_element = hashtable[tableID][pos[tableID]];
            cout << "Collision ! : Moviendo " << displaced_element << " hacia la tabla " << (tableID + 1) % ver << " en la pos " << pos[tableID] << endl;

            hashtable[tableID][pos[tableID]] = key;
            place(displaced_element, (tableID + 1) % ver, cnt + 1, n, total_attempts);
        }
        else
        {
            hashtable[tableID][pos[tableID]] = key;
        }
    }

    bool find(int key)
    {
        for (int i = 0; i < ver; i++)
        {
            int position = hash(i + 1, key);
            if (hashtable[i][position] == key)
            {
                cout << "Elemento " << key << " encontrado en la tabla " << i << ", pos  " << position << endl;
                return true;
            }
        }

        cout << "Elemento " << key << " no encontrado " << endl;
        return false;
    }

    void rehash()
    {
        int **old_table = hashtable;
        int old_size = size;

        size *= 2;
        hashtable = new int *[ver];
        for (int i = 0; i < ver; ++i)
            hashtable[i] = new int[size];
        initTable();

        for (int i = 0; i < ver; ++i)
            for (int j = 0; j < old_size; ++j)
                if (old_table[i][j] != INT_MIN)
                    place(old_table[i][j], 0, 0, size, total_attempts);

        // Inserta el último elemento no posicionado en el nuevo hash
        if (total_attempts > 3 * old_size)
        {
            cout << "Insertando último elemento no posicionado: " << total_attempts << endl;
            place(old_table[ver - 1][old_size - 1], 0, 0, size, total_attempts);
        }

        for (int i = 0; i < ver; ++i)
            delete[] old_table[i];
        delete[] old_table;
    }

    vector<string> printTable()
    {
        cout << "Tablas hash finales:" << endl;
        vector<string> tablas = {"", ""};

        for (int i = 0; i < ver; i++)
        {
            for (int j = 0; j < size; j++)
            {
                if (hashtable[i][j] == INT_MIN)
                {
                    //   cout << "- ";
                    tablas[i] += "- ";
                }
                else
                {
                    //     cout << hashtable[i][j] << " ";
                    tablas[i] += to_string(hashtable[i][j]);
                    tablas[i] += " ";
                }
            }
        }


        cout << tablas[0] << endl;
        cout << tablas[1] << endl;

        return tablas;

    }

    vector<string> insertAndPrint(int key)
    {
        total_attempts = 0;
        vector<string> result = {"","",""};
        cout << "Posibles posiciones para el hash:" << endl;
        for (int i = 0; i < ver; i++)
        {
            pos[i] = hash(i + 1, key);
            cout << "Tabla " << i << ": " << pos[i] << endl;
        }

        place(key, 0, 0, size, total_attempts);
        auto strings = printTable();

        result[0] = strings[0];
        result[1] = strings[1];

        result[2] = " test!! ";

        return result;
    }

    bool remove(int key)
    {
        for (int i = 0; i < ver; i++)
        {
            int position = hash(i + 1, key);
            if (hashtable[i][position] == key)
            {
                hashtable[i][position] = INT_MIN;
                cout << "Elemento " << key << " eliminado de la Tabla " << i << ", Posición " << position << endl;
                return true;
            }
        }

        cout << "Elemento " << key << " no encontrado en la tabla hash. No se realizó ninguna eliminación." << endl;
        return false;
    }

};

int main() {
    using namespace httplib;

    Server svr;
    CuckooHashing cuckooHash;

    svr.Get("/insert", [&](const httplib::Request &req, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        int key = std::stoi(req.get_param_value("key"));
        auto result = cuckooHash.insertAndPrint(key);
        res.set_content(result[0] + "\n" + result[1], "text/plain");
    });

    svr.Get("/search", [&](const httplib::Request &req, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");

        int key = std::stoi(req.get_param_value("key"));
        bool found = cuckooHash.find(key);
        std::string response = found ? "Elemento encontrado" : "Elemento no encontrado";
        res.set_content(response, "text/plain");
    });

    svr.Get("/delete", [&](const httplib::Request &req, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");

        int key = std::stoi(req.get_param_value("key"));
        bool removed = cuckooHash.remove(key);
        std::string response = removed ? "Elemento eliminado" : "Elemento no encontrado para eliminar";
        res.set_content(response, "text/plain");
    });

    svr.Get("/print", [&](const httplib::Request &req, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");

        auto tablas = cuckooHash.printTable();
        res.set_content(tablas[0] + "\n" + tablas[1], "text/plain");
    });

    svr.Options(R"(.*)", [](const Request&, Response& res) {
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.status = 204; // No Content
    });
    svr.listen("localhost", 8080);

    return 0;
}