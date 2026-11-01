#include "Airport.h"

#include <sstream>

Airport::Airport() : latitude(0.0), longitude(0.0) {}

std::string Airport::summary() const
{
    return iata + "  " + name + ", " + city + ", " + country;
}

std::string Airport::details() const
{
    std::ostringstream out;
    out << "  IATA:      " << iata << "\n"
        << "  Name:      " << name << "\n"
        << "  City:      " << city << "\n"
        << "  Country:   " << country << "\n"
        << "  ICAO:      " << icao << "\n"
        << "  Latitude:  " << latitude << "\n"
        << "  Longitude: " << longitude;
    return out.str();
}

std::vector<std::string> Airport::splitCsvLine(const std::string& line)
{
    std::vector<std::string> fields;
    std::string field;
    bool inQuotes = false;
    int n = (int)line.size();
    for (int i = 0; i < n; i++)
    {
        char c = line[i];
        if (inQuotes)
        {
            if (c == '"')
            {
                if (i + 1 < n && line[i + 1] == '"')   // "" inside quotes is one quote
                {
                    field += '"';
                    i++;
                }
                else
                {
                    inQuotes = false;                  // closing quote
                }
            }
            else
            {
                field += c;                            // commas inside quotes are kept
            }
        }
        else if (c == '"')
        {
            inQuotes = true;
        }
        else if (c == ',')
        {
            fields.push_back(field);                   // end of one field
            field.clear();
        }
        else if (c != '\r' && c != '\n')
        {
            field += c;
        }
    }
    fields.push_back(field);
    return fields;
}

static double toDouble(const std::string& text)
{
    try
    {
        return std::stod(text);
    }
    catch (...)
    {
        return 0.0;
    }
}

int Airport::parseLine(const std::string& line, Airport& out)
{
    std::vector<std::string> f = splitCsvLine(line);
    if (f.size() < 8) return -1;
    if (f[0] == "airport_id") return -1;           // header row of the CSV version
    if (f[4] == "\\N" || f[4].empty()) return 0;   // no IATA code: skip this line

    out.name = f[1];
    out.city = f[2];
    out.country = f[3];
    out.iata = f[4];
    out.icao = (f[5] == "\\N") ? "" : f[5];
    out.latitude = toDouble(f[6]);
    out.longitude = toDouble(f[7]);
    return 1;
}
