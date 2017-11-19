#ifndef PACKET_H
#define PACKET_H

#include <string>
#include <vector>

class Packet {
public:
  Packet();

  // Ajout à la fin du packet
  // void append(const void* data, std::size_t dataSize);
  void append(char data);

  // Vide le packet
  void clear();

  // Récupère les données du packet
  const void* getData() const;

  // Récupère la taille du packet
  std::size_t getDataSize() const;

private:
  std::vector<char> m_data;
};

#endif
