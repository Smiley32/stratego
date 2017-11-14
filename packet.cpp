#include "packet.h"
#include <cstring>

Packet::Packet() {}

void Packet::append(const void* data, std::size_t dataSize) {
  if(data && (dataSize > 0)) {
    std::size_t start = m_data.size();
    m_data.resize(start + dataSize);
    std::memcpy(&m_data[start], data, dataSize);
  }
}

void Packet::clear() {
  m_data.clear();
}

const void* Packet::getData() const {
  return !m_data.empty() ? &m_data[0] : NULL;
}

std::size_t Packet::getDataSize() const {
  return m_data.size();
}
