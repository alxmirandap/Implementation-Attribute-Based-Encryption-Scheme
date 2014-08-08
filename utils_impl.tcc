template<typename T> 
void addVector(std::vector<T> &storage, const std::vector<T> &data){
  for (unsigned int i = 0; i < data.size(); i++) {
    storage.push_back(data[i]);
  }
}