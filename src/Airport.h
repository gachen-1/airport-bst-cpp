#ifndef AIRPORT_H
#define AIRPORT_H

#include <string>
#include <vector>

// One airport record from the OpenFlights file airports.dat.
class Airport
{
public:
    std::string iata;       // 3-letter IATA code, the BST key
    std::string name;
    std::string city;
    std::string country;
    std::string icao;
    double latitude;
    double longitude;

    Airport();

    // "BOS  General Edward Lawrence Logan International Airport, Boston, United States"
    std::string summary() const;

    // Every stored field, one per line.
    std::string details() const;

    // Split one CSV line into fields. Commas inside double quotes stay in the field,
    // and a doubled quote "" inside a quoted field becomes one quote character.
    static std::vector<std::string> splitCsvLine(const std::string& line);

    // Fill 'out' from one line of airports.dat.
    // Returns 1 = valid airport, 0 = no IATA code (\N or empty), -1 = not an airport line.
    static int parseLine(const std::string& line, Airport& out);
};

#endif
