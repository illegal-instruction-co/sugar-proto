#include "user.pb.h"

#include "user.sugar.h"

#include <iostream>

using namespace std;

void dump(int32_t id, string_view city) {
  cout << "DUMP:" << endl;

  cout << "id: " << id << endl;
  cout << "city: " << city << endl;
}

int main() {
  User userRaw;
  UserWrapped sugarUser(userRaw);

  sugarUser.id = 123;
  sugarUser.tags.push_back("cpp");
  sugarUser.profile.city = "ISTANBUL";

  // Read from raw
  cout << userRaw.DebugString() << endl;

  // Read from sugar wrapper
  cout << "id: "<< sugarUser.id << endl;
  cout << "tags: " << endl;

  for (const auto&tag : sugarUser.tags) 
    cout << "  tag: " << tag << endl;
  
//  cout << "profile.city: "<< sugarUser.profile.city << endl;
  
  dump(sugarUser.id, sugarUser.profile.city);

  return 0;
}
