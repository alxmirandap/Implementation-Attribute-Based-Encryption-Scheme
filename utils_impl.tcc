template<typename T> 
int contains(const vector<T> &set, const T element){
  for (unsigned int i = 0; i < set.size(); i++){
     if (element == set[i]) {
       return i;
     }
  }
  return -1;
}

template<typename T> 
void addVector(std::vector<T> &storage, const std::vector<T> &data){
  for (unsigned int i = 0; i < data.size(); i++) {
    storage.push_back(data[i]);
  }
}

template<typename T>
void debugVector(std::string text, std::vector<T> list) {
     for (unsigned int i = 0; i < list.size(); i++) {
     	 DEBUG(text << "[" << i << "]: " << list[i]);
     }
}

template<typename T>
void debugVectorObj(std::string text, std::vector<T> list) {
     if (list.size() == 0) {
       DEBUG(text << ": no elements");
     }     
    for (unsigned int i = 0; i < list.size(); i++) {
     	 DEBUG(text << "[" << i << "]: " << list[i].to_string());
     }
}

template<typename T>
void outVector(std::vector<T> list, std::string text) {
     for (unsigned int i = 0; i < list.size(); i++) {
     	 OUT(text << "[" << i << "]: " << list[i]);
     }
}
