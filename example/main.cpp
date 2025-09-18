#include "user.pb.h"

#include "user.sugar.h"

#include <iostream>

using namespace std;

int main() {
  User userRaw;
  UserWrapped sugar(userRaw);

  sugar.id = 123;
  sugar.tags.push_back("cpp");
  sugar.profile.city = "ISTANBUL";

  // Read from raw
  cout << userRaw.DebugString() << endl;

  // Read from sugar wrapper
  cout << "id: "<< sugar.id << endl;
  cout << "tags: " << endl;

  for (const auto&tag : sugar.tags) 
    cout << "  tag: " << tag << endl;
  
  cout << "profile.city: "<< sugar.profile.city << endl;
  
  return 0;
}
